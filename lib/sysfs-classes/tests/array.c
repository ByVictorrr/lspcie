enum {DRV_FPATTN,FWV_FPATTN};

typedef char * string_t;
typedef string_t vfile_pattns_t [100][2];

vfile_pattns_t fc = {
    [10] = {
        [DRV_FPATTN] = "driver_version",
        [FWV_FPATTN] = "(.{0,}fw_version|optrom_.{1,}_version)"
    },
    [11] = {
        [DRV_FPATTN] = "lpfc_drvr_version",
        [FWV_FPATTN] = "option_rom_version"
    }
};

struct methods{
};
/*
const struct methods m={
    &fc_vfile_pattns
};
*/

int main(){
    
    char *(*f)[100][2] = &fc;
    char *(*f1)[2] = &fc[0];
    return 0;
}