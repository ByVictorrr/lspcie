#include "sysfs-class.h"
#include "internal.h"
#include "pread.h"
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "pci.h"



/*==========================================================*/
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @return to indicate what buffer is valid
* Assumptions: pcm isnt null (checked in the calling function)
*/
int sas_read_drv(struct pci_dev *dev, struct pci_class_methods *pcm,
                    char *dr_v, char *fw_v, int drv_size, int fwv_size){
    DIR *v_dir;
    struct pci_class_methods *pcm;
    int num_vfiles;
    #define DRV_FPATTN_MAX 100
    #define FWV_FPATTN_MAX 100
    char drv_fpattn[DRV_FPATTN_MAX], fwv_fpattn[FWV_FPATTN_MAX];
    // Step 0 - clear out the fpattn buffers
    memset(drv_fpattn, '\0', DRV_FPATTN_MAX);
    memset(fwv_fpattn, '\0', FWV_FPATTN_MAX);

    // Step 1 - open version dir
    if(!(v_dir = get_pci_dev_vers_dir(dev, pcm)){
        dev->warning("sas_read_version: not able to find/open version directory");
        return 0;
    }
    // Step 2 - get pattns from pcm (TODO )
    if(get_pci_dev_drv_fpattn(dev, pcm, drv_fpattn, DRV_FPATTN_MAX)){
        read_files(v_dir, drv_fpattn, "Driver", dr_v, drv_size);
    }
    if(get_pci_dev_fwv_fpattn(dev, pcm, fwv_fpattn, FWV_FPATTN_MAX)){
        read_files(v_dir, fwv_fpattn, "Firmware", fw_v, fwv_size);
    }
    closedir(v_dir);

    return 1;
}



 


int nvm_read_versions(struct pci_dev *dev, char *dr_v, char *fw_v){
    // Step 1 - get base folder
    char buff[MAX_PATH] = {'\0'};
    char nvme[MAX_PATH/2] = {'\0'};

    FILE *fp;
    // strcat(buff, "/firmware_rev");

    return 1;
}



/*=========================================================*/


/*========Class = 0x02(Network controllers)================*/
#include <linux/ethtool.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/sockios.h>

#define MAX_PATTN 100
/**
*
*/
int 
net_read_versions(struct pci_dev *dev, struct *char *dr_v, char *fw_v){
    /* TODO : (use for ib too) 
        step 1 : go to /sys/bus/devices/pciaddr/net
        step 2 : read for eth*
        step 3 : get buffer do below with that
    */
    char vdir_pattn[MAX_PATTN]={'\0'};
    char devdir_path[MAX_PATH] = {'\0'}
    char *vdir_path;

    char iface[MAX_PATH/2] = {'\0'};
    int fd = -1;
    struct ifreq ifr;
    struct ethtool_drvinfo info;
    struct pci_class_methods *pcm;
    u16 ven_id = dev->vendor_id;
    memset(&info, 0, sizeof(struct ethtool_drvinfo));
    memset(&ifr, 0, sizeof(struct ifreq));

    // Step 0 - get the pcm
    if (!(pcm = pcm_vers_map[get_class(dev)][get_subclass(dev)])){
        dev->warning("sas_read_version: sas (struct pci_class_methods) not instantiated");
        return  -1;
    }
    // Step 1 - get the dev directory
    get_pci_dev_dirname(dev, devdir_path);

    // Step 1 - Get the version directory pattern
    /* Returns 1 if sucess otherwise 0 */
    get_pci_dev_vdir_pattn(dev, pcm, vdir_patt, MAX_PATTN);

    // Step 2 - Get the name of the version folder (returns the name of the vers dir)
    vdir_path = find_pci_dev_vers_dir(devdir_path, vdir_patt);

    strcpy(ifr.ifr_name, basename(vdir_path));
    if((fd=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        fprintf(stderr, "Socket call to obtain ethtool information failed : %s", strerror(errno));
        return 0;
    }
    info.cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (char *)(&info);
    if(ioctl(fd, SIOCETHTOOL, &ifr) != 0){
        return 0;
    }
    strcpy(dr_v, info.version);
    strcpy(fw_v, info.fw_version);
    return 1;
}

int eth_read_versions(struct pci_dev * dev, char *dr_v, char *fw_v){

    return net_read_versions(dev, "eth*", dr_v, fw_v);
}
/*
int tr_read_versions(struct pci_dev * dev, char *dr_v, char *fw_v){
    return net_read_versions(dev, "tr*", dr_v, fw_v);
}
int fddi_read_versions(struct pci_dev dev, char *dr_v, char *fw_v){
    return net_read_versions(dev, "fddi*", dr_v, fw_v);
}
*/

int ib_read_versions(struct pci_dev * dev, char *dr_v, char *fw_v){
    return net_read_versions(dev, "ib*", dr_v, fw_v);
}

/*====================*/



/*=================== Serial bus controller(0x0c)===========*/

int fc_read_versions(struct pci_dev * dev, char *dr_v, char *fw_v){
    return 0;
}


/*=========================================================*/
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

