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
  #include "sysfs-classes/sysfs-class-sles.h"
#endif

#define SAS_READ_DRV
#define SAS_READ_FWV

#define ETH_READ_DRV
#define ETH_READ_FWV
#define IB_READ_DRV
#define IB_READ_FWV


/* sysfs-class-utils.c */
inline char * sysfs_name(struct pci_access *a);

int set_pci_dev_vers_dir(struct pci_dev *dev, struct pci_class_methods *pcm);
int read_vfiles(char *version_dir, const char *fpattn, char * string, char *vbuff, int buff_size);
int set_pci_dev_drv_fpattn(struct pci_dev *d, struct pci_class_methods *pcm, char *drv_fpattn_buff, int buff_size);
int set_pci_dev_fwv_fpattn(struct pci_dev *d, struct pci_class_methods *pcm, char *fwv_fpattn_buff, int buff_size);
 
 
 
extern struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];



#endif