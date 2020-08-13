#include <stdio.h> // NULL declaration
#include "vendors.h"
#include "sysfs-class.h"

/* ==========Class 0x01 ===================*/
const char *raid_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [INTEL_VENID] = { /* Vendor */
        [DRV_FPATTN] = "^version$"
    }
};
const char *sata_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [INTEL_VENID] = { /* Vendor */
        [DRV_FPATTN] = "^version$"
    }
};




const char *sas_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [ADAPTEC_VENID] = { /* Vendor */
        [DRV_FPATTN] = "(driver_version|^version$)", 
        [FWV_FPATTN] = "(firmware_version|^version$)"
    }
};

const char *nvm_vfile_pattns[PCI_VENDOR_MAX][2] = {
    [INTEL_VENID] = {
        [FWV_FPATTN] = "firmware_rev"
    }
};

/* ==========Class 0x0c ===================*/
const char *fc_vfile_pattns[PCI_VENDOR_MAX][2] = {
    /* QLogic same as REDHAT */
    [QLOGIC_VENID] = {
        [DRV_FPATTN] = "driver_version",
        [FWV_FPATTN] = "(.{0,}fw_version|optrom_.{1,}_version)"
    },
    [BROADCOM_VENID] = {
        [DRV_FPATTN] = "lpfc_drvr_version",
        [FWV_FPATTN] = "option_rom_version"
    },
    [EMULEX_VENID] = {
        [DRV_FPATTN] = "lpfc_drvr_version",
        [FWV_FPATTN] = "option_rom_version"
    }
};