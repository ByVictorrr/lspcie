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

/* Max String length for each entry */
/* Location string is 10 */
#define LOCN_SIZE 10
#define PCI_ADDR_SIZE 12
#define CARD_INFO_SIZE 30
#define VENDOR_INFO_SIZE 15
#define DRIVER_SIZE 11
#define DEVICE_INFO_SIZE 35




struct tab_entry{
    char pci_addr[PCI_ADDR_SIZE+1];
    char loc_num[LOCN_SIZE+1];
    char card_info[CARD_INFO_SIZE+1];
    char vendor[VENDOR_INFO_SIZE+1];
    char driver[DRIVER_SIZE+1];
    char dev_info[DEVICE_INFO_SIZE+1];
};
typedef FILE* locn_file;


/* Wrapper structure to point to the next io device (excluding internal PCI)*/
struct io_dev{
    struct io_dev *next;
    struct device *dev;
    char *locn;
};


/*
* Desc: THis function setups a io_dev structure and returns it
*/
static struct io_dev *
build_io_dev(struct device *d, char *locn){
    struct io_dev *i = xmalloc(sizeof(struct io_dev));
    i->dev = d;
    i->next = NULL;
    i->locn = locn;
    return i;
}



// Need to filter out all the PCI devices(other than the ones in )
static locn_file locn_open(char * fname){
    locn_file f;
    if(f=fopen(fname, "r")){
        return f;
    }
    perror("Cannot open location file");
    return NULL;
}
/*
* Desc: Parses a str using a deliminitor, pci_addr, into two vars pci_addr and loc
*/
static void 
get_loc_pair(char *str, char *del, char *pci_addr, char *loc){
   strcpy(pci_addr,strtok(str, del));
   strcpy(loc,strtok(NULL, "\n"));
}
/*
* Desc: Checks whether the given dom/BDF matches the devices
*/
static int 
is_pci_addr(struct pci_dev *d,
                            unsigned int dom, unsigned int bus, 
                            unsigned int dev, unsigned int func){
    return d->domain == dom && d->bus == bus && d->dev == dev 
            && d->func == func;
 }
/* 
* Desc: Assigns the location to a specific device( by scanning the file)
* Assumption: File is open 
*/
static char *
get_loc(struct pci_dev *d, locn_file f){
    ssize_t read;
    size_t len;
    char *line = NULL;
    unsigned int dom, bus, dev, func;
    char pci_addr[13] = {'\0'};
    char loc[11] = {'\0'};
    int found_loc=0;
    char *locn = NULL;

    // Step 1 - read through each line of the file 
    while((read = getline(&line, &len, f)) != -1){
        // step 2 - Seperate line string
        get_loc_pair(line, ",", pci_addr, loc);
        if (sscanf(pci_addr, "%x:%x:%x.%d", &dom, &bus, &dev, &func) < 4)
            fprintf(stderr,"get_loc: Couldn't parse entry name %s", pci_addr);
        // step 3 - look for a does this device match the pci_addr?
        if (is_pci_addr(d, dom, bus, dev, func)){
            found_loc=1;
            break;
        }else
            continue;
    }
    /* This means that it is an (IO device) */
    if (found_loc){
        locn = xmalloc(strlen(loc)+1);
        strcpy(locn, loc);
    }
    /* reset the file pointer */
    fseek(f, 0, SEEK_SET);
    return locn;
}

/* 
* Desc: Loads the slots for every device in pacc
*/
static struct io_dev *
build_io_devs(struct device * first_dev){
    #define LOCATION_FILE_MAP "get_locations/loc_map.txt"
    struct device *d;
    char *locn;
    locn_file f;
    struct io_dev *head = NULL, *curr = NULL, *next= NULL;
    // Step 1 - create file with PCI map to location( use file as buffer)
    if (system("bash get_locations/get_location_map.sh") == -1){
        //perror("Not able to run topology");
        printf("topology error");
    }else{
        f = locn_open(LOCATION_FILE_MAP);
        // Step 3 - read the file and associate the PCI_ADDRESS with locate
        for (d=first_dev; d; d=d->next){
            if ((locn = get_loc(d->dev, f))!=NULL){
                next = build_io_dev(d, locn);
                // first iteration 
                if(head== NULL){
                    head=curr=next;
                // After first iteration
                }else{
                    curr->next=next;
                    curr=next;
                }
                
            }
        }
        fclose(f);
    }

    return head;
}

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
show_dev_info(struct device *d, char *buff){
  struct pci_dev *p = d->dev;
  char namebuf[1024], *name;
  int c;
  word sv_id, sd_id;
  char classbuf[128], vendbuf[128], devbuf[128], svbuf[128], sdbuf[128];
  char *dt_node;
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id);
  return sprintf(buff, "%s", name);
       
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
show_device_entry(struct io_dev *i)
{

    struct tab_entry e = {'\0'};
    int pos;
    char dev_info_num[10]={'\0'};
    // Setting tab_entry
    // 1. PCI Adress Ouput
    show_slot_name(i->dev, e.pci_addr);
    // 2. LOCATION
    strcpy(e.loc_num, i->locn);
    // 3. Card info
    show_card_info(i->dev, e.card_info);
    // 4. Vendor (name of vender)
    show_vendor(i->dev, e.vendor);
    // 5. Driver (driver name)
    show_driver(i->dev, e.driver);
    // 6. Device_info
    if (table > 0)
        pos = show_dev_info(i->dev, e.dev_info);
    if (table > 1)
        sprintf(dev_info_num,"[%4.4x:%4.4x]", i->dev->dev->vendor_id, i->dev->dev->device_id);
        //sprintf(e.dev_info+pos,"[%4.4x:%4.4x]", i->dev->dev->vendor_id, i->dev->dev->device_id);
    

    printf("%-12.12s\t%-12.12s\t%-40.40s\t%-12.12s\t%-12.12s", 
            e.pci_addr, 
            e.loc_num, 
            e.card_info,
            e.vendor,
            e.driver);
    if(table > 1)
        printf("\t%-40.40s",e.dev_info);
    if(table > 2)
        printf("%s",dev_info_num);

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
    if(table > 1)
        printf("\t%-40.40s", "Device_info");
    printf("\n");


    for(i=0; i<line_width; i++)
        putchar('-');
    putchar('\n');
}
static void 
cleanup_io(struct io_dev *i){
    struct io_dev *p=i, *next;
    while(p){
        next=p->next;
        free(p->locn);
        free(p);
        p=next;
    }
}
void 
show_table(struct device *first_dev)
{
    struct io_dev *p; 
    struct io_dev *head;
    int i;
    print_hdr(150);

    head=build_io_devs(first_dev);
    for(p=head; p; p=p->next){
        show_device_entry(p);
    }

    cleanup_io(head);
}

