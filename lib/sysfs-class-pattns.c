#include <stdio.h> // NULL declaration
#include "vendors.h"
#include "sysfs-class.h"

#define PCI_DRV_FPATTN "(\
^version$|driver_version|lpfc_drvr_version|\
)"
#define PCI_FWV_FPATTN "(\
^version$|firmware_version|firmare_rev|option_rom_version|\
.{0,}fw_version|optrom_.{1,}_version\
)"
#define MAX_VDIR_RELPATH_PATTNS 4
/* ==========Class 0x01 ===================*/
const char *raid_vdir_relpath_pattns[2][MAX_VDIR_RELPATH_PATTNS] = {
    [VDIR_DR] = {

    },
    [VDIR_FW] = {

    }
};
const char *sata_vdir_relpath_pattns[2][MAX_VDIR_RELPATH_PATTNS] = {
    [VDIR_DR] = {

    },
    [VDIR_FW] = {

    }
};
const char *sas_vdir_relpath_pattns[2][MAX_VDIR_RELPATH_PATTNS] = {
    [VDIR_DR] = {

    },
    [VDIR_FW] = {

    }
};



/* Noticed for all nvm they have seperate version directories 
    Driver_vdir = (dev)/driver/module/version
    Fw_vdr = nvme/nvme0/fw_rev
*/
const char *nvm_vdir_relpath_pattns[2][MAX_VDIR_RELPATH_PATTNS] = {
    [VDIR_DR] = {

    },
    [VDIR_FW] = {

    }
};

/* ==========Class 0x0c ===================*/
const char *fc_vdir_relpath_pattns[2][MAX_VDIR_RELPATH_PATTNS] = {
    [VDIR_DR] = {

    },
    [VDIR_FW] = {

    }
};

