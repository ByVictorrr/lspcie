#include "lspci.h"



struct pci_access *pacc;

int main(){

    struct pci_dev *dev;

    pacc = pci_alloc();
    
    pci_init(pacc);
    pci_scan_bus(pacc);
    u8 buff;

    //int pci_read_block(struct pci_dev *, int pos, u8 *buf, int len) PCI_ABI;
    for(dev=pacc->devices; dev; dev=dev->next){
        // dev->base_addr[6] (type = pciaddr_t<uint32> & in types.h)
        // [uint32][uint32]...6 times
        pci_read_block(dev, dev->rom_base_addr+?, &buff, 8);
        
    }




    return 0;

}