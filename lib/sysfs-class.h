#include <stdio.h>

#define PCI_CLASS_MASK 0xFF00
#define PCI_SUBCLASS_MASK 0x00FF
#define MAX_PATH 1024
#define PCI_CLASS_MAX 256
#define PCI_SCLASS_MAX 256

// TODO : expand for other distros
#ifdef PCI_LINUX_OS_DISTRO_SLES
  #include "sysfs-classes/sys-class-sles.h"
#endif


extern inline char * sysfs_name(const struct pci_access *a);


/*===========0x01 class (mass storage controllers)==========*/
int sas_read_versions(const struct pci_dev *dev, char *dr_v, char *fw_v);
char **sas_fwv_file_pattn(const struct pci_dev *);
char **sas_drv_file_pattn(const struct pci_dev *);
int nvm_read_versions(const struct pci_dev *dev, char *dr_v, char *fw_v);

const struct pci_class_methods scsi = {
    "SCSI storage controller",
     SCSI_RELPATH_VDIR_PATTN,
     #ifdef SCSI_DRV_FILE_PATTNS
        sas_drv_file_pattns,
     #else 
        NULL,
     #endif

     #ifdef SCSI_FWV_FILE_PATTNS
        sas_fwv_file_pattns,
     #else 
        NULL,
     #endif
     NULL // scsi_read_versions
};
const struct pci_class_methods ide = {
    "IDE iface",
    IDE_RELPATH_VDIR_PATTN,
    #ifdef IDE_DRV_FILE_PATTNS
        IDE_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef IDE_FWV_FILE_PATTNS
        ide_fwv_file_pattns,
    #else 
        NULL,
    #endif

    NULL // ide_read_versions
};
const struct pci_class_methods floppy = {
    "Floppy disk controller",
    FLOPPY_RELPATH_VDIR_PATTN,
    #ifdef FLOPYY_DRV_FILE_PATTNS
        floppy_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef FLOPPY_FWV_FILE_PATTNS
        floppy_fwv_file_pattns,
    #else 
        NULL,
    #endif

    NULL //floppy_read_versions
};
const struct pci_class_methods ipi = {
    "IPI bus controller",
    IPI_RELPATH_VDIR_PATTN,
    #ifdef IPI_DRV_FILE_PATTNS
        ipi_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef IPI_FWV_FILE_PATTNS
        ipi_fwv_file_pattns,
    #else 
        NULL,
    #endif

    NULL //ipi_read_versions
};
const struct pci_class_methods raid = {
    "RAID bus controller",
    RAID_RELPATH_VDIR_PATTN,
    #ifdef RAID_DRV_FILE_PATTNS
        RAID_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef RAID_FWV_FILE_PATTNS
        RAID_fwv_file_pattns,
    #else 
        NULL,
    #endif

    NULL //raid_read_versions
};
const struct pci_class_methods ata = {
    "ATA controller",
    ATA_RELPATH_VDIR_PATTN,
    #ifdef ATA_DRV_FILE_PATTNS
        ata_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef ATA_FWV_FILE_PATTNS
        ata_fwv_file_pattns,
    #else 
        NULL,
    #endif

    NULL // ata_read_versions (do the same as above)
};
const struct pci_class_methods sata = {
    "SATA controller",
    SATA_RELPATH_VDIR_PATTN,
    #ifdef SATA_DRV_FILE_PATTNS
        sata_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef SATA_FWV_FILE_PATTNS
        sata_fwv_file_pattns,
    #else 
        NULL,
    #endif


    NULL //sata_read_versions
};
const struct pci_class_methods sas = {
    "Serial Attached SCSI controller",
    #ifdef SAS_DRV_FILE_PATTNS
        sas_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef SAS_FWV_FILE_PATTNS
        sas_fwv_file_pattns,
    #else 
        NULL,
    #endif

    sas_read_versions
};
const struct pci_class_methods nvm = {
    "Non-Volatile memory controller",
    NVM_RELPATH_VDIR_PATTN,
    #ifdef NVM_DRV_FILE_PATTNS
        nvm_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef NVM_FWV_FILE_PATTNS
        nvm_fwv_file_pattns,
    #else 
        NULL,
    #endif


    nvm_read_versions
};


/*========Class = 0x02(Network controllers)================*/
int eth_read_versions(const struct pci_dev * dev, char *dr_v, char *fw_v);
int ib_read_versions(const struct pci_dev * dev, char *dr_v, char *fw_v);

const struct pci_class_methods eth = {
    "Ethernet controller",
    ETH_RELPATH_VDIR_PATTN,
    #ifdef ETH_DRV_FILE_PATTNS
        eth_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef ETH_FWV_FILE_PATTNS
        eth_fwv_file_pattns,
    #else 
        NULL,
    #endif

 
    eth_read_versions
};
/*
const struct pci_class_methods tr = {
    "Token ring network controller",
    NULL,
    tr_read_versions
};
const struct pci_class_methods fddi = {
    "FDDI network controller",
    NULL,
    fddi_read_versions
};
*/
const struct pci_class_methods ib = {
    "Infiniband controller",
    IB_RELPATH_VDIR_PATTN,
    #ifdef IB_DRV_FILE_PATTNS
        ib_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef IB_FWV_FILE_PATTNS
        ib_fwv_file_pattns,
    #else 
        NULL,
    #endif
 
    ib_read_versions
};

/*=================== Serial bus controller(0x0c)===========*/
 
int fc_read_versions(const struct pci_dev * dev, char *dr_v, char *fw_v);
const struct pci_class_methods fc = {
    "Infiniband controller",
    FC_RELPATH_VDIR_PATTN,
    #ifdef FC_DRV_FILE_PATTNS
        fc_drv_file_pattns,
    #else 
        NULL,
    #endif

    #ifdef FC_FWV_FILE_PATTNS
        fc_fwv_file_pattns,
    #else 
        NULL,
    #endif

    fc_read_versions
};

/*==============PCM Table ==================================*/
const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX] = {
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




