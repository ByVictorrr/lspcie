
// #define SAS_GET_FWV_FILE_PATTN
#include <stdio.h>

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
int main(){
    struct bar b = {
        #ifdef SAS_GET_FWV_FILE_PATTN
            sas_getfwv
        #else
            NULL
        #endif
    };
    char *f = get_filev_pattn(1);

    return 0;
}