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
    int (*read_drv)(struct pci_dev *, char *dr_v, int drv_size);
    int (*read_fwv)(struct pci_dev *, char *fw_v, int fwv_size); 
};


/* README: How to update a device 
    In order to update a device, setup a macro
    #define $(DEVICE_NAME)_{FW,DR}V_FILE_PATTN
    This tells ../sysfs-class.h that the function exists,
    so the corresponding pci_class_method structure can
    invoke the function  
*/



/* sysfs-class-utils.c */
#define MSC_READ_DRV
#define MSC_READ_FWV
#define NET_READ_DRV
#define NET_READ_FWV
#define SBC_READ_DRV
#define SBC_READ_FWV


extern inline char * sysfs_name(struct pci_access *a);
int set_pci_dev_fwvdir(struct pci_dev *dev);
int set_pci_dev_drvdir(struct pci_dev *dev);
int read_vfiles(char *version_dir, const char *fpattn, char * string, char *vbuff, int buff_size);
extern const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX];

/* sysf-class-pattns.c */

#define PCI_DRV_FPATTNS "(\
^version$|driver_version|lpfc_drvr_version|\
srcversion\
)"
#define PCI_FWV_FPATTNS "(\
^version$|firmware_version|firmware_rev|option_rom_version|\
.{0,}fw_version|optrom_.{1,}_version\
)"
// FOR now net devices are using DR vdir
#define PCI_VDIR_DR_RELPATH_PATTNS "(\
host*/scsi_host/host*|net/e*|net/ib*|\
driver/module\
)"
#define PCI_VDIR_FW_RELPATH_PATTNS "(\
host*/scsi_host/host*|nvme/nvme*\
)"
/* Version directory patterns */
/*======== Class 0x01==============*/

#endif