#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>



struct pci_filter{
    int vendor, device;
    int super_class, sub_class;
};


#define PCI_FILTER_FIELDS 4
int main(int argc, char **argv){

    struct pci_filter **filters = NULL;

    if(argc != 2){
        printf("Usage: %s <file_filter>", argv[0]);
        return EXIT_FAILURE;
    }
    int fd = open(argv[1], O_RDONLY);
    int len = lseek(fd, 0, SEEK_END);
    char *data = (char*)mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
    json_value *json_data = json_parse(data, len);
    struct _json_value ** json_filters = json_data->u.object.values[1].value->u.array.values;
    int i, j, len_array = 2; // how do i get len_array

    filters=(struct pci_filter**)calloc(len_array, sizeof(struct pci_filter));

    for(i = 0; i < len_array; i++){
        filters[i]=(struct pci_filter*)calloc(1, sizeof(struct pci_filter));
        // Parse data here
        // condition could be && with values
        for(j=0; j < PCI_FILTER_FIELDS ; j++){
            struct _json_value *field = json_filters[i]->u.object.values[j].value;
            if(j ==0)
                filters[i]->vendor =  field->u.integer;
            if (j==1)
                filters[i]->device = field->u.integer;
            if (j==2)
                filters[i]->super_class = field->u.integer;
            if (j==3)
                filters[i]->sub_class = field->u.integer;

        }
    }

        
    
    

        
    return 0;
}