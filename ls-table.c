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
#define PCI_ADDR_SIZE 13
#define PHYS_SLOT_SIZE 20
#define CARD_INFO_SIZE 100
#define VENDOR_INFO_SIZE 15
#define DRIVER_SIZE 11
#define DEVICE_INFO_SIZE 100
#define VENDOR_ID_SIZE 5
#define DEVICE_ID_SIZE 5
#define DR_VERSION_SIZE 200
#define FW_VERSION_SIZE 200
#define OPT_VERSION_SIZE 200
#define DEV_INFO_NUM_SIZE 10


struct table_entry{
    char pci_addr[PCI_ADDR_SIZE];
    char phy_slot[PHYS_SLOT_SIZE];
    char card_info[CARD_INFO_SIZE];
    char vendor[VENDOR_INFO_SIZE];
    char driver[DRIVER_SIZE];
    char dev_info[DEVICE_INFO_SIZE];
    char ven_id[VENDOR_ID_SIZE];
    char dev_id[DEVICE_ID_SIZE];
    char dr_v[DR_VERSION_SIZE];
    char fw_v[FW_VERSION_SIZE];
    char opt_v[OPT_VERSION_SIZE];
};
/* Following the same output as topology io dev */
int 
is_io_dev(struct pci_dev *p){
  int is_io = 0;
  u8 class = p->device_class >> 8;
  u8 sclass = p->device_class & 0x00FF;
  u16 vendor = p->vendor_id;
  switch(class){
    case 0x01: /* mass storage controller */
      /* scsi or raid or sata or nvme*/
      if(sclass == 0x00 || sclass == 0x06 || sclass == 0x04 || sclass == 0x08)
        is_io = 1;
      break;
    case 0x02: /* nic */
      /* eth or ib or network */
      if(sclass == 0x00 || sclass == 0x07 || sclass == 0x80)
        is_io = 1;
      break;
    case 0x03: /* display controller */
      /* vga or 3d */
      if(sclass == 0x00 || sclass == 0x02)
        is_io = 1;
      break;
    case 0x04: /* multimedia controller */
      is_io = 1;
      /*bridge */
      break;
    case 0x06: 
      /* not intel */
      if(vendor != 0x8086) 
        is_io = 1;
      break;
    case 0x09: /* input device */
      is_io = 1;
      break;
    case 0x0c: /* serial bus controller */
      /* firbre channel or usb */
      if(sclass == 0x03 || sclass == 0x04)
        is_io = 1;
      break;
    case 0x40: /* co processor */
      is_io = 1;
      break;
     }
  return is_io;
}

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
show_json_obj(struct device *d, int (*filter)(struct pci_dev *p))
{
    static int device_num = 0;
    struct version_item *vitemss[3]={NULL}, *vitems=NULL; /* 0= drv, 1=fwv, 2=optv*/
    #define MAX_BUFF 1024
    struct table_entry e;
    json_value *dev_obj;
    if(!filter(d->dev))
        return;
    
    dev_obj = json_object_new(0);
    memset(&e, 0, sizeof(struct table_entry));
    // Step 1 - pci address output
    set_slot_name(d, e.pci_addr);
    // Step  2 - phys slot 
    set_phy_slot(d, e.phy_slot);
    // Step 3 - card info 
    set_card_info(d, e.card_info);
    // Step 4 - vendor name
    set_vendor(d, e.vendor);
    // Step 5 - driver
    set_driver(d, e.driver);

    json_object_push(dev_obj, "PCI Address", json_string_new(e.pci_addr));
    json_object_push(dev_obj, "Slot #", json_string_new(e.phy_slot));
    json_object_push(dev_obj, "Card Info", json_string_new(e.card_info));
    json_object_push(dev_obj, "Vendor", json_string_new(e.vendor));
    json_object_push(dev_obj, "Driver", json_string_new(e.driver));
    // Step 6 - device info
    if (json > 1){
        set_dev_info(d, e.dev_info, sizeof(e.dev_info)/(sizeof(char)));
        json_object_push(dev_obj, "Device Info", json_string_new(e.dev_info));
    }
    // Step 7 - vendor id and device id
    if (json > 2){
    
        sprintf(e.ven_id, "%4.4x", d->dev->vendor_id);
        json_object_push(dev_obj, "Vendor ID", json_string_new(e.ven_id));
        sprintf(e.dev_id ,"%4.4x", d->dev->device_id);
        json_object_push(dev_obj, "Device ID", json_string_new(e.dev_id));
    }
    // Show versions
    if(json > 3){
        if (pci_read_driver_version(d->dev, &vitemss[DRV_ITEMS])) {
            json_value *drv_obj = json_object_new(0);
            struct version_item *vitem; 

            for(vitem = vitems = vitemss[DRV_ITEMS] ;vitem; vitem=vitem->next)
                json_object_push(drv_obj, basename(vitem->src_path), json_string_new(vitem->data));
            json_object_push(dev_obj, "Driver Versions", drv_obj);
            free_version_items(vitems);
        }
    }
    if(json > 4){
        if (pci_read_firmware_version(d->dev, &vitemss[FWV_ITEMS])){
            json_value *fwv_obj = json_object_new(0);
            struct version_item *vitem; 
            for(vitem = vitems = vitemss[FWV_ITEMS] ;vitem; vitem=vitem->next)
                json_object_push(fwv_obj, basename(vitem->src_path), json_string_new(vitem->data));
            json_object_push(dev_obj, "Firmware Versions", fwv_obj);
            free_version_items(vitems);
        }
    }
    if( json > 5 ){
       if (pci_read_option_rom_version(d->dev, &vitemss[OPTV_ITEMS])){
        json_value *optv_obj = json_object_new(0);
        struct version_item *vitem; 
        for(vitem = vitems = vitemss[OPTV_ITEMS] ;vitem; vitem=vitem->next)
            json_object_push(optv_obj, basename(vitem->src_path), json_string_new(vitem->data));
        json_object_push(dev_obj, "Option ROM Versions", optv_obj);
        free_version_items(vitems);
       }
    }

    json_serialize_opts settings;
    settings.mode = json_serialize_mode_multiline;
    settings.opts = json_serialize_opt_CRLF;
    settings.indent_size = 2;

    char * print = malloc(json_measure_ex(dev_obj, settings));
    json_serialize_ex(print, dev_obj, settings);

    printf("%s", print);
    json_builder_free(dev_obj);
    if(print)
        free(print);
    device_num++;
    if(device_num != num_io_devs){
        printf(",\n");
    }else{
        printf("\n");
    }
}

void 
show_table_entry(struct device *d, int (*filter)(struct pci_dev *p))
{
    char dev_info_num[DEV_INFO_NUM_SIZE];
    struct table_entry e;
    int pos; 
    struct version_item *vitemss[3]={NULL, NULL, NULL}, *vitems=NULL; /* 0= drv, 1=fwv, 2=optv*/
    memset(dev_info_num, 0, DEV_INFO_NUM_SIZE);
    memset(&e, 0, sizeof(struct table_entry));
    if(!filter(d->dev))
        return;

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
        set_dev_info(d, e.dev_info, DEVICE_INFO_SIZE);
        printf("\t%-40.40s",e.dev_info);
    }
    if (table > 2){

        sprintf(e.dev_id ,"%4.4x", d->dev->device_id);
        sprintf(e.ven_id ,"%4.4x", d->dev->vendor_id);
        sprintf(dev_info_num," [%s:%s]", e.ven_id, e.dev_id);
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
        printf("\t%-20.20s", e.dr_v);
    }
    if(table > 4){
        if (!pci_read_firmware_version(d->dev, &vitemss[FWV_ITEMS])){
            memset(e.fw_v, '.', 1);
        }else{
            vitems = vitemss[FWV_ITEMS];
            fill_vbuff(vitems, e.fw_v, FW_VERSION_SIZE);
            free_version_items(vitems);
        }
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
    if(table > 1)
        printf("\t%-40.40s", "Device_info");
    if(table > 3)
        printf("\t%-20.20s", "Driver_Version");
    if(table > 4)
        printf("\t%-20.20s", "Firmware_Version");
    if(table > 5)
        printf("\t%-20.20s", "Option_Rom_Version");

    printf("\n");
    for(i=0; i<line_width; i++)
        putchar('-');
    putchar('\n');
}

