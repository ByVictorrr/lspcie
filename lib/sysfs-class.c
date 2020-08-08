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

/*===================read_vfile functions =============================================*/
/**
* Desc: Given the vendor id from the pci_dev, this function uses the pcm attribute \
*       to lookup the driver version file pattrn and stores that regex into the drv_fpattn_buff
*
*@param d : the pci device that we are using to look up the vendor id
*@param pcm : the pcm of that particular class & subclass of that device \
*             using it to get the driver file pattns using its lookup table
*@param drv_fpattn_buff: The buffer that is used to store the drv_fpattn string 
*@param buff_size: The max size of the drv_fpattn_buff
*@return an integer 1 if everything was sucessful in getting your drv_pattn stored in the buff \
*           integer 0 otherwise
**/
static int 
get_pci_dev_drv_fpattn(struct pci_dev *d, const struct pci_class_methods *pcm, char *drv_fpattn_buff, int buff_size){
    char  *drv_file_pattn; 
    /*** Step 1 - check to see if the pcm exists */
    if (!pcm){
        d->warning("get_pci_dev_drv_fpattn: Your pci_class_methods structure isn't defined");
        return 0;
    }else if (!(pcm->drv_file_pattns)){
        d->warning("get_pci_dev_drv_fpattn: the drv_file_pattns table needs to be added to your pcm structure");
        return 0;
    }else if(!(drv_file_pattn = pcm->drv_file_pattns[d->vendor_id])){
        d->warning("get_pci_dev_drv_fpattn: You need to add a pattern in the drv_file_pattns for that vendor and device in sysfs-class-sles.h");
        return 0;
    }else if(stlen(drv_file_pattn)+1 > buff_size){
        d->warning("get_pci_dev_drv_fpattn: the drv_fpattn string is to long to store inside the buffer");
        return 0;
    }

    strcpy(drv_fpattn_buff, drv_file_pattn);
    return 1; 
}
/**
* Desc: Given the vendor id from the pci_dev, this function uses the pcm attribute \
       to lookup the fw version file pattrn and stores that regex into the fwv_fpattn_buff

*@param d : the pci device that we are using to look up the vendor id

*@param pcm : the pcm of that particular class & subclass of that device \
              using it to get the driver file pattns using its lookup table

*@param fw_fpattn_buff: The buffer that is used to store the fwv_fpattn string 

*@param buff_size: The max size of the fw_fpattn_buff

*@return an integer 1 if everything was sucessful in getting your fwv_pattn stored in the buff \
            integer 0 otherwise
**/
static int 
get_pci_dev_fwv_fpattn(struct pci_dev *d, struct pci_class_methods *pcm, char *fwv_fpattn_buff, int buff_size){
    
    char *fwv_file_pattn; 
    if (!pcm){
        d->warning("get_pci_dev_fwv_fpattn: Your pci_class_methods structure isn't defined");
        return 0;
    }else if (!(pcm->fwv_file_pattns)){
        d->warning("get_pci_dev_fwv_fpattn: the fwv_file_pattns table needs to be added to your pcm structure");
        return 0;
    }else if(!(fwv_file_pattn = pcm->fwv_file_pattns[d->vendor_id])){
        d->warning("get_pci_dev_fwv_fpattn: You need to add a pattern in the fwv_file_pattns for that vendor and device in sysfs-class-sles.h");
        return 0;
    }else if(stlen(fwv_file_pattn)+1 > buff_size){
        d->warning("get_pci_dev_fwv_fpattn: the fwv_fpattn string is to long to store inside the buffer");
        return 0;
    }
    strcpy(fwv_fpattn_buff, fwv_file_pattn);
    return 1; 
}


/**
* Desc: Reads the version directory object and looks for \
*       a matches with the regex fpattn

* @param v_dir : the version directory that is used to look \
                 through for vfiles matching the fpattn

* @param fpattn : the regex to get the version files 

* @param files_buff : Stores a list of files that match the \
                     fpattn in the v_dir
                    
* @param buff_size : The max size of the buffer

* @return an int that denotes how many files were stored in files_buff \n 
         -1 represents something whent wrong 
          0 represents buffer is to small
*/
static int
get_vfiles(const DIR *v_dir, const char *fpattn, FILE **files_buff, int buff_size){
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
    }else if(regcomp(&regex, fpattn, 0)){ /* Compile the regex */
        fprintf(stderr, "get_vfiles: regex compilation error");
        return -1;
    }
    // Step 2 - read v_dir and look for matches of the regex
    while((entry = readdir(v_dir))){
        /* if the regex matches the file name open the file*/
         if(!regexec(&regex, entry->d_name, 0, NULL, 0)){
            if((file=fopen(entry->d_name, "r"))){
                if(i > buff_size-1){
                    fprintf(stderr, "get_vfiles: Make your files_buff max size bigger");
                    return i+1;
                }
                files_buff[i++] = file; 
            }else
                fprintf(stderr, "get_vfiles: Unable to open for reading %s", entry->d_name);
         }
    }
    return i+1;
}

/**
* Desc: looks for string in vfile (also it rewinds the vfile to start of file)

* @param string : a string that is used to find in a file

* @param vfile : a version file 

* @return The line number the string is on or -1 if it doesnt exist

**/
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
            rewind(vfile); 
            return line_num;
        }
        line_num++;
    }
    rewind(vfile); 
    return -1;

}
/* TODO : HOW DO WE DETERMINE TO LOOK FOR "FIRMWARE/VERSION" ?*/
/**
* Desc: This function reads the version file and stores the info into vbuff

* @param vfile: The file ponter used to read the version file

* @param string: The string to look for inside a file to get the data on that line

* @param vbuff: Where we store the data

* @param buff_size: The max size the buff can hold

* @return an int 1 if we correctly extracted the data from the file into the buffer \
             int 0 if something went wrong like the buffer isnt big enough to hold the information
**/
static int
read_vfile(FILE *vfile, char * string, char *vbuff, int buff_size){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int is_empty = 0, str_linenum, linenum=0;  

    // if no string firmware read the full file and store it into buffer
    // Step 1 - Search for in file the string = {"Firmware" or "Driver"}
    str_linenum = line_string_in_vfile(string, vfile);
    while((read = getline(&line, &len, fp)) != -1){
        int n;
        if(str_linenum == -1)
            // Read every line in the file and concat it into the buffer
            if((n = snprintf(vbuff, buff_size, "%s %s",vbuff, line)) < 0 || n>= buff_size){
                fprintf(stderr, "read_vfile: the data in the file is to long for destination buffer");
                return 0;
            }
        else
            // look for "firmware or driver in vfile"
            if(linenum==str_linenum)
                if((n = snprintf(vbuff, buff_size, "%s %s",vbuff, line)) < 0 || n>= buff_size){
                    fprintf(stderr, "read_vfile: the data in the file is to long for destination buffer");
                    return 0;
                }
        linenum++; 
        }
    return 1; 
}

#define MAX_VFILES 20
static int
read_vfiles(DIR *v_dir, const char *fpattn, char * string, char *vbuff, int buff_size){
    FILE *vfiles[MAX_VFILES], *vfile;
    int num_vfiles, i;
    char *fwv_buff;
    // Step 0 - Clear out the vbuffer
    memset(vbuff, '\0', buff_size);
    // Step 1 - get vfiles
    if((num_vfiles = get_vfiles(v_dir, fpattn, vfiles)) == -1)
        // buff isnt valid
        return 0;

    // Step 2 - go through file pointer
    for(i=0; i< num_vfiles; i++){
        vfile = vfiles[i];
        if(!read_vfile(vfile, vbuff, string)){
            // return if buffer is out of space
            fclose(file);
            return i;
        }
        fclose(file);
     }
    return i+1;
}



/*==========================================================*/
/*========Class = 0x01 (Mass storage controllers) ==========*/
/**
* @return to indicate what buffer is valid
*/
enum FVERS{DRV, FWV};
int sas_read_versions(struct pci_dev *dev, char *dr_v, char *fw_v){
    FILE *fp;
    DIR *v_dir;
    struct pci_class_methods *pcm;
    int num_vfiles;
    u16 ven_id = dev->vendor_id;
    char *drv_fpattn, *fwv_fpattn;
    int fpattn_flag[2] = {0};
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
    // Step 3- get pattns
    if(!(drv_fpattn = pcm->drv_file_pattns[ven_id])){
        dev->warning("sas_read_version: sas (struct pcm) driver file pattn not installed");
    }else{
        fpattn_flag[DRV] = 1;
    }
    if(!(fwv_fpattn = pcm->fwv_file_pattns[ven_id])){
       dev->warning("sas_read_version: sas (struct pcm) driver file pattn not installed");
     }else{
        fpattn_flag[FWV] = 1;
     }

    // both of them are installed (need flag)
    if( fpattn_flag[DRV] && read_files(v_dir, drv_fpattn, dr_v)){
        // condition based on return
    }
    if( fpattn_flag[FWV] &&read_files(v_dir, fwv_fpattn, fw_v)){
        // condition based on return
    }


    return ;
}



 


int nvm_read_versions(struct pci_dev *dev, char *dr_v, char *fw_v){
    // Step 1 - get base folder
    char buff[MAX_PATH] = {'\0'};
    char nvme[MAX_PATH/2] = {'\0'};

    FILE *fp;
    strcat(buff, "/firmware_rev");

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

