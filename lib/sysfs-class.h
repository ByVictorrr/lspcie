#include <stdio.h>

#define PCI_CLASS_MASK 0xFF00
#define PCI_SUBCLASS_MASK 0x00FF
#define MAX_PATH 1024
#define MAX_PATTN 100
#define MAX_FILENAME 10
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256


// TODO : expand for other distros
#ifdef PCI_LINUX_OS_DISTRO_SLES
  #include "sysfs-classes/sys-class-sles.h"
#endif

#define SAS_READ_DRV
#define SAS_READ_FWV

#define ETH_READ_DRV
#define ETH_READ_FWV
#define IB_READ_DRV
#define IB_READ_FWV

extern inline char * sysfs_name(const struct pci_access *a);
extern const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];


