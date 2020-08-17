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
    const char **vdir_relpath_pattns; /* File pattern of drv and fwv files */
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
#define RAID_READ_FWV
#define SATA_READ_DRV
#define SATA_READ_FWV
#define SAS_READ_DRV
#define SAS_READ_FWV
#define NVM_READ_DRV
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
int set_pci_dev_fwvdir(struct pci_dev *dev, const struct pci_class_methods *pcm);
int set_pci_dev_drvdir(struct pci_dev *dev, const struct pci_class_methods *pcm);
int read_vfiles(char *version_dir, const char *fpattn, char * string, char *vbuff, int buff_size);
extern const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];

/* sysf-class-pattns.c */

#define PCI_DRV_FPATTNS "(\
^version$|driver_version|lpfc_drvr_version|\
srcversion\
)"
#define PCI_FWV_FPATTNS "(\
^version$|firmware_version|firmware_rev|option_rom_version|\
.{0,}fw_version|optrom_.{1,}_version\
)"

/* Version directory patterns */
/*======== Class 0x01==============*/
#define RAID_VDIR_RELPATH_PATTNS
#define SAS_VDIR_RELPATH_PATTNS
#define SATA_VDIR_RELPATH_PATTNS
#define NVM_VDIR_RELPATH_PATTNS
extern const char *raid_vdir_relpath_pattns[2];
extern const char *sas_vdir_relpath_pattns[2];
extern const char *sata_vdir_relpath_pattns[2];
extern const char *nvm_vdir_relpath_pattns[2];
/*======== Class 0x02==============*/
#define ETH_VDIR_RELPATH_PATTNS
#define IB_VDIR_RELPATH_PATTNS
#define FAB_VDIR_RELPATH_PATTNS
extern const char *eth_vdir_relpath_pattns[2];
extern const char *ib_vdir_relpath_pattns[2];
extern const char *fab_vdir_relpath_pattns[2];
/*======== Class 0x0c==============*/
#define FC_VDIR_RELPATH_PATTNS
extern const char *fc_vdir_relpath_pattns[2];
#endif