#include <stdio.h> 
#include <regex.h> 
 
int match(const char *string, const char *pattern) 
{ 
    regex_t re; 
    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) return 0; 
    int status = regexec(&re, string, 0, NULL, 0); 
    regfree(&re); 
    if (status != 0) return 0; 
    return 1; 
} 
 
int main(void) 
{ 
    #define MAX_VEN 100
    enum VFILE_PATTNS{DRV_FPATTN, FWV_FPATTN};
    char *array[MAX_VEN][2] ={
        [0] = {
            [DRV_FPATTN] = "dfa",
            [FWV_FPATTN] = "ada"
        }
    };
    const char* s1 = "fw_version"; 
    const char* s2 = "optrom_efi_version"; 
    const char* re = "(.{0,}fw_version|optrom_.{1,}_version)"; 
    char arr[] = "host";
    printf("%s matches %s? %s\n", s1, re, match(arr, re) ? "true" : "false"); 
    printf("%s matches %s? %s\n", s2, re, match(s2, re) ? "true" : "false"); 
} 