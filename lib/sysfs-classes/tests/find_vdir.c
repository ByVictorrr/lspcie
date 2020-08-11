
// #define SAS_GET_FWV_FILE_PATTN
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <regex.h>


/*============ Path Utility functions ==============================*/
/**
* Desc: Given a path (absolute or relative) it returns a string of \
       the first directory in the path
* @param path - the string used to get the first directory in the path
* @param b_dirname - the buffer used to store the result of the return value
* @param size - size of the buffer (b_dirname)
* @return b_dirname if the operations were sucessful, NULL otherwise
*/
char* base_dirname(const char *path, char *b_dirname, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    int i=0;
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
char *next_relpath(const char *path, char *n_relpath, int size){
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
            fprintf(stderr, "find_pci_dev_vers_dir: strdup error\n"); 
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
int main(){
    DIR *d;
    #define CWD_SIZE 100
    #define CWD "/sys/bus/pci/devices/0001:c1:00.0"
    #define REL_VPATH_PATTN "infiniband_verbs/uverbs*"

    char cwd[CWD_SIZE], rel_vpath_pattn[100] = {'\0'};
    char *vdir;
    strcpy(cwd, CWD);
    strcpy(rel_vpath_pattn, REL_VPATH_PATTN);
    vdir=find_pci_dev_vers_dir(cwd, rel_vpath_pattn);
    return 0;
}