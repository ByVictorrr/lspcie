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


// TODO : expand for other distros
#ifdef PCI_LINUX_OS_DISTRO_SLES
  #include "sysfs-class-sles.h"
#endif
#ifdef PCI_LINUX_OS_DISTRO_REDHAT
  #include "sysfs-class-redhat.h"
#endif

/*====== Class 0x01 =====*/
#define SAS_READ_DRV
#define SAS_READ_FWV
#define NVM_READ_FWV


/*====== Class 0x02 =====*/
#define ETH_READ_DRV
#define ETH_READ_FWV
#define IB_READ_DRV
#define IB_READ_FWV
#define FAB_READ_DRV
#define FAB_READ_FWV

/*====== Class 0x0C =====*/
#define FC_READ_DRV
#define FC_READ_FWV


/* sysfs-class-utils.c */
extern inline char * sysfs_name(struct pci_access *a);

int set_pci_dev_vers_dir(struct pci_dev *dev, const struct pci_class_methods *pcm);
int read_vfiles(char *version_dir, const char *fpattn, char * string, char *vbuff, int buff_size);
const char* get_pci_dev_drv_fpattn(struct pci_dev *d, const struct pci_class_methods *pcm);
const char * get_pci_dev_fwv_fpattn(struct pci_dev *d, const struct pci_class_methods *pcm);
 
extern const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];


#endif