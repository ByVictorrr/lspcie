#include <stdio.h> // NULL declaration
#include "vendors.h"
#include "sysfs-class.h"

/* ==========Class 0x01 ===================*/
const char *raid_vdir_relpath_pattns[2]= {
    [VDIR_DR] = "driver/module",
    [VDIR_FW] = NULL
};
const char *sata_vdir_relpath_pattns[2]= {
    [VDIR_DR] = "driver/module",
    [VDIR_FW] = NULL
};
const char *sas_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "host*/scsi_host/host*",
    [VDIR_FW] = "host*/scsi_host/host*"
};



/* Noticed for all nvm they have seperate version directories 
    Driver_vdir = (dev)/driver/module/version
    Fw_vdr = nvme/nvme0/fw_rev
*/
const char *nvm_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "driver/module",
    [VDIR_FW] = "nvme/nvme*"
};

/* ==========Class 0x02 ===================*/
const char *eth_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "net/eth*",
    [VDIR_FW] = "net/eth*"
};
const char *ib_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "net/ib*",
    [VDIR_FW] = "net/ib*"
};

const char *fab_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "net/ib*",
    [VDIR_FW] = "net/ib*"
};




/* ==========Class 0x0c ===================*/
const char *usb_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "driver/module",
    [VDIR_FW] = NULL
};
const char *fc_vdir_relpath_pattns[2] = {
    [VDIR_DR] = "host*/scsi_host/host*",
    [VDIR_FW] = "host*/scsi_host/host*"
};

