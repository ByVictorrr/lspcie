/*
 *	The PCI Utilities -- List a table of important outputs

 *	HPE Developed
 */
 #include <stdlib.h>

// locn (location)
typedef FILE * locn_file;

char t_hdr[]=
"PCI Address\tSlot#\tCard_info\t\t\tVendor\tDriver";

// read all of the topology --io command into a struct
struct locns{
    char *PCI_ADDRESS;
    char *Location;
    // if PCI_address the same then assocate location with that dev
};

static locn_file locn_open(char * fname){
    locn_file f;
    if(f=fopen(fname)){
        return f;
    }
    return NULL;
}
static void parse_locnf(){
    
}

// loads the slots for every device in pacc
void load_locn(struct pacc *pacc){

    #define LOCATION_BUFFER "locn.txt"
    struct device *d;
    locn_file f;
    // Step 1 - create file with PCI map to location( use file as buffer)
    if (system("topology --io > locn.txt") != 0){
        // not ok
        perror("Not able to run topology");
    }else{
        // step 2 - open locn.txt file
        f = locn_file(LOCATION_BUFFER);
        // Step 3 - read the file and associate the PCI_ADDRESS with locate
        for (p=pacc->devices; p; p=p->next){
            p->locn=malloc(size);
        
        }

    }

}


void 
show_table(struct device *d)
{
    // PCI Adress Ouput
    show_slot_name(d);
    // Slot # (envoke topology command)
    show_io_slot(d);
    // Card_info (name of card or device id if DNE)

    // Vendor (name of vender)

    // Driver (driver name)
    // Device_info


}
