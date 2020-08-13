/*
 *	The PCI Utilities -- List a table of important outputs

 *	HPE Developed
 */
#include "lspci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Indents for table */
#define SLOT_IND 10
#define C_IND 20
#define V_IND 30
#define DR_IND 50
#define DEV_IND 60
#define DRV_IND 70
#define FWV_IND 80

/* Max String length for each entry */
/* Location string is 10 */
#define LOCN_SIZE 10
#define PCI_ADDR_SIZE 12
#define CARD_INFO_SIZE 30
#define VENDOR_INFO_SIZE 15
#define DRIVER_SIZE 11
#define DEVICE_INFO_SIZE 35
#define DR_VERSION_SIZE 30
#define FW_VERSION_SIZE 100

typedef enum {NOT_FOUND, FOUND} locn_found_t;



struct tab_entry{
    char pci_addr[PCI_ADDR_SIZE];
    char loc_num[LOCN_SIZE];
    char card_info[CARD_INFO_SIZE];
    char vendor[VENDOR_INFO_SIZE];
    char driver[DRIVER_SIZE];
    char dev_info[DEVICE_INFO_SIZE];
    char dr_v[DR_VERSION_SIZE];
    char fw_v[FW_VERSION_SIZE];
};

/*===============LOCATION MAP======================*/
typedef FILE* locn_map_t;
typedef char locn_t[LOCN_SIZE];
/** 
* Desc: location map is just a file pointer 
* @return a locn map resource used to map pci bus -> locn
*/
static locn_map_t 
build_locn_map(void){
    #define LOCN_MAP_FNAME "get_locations/loc_map.txt"
    //
    locn_map_t f;
    // Step 1 - create file that maps PCI bus to location
    if (system("bash get_locations/get_location_map.sh") == -1){
        fprintf(stderr, "Cannot create the location map");
        return NULL;
    }
    if((f=fopen(LOCN_MAP_FNAME, "r")) == NULL){
        fprintf(stderr, "Not able to open location map");
        return NULL;
    }
    return f;
}
/**
* Desc: Releases resources used by the locn_map
* @param f - locn map that is asked to be released
* @return an integer (0) if not sucessful, (1) if it is
*/
static int 
clean_locn_map(locn_map_t f){
    if(fclose(f) != 0){
        fprintf(stderr, "Not able to close the location map");
        return 0;
    }
    return 1;
}
/**
* Desc: Parses a str stores them into pci_addr, and loc
* @param str - Is assumed to be of the form (pci_addr,locn)
* @param del - deli
*/
static void 
get_loc_pair(char *str, char *pci_addr, char *loc){
   strcpy(pci_addr,strtok(str, ","));
   strcpy(loc,strtok(NULL, "\n"));
}
/**
* Desc: Helper of get_loc; Used to determine if Dom/BDF is equal to pci_dev
*/
static int 
is_pci_addr(struct pci_dev *d,
                            unsigned int dom, unsigned int bus, 
                            unsigned int dev, unsigned int func){
    return d->domain == dom && d->bus == bus && d->dev == dev 
            && d->func == func;
}
/**
* Desc: Assigns the location to a specific device( by scanning the file)
* @param d - pci device used to map the location to pci bus address
* @param f - file that is used as a table to map (bus_addr->slot number) 
* @param locn - buffer used to store the corresponding slot number 
* @return found_loc - used to determine if the locn of the pci_dev is found (1 if so 0 if not)
*/
static locn_found_t
get_locn(struct pci_dev *d, locn_map_t f, char *locn){
    ssize_t read;
    size_t len;
    char *line = NULL;
    unsigned int dom, bus, dev, func;
    char pci_addr[13] = {'\0'};
    char loc[11] = {'\0'};
    locn_found_t found_loc=NOT_FOUND;

    // Step 1 - read through each line of the file 
    while((read = getline(&line, &len, f)) != -1){
        // step 2 - Seperate line string
        get_loc_pair(line, pci_addr, loc);
        if (sscanf(pci_addr, "%x:%x:%x.%d", &dom, &bus, &dev, &func) < 4)
            fprintf(stderr,"get_loc: Couldn't parse entry name %s", pci_addr);
        // step 3 - look for a does this device match the pci_addr?
        if (is_pci_addr(d, dom, bus, dev, func)){
            found_loc=FOUND;
            break;
        }else
            continue;
    }
    /* reset the file pointer */
    fseek(f, 0, SEEK_SET);

    /* This means that it is an (IO device) */
    strcpy(locn, loc);
    return found_loc;
 }
/*=============================================================*/

/*===================Tabulated entry (for printing)===============*/
/* Come back to */
static void
show_card_info(struct device *d, char *buff){
    word subsys_v, subsys_d;
    char ssnamebuf[256];
    struct pci_dev *p = d->dev;
    get_subid(d, &subsys_v, &subsys_d);
    if (subsys_v && subsys_v != 0xffff)
	    sprintf(buff,"%s",
		pci_lookup_name(pacc, ssnamebuf, sizeof(ssnamebuf),
			PCI_LOOKUP_SUBSYSTEM | PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id,
			subsys_v, subsys_d));
    else
	    sprintf(buff,"%s", "?");

}
static int
show_dev_info(struct device *d, char *buff, int buff_size){
  struct pci_dev *p = d->dev;
  char namebuf[1024], *name;
  int c;
  word sv_id, sd_id;
  char classbuf[128], vendbuf[128], devbuf[128], svbuf[128], sdbuf[128];
  char *dt_node;
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id);
  return snprintf(buff, buff_size, "%s", name);
       
}
static void
show_vendor(struct device *d, char *buff){
  struct pci_dev *p = d->dev;
  char namebuf[1024], *name;
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, p->vendor_id);
  sprintf(buff, "%s", name);

}
static void
show_driver(struct device *d, char * buff){
    
    char buf[DRIVER_BUF_SIZE];
    const char *driver;
    if (driver = find_driver(d, buf))
        sprintf(buff, "%s", driver);
}


static void 
show_device_entry(struct device *d, locn_map_t f)
{

    #define DEV_INFO_NUM_SIZE 10
    char dev_info_num[DEV_INFO_NUM_SIZE];
    char locn[LOCN_SIZE];
    struct tab_entry e;
    int pos; 
    // clear out local buffers
    memset(dev_info_num, 0, DEV_INFO_NUM_SIZE);
    memset(locn, 0, LOCN_SIZE);
    memset(&e, 0, sizeof(struct tab_entry));


    // Step 0 - See if the device if found in the locn map (dont print out device)
    /*
    if(get_locn(d->dev, f, locn) == NOT_FOUND){
        return;
    }
    */
    // Setting tab_entry
    // 1. PCI Adress Ouput
    show_slot_name(d, e.pci_addr);
    // 2. LOCATION
    strcpy(e.loc_num, locn);
    // 3. Card info
    show_card_info(d, e.card_info);
    // 4. Vendor (name of vender)
    show_vendor(d, e.vendor);
    // 5. Driver (driver name)
    show_driver(d, e.driver);
   

    printf("%-12.12s\t%-12.12s\t%-40.40s\t%-12.12s\t%-12.12s", 
            e.pci_addr, 
            e.loc_num, 
            e.card_info,
            e.vendor,
            e.driver);

     // 6. Device_info
    if (table > 1){
        pos = show_dev_info(d, e.dev_info, DEVICE_INFO_SIZE);
        printf("\t%-40.40s",e.dev_info);
    }
    if (table > 2){
        sprintf(dev_info_num,"[%4.4x:%4.4x]", d->dev->vendor_id, d->dev->device_id);
        printf("%s",dev_info_num);
    }
    if(table > 3){
    // 7. Driver/fw version (TODO: handle the ret value)
        if (!pci_read_driver_version(d->dev, e.dr_v, DR_VERSION_SIZE)) {
            memset(e.dr_v, '.', 1);
        }
        if (!pci_read_firmware_version(d->dev, e.fw_v, FW_VERSION_SIZE)) {
            memset(e.fw_v, '.', 1);
        }
        printf("\t%-40.40s", e.dr_v);
        printf("\t%-40.40s", e.fw_v);
    }
    printf("\n");
}
static void 
print_hdr(int line_width){
    int i;
    printf("%-12.12s\t%-12.12s\t%-40.40s\t%-12.12s\t%-12.12s", 
            "PCI_Address", 
            "Slot#", 
            "Card_info",
            "Vendor",
            "Driver");
    if(table > 1){
        printf("\t%-40.40s", "Device_info");
    }
    if(table > 3){
        printf("\t%-20.20s", "Driver Version");
        printf("\t%-20.20s", "Firmware Version");
    }

    printf("\n");
    for(i=0; i<line_width; i++)
        putchar('-');
    putchar('\n');
}
void 
show_table(struct device *first_dev)
{
    struct device *head = first_dev, *p;
    locn_map_t f;

    print_hdr(200);

    // Step 1 - build loc_map (returns FILE *ptr)
    /*
    if(!(f=build_locn_map())){
        return;
    }
    */

    for(p=head; p; p=p->next){
        show_device_entry(p, f);
    }
    /*
    clean_locn_map(f);
    */

}

