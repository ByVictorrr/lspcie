#include "sysfs-class.h"
#include "internal.h"
#include "pread.h"
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include "pci.h"
#include <unistd.h>

#define DRV_FPATTN_MAX 100
#define FWV_FPATTN_MAX 100
    

/*==========================================================*/
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @return to indicate what buffer is valid
* Assumptions: pcm isnt null (checked in the calling function)
*/
static int 
sas_read_drv(struct pci_dev *dev, struct pci_class_methods *pcm, char *dr_v, int drv_size){
    char drv_fpattn[DRV_FPATTN_MAX];
    // Step 0 - clear out the fpattn buffers
    memset(drv_fpattn, '\0', DRV_FPATTN_MAX);
    // Step 1 - set the pci attribute version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        dev->access->error("sas_read_drv: not able to find/open version directory");
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!set_pci_dev_drv_fpattn(dev, pcm, drv_fpattn, DRV_FPATTN_MAX)){
        return 0;
    }else if(!read_vfiles(dev->version_dir, drv_fpattn, "Driver", dr_v, drv_size)){
        return 0;
    }
    return 1;
}

static int 
sas_read_fwv(struct pci_dev *dev, struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    char fwv_fpattn[FWV_FPATTN_MAX];
    // Step 0 - clear out the local buffers
    memset(fwv_fpattn, '\0', DRV_FPATTN_MAX);
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!set_pci_dev_fwv_fpattn(dev, pcm, fwv_fpattn, FWV_FPATTN_MAX)){
        return 0;
    }else if(!read_vfiles(dev->version_dir, fwv_fpattn, "Firmware", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}




/*=========================================================*/


/*========Class = 0x02(Network controllers)================*/
#include <linux/ethtool.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/sockios.h>

static int 
net_read_info(struct pci_dev *dev, struct pci_class_methods *pcm, struct ethtool_drvinfo *info){
    char vdir_pattn[MAX_PATTN];
    struct ifreq ifr;

    int fd;

    memset(vdir_pattn, 0, MAX_PATTN);
    memset(vdir_pattn, 0, MAX_PATTN);
    memset(&ifr, 0, sizeof(struct ifreq));

    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 1 - Get the version directory pattern
    strcpy(ifr.ifr_name, basename(dev->version_dir));
    if((fd=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        dev->access->error("net_read_info: socket call from ethtool access->error");
        return 0;
    }
    info->cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (char *)(info);
    if(ioctl(fd, SIOCETHTOOL, &ifr) != 0){
        dev->access->error("net_read_info: ioctl access->error");
        return 0;
    }
    close(fd);
    return 1;
}
static int 
eth_read_drv(struct pci_dev * dev, struct pci_class_methods *pcm, 
                    char *dr_v, int drv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.version)+1 > drv_size){
        dev->access->error("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(dr_v, info.version);
    return 1; 
}
static int 
eth_read_fwv(struct pci_dev * dev, struct pci_class_methods *pcm, 
                    char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.fw_version)+1 > fwv_size){
        dev->access->error("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(fw_v, info.fw_version);
    return 1; 
}
static int 
ib_read_drv(struct pci_dev * dev, struct pci_class_methods *pcm, char *dr_v, int drv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.version)+1 > drv_size){
        dev->access->error("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(dr_v, info.version);
    return 1; 
}
static int 
ib_read_fwv(struct pci_dev * dev, struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.fw_version)+1 > fwv_size){
        dev->access->error("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(fw_v, info.fw_version);
    return 1; 
}



/*=================== Serial bus controller(0x0c)===========*/
static int 
fc_read_drv(struct pci_dev *dev, struct pci_class_methods *pcm, char *dr_v, int drv_size){
    char drv_fpattn[DRV_FPATTN_MAX];
    // Step 0 - clear out the fpattn buffers
    memset(drv_fpattn, '\0', DRV_FPATTN_MAX);
    // Step 1 - set the pci attribute version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!set_pci_dev_drv_fpattn(dev, pcm, drv_fpattn, DRV_FPATTN_MAX)){
        return 0;
    }else if(!read_vfiles(dev->version_dir, drv_fpattn, "Driver", dr_v, drv_size)){
        return 0;
    }
    return 1;
}

static int 
fc_read_fwv(struct pci_dev *dev, struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    char fwv_fpattn[FWV_FPATTN_MAX];
    // Step 0 - clear out the local buffers
    memset(fwv_fpattn, '\0', DRV_FPATTN_MAX);
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!set_pci_dev_fwv_fpattn(dev, pcm, fwv_fpattn, FWV_FPATTN_MAX)){
        return 0;
    }else if(!read_vfiles(dev->version_dir, fwv_fpattn, "Firmware", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}





/*===================PCM defs===================================*/
/*===================0x01==========================*/
const struct pci_class_methods raid = {
    "RAID bus controller",
    RAID_RELPATH_VDIR_PATTN,
    #ifdef RAID_DRV_FILE_PATTNS
        raid_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef RAID_FWV_FILE_PATTNS
        raid_fwv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef RAID_READ_DRV
       raid_read_drv,
    #else 
       NULL,
    #endif
    #ifdef RAID_READ_FWV
       raid_read_fwv,
    #else 
       NULL,
    #endif
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
    #ifdef ATA_READ_DRV
       ata_read_drv,
    #else 
       NULL,
    #endif
    #ifdef ATA_READ_FWV
       ata_read_fwv,
    #else 
       NULL,
    #endif
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
    #ifdef SATA_READ_DRV
       sata_read_drv,
    #else 
       NULL,
    #endif
    #ifdef SATA_READ_FWV
       sata_read_fwv,
    #else 
       NULL,
    #endif
};
const struct pci_class_methods sas = {
    "Serial Attached SCSI controller",
    SAS_RELPATH_VDIR_PATTN,
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
    #ifdef SAS_READ_DRV
       sas_read_drv,
    #else 
       NULL,
    #endif
    #ifdef SAS_READ_FWV
       sas_read_fwv,
    #else 
       NULL,
    #endif
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
    #ifdef NVM_READ_DRV
       nvm_read_drv,
    #else 
       NULL,
    #endif
    #ifdef NVM_READ_FWV
       nvm_read_fwv,
    #else 
       NULL,
    #endif
};

/*========0x02(Network controllers)================*/

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

    #ifdef ETH_READ_DRV
       eth_read_drv,
    #else 
       NULL,
    #endif
    #ifdef ETH_READ_FWV
       eth_read_fwv,
    #else 
       NULL,
    #endif
};
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
    #ifdef IB_READ_DRV
       ib_read_drv,
    #else 
       NULL,
    #endif
    #ifdef IB_READ_FWV
       ib_read_fwv,
    #else 
       NULL,
    #endif
};

/*=================== Serial bus controller(0x0c)===========*/
 
const struct pci_class_methods fc = {
    "Fibre Channel",
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
    #ifdef FC_READ_DRV
       fc_read_drv,
    #else 
       NULL,
    #endif
    #ifdef FC_READ_FWV
       fc_read_fwv,
    #else 
       NULL,
    #endif
};

/*==============PCM Table ==================================*/
struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX] = {
    {NULL}, /* Unclassified devices */
    {NULL, NULL, NULL, NULL, &raid, &ata, &sata, &sas, &nvm}, /* Mass storage controllers */
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

