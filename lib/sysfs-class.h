#include <stdio.h>

#define PCI_CLASS_MASK 0xFF00
#define PCI_SUBCLASS_MASK 0x00FF
#define MAX_PATH 1024
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256

extern inline char * sysfs_name(struct pci_access *a);
extern struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX];

/*===========0x01 class (mass storage controllers)==========*/
extern struct pci_class_methods scsi;
extern struct pci_class_methods ide; 
extern struct pci_class_methods floppy; 
extern struct pci_class_methods ipi; 
extern struct pci_class_methods raid; 
extern struct pci_class_methods ata; 
extern struct pci_class_methods sata; 
extern struct pci_class_methods sas; 
extern struct pci_class_methods nvm; 




/*========Class = 0x02(Network controllers)================*/
extern struct pci_class_methods eth;
extern struct pci_class_methods ib;



/*=================== Serial bus controller(0x0c)===========*/

extern struct pci_class_methods fc;


