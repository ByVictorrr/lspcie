#include "class.h"
#include "lspci.h"
#include <dirent.h>
#include <regex.h>
#include <stdio.h>

#define DRIVER_VERSION 0
#define FIRMWARE_VERSION 1
/*=============Utility functions===============================*/
#define MAX_PATH 1024


static int
get_dev_folder(struct pci_dev *d, char *buf, char * object)
{
  int n = snprintf(buf, MAX_PATH, "%s/devices/%04x:%02x:%02x.%d/",
		   sysfs_name(d->access), d->domain, d->bus, d->dev, d->func);
  if (n < 0 || n >= MAX_PATH){
    d->access->error("File name too long");
    return 0;
  }
  if (object != NULL && strlen(object)+strlen(buff)+1 <= MAX_PATH){
      // check return value
      strcat(buff, object);
  }

   return 1;
}
/* 
*@param folder - absolute path of the folder in which we want to look for a file
*@param file_pattern - the regex to find a like file
*@param file_bname - the returned file basename 
*@return 0 if file_pattern not found in the folder (file_bname not correct)
*/
static int 
find_file_in_folder(const char *folder, const char *file_pattern, char *file_bname){
    // step 2 - check to see if something like host in the dir
    DIR *dir;
    struct dirent dp;
    regex_t regex;
    if(!(dir=opendir(buf))){
        fprintf(stderr, "Not able to open directory %s", folder);
        return NULL;
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
* @param dev - used for bus info
* @return char[2] - where [0] is driver version \
                    where [1] is firmware version
*/
char **sas_get_versions(struct pci_dev *dev){
    char buff[MAX_PATH] = {'\0'};
    char host[MAX_PATH/2] = {'\0'};
    FILE *f;
    regex_t regex;
    char *versions[2] = NULL;
    // step 1 - build path base path of device
    if(!get_dev_folder(dev, buff, NULL))
        return NULL;

    // Step 2 - read folder (buff)
    if (!find_file_in_folder(buff, "host*", host)){
       return NULL;
    }

    /* TODO check buff size */
    strcat(buff, host);
    strcat(buff, "/scsi_host/");
    strcat(buff, host);
    strcat(buff, "/version");

    // step 4 - open up file version if doesnt exists then return null
    if(!(fp=fopen(buff, "r"))){
        fprintf(stderr, "Not able to open file %s", buf);
        return NULL;
    }
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int index = 0;  
    while((read = getline(&line, &len, fp)) != -1){
        // validate
        versions[index++] = strdup(line);
    }
    fclose(fp);
    return versions;
}
char **raid_get_versions(struct pci_dev *dev){
    
}

struct pci_class_methods scsi{
    "SCSI storage controller",
    scsi_get_versions
};
struct pci_class_methods ide{
    "IDE iface",
    ide_get_versions
};
struct pci_class_methods floppy{
    "Floppy disk controller",
    floppy_get_versions
};
struct pci_class_methods ipi{
    "IPI bus controller",
    ipi_get_versions
};
struct pci_class_methods raid{
    "RAID bus controller",
    raid_get_versions
};
struct pci_class_methods ata{
    "ATA controller",
    ata_get_versions
};
struct pci_class_methods sata{
    "SATA controller",
    sata_get_versions
};
struct pci_class_methods sas{
    "Serial Attached SCSI controller",
    sas_get_versions
};





/*=========================================================*/


/*========Class = 0x02(Network controllers)================*/
#include <linux/ethtool.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/sockios.h>
char **net_get_versions(struct pci_dev * dev, char *iface_pattern){
    /* TODO : (use for ib too) 
        step 1 : go to /sys/bus/devices/pciaddr/net
        step 2 : read for eth*
        step 3 : get buffer do below with that
    */
    char buff[MAX_PATH]={'\0'};
    char iface[MAX_PATH/2] = {'\0'};
    char *versions[2];
    char *fw_v, *dr_v;
    int fd = -1;
    struct ifreq ifr;
    struct ethtool_drvinfo info;
    memset(&info, 0, sizeof(struct ethtool_drvinfo));
    memset(&ifr, 0, sizeof(struct ifreq));

    // step 1 - build path base path of device
    if(!get_dev_folder(dev, buff, "net"))
        return NULL;
    // step 2 - check to see if something like host in the dir
    // TODO : make this function avaiable for ib (so make eth* a parm)
    if (!find_file_in_folder(buff, iface_pattern, iface)){
       return NULL;
    }

    strcpy(ifr.ifr_name, iface);
    if((fd=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        fprintf(stderr, "Socket call to obtain ethtool information failed : %s", strerror(errno));
        return NULL;
    }
    dest->cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (char *)(&info);
    if(ioctl(fd, SIOCETHTOOL, &ifr) != 0){
        return NULL;
    }
    versions[DRIVER_VERSION]= strdup(info.version);
    versions[FIRMWARE_VERSION]= strdup(info.fw_version);

    return versions;
}

char **eth_get_versions(struct pci_dev * dev){
    return net_get_versions(dev, "eth*");
}
char **tr_get_versions(struct pci_dev * dev){
    return net_get_versions(dev, "tr*");
}
char **fddi_get_versions(struct pci_dev dev){
    return net_get_versions(dev, "fddi*");
}

struct pci_class_methods eth{
    "Ethernet controller",
    eth_get_versions
};
struct pci_class_methods tr{
    "Token ring network controller",
    tr_get_versions
}
struct pci_class_methods fddi{
    "FDDI network controller",
    fddi_get_versions
}
/*=========================================================*/



