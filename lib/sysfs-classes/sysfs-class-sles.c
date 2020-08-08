#include <stdio.h> // NULL declaration
/* Class 0x01 */
#define SCSI_RELPATH_VDIR_PATTN "host*"
#define IDE_RELPATH_VDIR_PATTN NULL
#define FLOPPY_RELPATH_VDIR_PATTN NULL
#define IPI_RELPATH_VDIR_PATTN NULL
#define RAID_RELPATH_VDIR_PATTN NULL
#define ATA_RELPATH_VDIR_PATTN NULL
#define SATA_RELPATH_VDIR_PATTN NULL
#define SAS_RELPATH_VDIR_PATTN "host*/scsi_host/host*"
#define NVDIRM_RELPATH_VDIR_PATTN "nvme/nvme*"
/* Class 0x02 */
#define ETH_RELPATH_VDIR_PATTN "net/eth*"
#define IB_RELPATH_VDIR_PATTN "net/ib*"
/* Class 0x0c */
#define FC_RELPATH_VDIR_PATTN "host*/scsi_host/host*"
char **sas_fwv_file_pattns_table[vendor] = {
    {"fw*", "optrom_*"}, //vendor 0x0000
    {}, //vendor 0x0001
};

char **sas_fwv_file_pattns(struct pci_dev *); /* File pattern of fw version info(fn of class,sclass,os, and vendor) */
char **sas_drv_file_pattns(struct pci_dev *); /* File pattern of fw version info(fn of class,sclass,os, and vendor) */