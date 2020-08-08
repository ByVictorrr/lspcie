
// #define SAS_GET_FWV_FILE_PATTN
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
/*
struct bar{
    void (*ptr)();
};
#define VENDOR_MAX 0xffff
char *sas_drv_file_pattns[VENDOR_MAX] = {
    [21] = "(fw|optrom_)*"
};
const char *get_filev_pattn(int v){
    if(v==1){
        return "(hi|victor)*";
    }else{
        return "(fuck|hpe|hello)*";
    }

}
*/
void get_dir_name(DIR *dir, char *dirname_buff, int buff_size){
    struct dirent *dp;
    struct stat info;
    ino_t inode;
    DIR *pdir;
    char *ppath;
    while((dp=readdir(dir))){
        if (strcmp(dp->d_name, ".") == 0 && !stat(dp->d_name, &info)){
            inode = info.st_ino;
            ppath = realpath(dp->d_name, NULL);
            break;
        }
    }
    if(!(pdir=opendir(ppath))){
        return;
    }
     while((dp=readdir(pdir))){
        if(!stat(dp->d_name, &info) && inode == info.st_ino){
            strcpy(dirname_buff, dp->d_name);
            break;
        }
    }

    closedir(pdir);
}
int main(){
    DIR *d;
    char dirname[10] = {'\0'};
    /*
    struct bar b = {
        #ifdef SAS_GET_FWV_FILE_PATTN
            sas_getfwv
        #else
            NULL
        #endif
    };
    */
    if(!(d=opendir("/root/lspcie"))){
        return -1;
    }
    get_dir_name(d, dirname, 10);

    return 0;
}