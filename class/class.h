#ifndef CLASS_H_
#define CLASS_H_

struct pci_class_methods{
    char *name;
    char *(*get_versions)(struct pci_dev *); /*returns [0] = dr_v, [1] = fw_v*/
};

#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256

struct pci_class_methods[PCI_CLASS_MAX][PCI_SCLASS_MAX] = {
    {NULL}, /* Unclassified devices */
    {scsi, ide, floppy, ipi, raid, ata, sata, sas}, /* Mass storage controllers */
    {eth},
    {},
    {},
    {}

};


/* THIS IS FOR GUIDENCE
struct pci_methods {
  char *name;
  char *help;
  void (*config)(struct pci_access *);
  int (*detect)(struct pci_access *);
  void (*init)(struct pci_access *);
  void (*cleanup)(struct pci_access *);
  void (*scan)(struct pci_access *);
  unsigned int (*fill_info)(struct pci_dev *, unsigned int flags);
  int (*read)(struct pci_dev *, int pos, byte *buf, int len);
  int (*write)(struct pci_dev *, int pos, byte *buf, int len);
  int (*read_vpd)(struct pci_dev *, int pos, byte *buf, int len);
  void (*init_dev)(struct pci_dev *);
  void (*cleanup_dev)(struct pci_dev *);
}; 
*/
#endif