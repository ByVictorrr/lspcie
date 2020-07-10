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
"PCI Address\tSlot#\tCard_info\t\t\tVendor\tDriver";


/* Wrapper structure to point to the next io device (excluding internal PCI)*/
struct io_dev{
    struct io_dev *next;
    struct pci_dev *dev;
    char *locn;
};

/*
* Desc: THis function setups a io_dev structure and returns it
*/
static struct io_dev *
build_io_dev(struct pci_dev *d, char *locn){
    struct io_dev *i = xmalloc(sizeof(struct io_dev));
    i->dev = d;
    i->next = NULL;
    i->locn = xmalloc(sizeof(char)*(LOCN_SIZE+1));
    strcpy(i->locn, locn);
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
    struct io_dev *i = NULL;
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
        strcpy(i->locn, loc);
    }
    return locn;
}

/* 
* Desc: Loads the slots for every device in pacc
*/
static struct io_dev *
build_io_devs(struct pci_access *pacc){
    #define LOCATION_FILE_MAP "get_locations/loc_map.txt"
    struct pci_dev *d;
    char *locn;
    locn_file f;
    struct io_dev *head = NULL, *curr = NULL;
    // Step 1 - create file with PCI map to location( use file as buffer)
    if (system("bash get_locations/get_location_map.sh") == -1){
        //perror("Not able to run topology");
        printf("topology error");
    }else{
        f = locn_open(LOCATION_FILE_MAP);
        // Step 3 - read the file and associate the PCI_ADDRESS with locate
        for (d=pacc->devices; d; d=d->next){
            if ((locn = get_loc(d, f))!=NULL){
                curr = build_io_dev(d, locn);
                free(locn);
                // first iteration 
                if(head== NULL){
                    head=curr;
                }
                curr=curr->next;
            }
        }
    }

    return head;
}


static void 
show_device_entry(struct io_dev *i)
{
    // PCI Adress Ouput
    show_slot_name(i->dev);
    // Slot # (envoke topology command)
    printf("%s", i->locn);
    // Card_info (name of card or device id if DNE)
    // Vendor (name of vender)

    // Driver (driver name)
    // Device_info
}
void 
show_io(struct io_dev *head){
    int i=0;
    if (i==0){
        printf("hi");
    }

}
void 
show_table(struct pci_access *a)
{
    struct io_dev *p; 
    struct io_dev *head;
    int i;
    printf("%s\n", t_hdr);
    head=build_io_devs(a);
    // debugging
    show_io(head);
    for(p=head; p; p=p->next){
        show_device_entry(p);
    }
}
