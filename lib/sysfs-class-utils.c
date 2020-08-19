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
    memset(b_dirname, 0, BASE_DIRNAME_SIZE);
    memset(n_relvpath_pattn, 0, NEXT_RELPATH_SIZE);
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

    closedir(dir);
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

char *
get_pci_dev_vdir_path(struct pci_dev *dev, const char *vdir_rel_path_pattn){
    char ddir[MAX_PATH];
    const char *relvdir_pattn;
    char *vdir_path;
    memset(ddir, '\0', MAX_PATH);
    if(!get_pci_dev_dirname(dev, ddir, MAX_PATH))
        return NULL;
    else if(!(vdir_path=find_pci_dev_vers_dir(ddir, vdir_rel_path_pattn)))
        return NULL;
    return vdir_path;
}
/*================= read vfile functions==================*/
static int
str_linenum_in_vfile(char *string, FILE *vfile){
    #define MAX_LINE 100
    char *line;
    size_t len = 0;
    int line_num = 0;  
    ssize_t read;
    if(!string)
        return -1;
    while((read = getline(&line, &len, vfile)) != -1){
        if(strstr(line, string)){
            rewind(vfile);
            return line_num;
        }
        line_num++;
    }
    rewind(vfile);
    return -1;
}
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
static int
set_vfiles(const char *vdir_path, struct version_item *head, const char *fpattn){
    struct dirent *entry;
    regex_t regex;
    FILE *file;
    DIR *dir;
    int i=0, n;
    struct version_item *curr;
    char *vfile_path;

    // Step 1 - check params
    if(!vdir_path){
        fprintf(stderr, "get_vfiles: the vdir_path is null");
        return 0;
    }else if (!fpattn){
        fprintf(stderr, "get_vfiles: the fpattn object is null");
        return 0;
    }else if(regcomp(&regex, fpattn, REG_EXTENDED)){ /* Compile the regex */
        fprintf(stderr, "get_vfiles: regex compilation error");
        return 0;
    }else if(!(dir=opendir(vdir_path))){
        fprintf(stderr, "get_vfiles: opendir error");
        return 0; 
    }
    
    // Step 2 - read v_dir and look for matches of the regex
    while((entry = readdir(dir))){
        /* if the regex matches the file name open the file*/
         if(entry->d_type == DT_REG && !regexec(&regex, entry->d_name, 0, NULL, 0)){
            size_t nitems = strlen(vdir_path)+strlen(entry->d_name)+1;
            if(!(vfile_path = calloc(nitems+1, sizeof(char)))){
                fprintf(stderr, "get_vfiles: opendir error");
                return i;
            }
            n=snprintf(vfile_path, nitems, "%s/%s", vdir_path, entry->d_name);
            if (n < 0){
                fprintf(stderr, "get_vfiles: Folder name too long\n");
                regfree(&regex);
                return 0;
            }else{
                if(!head){
                    if(!(head=curr=malloc(sizeof(struct version_item)))){
                        fprintf(stderr, "malloc error\n");
                        return i;
                    }
                    curr->src_path=vfile_path;
                }else{
                    if(!(curr->next=malloc(sizeof(struct version_item)))){
                        fprintf(stderr, "malloc error\n");
                        return i;
                    }
                    curr=curr->next;
                    curr->src_path=vfile_path;
                }
               i++; 
            }
         }
    }
    regfree(&regex);
    return i;
}
static int
read_vfile(struct version_item *vitem, char * str_in_file){
    int str_linenum=0, linenum=0;  
    char *vinfo;
    char line_buff[MAX_LINE];
    FILE *vfile;
    memset(line_buff, 0, MAX_LINE);
    if(!(vfile=fopen(vitem->src_path, "r"))){
        fprintf(stderr, "read_vfile: fopen error\n");
        return 0;
    }

    /* Step 1 - get the line number the string is on 
              - or -1 if string not found in vfile
    */
    str_linenum = str_linenum_in_vfile(str_in_file, vfile);
    /* Step 2 - read through the vfile */
    while(fgets(line_buff, MAX_LINE-1, vfile)){
        /* Step 3 - trim white spaces */
        /* Step 4 - if new line exists then take it out */
        if(line_buff[strlen(line_buff)-1]=='\n')
            line_buff[strlen(line_buff)-1]=0;
        
        /* Step 5 - if the string is not found in vfile(read one line) */
        if(str_linenum == -1){
           /* Step 5.1 - get the version part of the line_buff*/
           if(!(vinfo = get_version_info(line_buff))){
               fclose(vfile);
               return 0;
           /* Step 5.2 - copy the vinfo into vitem struct */
           }else if(!(vitem->data=strdup(vinfo))){
               fprintf(stderr, "read_vfile: strdup error \n");
               fclose(vfile);
               return 0; 
           }
           return 1;
        /* Step 6 - if the string is found on that line */
        }else if(str_linenum == linenum){
           /* Step 6.1 - get the version part of the line_buff*/
            if(!(vinfo = get_version_info(line_buff))){
               fclose(vfile);
               return 0;
            }else if(!(vitem->data=strdup(vinfo))){
               fprintf(stderr, "read_vfile: strdup error \n");
               fclose(vfile);
               return 0; 
            }
            return 1;
        }
        linenum++; 
    }

    fclose(vfile);
    return 0; 
}

int
read_vfiles(char *version_dir, const char *fpattn, char *str_in_file, struct version_item *vitems){
    FILE *vfile;
    int num_vfiles, i;
    struct version_item *curr;
    // Step 1 - Get the number of matched vfiles and get the files
    if(!(num_vfiles = set_vfiles(version_dir, vitems, fpattn)))
        return 0;

    // Step 2 - Iterate through all items
    for(i=0, curr=vitems; curr; i++, curr=curr->next){
        if(!read_vfile(curr, str_in_file))
            break;
    }
    return i;
}