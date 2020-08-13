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


/*==========================================================*/
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @return to indicate what buffer is valid
* Assumptions: pcm isnt null (checked in the calling function)
*/
static int 
sas_read_drv(struct pci_dev *dev, const struct pci_class_methods *pcm, char *dr_v, int drv_size){
    const char *drv_fpattn;
    // Step 1 - set the pci attribute version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        dev->access->warning("sas_read_drv: not able to find/open version directory");
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!(drv_fpattn=get_pci_dev_drv_fpattn(dev, pcm))){
        return 0;
    }else if(!read_vfiles(dev->version_dir, drv_fpattn, "driver:", dr_v, drv_size)){
        return 0;
    }
    return 1;
}

static int 
sas_read_fwv(struct pci_dev *dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    const char *fwv_fpattn;
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!(fwv_fpattn=get_pci_dev_fwv_fpattn(dev, pcm))){
        return 0;
    }else if(!read_vfiles(dev->version_dir, fwv_fpattn, "firmware:", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}



static int 
nvm_read_fwv(struct pci_dev *dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    const char *fwv_fpattn;
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!(fwv_fpattn=get_pci_dev_fwv_fpattn(dev, pcm))){
        return 0;
    }else if(!read_vfiles(dev->version_dir, fwv_fpattn, "firmware:", fw_v, fwv_size)){
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
net_read_info(struct pci_dev *dev, const struct pci_class_methods *pcm, struct ethtool_drvinfo *info){
    struct ifreq ifr;
    int fd;
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 1 - Get the version directory pattern
    strcpy(ifr.ifr_name, basename(dev->version_dir));
    if((fd=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        dev->access->warning("net_read_info: socket call from ethtool access->warning");
        return 0;
    }
    info->cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (char *)(info);
    if(ioctl(fd, SIOCETHTOOL, &ifr) != 0){
        dev->access->warning("net_read_info: ioctl access->warning");
        return 0;
    }
    close(fd);
    return 1;
}
static int
net_read_drv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *dr_v, int drv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.version)+1 > drv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(dr_v, info.version);
    return 1; 
}

static int 
net_read_fwv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.fw_version)+1 > fwv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(fw_v, info.fw_version);
    return 1; 

}
 
/*
static int 
eth_read_drv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *dr_v, int drv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.version)+1 > drv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(dr_v, info.version);
    return 1; 
}
static int 
eth_read_fwv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.fw_version)+1 > fwv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(fw_v, info.fw_version);
    return 1; 
}
static int 
ib_read_drv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *dr_v, int drv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.version)+1 > drv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(dr_v, info.version);
    return 1; 
}
static int 
ib_read_fwv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.fw_version)+1 > fwv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(fw_v, info.fw_version);
    return 1; 
}

static int 
fab_read_drv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *dr_v, int drv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.version)+1 > drv_size){
        dev->access->warning("eth_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(dr_v, info.version);
    return 1; 
}
static int 
fab_read_fwv(struct pci_dev * dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    if(!net_read_info(dev, pcm, &info)){
        return 0;
    }
    if(strlen(info.fw_version)+1 > fwv_size){
        dev->access->warning("fab_read_drv: buffer not big enough to store info");
        return 0;
    }
    strcpy(fw_v, info.fw_version);
    return 1; 
}
*/

/*=================== Serial bus controller(0x0c)===========*/
static int 
fc_read_drv(struct pci_dev *dev, const struct pci_class_methods *pcm, char *dr_v, int drv_size){
    const char *drv_fpattn;
    // Step 1 - set the pci attribute version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!(drv_fpattn = get_pci_dev_drv_fpattn(dev, pcm))){
        return 0;
    }else if(!read_vfiles(dev->version_dir, drv_fpattn, "driver:", dr_v, drv_size)){
        return 0;
    }
    return 1;
}

static int 
fc_read_fwv(struct pci_dev *dev, const struct pci_class_methods *pcm, char *fw_v, int fwv_size){
    const char *fwv_fpattn;
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_vers_dir(dev, pcm)){
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(!(fwv_fpattn=get_pci_dev_fwv_fpattn(dev, pcm))){
        return 0;
    }else if(!read_vfiles(dev->version_dir, fwv_fpattn, "firmware:", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}


/*===================PCM defs===================================*/
/*===================0x01==========================*/
const struct pci_class_methods raid = {
    "RAID bus controller",
    RAID_RELPATH_VDIR_PATTN,
    #ifdef RAID_VFILE_PATTNS
        raid_vfile_pattns,
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
    #ifdef ATA_VFILE_PATTNS
        ata_vfile_pattns,
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
    #ifdef SATA_VFILE_PATTNS
        sata_vfile_pattns,
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
    #ifdef SAS_VFILE_PATTNS
        sas_vfile_pattns,
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
    #ifdef NVM_VFILE_PATTNS
        nvm_vfile_pattns,
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
    #ifdef ETH_VFILE_PATTNS
        eth_vfile_pattns,
    #else 
        NULL,
    #endif
    #ifdef ETH_READ_DRV
       net_read_drv,
    #else 
       NULL,
    #endif
    #ifdef ETH_READ_FWV
       net_read_fwv,
    #else 
       NULL,
    #endif
};
const struct pci_class_methods ib = {
    "Infiniband controller",
    IB_RELPATH_VDIR_PATTN,
    #ifdef IB_VFILE_PATTNS
        net_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef IB_READ_DRV
       net_read_drv,
    #else 
       NULL,
    #endif
    #ifdef IB_READ_FWV
       net_read_fwv,
    #else 
       NULL,
    #endif
};
const struct pci_class_methods fab = {
    "Fabric controller",
    FAB_RELPATH_VDIR_PATTN,
    #ifdef FAB_VFILE_PATTNS
        fab_drv_file_pattns,
    #else 
        NULL,
    #endif
    #ifdef FAB_READ_DRV
       net_read_drv,
    #else 
       NULL,
    #endif
    #ifdef FAB_READ_FWV
       net_read_fwv,
    #else 
       NULL,
    #endif
};

/*=================== Serial bus controller(0x0c)===========*/
 
const struct pci_class_methods fc = {
    "Fibre Channel",
    FC_RELPATH_VDIR_PATTN,
    #ifdef FC_VFILE_PATTNS
        fc_vfile_pattns,
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
const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX][PCI_SCLASS_MAX] = {
    {NULL}, /* Unclassified devices */
    {NULL, NULL, NULL, NULL, &raid, &ata, &sata, &sas, &nvm}, /* Mass storage controllers */
    {&eth, NULL, NULL, NULL, NULL, NULL, NULL, &ib, &fab},
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

