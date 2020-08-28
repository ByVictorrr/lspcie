#ifndef SYSFS_CLASS_H_
#define SYSFS_CLASS_H_
#include <stdio.h>
#include "pci.h"
#include "internal.h"

#define MAX_LINE 100

#define MAX_PATH 1024
#define MAX_PATTN 100
#define MAX_FILENAME 10
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256
#define PCI_VENDOR_MAX 0xffff
enum VDIR_RELPATHS{VDIR_DR, VDIR_FW};

struct pci_class_methods{
    const char *name; /* Name of the device */
    int (*read_drv)(struct pci_dev *, struct version_info **);
    int (*read_fwv)(struct pci_dev *, struct version_info **); 
    int (*read_optv)(struct pci_dev *, struct version_info **); 
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
#define MSC_READ_OPTV

#define NET_READ_DRV
#define NET_READ_FWV

/*
#define DC_READ_DRV
#define DC_READ_FWV
#define DC_READ_OPTV
*/

#define SBC_READ_DRV
#define SBC_READ_FWV
#define SBC_READ_OPTV

extern inline char * sysfs_name(struct pci_access *a);
extern char * get_pci_dev_vdir_path(struct pci_dev *dev, const char *vidr_relpath_pattn);
extern int read_vfiles(char *version_dir, const char *fpattn, char *str_in_file, struct version_item **vitems);
extern const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX];



#define PCI_DRV_FPATTN "(\
^version$|driver_version|lpfc_drvr_version|beiscsi_drvr_ver\
)"

#define PCI_FWV_FPATTN "(\
^version$|firmware_version|firmware_rev|\
^fw_version$|beiscsi_fw_ver\
)"
#define PCI_OPTV_FPATTN "(\
option_rom_version|optrom_.{1,}_version\
)"
// FOR now net devices are using DR vdir
#define PCI_VDIR_DR_RELPATH_PATTN "(\
host*/scsi_host/host*|net/e*|net/ib*|\
driver/module\
)"
#define PCI_VDIR_FW_RELPATH_PATTN "(\
host*/scsi_host/host*|nvme/nvme*\
)"
#define PCI_VDIR_OPT_RELPATH_PATTN PCI_VDIR_FW_RELPATH_PATTN

#endif