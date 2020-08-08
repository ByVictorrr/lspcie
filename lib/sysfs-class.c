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

/**
* Desc: Given a path (absolute or relative) it returns a string of \
*       the first directory in the path
* @param path - the string used to get the first directory in the path
* @param b_dirname - the buffer used to store the result of the return value
* @param size - size of the buffer (b_dirname)
* @return b_dirname if the operations were sucessful, NULL otherwise
*/
char* base_dirname(const char *path, char *b_dirname, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    int i=0;
    // Step 0 - check if either parm is NULL
    if(!path || !b_dirname){
        fprintf(stderr, "get_base_dir: both parameters need to be non NULL");
        return NULL;
    }
    // Step 1 - cpy data from path to buff (so we can use strtok)
    if(strlen(path)+1 > MAX_PATH){
        fprintf(stderr, "get_base_dir: *path arg is to large");
        return NULL;
    }
    strcpy(buff, path);
    memset(b_dirname, '\0', size);
    if((token = strtok(buff, "/"))!=NULL){
        strcpy(b_dirname, token);
    }else{
        strcpy(b_dirname, path);
    }
   return b_dirname; 
}
/**
* Desc: Given a path (absolute or relative) it returns a string of \
*       everything excluding the first folder
* @param path - the string used to get the first directory in the path
* @param n_relpath - the buffer used to store the result of the return value
* @param size - size of the buffer 
* @return n_relpath if there is a next path in path, NULL otherwise
*/
char *next_relpath(const char *path, char *n_relpath, int size){
    char *token;
    char buff[MAX_PATH] = {'\0'};
    int exists=0;
    // Step 0 - check if either parm is NULL
    if(!path || !n_relpath){
        fprintf(stderr, "next_relpath: both parameters need to be non NULL");
        return NULL;
    }
    // Step 1 - cpy data from path to buff (so we can use strtok)
    if(strlen(path)+1 > MAX_PATH){
        fprintf(stderr, "get_base_next: *path arg is to large");
        return 0;
    }
    strcpy(buff, path);
    memset(n_relpath, '\0', size);
    // Case 1 - No next path
    if((token = strtok(buff, "/"))==NULL){
        return NULL;
    }
    // Case 2 - next path (exists)
    for(token=strtok(NULL, "/"); token; strcat(n_relpath, "/")){
        strcat(n_relpath, token);
        token=strtok(NULL, "/");
        exists=1;
    }
    // To see if there exists another folder
    if(!exists){
        return NULL;
    }
    n_relpath[strlen(n_relpath)-1] = '\0';
    // need to get rid of the last "/"

    return n_relpath;
}
static DIR *
pci_opendir(const char *dir_name){
    DIR * dir;
    if(!(dir=opendir(dir_name))){
        fprintf(stderr, "Not able to open the directory: %s", dir_name)
        return NULL;
    }
    return dir;
}
/**
* @param d - pci_dev struct ptr used to open up its device folder
* @return a pointer to the base device folder (NULL if an error occured)
*/
static char *
get_pci_dev_dirname(struct pci_dev *d, char *dev_dirname){

    int n = snprintf(dev_dirname, MAX_PATH, "%s/devices/%04x:%02x:%02x.%d",
		   sysfs_name(d->access), d->domain, d->bus, d->dev, d->func);
    // Step 1 - check to see if buff is of correct size 
    if (n < 0 || n >= MAX_PATH){
        d->access->error("Folder name too long");
        return NULL;
    }
    // Step 2 - open that folder
    if(!(dir=pci_opendir(dev_dirname)){
        d->access->error("Not able to open directory: %s", dev_dirname);
        return NULL;
    }
    return dev_dirname;
}
/**
*@param folder - absolute path of the folder in which we want to look for a file
*@param file_pattern - the regex to find a like file
*@param file_bname - the returned file basename 
*@return 0 if file_pattern not found in the folder (file_bname not correct)
**/
static char *
find_pci_dev_vers_dir(char * cwd, char *path_pattn){
    // step 2 - check to see if something like host in the dir
    DIR *dir;
    struct dirent *dp;
    regex_t regex;
    #define BASE_DIRNAME_SIZE 100
    #define NEXT_RELPATH_SIZE 100
    char b_dirname[BASE_DIRNAME_SIZE];
    char n_relpath[NEXT_RELPATH_SIZE];
    char n_path[MAX_PATH] = {'\0'};

    // Should never return NULL
    if(!base_dirname(path_pattn, b_dirname, BASE_DIRNAME_SIZE))
        return NULL;
    // When there is no more to be parsed
    if (!next_dirname(path_pattn, n_relpath, NEXT_DIRNAME_SIZE)){
        strcat(cwd, "/");
        strcat(cwd, b_dirname);
        return cwd;
    }
    if (!(dir=pci_opendir(cwd)))
        return NULL;
   
    if (regcomp(&regex, b_dirname, 0)){
       fprintf(stderr, "Could not compile regex\n"); 
       return NULL;
    }
    // Step 3 - go through dir and try to find  (#TODO )
    while((dp=readdir(dir)) != NULL){
        // if entry is directory and not .. or .
        if (dp->d_type == DT_DIR && 
        (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))){
            if(!regexec(&regex, dp->d_name, 0, NULL, 0)){
                snprintf(n_path, sizeof(n_path), "%s/%s", cwd, dp->d_name);
                closedir(dir);
                return find_pci_dev_vers_dir(n_path, n_relpath)
            }
        }
    }
    fprintf(stderr, "Could not find a file pattern of %s in %s\n", file_pattern, folder); 
    return NULL;
}
static int 
get_pci_dev_drv_fpattn(struct pci_dev *d, const struct pci_class_methods *pcm, char *drv_fpattn_buff){
    
    char  *drv_file_pattn; 
    /*** Step 1 - check to see if the pcm exists */
    if (!pcm || !(pcm->drv_file_pattns)){
            d->warning("get_pci_dev_drv_fpattn: Your pcm or drv_file_pattns is NULL");
            return 0;
    }
    if(!(drv_file_pattn = pcm->drv_file_pattns[d->vendor_id])){
            d->warning("get_pci_dev_drv_fpattn: You have to added an entry for to the table in sysfs-class-sles.h");
            return 0;
    }
    strcpy(drv_fpattn_buff, drv_file_pattn);
    return 1;
 
}
static int 
get_pci_dev_fwv_fpattn(struct pci_dev *d, struct pci_class_methods *pcm, char *fwv_fpattn_buff){
    
    char *fwv_file_pattn; 
    /*** Step 1 - check to see if the pcm exists */
    if (!pcm || !(pcm->fwv_file_pattns)){
            d->warning("get_pci_dev_fwv_fpattn: Your pcm or fwv_file_pattns is NULL");
            return 0;
    }
    if(!(fwv_file_pattn = type->fwv_file_pattns[d->vendor_id])){
            d->warning("get_pci_dev_fwv_fpattn: You have to added an entry for to the table in sysfs-class-sles.h");
            return 0;
    }
    strcpy(fwv_fpattn_buff, fwv_file_pattn);
    return 1;
 
}



#define MAX_FPATTN 100
static DIR * 
get_pci_dev_vers_dir(struct pci_dev *d, struct pci_class_methods *type){
    char v_dir[MAX_PATH]={'\0'}, d_dir[MAX_PATH] = {'\0'};
    char relvpath[MAX_PATH] = {'\0'};
    char fwv_file_pattn[MAX_FPATTN], drv_file_pattn[MAX_FPATTN]; 
    DIR *dir;
    /*** Step 0 - get file pattns for fw and dr files ***/
    if(!get_pci_dev_drv_fpattn(d, type, drv_file_pattn)){
        return NULL;
    }
    if(!get_pci_dev_fwv_fpattn(d, type, fwv_file_pattn)){
        return NULL;
    }

    // Step 1 - get the pci device base folder
    if(!get_pci_dev_dirname(d, d_dir)){
        return NULL;
    }
    // Step 2 - go through each type and get the vers folder(abs path)
    if(!find_pci_dev_vers_dir(d_dir, rel_vpath_pattn)){
        return NULL;
    }
    // Else open file (d_dir is now vabs path)
    if(!(dir=pci_opendir(d_dir))){
        return NULL;
    }
    return dir;
}


/**
* Desc: read the v_dir and looks for matches with the fpattn
* @return files_buff size (-1 if something whent wrong)
*/
static int
get_vfiles(DIR *v_dir, const char *fpattn, FILE **files_buff){
    struct dirent *entry;
    regex_t regex;
    FILE *file;
    int i=0;
    // Step 1 - check params
    if(!v_dir){
        fprintf(stderr, "get_vfiles: the directory object is null");
        return -1;
    }else if (!fpattn){
        fprintf(stderr, "get_vfiles: the fpattn object is null");
        return -1;
    }else if (!files_buff){
        fprintf(stderr, "get_vfiles: files_buff is null");
        return -1;
    }
    // Step 2 - compose regex obj
    if(regcomp(&regex, fpattn, 0)){
        fprintf(stderr, "get_vfiles: regex compilation error");
        return 0;
    }

    // Step 2 - read v_dir
    while((entry = readdir(v_dir))){
        // if the regex matches the file name open the file
         if(!regexec(&regex, entry->d_name, 0, NULL, 0)){
            if((file=fopen(entry->d_name, "r")))
                files_buff[i++] = file; 
            else
                fprintf(stderr, "get_vfiles: unable to open for reading %s", entry->d_name);
         }
    }
    return i;
}

/**
* Desc: looks for string in vfile
* @return the line number the string is on (-1 if didnt find it)
*/
static int
line_string_in_vfile(const char *string, FILE *vfile){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_num = 0;  

    // Step 1 - start of a file
    rewind(vfile); 
    while((read = getline(&line, &len, fp)) != -1){
        if(strstr(string, line)){
            return line_num;
        }
        line_num++;
    }
    rewind(vfile); 
    return -1;

}
/* TODO : HOW DO WE DETERMINE TO LOOK FOR "FIRMWARE/VERSION" ?*/
static int
read_vfile(FILE *vfile, char *vbuff, char *string){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int is_empty = 0, str_linenum, linenum=0;  

    // Step 1 - search for in file the string "Firmware" or "Driver"
    // if no string firmware read the full file and store it into buffer
    str_linenum = line_string_in_vfile(string, vfile);
    // is "firmware or driver in the string"
    #define VERSION_SIZE 1024
    memset(vbuff, '\0', VERSION_SIZE);

    while((read = getline(&line, &len, fp)) != -1){
        // check to see if file is empty
        if(!read){
            is_empty=1;
        }
        if(str_linenum == -1){
            // dont look for "firmware or driver in vfile"
            strcat(vbuff, line);
            int n = snprintf(vbuff, VERSION_SIZE, "%s %s",vbuff, line);
        }else{
            // look for "firmware or driver in vfile"
            if(linenum==str_linenum){
                int n = snprintf(vbuff, VERSION_SIZE, "%s %s",vbuff, line);
            }

        }
        linenum++;
    }

    if(is_empty)
        return 0;

    return 1; 

}
static int
read_vfiles(DIR *v_dir, const char *fpattn, char *vbuff, const char *string){
    #define MAX_V_FILES 20
    FILE *vfiles[MAX_V_FILES], *vfile;
    int num_vfiles, i;
    char *fwv_buff;
    // error occured
    if((num_vfiles = get_vfiles(v_dir, fpattn, vfiles)) == -1){
        return -1;
    }else if(num_vfiles == 0){
        return 0;
    }
    // Can start reading 
    for(i=0; i< num_vfiles; i++){
        vfile = vfiles[i];
        if(!read_vfile(vfile, vbuff, string)){
            continue;
        }
        fclose(file);
     }
    return i+1;
}


/*==========================================================*/
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @return {1 on sucess, 0 on failure}
*/
int sas_read_versions(struct pci_dev *dev, char *dr_v, char *fw_v){
    FILE *fp;
    DIR *v_dir;
    struct pci_class_methods *pcm;
    int num_vfiles;
    u16 ven_id = dev->vendor_id;
    char *drv_fpattn, *fwv_fpattn;
    // step 0 - get pcm
    if (!(pcm = pcm_vers_map[get_class(dev)][get_subclass(dev)])){
        dev->warning("sas_read_version: sas (struct pci_class_methods) not instantiated");
        return  0;
    }
    // Step 1 - open version dir
    if(!(v_dir = get_pci_dev_vers_dir(dev, pcm)){
        dev->warning("sas_read_version: not able to find/open version directory");
        return 0;
    }
    // Step 2 - get pattns from pcm (use vendor_id )
    if(!(pcm->drv_file_pattns)){
        dev->warning("sas_read_version: sas (struct pcm) driver file pattn not installed");
        return 0;
    }
     if(!(pcm->fwv_file_pattns)){
        dev->warning("sas_read_version: sas (struct pcm) driver file pattn not installed");
        return 0;
    }
    // need only one of the pattns to get here (need logic to get her)
    if(!(drv_fpattn = pcm->drv_file_pattns[ven_id])){
        // throw error
    }
    if(!(fwv_fpattn = pcm->fwv_file_pattns[ven_id])){
        // throw error
    }

    // both of them are installed (need flag)
    if(read_files(v_dir, drv_fpattn, dr_v)){
        // condition based on return
    }
    if(read_files(v_dir, fwv_fpattn, fw_v)){
        // condition based on return
    }


    return 1;
}


   }

 

    // Step 3 - use file patterns to read files( first dr_v)
    if((num_vfiles = read_files(v_dir, const char *fpattn, fw_v))== -1 ){
        dev->warning("sas_read_version: error reading driver firmware files");
        return 0;
    }
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

