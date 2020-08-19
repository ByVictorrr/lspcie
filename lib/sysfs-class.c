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
msc_set_drv(struct pci_dev *dev, char *dr_v, int drv_size){
    if(!set_pci_dev_drvdir(dev)){
        return 0;
    }else if(!read_drvfiles(dev->drvdir_path, PCI_DRV_FPATTNS, "driver:")){
        return 0;
    }
    return 1;
}


static int 
msc_set_fwv(struct pci_dev *dev){
    if(!set_pci_dev_fwvdir(dev)){
        return 0;
    }else if(!read_vfiles(dev->fwvdir_path, PCI_FWV_FPATTNS, "firmware:", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}
static int 
msc_set_optv(struct pci_dev *dev){
    if(!set_pci_dev_optvdir(dev)){
        return 0;
    }else if(!read_vfiles(dev->optdir_path, PCI_FWV_FPATTNS, "firmware:", fw_v, fwv_size)){
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
net_set_info(struct pci_dev *dev, struct ethtool_drvinfo *info){
    struct ifreq ifr;
    int fd;
    // Step 1 - set pci->version_dir
    if(!set_pci_dev_drvdir(dev)){
        return 0;
    }
    // Step 1 - Get the version directory pattern
    strcpy(ifr.ifr_name, basename(dev->drvdir_path));
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
nc_set_drv(struct pci_dev * dev){
    struct ethtool_drvinfo info;
    struct version_item *drvitem = dev->vitem[DRV_ITEM];
    if(!net_set_info(dev, &info)){
        return 0;
    }
    if(!(drvitem=malloc(sizeof(struct version_item)))){
        fprintf("nc_set_drv: malloc error");
        return 0;
    }
    drvitem->src_name = strdup("Ethtool");
    drvitem->data = strdup(info.version);
    return 1; 
}

static int 
nc_read_fwv(struct pci_dev * dev, char *fw_v, int fwv_size){
    struct ethtool_drvinfo info;
    struct version_item *fwvitem = dev->vitem[FWV_ITEM];
    if(!net_read_info(dev, &info)){
        return 0;
    }
    if(!(fwvitem=malloc(sizeof(struct version_item)))){
        fprintf("nc_set_fwv: malloc error");
        return 0;
    }
    fwvitem->src_name = strdup("Ethtool");
    fwvitem->data = strdup(info.fw_version);
    return 1; 
}
 
/*===================Display controller(0x03)==============*/
static int 
dc_read_drv(struct pci_dev *dev, char *dr_v, int drv_size){
    // Step 1 - set directory
    if(!set_pci_dev_drvdir(dev)){
        return 0;
    // Step 2 - get drv files
    }else if(!read_vfiles(dev->drvdir_path, PCI_DRV_FPATTNS, "driver:", dr_v, drv_size)){
        return 0;
    }
    return 1;
}


static int 
dc_read_fwv(struct pci_dev *dev, char *fw_v, int fwv_size){
    if(!set_pci_dev_fwvdir(dev)){
        return 0;
    }else if(!read_vfiles(dev->fwvdir_path, PCI_FWV_FPATTNS, "firmware:", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}



/*=================== Serial bus controller(0x0c)===========*/
static int 
sbc_read_drv(struct pci_dev *dev, char *dr_v, int drv_size){
    if(!set_pci_dev_drvdir(dev)){
        return 0;
    }else if(!read_vfiles(dev->drvdir_path, PCI_DRV_FPATTNS, "driver:", dr_v, drv_size)){
        return 0;
    }
    return 1;
}


static int 
sbc_read_fwv(struct pci_dev *dev, char *fw_v, int fwv_size){
    if(!set_pci_dev_fwvdir(dev)){
        return 0;
    }else if(!read_vfiles(dev->fwvdir_path, PCI_FWV_FPATTNS, "firmware:", fw_v, fwv_size)){
        return 0;
    }
    return 1;
}



/*===================PCM defs===================================*/
/*===================mass storage controllers 0x01==========================*/
const struct pci_class_methods msc = {
    "Mass storage controller",
    #ifdef MSC_READ_DRV
       msc_read_drv,
    #else 
       NULL,
    #endif
    #ifdef MSC_READ_FWV
       msc_read_fwv,
    #else 
       NULL,
    #endif
};
/*========0x02(Network controllers)================*/

const struct pci_class_methods nc = {
    "Network controller",
    #ifdef NET_READ_DRV
       nc_read_drv,
    #else 
       NULL,
    #endif
    #ifdef NET_READ_FWV
       nc_read_fwv,
    #else 
       NULL,
    #endif
};

/*=================== Display controller(0x03)===========*/
const struct pci_class_methods dc = {
    "Display controller",
    #ifdef DC_READ_DRV
       dc_read_drv,
    #else 
       NULL,
    #endif
    #ifdef DC_READ_FWV
       dc_read_fwv,
    #else 
       NULL,
    #endif
};


/*=================== Serial bus controller(0x0c)===========*/
 const struct pci_class_methods sbc = {
    "Serial bus controller",
    #ifdef SBC_READ_DRV
       sbc_read_drv,
    #else 
       NULL,
    #endif
    #ifdef SBC_READ_FWV
       sbc_read_fwv,
    #else 
       NULL,
    #endif
};
 

/*==============PCM Table ==================================*/
const struct pci_class_methods *pcm_vers_map[PCI_CLASS_MAX] = {
    NULL,  /* Unclassified device */
    &msc,  /* Mass storage controller */
    &nc,  /* Network controller */
    &dc,  /* Display controller */
    NULL, /* Multimedia controller */
    NULL, /* Memory controller */
    NULL, /* Bridge */
    NULL, /* Communication controller */
    NULL, /* Generic system peripherial */
    NULL, /* Input device controller */
    NULL, /* Docking station */
    NULL, /* Processor */
    &sbc, /* Serial bus controller */
    NULL, /* Wireless controller */
	NULL, /* Intelligent controller */
	NULL, /* Satellite communications controller */
	NULL, /* Encryption controller */
	NULL, /* Signal processing controller */
	NULL, /* Processing accelerators */
	NULL /* Non-Essential Instrumentation */
};

