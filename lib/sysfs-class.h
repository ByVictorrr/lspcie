#include <stdio.h>

#define PCI_CLASS_MASK 0xFF00
#define PCI_SUBCLASS_MASK 0x00FF
#define MAX_PATH 1024
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256

extern inline char * sysfs_name(struct pci_access *a);
extern struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];


