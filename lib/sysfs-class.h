#ifndef SYSFS_CLASS_H_
#define SYSFS_CLASS_H_
#include <stdio.h>
#include "pci.h"
#include "internal.h"

#define MAX_PATH 1024
#define MAX_PATTN 100
#define MAX_FILENAME 10
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256
#define PCI_VENDOR_MAX 0xffff
enum VDIR_RELPATHS{VDIR_DR, VDIR_FW};


struct pci_class_methods{
    const char *name; /* Name of the device */
    const char *(*vdir_relpath_pattns)[2]; /* File pattern of drv and fwv files */
    int (*read_drv)(struct pci_dev *, const struct pci_class_methods *pcm, char *dr_v, int drv_size);
    int (*read_fwv)(struct pci_dev *, const struct pci_class_methods *pcm, char *fw_v, int fwv_size); 
};


/* README: How to update a device 
    In order to update a device, setup a macro
    #define $(DEVICE_NAME)_{FW,DR}V_FILE_PATTN
    This tells ../sysfs-class.h that the function exists,
    so the corresponding pci_class_method structure can
    invoke the function  
*/



/* sysfs-class-utils.c */
#define RAID_READ_DRV
#define SATA_READ_DRV
#define SAS_READ_DRV
#define SAS_READ_FWV
#define NVM_READ_FWV
#define ETH_READ_DRV
#define ETH_READ_FWV
#define IB_READ_DRV
#define IB_READ_FWV
#define FAB_READ_DRV
#define FAB_READ_FWV
#define FC_READ_DRV
#define FC_READ_FWV


extern inline char * sysfs_name(struct pci_access *a);
int set_pci_dev_vers_dir(struct pci_dev *dev, const struct pci_class_methods *pcm);
int read_vfiles(char *version_dir, const char *fpattn, char * string, char *vbuff, int buff_size);
const char* get_pci_dev_drv_fpattn(struct pci_dev *d, const struct pci_class_methods *pcm);
const char * get_pci_dev_fwv_fpattn(struct pci_dev *d, const struct pci_class_methods *pcm); 
extern const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];

/* sysf-class-pattns.c */
/* Version directory patterns */
#define RAID_RELPATH_VDIR_PATTN "driver/module"
#define ATA_RELPATH_VDIR_PATTN NULL
#define SATA_RELPATH_VDIR_PATTN RAID_RELPATH_VDIR_PATTN
#define SAS_RELPATH_VDIR_PATTN "host*/scsi_host/host*"
#define NVM_RELPATH_VDIR_PATTN "nvme/nvme*"
#define ETH_RELPATH_VDIR_PATTN "net/eth*"
#define IB_RELPATH_VDIR_PATTN "net/ib*"
#define FAB_RELPATH_VDIR_PATTN IB_RELPATH_VDIR_PATTN
#define FC_RELPATH_VDIR_PATTN "host*/scsi_host/host*"


/* Version file patterns */
#define SAS_VFILE_PATTNS 
#define NVM_VFILE_PATTNS
#define RAID_VFILE_PATTNS 
#define SATA_VFILE_PATTNS 
#define FC_VFILE_PATTNS 

extern const char *sas_vfile_pattns[PCI_VENDOR_MAX][2];
extern const char *sata_vfile_pattns[PCI_VENDOR_MAX][2];
extern const char *nvm_vfile_pattns[PCI_VENDOR_MAX][2];
extern const char *raid_vfile_pattns[PCI_VENDOR_MAX][2];
extern const char *fc_vfile_pattns[PCI_VENDOR_MAX][2];

#endif