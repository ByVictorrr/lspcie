#ifndef SYSFS_CLASS_H_
#define SYSFS_CLASS_H_
#include <stdio.h>

#define PCI_CLASS_MASK 0xFF00
#define PCI_SUBCLASS_MASK 0x00FF
#define MAX_PATH 1024
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256
;
/*=============Mass storage controlers(class=0x01)=================*/
extern struct pci_class_methods scsi; 
extern struct pci_class_methods ide; 
extern struct pci_class_methods floppy; 
extern struct pci_class_methods ipi; 
extern struct pci_class_methods raid; 
extern struct pci_class_methods ata; 
extern struct pci_class_methods sata; 
extern struct pci_class_methods sas; 
extern struct pci_class_methods nvm; 

/*=========================================================*/


/*========Class = 0x02(Network controllers)================*/

extern struct pci_class_methods eth;
extern struct pci_class_methods ib;
/*=================== Serial bus controller(0x0c)===========*/
extern struct pci_class_methods fc;



struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX] = {
    {NULL}, /* Unclassified devices */
    {&scsi, &ide, &floppy, &ipi, &raid, &ata, &sata, &sas, &nvm}, /* Mass storage controllers */
    {&eth, NULL, NULL, NULL, NULL, NULL, NULL, &ib, NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL},
    {NULL, NULL, NULL, NULL, &fc, NULL, NULL, NULL, NULL} // Serial bus controller
};



#endif