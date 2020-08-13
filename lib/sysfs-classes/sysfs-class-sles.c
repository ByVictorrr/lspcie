#include <stdio.h> // NULL declaration
#include "vendors.h"
#include "sysfs-class-sles.h"

/* ==========Class 0x01 ===================*/

const char *sas_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [ADAPTEC_VENID] = {
        [DRV_FPATTN] = "version",
        [FWV_FPATTN] = "version"
    }
};

const char *nvm_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [INTEL_VENID] = {
        [FWV_FPATTN] = "firmware_rev" 
    }
};

/*========== Class 0x02 ====================*/
/*
const char *fab_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [INTEL_VENID] = {
        [DRV_FPATTN] = "",
        [FWV_FPATTN] = ""
    }
}
*/
/* ==========Class 0x0c ===================*/
const char *fc_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [QLOGIC_VENID] = {
        [DRV_FPATTN] = "driver_version",
        [FWV_FPATTN] = "(.{0,}fw_version|optrom_.{1,}_version)"
    },
    [BROADCOM_VENID] = {
        [DRV_FPATTN] = "lpfc_drvr_version",
        [FWV_FPATTN] = "option_rom_version"
    },
    [EMULEX_VENID] = {
        [DRV_FPATTN] = "lpfc_drvr_version" ,
        [FWV_FPATTN] = "option_rom_version"
    }
};