
#include <stdio.h>
#include <string.h>
const char *raid_vdir_relpath_pattns[2]= {
    [0] = "driver/module",
    [1] = (char*)0
};

#define VDIR_PATHS "(driver/module|host*/scsi_host/host*|net/eth*|net/ib*)"

int get_next_vdirpath(int start, const char *vdir_paths){
    int i=0;
    for(i = start; vdir_paths[i] != '|' && vdir_paths[i] != ')'; i++)
        ;
    return i-1;
}

int main(){
    int start, end; 
    start=end=1;
    char buff[100];
    memset(buff, 0, 100);
    while(end != strlen(VDIR_PATHS)-2){
        end = get_next_vdirpath(start, VDIR_PATHS);
        memset(buff, 0, 100);
        memcpy(buff, &VDIR_PATHS[start], end-start+1);
        printf("%s\n", buff);
        start=end+2;
    }
    return 0;
}