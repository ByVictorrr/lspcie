#ifndef SYSFS_CLASS_SLES_H_
#define SYSFS_CLASS_SLES_H_
#include <stdio.h> // NULL declaration
#include "vendors.h"
enum VFILE_PATTNS{DRV_FPATTN, FWV_FPATTN};
#define PCI_VENDOR_MAX 0xffff
/* README: How to update a device 
    In order to update a device, setup a macro
    #define $(DEVICE_NAME)_{FW,DR}V_FILE_PATTN
    This tells ../sysfs-class.h that the function exists,
    so the corresponding pci_class_method structure can
    invoke the function  
*/

/* =========Class 0x01 ==============*/
/* RELATIVE VDIR PATTNS */
#define RAID_RELPATH_VDIR_PATTN NULL
#define ATA_RELPATH_VDIR_PATTN NULL
#define SATA_RELPATH_VDIR_PATTN NULL
#define SAS_RELPATH_VDIR_PATTN "host*/scsi_host/host*"
#define NVM_RELPATH_VDIR_PATTN "nvme/nvme*"



/*========== Class 0x02 ===========*/
/* RELATIVE VDIR PATTNS */
#define ETH_RELPATH_VDIR_PATTN "net/eth*"
#define IB_RELPATH_VDIR_PATTN "net/ib*"
#define FAB_RELPATH_VDIR_PATTN IB_RELPATH_VDIR_PATTN

/*======== Class 0x0c =============*/
/* RELATIVE VDIR PATTNS */
#define FC_RELPATH_VDIR_PATTN "host*/scsi_host/host*"

#endif