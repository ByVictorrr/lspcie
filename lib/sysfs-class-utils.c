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

static DIR *
pci_opendir(const char *dir_name){
    DIR * dir;
    if(!(dir=opendir(dir_name))){
        fprintf(stderr, "Not able to open the directory: %s", dir_name);
        return NULL;
    }
    return dir;
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
        dev->access->error("get_pci_dev_dirname: Folder name too long to store in desired buffer");
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
find_pci_dev_vers_dir(const char * cwd, const char *rel_vpath_pattn){
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
            fprintf(stderr, "find_pci_dev_vers_dir: strdup access->error\n"); 
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
        if (dp->d_type == DT_DIR && 
        (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))){
            // Case 1.1 -  See if there is a match
            if(!regexec(&regex, dp->d_name, 0, NULL, 0)){
                n = snprintf(n_cwd, MAX_PATH, "%s/%s", cwd, dp->d_name);
                if (n < 0 || n >= MAX_PATH){
                    fprintf(stderr, "find_pci_dev_vers_dir: Folder name too long\n");
                    closedir(dir);
                    return NULL;
                }
                closedir(dir);
                return find_pci_dev_vers_dir(n_cwd, n_relvpath_pattn);
            }
        }
    }
    fprintf(stderr, "find_pci_dev_vers_dir: Could not find a file pattern of %s in %s\n", rel_vpath_pattn, cwd); 
    return NULL;

}

/**
* Desc: Given a pci_dev d and a pcm find the version directory pattern and store in \
        the vidir_pattn_buff
  Assumption: pcm and d are not null 
@return: 1 if successful storing the rel vdir pattn in the buffer \
         0 if the buffer is too small to store the pattn in or if the relpath_vdir_pattn DNE
*/
static int 
set_pci_dev_relvdir_pattn(struct pci_dev *d, const struct pci_class_methods *pcm, 
                        char *vdir_pattn_buff, int buff_size){
    /*** Step 1 - check to see if the pcm exists */
    if (!(pcm->relpath_vdir_pattn)){
        d->access->error("get_pci_dev_relvdir_pattn: pcm->relpath_vdir_pattn is NULL");
        return 0;
    }else if(strlen(vdir_pattn_buff)+1 > buff_size){
        d->access->error("get_pci_dev_relvdir_pattn: the pcm->relpath_vdir_pattn string is to long to store in the desired buffer");
        return 0;
    }
    strcpy(vdir_pattn_buff, pcm->relpath_vdir_pattn);
    return 1;
}
/**
* Description: This function sets the pci_dev field version_folder
* @param 
*
*/
int 
set_pci_dev_vers_dir(struct pci_dev *dev, struct pci_class_methods *pcm){
    /* dev directory, relative path from dev directory to v dir pattern */
    char ddir[MAX_PATH], rel_vpath_pattn[MAX_PATH]; 
    char *vdir_name;
    // Step 0 - clear out the buffers used in this function
    memset(ddir, '\0', MAX_PATH);
    memset(rel_vpath_pattn, '\0', MAX_PATH);

    // Step 1 - check if the pci_dev has set the pci_ver_dir already
    if(dev->version_dir){
        return 1;
    }
    // Step 2 - get the relative path to the version directory
    if (!set_pci_dev_relvdir_pattn(dev, pcm, rel_vpath_pattn, MAX_PATH)){
        return 0;
    }// Step 3 - get the pci device base folder
    else if(!get_pci_dev_dirname(dev, ddir, MAX_PATH)){
        return 0;
    } // Step 4 - go through each type and get the vers folder(abs path)
    else if(!(vdir_name=find_pci_dev_vers_dir(ddir, rel_vpath_pattn))){
        return 0;
    }
    dev->version_dir=vdir_name;
    return 1;
}

/*===================read_vfile functions =============================================*/

/**
* Desc: Given the vendor id from the pci_dev, this function uses the pcm attribute \
*       to lookup the driver version file pattrn and stores that regex into the drv_fpattn_buff
*
*@param d : the pci device that we are using to look up the vendor id
*@param pcm : the pcm of that particular class & subclass of that device \
*             using it to get the driver file pattns using its lookup table
*@param drv_fpattn_buff: The buffer that is used to store the drv_fpattn string 
*@param buff_size: The max size of the drv_fpattn_buff
*@return an integer 1 if everything was sucessful in getting your drv_pattn stored in the buff \
*           integer 0 otherwise
**/
int 
set_pci_dev_drv_fpattn(struct pci_dev *d, struct pci_class_methods *pcm, 
                        char *drv_fpattn_buff, int buff_size){
    char  *drv_file_pattn; 
    /*** Step 1 - check to see if the pcm drv file pattn exists */
    if (!(pcm->drv_file_pattns)){
        d->access->error("get_pci_dev_drv_fpattn: the drv_file_pattns table needs to be added to your pcm structure");
        return 0;
    }else if(!(drv_file_pattn = pcm->drv_file_pattns[d->vendor_id])){
        d->access->error("get_pci_dev_drv_fpattn: You need to add a pattern in the drv_file_pattns for that vendor and device in sysfs-class-sles.h");
        return 0;
    }else if(strlen(drv_file_pattn)+1 > buff_size){
        d->access->error("get_pci_dev_drv_fpattn: the drv_fpattn string is to long to store inside the buffer");
        return 0;
    }

    strcpy(drv_fpattn_buff, drv_file_pattn);
    return 1; 
}

/**
* Desc: Given the vendor id from the pci_dev, this function uses the pcm attribute \
       to lookup the fw version file pattrn and stores that regex into the fwv_fpattn_buff
*@param d : the pci device that we are using to look up the vendor id
*@param pcm : the pcm of that particular class & subclass of that device \
              using it to get the driver file pattns using its lookup table
*@param fw_fpattn_buff: The buffer that is used to store the fwv_fpattn string 
*@param buff_size: The max size of the fw_fpattn_buff
*@return an integer 1 if everything was sucessful in getting your fwv_pattn stored in the buff \
**/
int 
set_pci_dev_fwv_fpattn(struct pci_dev *d, struct pci_class_methods *pcm, 
                        char *fwv_fpattn_buff, int buff_size){
    
    char *fwv_file_pattn; 
    if (!(pcm->fwv_file_pattns)){
        d->access->error("get_pci_dev_fwv_fpattn: the fwv_file_pattns table needs to be added to your pcm structure");
        return 0;
    }else if(!(fwv_file_pattn = pcm->fwv_file_pattns[d->vendor_id])){
        d->access->error("get_pci_dev_fwv_fpattn: You need to add a pattern in the fwv_file_pattns for that vendor and device in sysfs-class-sles.h");
        return 0;
    }else if(strlen(fwv_file_pattn)+1 > buff_size){
        d->access->error("get_pci_dev_fwv_fpattn: the fwv_fpattn string is to long to store inside the buffer");
        return 0;
    }
    strcpy(fwv_fpattn_buff, fwv_file_pattn);
    return 1; 
}


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
get_vfiles(const DIR *v_dir, const char *fpattn, FILE **files_buff, int buff_size){
    struct dirent *entry;
    regex_t regex;
    FILE *file;
    int i=0;
    // Step 1 - check params
    if(!v_dir){
        fprintf(stderr, "get_vfiles: the directory object is null");
        return -1;
    }else if (!fpattn){
        fprintf(stderr, "get_vfiles: the fpattn object is null");
        return -1;
    }else if (!files_buff){
        fprintf(stderr, "get_vfiles: files_buff is null");
        return -1;
    }else if(regcomp(&regex, fpattn, 0)){ /* Compile the regex */
        fprintf(stderr, "get_vfiles: regex compilation access->error");
        return -1;
    }
    // Step 2 - read v_dir and look for matches of the regex
    while((entry = readdir(v_dir))){
        /* if the regex matches the file name open the file*/
         if(!regexec(&regex, entry->d_name, 0, NULL, 0)){
            if((file=fopen(entry->d_name, "r"))){
                if(i > buff_size-1){
                    fprintf(stderr, "get_vfiles: Make your files_buff max size bigger");
                    return i+1;
                }
                files_buff[i++] = file; 
            }else
                fprintf(stderr, "get_vfiles: Unable to open for reading %s", entry->d_name);
         }
    }
    return i+1;
}

/**
* Desc: looks for string in vfile (also it rewinds the vfile to start of file)
* @param string : a string that is used to find in a file
* @param vfile : a version file 
* @return The line number the string is on or -1 if it doesnt exist
**/
static int
line_string_in_vfile(const char *string, FILE *vfile){
    char * line = NULL;
    size_t len = 0;
    int line_num = 0;  

    // Step 1 - start of a file
    rewind(vfile); 
    while(getline(&line, &len, vfile) != -1){
        if(strstr(string, line)){
            rewind(vfile); 
            return line_num;
        }
        line_num++;
    }
    rewind(vfile); 
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
static int
read_vfile(FILE *vfile, char * string, char *vbuff, int buff_size){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int str_linenum, linenum=0;  
    // if no string firmware read the full file and store it into buffer
    // Step 1 - Search for in file the string = {"Firmware" or "Driver"}
    str_linenum = line_string_in_vfile(string, vfile);
    while((read = getline(&line, &len, vfile)) != -1){
        int n;
        if(str_linenum == -1)
            // Read every line in the file and concat it into the buffer
            if((n = snprintf(vbuff, buff_size, "%s %s",vbuff, line)) < 0 || n>= buff_size){
                fprintf(stderr, "read_vfile: the data in the file is to long for destination buffer");
                return 0;
            }
        else
            // look for "firmware or driver in vfile"
            if(linenum==str_linenum)
                if((n = snprintf(vbuff, buff_size, "%s %s",vbuff, line)) < 0 || n>= buff_size){
                    fprintf(stderr, "read_vfile: the data in the file is to long for destination buffer");
                    return 0;
                }
        linenum++; 
        }
    return 1; 
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
read_vfiles(char *version_dir, const char *fpattn, char * string, char *vbuff, int buff_size){
    FILE *vfiles[MAX_VFILES], *vfile;
    DIR *vdir;
    int num_vfiles, i;
    char *fwv_buff;
    // Step 1 - Open version_dir
    if(!(vdir=opendir(version_dir))){
        fprintf(stderr, "read_vfiles: not able to open dir %s", version_dir);
        return 0;
    }
    // Step 2 - Get the number of matched vfiles and get the files
    if((num_vfiles = get_vfiles(vdir, fpattn, vfiles, MAX_VFILES)) == -1)
        return 0;

    // Step 3 - Iterate through the vfiles
    for(i=0; i< num_vfiles; i++){
        vfile = vfiles[i];
        // Step 4 - read the file
        if(!read_vfile(vfile, string, vbuff, buff_size)){
            // if buffer is out of space
            fclose(vfile);
            closedir(vdir);
            return 0;
        }
        fclose(vfile);

     }

    closedir(vdir);
    return i+1;
}
