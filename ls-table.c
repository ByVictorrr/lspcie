/*
 *	The PCI Utilities -- List a table of important outputs

 *	HPE Developed
 */
#include "lspci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Location string is 10 */
#define LOCN_SIZE 10

typedef FILE * locn_file;

char t_hdr[]=
"PCI Address\tSlot#\t\tCard_info\t\t\t\t\t\t\tVendor\tDriver\t\tDevice_info";


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

static void
show_card_info(struct device *d){
  struct pci_dev *dev = d->dev;
  char namebuf[1024], *name;
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
  printf("%s",name);
       
}
/* Come back to */
static void
show_dev_info(struct device *d){
    word subsys_v, subsys_d;
    char ssnamebuf[256];
    struct pci_dev *p = d->dev;
    get_subid(d, &subsys_v, &subsys_d);
    if (subsys_v && subsys_v != 0xffff)
	    printf("\t%s\n",
		pci_lookup_name(pacc, ssnamebuf, sizeof(ssnamebuf),
			PCI_LOOKUP_SUBSYSTEM | PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id,
			subsys_v, subsys_d));
}
static void
show_vendor(struct device *d){
  struct pci_dev *dev = d->dev;
  char namebuf[1024], *name;
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, dev->vendor_id);
  printf("\t\t%s",name);

}
static void
show_driver(struct device *d){
    
    char buf[DRIVER_BUF_SIZE];
    const char *driver;
    if (driver = find_driver(d, buf))
        printf("\t\t%s", driver);
}


static void 
show_device_entry(struct io_dev *i)
{
    // PCI Adress Ouput
    show_slot_name(i->dev);
    // Slot # (envoke topology command)
    printf("\t%s\t", i->locn);
    // Card_info (name of card or device id if DNE)
    show_card_info(i->dev);
    // Vendor (name of vender)
    show_vendor(i->dev);
    // Driver (driver name)
    show_driver(i->dev);
    // Device_info
    show_dev_info(i->dev);
    printf("\n");
}
static void 
print_hdr(int line_width){
    int i;
    printf("%10s %10s %30s %10s %10s %30s\n", 
            "PCI_Address", "Slot#", "Card_info",
            "Vendor", "Driver", "Device_info");
    
    for(i=0; i<line_width; i++){
        putchar('-');
    }
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

