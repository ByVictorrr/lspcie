enum {DRV_FPATTN,FWV_FPATTN};

typedef char * string_t;
typedef string_t vfile_pattns_t [100][2];

#define PCI_VENDOR_MAX 0xffff
#define PCI_MAX_FPATTNS 4
const char *sas[PCI_VENDOR_MAX][2][PCI_MAX_FPATTNS] = {
    [99] = { /* Vendor */
        [DRV_FPATTN] = { /* Driver version file pattern */
            [0] = "driver_version", /* one of the patterns */
            [1] = "version"
        },
        [FWV_FPATTN] = {
            [0] = "firmware_version",
            [1] = "version"
        }
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
    
    const char *(*f1)[2][4] = &(sas[0]);
    return 0;
}