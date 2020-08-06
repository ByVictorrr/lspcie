#include "sysfs-class.h"
#include "internal.h"
#include "pread.h"
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "pci.h"


/*=============Utility functions===============================*/
inline u8 get_class(struct pci_dev * d){
     return (d->device_class & PCI_CLASS_MASK) >> 8;
}
inline u8 get_subclass(struct pci_dev *d){
     return (d->device_class & PCI_SUBCLASS_MASK);
}

static int
get_dev_folder(struct pci_dev *d, char *buff, char * object)
{
  int n = snprintf(buff, MAX_PATH, "%s/devices/%04x:%02x:%02x.%d/",
		   sysfs_name(d->access), d->domain, d->bus, d->dev, d->func);
  if (n < 0 || n >= MAX_PATH){
    d->access->error("File name too long");
    return 0;
  }
  if (object != NULL && strlen(object)+strlen(buff)+1 <= MAX_PATH){
      // check return value
      strcat(buff, object);
      strcat(buff, "/");
  }

   return 1;
}
/**
*@param folder - absolute path of the folder in which we want to look for a file
*@param file_pattern - the regex to find a like file
*@param file_bname - the returned file basename 
*@return 0 if file_pattern not found in the folder (file_bname not correct)
**/
static int 
find_file_in_folder(const char *folder, const char *file_pattern, char *file_bname){
    // step 2 - check to see if something like host in the dir
    DIR *dir;
    struct dirent *dp;
    regex_t regex;
    if(!(dir=opendir(folder))){
        fprintf(stderr, "Not able to open directory %s", folder);
        return 0;
    }
    if (regcomp(&regex, file_pattern, 0)){
       fprintf(stderr, "Could not compile regex\n"); 
       return 0;
    }
    // Step 3 - go through dir and try to find  (#TODO )
    while((dp=readdir(dir)) != NULL){
        // look for something like host
        if(!regexec(&regex, dp->d_name, 0, NULL, 0)){
            strcpy(file_bname, dp->d_name);
            closedir(dir);
            return 1;
        }

    }
    fprintf(stderr, "Could not find a file pattern of %s in %s\n", file_pattern, folder); 
    return 0;
}
/*==========================================================*/
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @return {1 on sucess, 0 on failure}
*/
int sas_read_versions(struct pci_dev *dev, char *dr_v, char *fw_v){
    char buff[MAX_PATH] = {'\0'};
    char host[MAX_PATH/2] = {'\0'};
    FILE *fp;
    // step 1 - build path base path of device
    if(!get_dev_folder(dev, buff, NULL))
        return 0;

    // Step 2 - read folder (buff)
    if (!find_file_in_folder(buff, "host*", host))
       return 0;

    /* TODO check buff size */
    strcat(buff, host);
    strcat(buff, "/scsi_host/");
    strcat(buff, host);
    strcat(buff, "/version");

    // step 4 - open up file version if doesnt exists then return null
    if(!(fp=fopen(buff, "r"))){
        fprintf(stderr, "Not able to open file %s", buff);
        return 0;
    }
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int index = 0;  
    while((read = getline(&line, &len, fp)) != -1){
        if (index == 0)
            strcpy(dr_v, line);
        else if(index == 1)
            strcpy(fw_v, line);

        index++;
    }
    fclose(fp);
    return 1;
}
int nvm_read_versions(struct pci_dev *dev, char *dr_v, char *fw_v){
    // Step 1 - get base folder
    char buff[MAX_PATH] = {'\0'};
    char nvme[MAX_PATH/2] = {'\0'};

    FILE *fp;
    // step 1 - build path base path of device
    if(!get_dev_folder(dev, buff, "nvme"))
        return 0;

    // Step 2 - read folder (buff)
    if (!find_file_in_folder(buff, "nvme*",nvme))
       return 0;

    strcat(buff, nvme);
    strcat(buff, "/firmware_rev");

    // step 4 - open up file version if doesnt exists then return null
    if(!(fp=fopen(buff, "r"))){
        fprintf(stderr, "Not able to open file %s", buff);
        return 0;
    }
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int index = 0;  
    getline(&line, &len, fp);
    strcpy(fw_v, line);
    // memset(dr_v, "")
    fclose(fp);
    return 1;
}

struct pci_class_methods scsi = {
    "SCSI storage controller",
     NULL // scsi_read_versions
};
struct pci_class_methods ide = {
    "IDE iface",
    NULL // ide_read_versions
};
struct pci_class_methods floppy = {
    "Floppy disk controller",
    NULL //floppy_read_versions
};
struct pci_class_methods ipi = {
    "IPI bus controller",
    NULL //ipi_read_versions
};
struct pci_class_methods raid = {
    "RAID bus controller",
    NULL //raid_read_versions
};
struct pci_class_methods ata = {
    "ATA controller",
    NULL // ata_read_versions
};
struct pci_class_methods sata = {
    "SATA controller",
    NULL //sata_read_versions
};
struct pci_class_methods sas = {
    "Serial Attached SCSI controller",
    sas_read_versions
};
struct pci_class_methods nvm = {
    "Non-Volatile memory controller",
    nvm_read_versions
};




/*=========================================================*/


/*========Class = 0x02(Network controllers)================*/
#include <linux/ethtool.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/sockios.h>
/**
*
*/
int 
net_read_versions(struct pci_dev *dev, const char *iface_patrn,
                  char *dr_v, char *fw_v){
    /* TODO : (use for ib too) 
        step 1 : go to /sys/bus/devices/pciaddr/net
        step 2 : read for eth*
        step 3 : get buffer do below with that
    */
    char buff[MAX_PATH]={'\0'};
    char iface[MAX_PATH/2] = {'\0'};
    int fd = -1;
    struct ifreq ifr;
    struct ethtool_drvinfo info;
    memset(&info, 0, sizeof(struct ethtool_drvinfo));
    memset(&ifr, 0, sizeof(struct ifreq));

    // step 1 - build path base path of device
    if(!get_dev_folder(dev, buff, "net"))
        return 0;

    // step 2 - check to see if something like host in the dir
    if (!find_file_in_folder(buff, iface_patrn, iface)){
       return 0;
    }

    strcpy(ifr.ifr_name, iface);
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

struct pci_class_methods eth = {
    "Ethernet controller",
    eth_read_versions
};
/*
struct pci_class_methods tr = {
    "Token ring network controller",
    tr_read_versions
};
struct pci_class_methods fddi = {
    "FDDI network controller",
    fddi_read_versions
};
*/
struct pci_class_methods ib = {
    "Infiniband controller",
    ib_read_versions
};

/*=========================================================*/



/*=================== Serial bus controller(0x0c)===========*/

int fc_read_versions(struct pci_dev * dev, char *dr_v, char *fw_v){
    return 0;
}

struct pci_class_methods fc = {
    "Infiniband controller",
    fc_read_versions
};



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

