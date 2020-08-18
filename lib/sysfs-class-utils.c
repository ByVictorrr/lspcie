#include "sysfs-class.h"
#include "internal.h"
#include "pread.h"
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "pci.h"
inline u8 get_class(struct pci_dev * d){
     return (d->device_class & PCI_CLASS_MASK) >> 8;
}
inline u8 get_subclass(struct pci_dev *d){
     return (d->device_class & PCI_SUBCLASS_MASK);
}

char *strremove(char *str, const char *sub) {
    char *p, *q, *r;
    if ((q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            memmove(q, p, r - p);
            q += r - p;
        }
        memmove(q, p, strlen(p) + 1);
        return str;
    }
    return NULL;
}
void trimstr(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

/**
* Desc: function looks for data part of the string (i.e the version data) \
        excluding the word parts of the string
* @param string - the string that we are looking through
* @return the starting address of the data part in the string \
            NULL if an error occured or no match
*/
static const char *
get_version_info(const char *string)
{ 
    const char* info_pattn = "[0-9].{1,}$"; 
    regex_t re; 
    regmatch_t pmatch;
    const char *cursor;
    if (regcomp(&re, info_pattn, REG_EXTENDED) != 0){
        fprintf(stderr, "get_version_info: regcomp error\n");
        return NULL; 
    }
    int status = regexec(&re, string, 1, &pmatch, 0); 
    if (status != 0){
        fprintf(stderr, "get_version_info: no match found\n");
        return NULL; 
    }
    cursor=string;
    regfree(&re);

   return &cursor[pmatch.rm_so];
} 

/*============ Path Utility functions ==============================*/
/**
* Desc: Given a path (absolute or relative) it returns a string of \
       the first directory in the path
* @param path - the string used to get the first directory in the path
* @param b_dirname - the buffer used to store the result of the return value
* @param size - size of the buffer (b_dirname)
* @return b_dirname if the operations were sucessful, NULL otherwise
*/
static char * 
base_dirname(const char *path, char *b_dirname, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    // Step 0 - check if either parm is NULL
    if(!path || !b_dirname){
        fprintf(stderr, "get_base_dir: both parameters need to be non NULL");
        return NULL;
    // Step 1.5 - check to see if path is ""
    }else if(strlen(path) == 0){
        return NULL;
    }
    // Step 1 - cpy data from path to buff (so we can use strtok)
    if(strlen(path)+1 > MAX_PATH){
        fprintf(stderr, "get_base_dir: *path arg is to large");
        return NULL;
    }
    strcpy(buff, path);
    memset(b_dirname, '\0', size);
    if((token = strtok(buff, "/"))!=NULL){
        strcpy(b_dirname, token);
    }else{
        strcpy(b_dirname, path);
    }
   return b_dirname; 
}
/**
* Desc: Given a path (absolute or relative) it returns a string of \
*       everything excluding the first folder
* @param path - the string used to get the first directory in the path
* @param n_relpath - the buffer used to store the result of the return value
* @param size - size of the buffer (n_relpath)
* @return n_relpath if there is a next path in path, NULL otherwise
*/

static char 
*next_relpath(const char *path, char *n_relpath, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    int exists=0;
    // Step 0 - check if either parm is NULL
    if(!path || !n_relpath){
        fprintf(stderr, "next_relpath: both parameters need to be non NULL");
        return NULL;
    }
    // Step 1 - cpy data from path to buff (so we can use strtok)
    if(strlen(path)+1 > MAX_PATH){
        fprintf(stderr, "get_base_next: *path arg is to large");
        return 0;
    }
    strcpy(buff, path);
    memset(n_relpath, '\0', size);
    // Case 1 - No next path
    if((token = strtok(buff, "/"))==NULL){
        return NULL;
    }
    // Case 2 - next path (exists)
    for(token=strtok(NULL, "/"); token; strcat(n_relpath, "/")){
        strcat(n_relpath, token);
        token=strtok(NULL, "/");
        exists=1;
    }
    // To see if there exists another folder
    if(!exists){
        return NULL;
    }
    n_relpath[strlen(n_relpath)-1] = '\0';
    // need to get rid of the last "/"

    return n_relpath;
}


/*===================get_vdir functions =============================================*/
/**
* @param dev - pci_dev structused to find up its device folder
* @param dev_dirname - where to store the desired device folder name
* @param devdir_size - the size of the dev_dirname
* @return the base device folder name or NULL if buffer too small
*/
static char *
get_pci_dev_dirname(struct pci_dev *dev, char *dev_dirname, int devdir_size){
    int n = snprintf(dev_dirname, devdir_size, "%s/devices/%04x:%02x:%02x.%d",
		   sysfs_name(dev->access), dev->domain, dev->bus, dev->dev, dev->func);
    // Step 1 - check to see if buff is of correct size 
    if (n < 0 || n >= dev_dirname){
        dev->access->warning("get_pci_dev_dirname: Folder name too long to store in desired buffer");
        return NULL;
    }
    return dev_dirname;
}

/**
*@param cwd - absolute path of the folder in which we want to look for a folder
*@param rel_vpath_pattn - the pattn to the version directory
*@param cwd_size - size of the cwd buffer
*@return the pci device version directory if sucessful \
        NULL if it wasnt able to find the version directory (or buffer overflow)
**/
static char *
_find_pci_dev_vers_dir(const char * cwd, const char *rel_vpath_pattn){
    DIR *dir;
    struct dirent *dp;
    regex_t regex;
    #define BASE_DIRNAME_SIZE 100
    #define NEXT_RELPATH_SIZE 100
    char b_dirname[BASE_DIRNAME_SIZE];
    char n_relvpath_pattn[NEXT_RELPATH_SIZE];
    char n_cwd[MAX_PATH];
    char *vdir, *ptr_bdirname, *ptr_nrelvpath;
    
    int n;
    // Step 0 - clear local buffers
    memset(b_dirname, 0, BASE_DIRNAME_SIZE);
    memset(n_relvpath_pattn, 0, NEXT_RELPATH_SIZE);

    /* Step 1 - get the base directory name
    * Example: if path_pattn = /sys/bus/pci/devices
    *          b_dirname = sys 
    * Returns: NULL if 
    */
    /* Step 2 - get the next relpath after the base_dirname
    * Example: if path_pattn = /sys/bus/pci/devices
    *          n_relpath = bus/pci/devices
    * Returns: NULL if there is no next path (example path_pattn = devices)
    */
 
    ptr_bdirname = base_dirname(rel_vpath_pattn, b_dirname, BASE_DIRNAME_SIZE);
    ptr_nrelvpath = next_relpath(rel_vpath_pattn, n_relvpath_pattn, NEXT_RELPATH_SIZE);
    if(!ptr_bdirname && !ptr_nrelvpath){
        if(!(vdir=strdup(cwd))){
            fprintf(stderr, "find_pci_dev_vers_dir: strdup access->warning\n"); 
            return NULL;
        }
        return vdir;
    }
    /* Step 3 - open the cwd direcotry */
    if (!(dir=opendir(cwd)))
        return NULL;
   
    /* Step 4 - compose the regex (look for the base dirname found above*/
    if (regcomp(&regex, b_dirname, 0)){
       fprintf(stderr, "find_pci_dev_vers_dir: Could not compile regex\n"); 
       return NULL;
    }
    /* Step 5 - go through dir and try to find b_dirname */
    while((dp=readdir(dir)) != NULL){
        // Case 1 - if entry is directory and not .. or .
        if ((dp->d_type == DT_DIR || dp->d_type == DT_LNK) && 
        (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))){
            // Case 1.1 -  See if there is a match
            if(!regexec(&regex, dp->d_name, 0, NULL, 0)){
                n = snprintf(n_cwd, MAX_PATH, "%s/%s", cwd, dp->d_name);
                if (n < 0 || n >= MAX_PATH){
                    fprintf(stderr, "find_pci_dev_vers_dir: Folder name too long\n");
                    closedir(dir);
                    regfree(&regex);
                    return NULL;
                }
                closedir(dir);
                regfree(&regex);
                return _find_pci_dev_vers_dir(n_cwd, n_relvpath_pattn);
            }
        }
    }
    fprintf(stderr, "find_pci_dev_vers_dir: Could not find a file pattern of %s in %s\n", rel_vpath_pattn, cwd); 
    regfree(&regex);
    return NULL;

}
static int 
get_next_vdirpath(int start, const char *vdir_paths){
    int i=0;
    for(i = start; vdir_paths[i] != '|' && vdir_paths[i] != ')'; i++)
        ;
    return i-1;
}
char *
find_pci_dev_vers_dir(const char * cwd, const char *rel_vpath_pattns){
    char rel_vpath_pattn[MAX_PATH];
    char *vdir;
    int start, end; 
    memset(rel_vpath_pattn, 0, MAX_PATH);
    // Step 1 - parse the rel_vpath_pattn[rvp] (rvp1|rvp2|rvp3|..|rvpn)
    if(rel_vpath_pattns[0] != '(' || rel_vpath_pattns[strlen(rel_vpath_pattns)-1] != ')'){
        fprintf(stderr, "find_pci_dev_vers_dir: rel_vpath_pattns not of the right form\n");
        return NULL;
    }
    // step 2 - parse pattns
    start=end=1;
    while(end != strlen(rel_vpath_pattns)-2){
        end = get_next_vdirpath(start, rel_vpath_pattns);
        memset(rel_vpath_pattn, 0, MAX_PATH);
        memcpy(rel_vpath_pattn, &rel_vpath_pattns[start], end-start+1);
        if((vdir=_find_pci_dev_vers_dir(cwd, rel_vpath_pattn))){
            return vdir;
        }
        start=end+2;
    }

    fprintf(stderr, "find_pci_dev_vers_dir: couldnt find a version dir associated with this dev\n");
    return NULL;
}

/**
* Description: This function sets the pci_dev field version_folder
* @param 
*
*/
int 
set_pci_dev_drvdir(struct pci_dev *dev){
    /* dev directory, relative path from dev directory to v dir pattern */
    char ddir[MAX_PATH];
    const char *relvdir_pattn;
    char *vdir_name;
    // Step 0 - clear out the buffers used in this function
    memset(ddir, '\0', MAX_PATH);
    // Step 1 - check if the pci_dev drvdir is set already
    if(dev->drvdir_path){
        return 1;
    }
    // Step 3 - get the pci device base folder
    else if(!get_pci_dev_dirname(dev, ddir, MAX_PATH)){
        return 0;
    } // Step 4 - go through each type and get the vers folder(abs path)
    else if(!(vdir_name=find_pci_dev_vers_dir(ddir, PCI_VDIR_DR_RELPATH_PATTNS))){
        return 0;
    }
    dev->drvdir_path=vdir_name;
    return 1;
}
int 
set_pci_dev_fwvdir(struct pci_dev *dev){
    /* dev directory, relative path from dev directory to v dir pattern */
    char ddir[MAX_PATH];
    const char *relvdir_pattn;
    char *vdir_name;
    // Step 0 - clear out the buffers used in this function
    memset(ddir, '\0', MAX_PATH);
    // Step 1 - check if the pci_dev drvdir is set already
    if(dev->fwvdir_path){
        return 1;
    }
    // Step 3 - get the pci device base folder
    else if(!get_pci_dev_dirname(dev, ddir, MAX_PATH)){
        return 0;
    } // Step 4 - go through each type and get the vers folder(abs path)
    else if(!(vdir_name=find_pci_dev_vers_dir(ddir, PCI_VDIR_FW_RELPATH_PATTNS))){
        return 0;
    }
    dev->fwvdir_path=vdir_name;
    return 1;
}

/*===================read_vfile functions =============================================*/


/**
* Desc: Reads the version directory object and looks for \
*       a matches with the regex fpattn
* @param v_dir : the version directory that is used to look \
                 through for vfiles matching the fpattn
* @param fpattn : the regex to get the version files 
* @param files_buff : Stores a list of files that match the \
                     fpattn in the v_dir
* @param buff_size : The max size of the buffer
* @return an int that denotes how many files were stored in files_buff \n 
         -1 represents something whent wrong 
          0 represents buffer is to small
*/
static int
get_vfiles(const char *vdir, const char *fpattn, FILE **files_buff, int buff_size){
    struct dirent *entry;
    regex_t regex;
    FILE *file;
    DIR *dir;
    int i=0, n;
    char vfile_path[MAX_PATH];
    memset(vfile_path, 0, MAX_PATH);

    // Step 1 - check params
    if(!vdir){
        fprintf(stderr, "get_vfiles: the vdir string is null");
        return 0;
    }else if (!fpattn){
        fprintf(stderr, "get_vfiles: the fpattn object is null");
        return 0;
    }else if (!files_buff){
        fprintf(stderr, "get_vfiles: files_buff is null");
        return 0;
    }else if(regcomp(&regex, fpattn, REG_EXTENDED)){ /* Compile the regex */
        fprintf(stderr, "get_vfiles: regex compilation error");
        return 0;
    }else if(!(dir=opendir(vdir))){
        fprintf(stderr, "get_vfiles: opendir error");
        return 0; 
    }
    
    // Step 2 - read v_dir and look for matches of the regex
    while((entry = readdir(dir))){
        /* if the regex matches the file name open the file*/
         if(entry->d_type == DT_REG && !regexec(&regex, entry->d_name, 0, NULL, 0)){
            n=snprintf(vfile_path, MAX_PATH, "%s/%s", vdir, entry->d_name);
            if (n < 0 || n >= MAX_PATH){
                fprintf(stderr, "get_vfiles: Folder name too long\n");
                regfree(&regex);
                return 0;
            }else if((file=fopen(vfile_path, "r"))){
                if(i > buff_size-1){
                    fprintf(stderr, "get_vfiles: Make your files_buff max size bigger");
                    regfree(&regex);
                    return i+1;
                }
                files_buff[i++] = file; 
            }else
                fprintf(stderr, "get_vfiles: Unable to open for reading %s", entry->d_name);
         }
    }
    regfree(&regex);
    return i;
}

/**
* Desc: looks for string in vfile (also it rewinds the vfile to start of file)
* @param string : a string that is used to find in a file
* @param vfile : a version file 
* @return The line number the string is on or -1 if it doesnt exist
**/
static int
line_string_in_vfile(char *string, FILE *vfile){
    #define MAX_LINE 100
    char *line;
    size_t len = 0;
    int line_num = 0;  
    ssize_t read;
    // Step 1 - start of a file
    while((read = getline(&line, &len, vfile)) != -1){
        if(strstr(line, string))
            return line_num;
        line_num++;
    }
    return -1;

}
/**
* Desc: This function reads the version file and stores the info into vbuff
* @param vfile: The file ponter used to read the version file
* @param string: The string to look for inside a file to get the data on that line
* @param vbuff: Where we store the data
* @param buff_size: The max size the buff can hold
* @return an int 1 if we correctly extracted the data from the file into the buffer \
             int 0 if something went wrong like the buffer isnt big enough to hold the information
**/
// TODO : this function only reads the first line of the file
static int
read_vfile(FILE *vfile, char * string, char *vbuff, int buff_size){
    int str_linenum=0, linenum=0;  
    char *vinfo;
    // if no string firmware read the full file and store it into buffer
    // Step 1 - Search for in file the string = {"Firmware" or "Driver"}
    str_linenum = line_string_in_vfile(string, vfile);
    // Step 1 - start of a file
    rewind(vfile);
    while(fgets(vbuff, buff_size-1, vfile)){
        // Step 2 - trim white spaces 
        trimstr(vbuff);
        // Step 3 - check if new line is on the read data
        if(vbuff[strlen(vbuff)-1]=='\n')
            vbuff[strlen(vbuff)-1]=0;
        
        // Step 4  - no string found in the file
        if(str_linenum == -1){
           // Step 1 - look for only the info part
           if(!(vinfo = get_version_info(vbuff))){
               return 1;
           }
           strcpy(vbuff, vinfo);
           return 1; 
        }
        // Step 4 - if the string is a substring of the line
        else if(str_linenum == linenum){
            // remove substring
            // strremove(vbuff, string);
            if(!(vinfo = get_version_info(vbuff))){
               return 1;
            }
            strcpy(vbuff, vinfo);
            return 1;
        }

        linenum++; 
    }
    return 0; 
}

#define MAX_VFILES 20
/**
* Desc: Given the version_dir and file pattern read all those file patterns \
        that match and store in the buffer
* @param version_dir - version directory name (absolute path)
* @param fpattn - the regex pattern of files to look for
* @param string - in files look for this line and use it to store info on \
                  that line in vbuff
* @param vbuff - the buffer where the info is to be stored
* @param buff_size - the size of the vbuffer
* @return An int denoting the number of files read 
*/
int
read_vfiles(char *version_dir, const char *fpattn, char * string, char *vsbuff, int buff_size){
    FILE *vfiles[MAX_VFILES], *vfile;
    char vbuff[MAX_LINE], last_vbuff[MAX_LINE];
    DIR *vdir;
    int num_vfiles, i;
    int n;
    memset(last_vbuff, 0, MAX_LINE);
    // Step 1 - Get the number of matched vfiles and get the files
    if(!(num_vfiles = get_vfiles(version_dir, fpattn, vfiles, MAX_VFILES)))
        return 0;

    // Step 2 - Iterate through the vfiles
    for(i=0; i< num_vfiles; i++){
        vfile = vfiles[i];
        memset(vbuff, 0, MAX_LINE);
        // Step 3 - read the file
        if(!read_vfile(vfile, string, vbuff, MAX_LINE)){
            // if buffer is out of space
            fclose(vfile);
            return 0;
       }
       if(last_vbuff[0]=='\0'){
           strcpy(last_vbuff, vbuff);
       }else{
            strcat(last_vbuff, " ");
            strcat(last_vbuff, vbuff);
       } 
        fclose(vfile);
    }
    if(strlen(last_vbuff)+1 > MAX_LINE){
        fprintf(stderr, "read_vfile: the data in the file is to long for destination buffer");
        return 0;
    }
    strcpy(vsbuff, last_vbuff);

    return i+1;
}
