#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>

#define MAX_PATH 1024
char* base_dirname(const char *path, char *b_dirname, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    int i=0;
    // Step 0 - check if either parm is NULL
    if(!path || !b_dirname){
        fprintf(stderr, "get_base_dir: both parameters need to be non NULL");
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

char *next_path(const char *path, char *n_path, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    int exists=0;
    // Step 0 - check if either parm is NULL
    if(!path || !n_path){
        fprintf(stderr, "get_base_dir: both parameters need to be non NULL");
        return NULL;
    }
    // Step 1 - cpy data from path to buff (so we can use strtok)
    if(strlen(path)+1 > MAX_PATH){
        fprintf(stderr, "get_base_next: *path arg is to large");
        return 0;
    }
    strcpy(buff, path);
    memset(n_path, '\0', size);
    // Case 1 - No next path
    if((token = strtok(buff, "/"))==NULL){
        return NULL;
    }
    // Case 2 - next path (exists)
    for(token=strtok(NULL, "/"); token; strcat(n_path, "/")){
        strcat(n_path, token);
        token=strtok(NULL, "/");
        exists=1;
    }
    // To see if there exists another folder
    if(!exists){
        return NULL;
    }
    n_path[strlen(n_path)-1] = '\0';
    // need to get rid of the last "/"

    return n_path;
}

int main(int argc, char *argv[]){
    char *rel_path= argv[1];
    char b_dirname[100]= {'\0'};
    char n_path[100]={'\0'};
    base_dirname(rel_path, b_dirname, 100);
    next_path(rel_path, n_path, 100);

    printf("base name of path %s is %s\n", rel_path, b_dirname);
    printf("next path of path %s is %s\n", rel_path, n_path);
    return 0;
}