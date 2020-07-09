/*
 *	The PCI Utilities -- List a table of important outputs

 *	HPE Developed
 */
 #include <stdlib.h>

char t_hdr[]=
"PCI Address\tSlot#\tCard_info\t\t\tVendor\tDriver";

// read all of the topology --io command into a struct
struct io_slots {

};


// loads the slots for every device in pacc
void load_io_slots(struct pacc *pacc){

    struct device *d;
    // Step 1 - create file with PCI map to location
    if (system("topology --io > io_slots.txt") != 0){
        // not ok
        perror("Not able to run topology");
    }else{
        // ok
    }
    // Step 2 - read the file and associate the PCI_ADDRESS with location
    for (p=pacc->devices; p; p=p->next){
        
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
