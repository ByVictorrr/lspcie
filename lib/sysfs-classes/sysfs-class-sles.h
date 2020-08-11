#ifndef SYSFS_CLASS_SLES_H_
#define SYSFS_CLASS_SLES_H_
#include <stdio.h> // NULL declaration
#include "vendors.h"

#define PCI_VENDOR_MAX 0xffff
/* README: How to update a device 
    In order to update a device, setup a macro
    #define $(DEVICE_NAME)_{FW,DR}V_FILE_PATTN
    This tells ../sysfs-class.h that the function exists,
    so the corresponding pci_class_method structure can
    invoke the function  
*/

/* Class 0x01 */
/*========SCSI======================*/
#define RAID_RELPATH_VDIR_PATTN NULL
#define ATA_RELPATH_VDIR_PATTN NULL
#define SATA_RELPATH_VDIR_PATTN NULL


/*===========SAS ====================================*/
#define SAS_RELPATH_VDIR_PATTN "host*/scsi_host/host*"
#define SAS_DRV_FILE_PATTNS 
extern char *sas_drv_file_pattns[PCI_VENDOR_MAX];
#define SAS_FWV_FILE_PATTNS
extern char *sas_fwv_file_pattns[PCI_VENDOR_MAX];

/*================NVM===============================*/
#define NVM_RELPATH_VDIR_PATTN "nvme/nvme*"
#define NVM_FWV_FILE_PATTNS
extern char *nvm_fwv_file_pattns[PCI_VENDOR_MAX];



/* Class 0x02 */
#define ETH_RELPATH_VDIR_PATTN "net/eth*"
#define IB_RELPATH_VDIR_PATTN "net/ib*"

/* Class 0x0c */
#define FC_RELPATH_VDIR_PATTN "host*/scsi_host/host*"
/*=================FC================*/

#define FC_DRV_FILE_PATTNS 
extern char *fc_drv_file_pattns[PCI_VENDOR_MAX];

#define FC_FWV_FILE_PATTNS
extern char *fc_fwv_file_pattns[PCI_VENDOR_MAX];
/*=================================================*/
#endif