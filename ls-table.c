/*
 *	The PCI Utilities -- List a table of important outputs

 *	HPE Developed
 */
#include "lspci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "json-builder.h"


/* Max String length for each entry */
#define PCI_ADDR_SIZE 12
#define PHYS_SLOT_SIZE 20
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



/*===================Tabulated entry (for printing)===============*/
/* Come back to */

static char *
get_card_info(struct device *d){
    word subsys_v, subsys_d;
    char ssnamebuf[256], *c_info;
    struct pci_dev *p = d->dev;
    memset(ssnamebuf, 0, 256);
    get_subid(d, &subsys_v, &subsys_d);
    if (subsys_v && subsys_v != 0xffff)
        c_info = pci_lookup_name(pacc, ssnamebuf, sizeof(ssnamebuf),
			PCI_LOOKUP_SUBSYSTEM | PCI_LOOKUP_DEVICE, p->vendor_id, p->device_id,
			subsys_v, subsys_d);
            return strdup(c_info);
    else
        return NULL;
        

}
static char
set_buff(char *(*get_data)(struct device *d), struct device *d, char *buff){
    char *ret;
    if(!(ret=get_card_info(d)))
    {
        strcpy(buff, ret);
    }
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
    struct pci_dev *p = d->dev;
    if(p->phy_slot){
        strcpy(buff, p->phy_slot);
    }else{
        buff[0] = '-';
    }
}

#define MAX_BUFF 100

static void 
fill_vbuff(struct version_item *vitems, char *buff, int size){
    char *temp;
    struct version_item *vitem;
    int i;
    temp = xmalloc(size);
    for(vitem=vitems, i=0; vitem; vitem=vitem->next, i++){
        if(i==0){
            if(strlen(vitem->data)+1 > size)
                goto free_temp;
            else{
                strcpy(temp, vitem->data);
                strcpy(buff, vitem->data);
            }
        }else{
            int n = snprintf(buff, size, "%s %s", temp, vitem->data);
            if(n < 0){
                goto free_temp;
            }
            if(strlen(temp) < strlen(buff)){
                temp=xrealloc(temp, strlen(buff)+1);
            }
            strcpy(buff, temp);
        }
     
    }
free_temp:
    free(temp);
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
show_json_obj(struct device *d, json_value)
{
    struct version_item *vitemss[3]={NULL}, *vitems=NULL; /* 0= drv, 1=fwv, 2=optv*/
    #define MAX_BUFF 1024
    char buff[MAX_BUFF];
    json_value *dev_obj = json_object_new(0);

    memset(buff, 0, MAX_BUFF);
    // Step 1 - pci address output
    set_slot_name(d, buff);
    json_object_push(dev_obj, "pci address", json_string_new(buff));
    // Step  2 - phys slot 
    memset(buff, 0, MAX_BUFF);
    set_phy_slot(d, buff);
    json_object_push(dev_obj, "slot #", json_string_new(buff));
    // Step 3 - card info 
    memset(buff, 0, MAX_BUFF);
    set_phy_slot(d, buff);
    json_object_push(dev_obj, "card info", json_string_new(buff));
    // Step 4 - vendor name
    memset(buff, 0, MAX_BUFF);
    set_vendor(d, buff);
    json_object_push(dev_obj, "vendor", json_string_new(buff));
    // Step 5 - driver
    memset(buff, 0, MAX_BUFF);
    set_driver(d, buff);
    json_object_push(dev_obj, "driver", json_string_new(buff));
    return dev_obj;
}
#define DEV_INFO_NUM_SIZE 10

void 
show_table_entry(struct device *d)
{
    char dev_info_num[DEV_INFO_NUM_SIZE];
    struct table_entry e;
    int pos; 
    struct version_item *vitemss[3]={NULL}, *vitems=NULL; /* 0= drv, 1=fwv, 2=optv*/
    memset(dev_info_num, 0, DEV_INFO_NUM_SIZE);
    memset(&e, 0, sizeof(struct table_entry));
    json_value *ob = get_dev_json_obj(d);

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
    /* Could it be the case that we want json object format*/
   

    printf("%-12.12s\t%-20.20s\t%-40.40s\t%-12.12s\t%-12.12s", 
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
    // Show versions
    if(table > 3){
        if (!pci_read_driver_version(d->dev, &vitemss[DRV_ITEMS])) {
            memset(e.dr_v, '.', 1);
        }else{
            vitems = vitemss[DRV_ITEMS];
            fill_vbuff(vitems, e.dr_v, DR_VERSION_SIZE);
            free_version_items(vitems);
        }
    if(table > 4){
        if (!pci_read_firmware_version(d->dev, &vitemss[FWV_ITEMS])){
            memset(e.fw_v, '.', 1);
        }else{
            vitems = vitemss[FWV_ITEMS];
            fill_vbuff(vitems, e.fw_v, FW_VERSION_SIZE);
            free_version_items(vitems);
        }
        printf("\t%-20.20s", e.dr_v);
        printf("\t%-20.20s", e.fw_v);
    }
    if( table > 5 ){
        if (!pci_read_option_rom_version(d->dev, &vitemss[OPTV_ITEMS])){
            memset(e.opt_v, '.', 1);
        }else{
            vitems = vitemss[OPTV_ITEMS];
            fill_vbuff(vitems, e.opt_v, OPT_VERSION_SIZE);
            free_version_items(vitems);
        }
        printf("\t%-20.20s", e.opt_v);

    }
    printf("\n");
}
void 
print_hdr(int line_width){
    int i;
    printf("%-12.12s\t%-20.20s\t%-40.40s\t%-12.12s\t%-12.12s", 
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

