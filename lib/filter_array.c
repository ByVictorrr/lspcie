#include "json-parser.h"
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


/*
TODO: 
    1. Function error checking
    2. Find a way to return the length of the filter( output param  or wrapper struct with list and length)
    3. get rid of dynammically allocating int[4]  by just an array
    4.
*/
struct pci_access{
    int a;
};

void pci_filter_array_init(struct pci_access *a, struct pci_filter_array *fa)
{
    fa->filters = NULL;
    fa->len = 0;
}
void pci_filter_array_delete(struct pci_filter_array *arr)
{
    int i;
    if(arr->len == 0)
        return;
    
    for(i=0; i < arr->len; i++)
        free(arr->filters[i]);

}

struct pci_filter *pci_filter_alloc(){
    struct pci_filter *f;
    if((f = (struct pci_filter*)malloc(sizeof(struct pci_filter) ))){
        f->vendor = -1;
        f->device = -1;
        f->super_class = - 1;
        f->sub_class = -1;
        return f;
    }
    return NULL;
}


// return error 
char *pci_filter_array_parse_file(const char * filter_file, struct pci_filter_array *fa)
{

    #define ERROR_BUFF_MAX 20
    static char error[ERROR_BUFF_MAX], *f_data, *field_name;
    struct pci_filter **arr = NULL;
    int fd, f_len, i, j, len_array, num_of_fields;
    json_value *json_data;
    struct _json_value ** json_filters, *field; 
    json_settings settings = {0};
    

    if((fd=open(filter_file, O_RDONLY)) < 0)
        return "Unable to open the file submitted";
    else if((f_len=lseek(fd, 0, SEEK_END)) < 0)
        return "Unable to read through the file submitted";
    else if ((f_data = (char*)mmap(0, f_len, PROT_READ, MAP_PRIVATE, fd, 0)) < 0)
        return "Not able to memory map your file";

    memset(error, 0, ERROR_BUFF_MAX);
    if(!(json_data = json_parse_ex(&settings, (const char*)f_data, f_len, error)))
        return error;
    
    json_filters = json_data->u.object.values[1].value->u.array.values;
    len_array = json_data->u.object.length; 


    arr=(struct pci_filter**)calloc(len_array, sizeof(struct pci_filter));

    for(i = 0; i < len_array; i++){
        arr[i]=pci_filter_alloc();

        // Parse data here
        num_of_fields = json_filters[i]->u.object.length;
        // condition could be && with values
        for(j=0; j < num_of_fields && j < PCI_FILTER_FIELDS; j++){
            field = json_filters[i]->u.object.values[j].value;
            field_name = json_filters[i]->u.object.values[j].name;
            // json_filters[i]->u.object.values[j].<name, name_len> = key, string length
            
            if (strcmp(field_name,"vendor") == 0)
                arr[i]->vendor =  field->u.integer; // #TODO: conversion to decimal
            else if(strcmp(field_name,"device") == 0)
                arr[i]->device = field->u.integer;

            else if(strcmp(field_name, "super_class") == 0)
                arr[i]->super_class = field->u.integer;

            else if(strcmp(field_name, "sub_class") == 0)
                arr[i]->sub_class = field->u.integer;

        }
    }
    fa->filters = arr;
    fa->len =  len_array;
    /* CLEANUP : close mmap and fd*/
    return NULL;
}
        

int main(int argc, char **argv){

    struct pci_filter_array filters;

    if(argc != 2){
        printf("Usage: %s <file_filter>", argv[0]);
        return EXIT_FAILURE;
    }
   
    
   pci_filter_array_parse_file(argv[1], &filters);

        
    return 0;
}