#include "get_fw_v.h"
#include "lspci.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



static int get_hdr_type(const struct device * d){
    switch((d->config[PCI_HEADER_TYPE] & 0x7f)){
        case PCI_HEADER_TYPE_NORMAL:
            return 0;
            break;
        case PCI_HEADER_TYPE_BRIDGE:
            return 1;
            break;
        case PCI_HEADER_TYPE_CARDBUS:
            return 2;
            break;
        default:
            return -1;
            break;
    }
    return -1;
}

int memfd;
struct erom_data edata;

static 
long get_page_size(){

    long page_size;
    if((page_size = sysconf(_SC_PAGE_SIZE))==-1){
        printf(stderr, "Wrong variable name for sysconf");
    }
    return page_size;
}
static 
uint64_t align_offset(uint64_t phy_addr){
    long page_size;
    uint64_t aligned;
    if((page_size=get_page_size())==-1){
        return page_size;
    }
    // else align
    aligned = ((uint64_t)(phy_addr/page_size))*page_size;

    return aligned;
}
static 
void read_erom_data(struct erom_data * dest, uint64_t rom_bar){
    unsigned char *shmem;
    shmem = mmap(0, sizeof(*dest), PROT_READ, MAP_SHARED, memfd, rom_bar);            
    memcpy(dest, &shmem, sizeof(*dest));
}


void get_fw_v(struct device *first_d){

    struct device *d;
        
    /* For erom testing */
    int hdr_type;
    uint64_t rom_hdr_addr, rom_bar, aligned;


    
    if((memfd = open("/dev/mem", O_RDWR))==-1){
        return;
    }
    for(d=first_d; d; d=d->next){
        if((hdr_type = get_hdr_type(d)) == PCI_HEADER_TYPE_NORMAL){
            rom_hdr_addr=PCI_ROM_ADDRESS;
            memcpy(&rom_bar, &d->config[rom_hdr_addr], sizeof(rom_bar));
            if ((aligned = align_offset(rom_bar)) == - 1){
                continue;
            }

            read_erom_data(&edata, aligned);
            
        }else if (hdr_type == PCI_HEADER_TYPE_BRIDGE){
            rom_hdr_addr=PCI_ROM_ADDRESS1;
            memcpy(&rom_bar, &d->config[rom_hdr_addr], sizeof(rom_bar));
            if ((aligned = align_offset(rom_bar)) == - 1){
                continue;
            }

            read_erom_data(&edata, aligned);

        }else if(hdr_type == PCI_HEADER_TYPE_CARDBUS){
            ;
        }else{
            ;
        }
    }




}