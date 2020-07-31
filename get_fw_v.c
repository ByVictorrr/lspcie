#include "get_fw_v.h"
#include "lspci.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

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

struct erom_hdr ehdr;
uint64_t rom_bar;

void get_fw_v(struct device *first_d){

    struct device *d;
        
    /* For erom testing */
    int hdr_type;
    int rom_addr;
    int memfd;
    unsigned char *shmem;
    #define MIMO_LEN 18


    //int pci_read_block(struct pci_dev *, int pos, u8 *buf, int len) PCI_ABI;
    for(d=first_d; d; d=d->next){
        // dev->rom_base_addr (type = pciaddr_t = uint64)
        //show_rom(dev, PCI_ROM_ADDRESS);
        if((hdr_type = get_hdr_type(d)) == PCI_HEADER_TYPE_NORMAL){
            rom_addr=PCI_ROM_ADDRESS;
            memcpy(&rom_bar, d->config+rom_addr, sizeof(rom_bar));
            memfd = open("/dev/mem", O_RDWR);
            // map the range [MMIO_ADDR, MMIO_ADDR+MMIO_LEN] into your virtual address space
            shmem = mmap(0, MMIO_LEN, PROT_WRITE | PROT_READ, MAP_SHARED, memfd, rom_bar);            
            


        }else if (hdr_type == PCI_HEADER_TYPE_BRIDGE){
            rom_addr=PCI_ROM_ADDRESS1;
            memcpy(&rom_bar, d->config+rom_addr, sizeof(rom_bar));
            memfd = open("/dev/mem", O_RDWR);
            shmem = mmap(0, MMIO_LEN, PROT_WRITE | PROT_READ, MAP_SHARED, memfd, rom_bar);            

        }else if(hdr_type == PCI_HEADER_TYPE_CARDBUS){
            ;
        }else{
            ;
        }
    }




}