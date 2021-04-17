#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>


#define PCI_FILTER_FIELDS 4

struct pci_filter{
    int vendor, device;
    int super_class, sub_class;
};
// TODO (2) - example of what you can do 

struct pci_filter_list{
    struct pci_filter **list;
    int len;
};

/*
TODO: 
    1. Function error checking
    2. Find a way to return the length of the filter( output param  or wrapper struct with list and length)
    3. get rid of dynammically allocating int[4]  by just an array
    4.
*/
void build_filters(const char * filter_file, struct pci_filter_list *filters)
{

   struct pci_filter **list = NULL;
    int fd = open(filter_file, O_RDONLY);
    int len = lseek(fd, 0, SEEK_END);
    char *data = (char*)mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
    json_value *json_data = json_parse((const char*)data, len);
    struct _json_value ** json_filters = json_data->u.object.values[1].value->u.array.values;
    int i, j, len_array = json_data->u.object.length; // how do i get len_array

    list=(struct pci_filter**)calloc(len_array, sizeof(struct pci_filter));

    for(i = 0; i < len_array; i++){
        list[i]=(struct pci_filter*)malloc(sizeof(struct pci_filter));
        list[i]->vendor = -1;
        list[i]->device = -1;
        list[i]->super_class = - 1;
        list[i]->sub_class = -1;

        // Parse data here
        int field_len = json_filters[i]->u.object.length;
        // condition could be && with values
        for(j=0; j < field_len && j < PCI_FILTER_FIELDS; j++){
            struct _json_value *field = json_filters[i]->u.object.values[j].value;
            const char *name = json_filters[i]->u.object.values[j].name;
            // json_filters[i]->u.object.values[j].<name, name_len> = key, string length
            
            if (strcmp(name,"vendor") == 0)
                list[i]->vendor =  field->u.integer; // #TODO: conversion to decimal
            else if(strcmp(name,"device") == 0)
                list[i]->device = field->u.integer;

            else if(strcmp(name, "super_class") == 0)
                list[i]->super_class = field->u.integer;

            else if(strcmp(name, "sub_class") == 0)
                list[i]->sub_class = field->u.integer;

        }
    }
    filters->list = list;
    filters->len =  len_array;

}
        

int main(int argc, char **argv){

    struct pci_filter_list filters;

    if(argc != 2){
        printf("Usage: %s <file_filter>", argv[0]);
        return EXIT_FAILURE;
    }
   
    
   build_filters(argv[1], &filters);

        
    return 0;
}