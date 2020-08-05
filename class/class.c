#include "class.h"
#include "lspci.h"
#include <dirent.h>
#include <regex.h>
#include <stdio.h>

#define MAX_PATH 1000
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @param dev - used for bus info
* @return char[2] - where [0] is driver version \
                    where [1] is firmware version
*/
char **sas_get_versions(struct pci_dev *dev){
    char buff[MAX_PATH] = {'\0'};
    DIR *dir;
    FILE *f;
    struct dirent *dp;
    regex_t regex;
    char *versions[2] = NULL;
    // step 1 - build path base path of device
    sysfs_name(d->access);
    int n = snprintf(buf,MAX_PATH,"%s/devices/%04x:%02x:%02x.%d/",
		   sysfs_name(d->access), d->domain, d->bus, d->dev, d->func);
    if (n < 0 || n >= OBJNAMELEN)
        d->access->error("File name too long");
    // step 2 - check to see if something like host in the dir
    if(!(dir=opendir(buf))){
        fprintf(stderr, "Not able to open directory %s", buf);
        return NULL;
    }
    if (regcomp(&regex, "^host*", 0)){
       fprintf(stderr, "Could not compile regex\n"); 
       return NULL;
    }
    // Step 3 - go through dir and try to find  (#TODO )
    while((dp=readdir(dir)) != NULL){
        // look for something like host
        if(!regexec(&regex, dp->d_name, 0, NULL, 0)){
            strcat(buff, dp->d_name);
            strcat(buff, "/scsi_host/");
            strcat(buff, dp->d_name);
        }

    }
    closedir(dir);
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
        versions[index++] = malloc(strlen(line)+1);
    }
    return versions;
}

struct pci_class_methods scsci{
    "SCSI storage controller",
    scsci_get_versions
};
struct pci_class_methods ide{
    "IDE interface",
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
struct pci_class_methods eth{
    "Ethernet controller",
    eth_get_versions
};
/*=========================================================*/



