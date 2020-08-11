#include <stdio.h>
#include <string.h>

static int
line_string_in_vfile(char *string, FILE *vfile){
    #define MAX_LINE 100
    char line[MAX_LINE];
    size_t len = 0;
    int line_num = 0;  
    ssize_t read;
    memset(line, 0, MAX_LINE);
    while(fgets(line, MAX_LINE, vfile)){
        if(strstr(line, string))
            return line_num;
        line_num++;
    }
    return -1;

}
int main(){
    char *ptr;
    char *text = "  driver: 1.1.4-130\n";
    char *search = "driver";
    int line_num;
    FILE *fp;
    if(!(fp=fopen("/root/lspcie/lib/sysfs-classes/tests/file", "r"))){
        return -1;
    }
    line_num = line_string_in_vfile("driver", fp);

    return 0;
}