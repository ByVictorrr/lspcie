/*
 *	The PCI Utilities -- List a table of important outputs

 *	HPE Developed
 */
#include "lspci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* Max String length for each entry */
/* Location string is 10 */
#define PHYS_SLOT_SIZE 5
#define PCI_ADDR_SIZE 12
#define CARD_INFO_SIZE 30
#define VENDOR_INFO_SIZE 15
#define DRIVER_SIZE 11
#define DEVICE_INFO_SIZE 35
#define DR_VERSION_SIZE 200
#define FW_VERSION_SIZE 200
#define OPT_VERSION_SIZE 200


struct table_entry{
    char pci_addr[PCI_ADDR_SIZE];
    char phy_slot[PHYS_SLOT_SIZE];
    char card_info[CARD_INFO_SIZE];
    char vendor[VENDOR_INFO_SIZE];
    char driver[DRIVER_SIZE];
    char dev_info[DEVICE_INFO_SIZE];
    char dr_v[DR_VERSION_SIZE];
    char fw_v[FW_VERSION_SIZE];
    char opt_v[OPT_VERSION_SIZE];
};

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
/*=============================================================*/

/*===================Tabulated entry (for printing)===============*/
/* Come back to */
static void
set_card_info(struct device *d, char *buff){
    word subsys_v, subsys_d;
    char ssnamebuf[256];
    struct pci_dev *p = d->dev;
    memset(ssnamebuf, 0, 256);
    get_subid(d, &subsys_v, &subsys_d);
    if (subsys_v && subsys_v != 0xffff)
	    sprintf(buff,"%s",
		pci_lookup_name(pacc, ssnamebuf, sizeof(ssnamebuf),
			PCI_LOOKUP_SUBSYSTEM | PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id,
			subsys_v, subsys_d));
    else
	    sprintf(buff,"%s", "?");

}
static void
set_slot_name(struct device *d, char *buf){
    show_slot_name(d,buf);
}

static int
set_dev_info(struct device *d, char *buff, int buff_size){
  struct pci_dev *p = d->dev;
  char namebuf[1024], *name;
  memset(namebuf, 0, 1024);
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id);
  return snprintf(buff, buff_size, "%s", name);
       
}
static void
set_vendor(struct device *d, char *buff){
  struct pci_dev *p = d->dev;
  char namebuf[1024], *name;
  memset(namebuf, 0, 1024);
  name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, p->vendor_id);
  sprintf(buff, "%s", name);

}
static void
set_driver(struct device *d, char * buff){
    
    char buf[DRIVER_BUF_SIZE];
    const char *driver;
    memset(buf, 0, DRIVER_BUF_SIZE);
    if (driver = find_driver(d, buf))
        sprintf(buff, "%s", driver);
}
static void
set_phy_slot(struct device *d, char *buff){
    int phy_slot;
    struct pci_dev *p = d->dev;
    pci_fill_info(p, PCI_FILL_PHYS_SLOT | PCI_FILL_NUMA_NODE | PCI_FILL_DT_NODE);
    if(p->phy_slot){
        phy_slot = (int)strtol(d->dev->phy_slot, NULL, 16)&0x1f;
        sprintf(buff, "%d", phy_slot);
    }else{
        buff[0] = '-';
    }
}

#define MAX_BUFF 100
/* for each of the v's*/
static void 
fill_vbuff_tuple(struct version_item *vitems, char *buff, int size){
    struct version_item *vitem;
    char src_buff[MAX_BUFF], data_buff[MAX_BUFF];
    int i, n1, n2;
    memset(src_buff, 0, MAX_BUFF);
    memset(data_buff, 0, MAX_BUFF);
    int data_len, srcpath_len, data_buff_len, src_buff_len;
    src_buff[0]='(';
    data_buff[0]='(';
    for(vitem=vitems, i=0; vitem && vitem->data && vitem->src_path; vitem=vitem->next, i++){
        // First iteration
        data_len = strlen(vitem->data);
        srcpath_len = strlen(basename(vitem->src_path));
        if(!i){
            n1 = snprintf(&data_buff[1], data_len, "%s", vitem->data);
            n2 = snprintf(&src_buff[1], srcpath_len, "%s", basename(vitem->src_path));
            src_buff_len = strlen(src_buff);
            data_buff_len = strlen(data_buff);
        // middle iteration
        }else{
            n1 = snprintf(&data_buff[data_buff_len], data_len+2, ",%s", vitem->data);
            n2 = snprintf(&src_buff[src_buff_len], srcpath_len+2, ",%s", basename(vitem->src_path));
            src_buff_len = strlen(src_buff);
            data_buff_len = strlen(data_buff);
        }

    }
    src_buff[strlen(src_buff)]=')';
    data_buff[strlen(data_buff)]=')';

    n1 = snprintf(buff, size, "%s=%s", src_buff, data_buff);
}

static void 
fill_vbuff(struct version_item *vitems, char *buff, int size){

    char *temp[MAX_BUFF];
    memset(temp, 0, MAX_BUFF);
    while(vitems){

        if(buff[0] = '\0'){
            if(strlen(vitems->data)+1 > size)
                return;
            else{
            strcpy(temp, vitems->data);
            strcpy(buff, vitems->data);
            }
        }else{
            int n = snprintf(buff, size, "%s %s", temp, vitems->data);
            if(n < 0){
                return;
            }
        }
        vitems=vitems->next;
    }

}
static void free_version_items(struct version_item* vitems){
    struct version_item *next;
    while(vitems){
        if(vitems->src_path){
            free(vitems->src_path);
        }
        if(vitems->data){
            free(vitems->data);
        }

        next=vitems->next;
        free(vitems);
        vitems=next;
    }
}
void 
show_table_entry(struct device *d)
{

    #define DEV_INFO_NUM_SIZE 10
    char dev_info_num[DEV_INFO_NUM_SIZE];
    struct table_entry e;
    int pos; 
    struct version_item *vitemss[3]={NULL}, *vitems=NULL; /* 0= drv, 1=fwv, 2=optv*/
    // clear out local buffers
    memset(dev_info_num, 0, DEV_INFO_NUM_SIZE);
    memset(&e, 0, sizeof(struct table_entry));
    // Setting tab_entry
    // 1. PCI Adress Ouput
    set_slot_name(d, e.pci_addr);
    // 2. PHYS slot number
    set_phy_slot(d, e.phy_slot);
    // 3. Card info
    set_card_info(d, e.card_info);
    // 4. Vendor (name of vender)
    set_vendor(d, e.vendor);
    // 5. Driver (driver name)
    set_driver(d, e.driver);
   

    printf("%-12.12s\t%-12.12s\t%-40.40s\t%-12.12s\t%-12.12s", 
            e.pci_addr, 
            e.phy_slot, 
            e.card_info,
            e.vendor,
            e.driver);

     // 6. Device_info
    if (table > 1){
        pos = set_dev_info(d, e.dev_info, DEVICE_INFO_SIZE);
        printf("\t%-40.40s",e.dev_info);
    }
    if (table > 2){
        sprintf(dev_info_num,"[%4.4x:%4.4x]", d->dev->vendor_id, d->dev->device_id);
        printf("%s",dev_info_num);
    }
    if(table > 5){
        if (!pci_read_driver_version(d->dev, &vitemss[DRV_ITEMS])) {
            memset(e.dr_v, '.', 1);
        }else{
            vitems = vitemss[DRV_ITEMS];
            fill_vbuff_tuple(vitems, e.dr_v, DR_VERSION_SIZE);
            free_version_items(vitems);
        }
        if (!pci_read_firmware_version(d->dev, &vitemss[FWV_ITEMS])){
            memset(e.fw_v, '.', 1);
        }else{
            vitems = vitemss[FWV_ITEMS];
            fill_vbuff_tuple(vitems, e.fw_v, FW_VERSION_SIZE);
            free_version_items(vitems);
        }
        if (!pci_read_option_rom_version(d->dev, &vitemss[OPTV_ITEMS])){
            memset(e.fw_v, '.', 1);
        }else{
            vitems = vitemss[OPTV_ITEMS];
            fill_vbuff_tuple(vitems, e.opt_v, OPT_VERSION_SIZE);
            free_version_items(vitems);
        }
     

/*
        printf("\t%-40.40s", e.dr_v);
        printf("\t%-40.40s", e.fw_v);
*/
        printf("\t%s", e.opt_v);

    }else if(table > 3){
    // 7. Driver/fw version (TODO: handle the ret value)
        if (!pci_read_driver_version(d->dev, &vitemss[DRV_ITEMS])) {
            // TODO store items in e.dr_v
            memset(e.dr_v, '.', 1);
        }else{
            vitems = vitemss[DRV_ITEMS];
            fill_vbuff(vitems, e.dr_v, DR_VERSION_SIZE);
            free_version_items(vitems);
        }
        if (!pci_read_firmware_version(d->dev, &vitemss[FWV_ITEMS])){
            memset(e.fw_v, '.', 1);
        }else{
            vitems = vitemss[FWV_ITEMS];
            fill_vbuff(vitems, e.fw_v, FW_VERSION_SIZE);
            free_version_items(vitems);
        }
        printf("\t%-20.20s", e.dr_v);
        printf("\t%-20.20s", e.fw_v);
        if(table > 4){
            if (!pci_read_option_rom_version(d->dev, &vitemss[OPTV_ITEMS])){
                memset(e.fw_v, '.', 1);
            }else{
                vitems = vitemss[OPTV_ITEMS];
                fill_vbuff(vitems, e.opt_v, OPT_VERSION_SIZE);
                free_version_items(vitems);
            }
            printf("\t%-20.20s", e.opt_v);

        }
      }

    printf("\n");
}
void 
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
        printf("\t%-20.20s", "Driver_Version");
        printf("\t%-20.20s", "Firmware_Version");
    }
    if(table > 4 ){
        printf("\t%-20.20s", "Option_Rom_Version");
    }

    printf("\n");
    for(i=0; i<line_width; i++)
        putchar('-');
    putchar('\n');
}

