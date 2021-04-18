#include "json-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>


#define PCI_FILTER_FIELDS 4

struct pci_filter{
    int vendor, device;
    int super_class, sub_class;
};
// TODO (2) - example of what you can do 

struct pci_filter_array{
    struct pci_filter *filters;
    int len;
};

/*
TODO:  check to see if malloc is returns an error
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
        free(arr->filters);

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
    static char error[ERROR_BUFF_MAX], *f_data, *field_name, *endptr;
    struct pci_filter *arr = NULL;
    int fd, f_len, i, j, len_array, num_of_fields;
    json_value *json_data;
    struct _json_value ** json_filters, *field; 
    json_settings settings = {0};
    settings.settings|=json_enable_comments;
    long int field_int; 
    


    if((fd=open(filter_file, O_RDONLY)) < 0){
        return "Unable to open the file submitted";
    }else if((f_len=lseek(fd, 0, SEEK_END)) < 0)
        return "Unable to read through the file submitted";
    else if ((f_data = (char*)mmap(0, f_len, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
        return "Not able to memory map your file";

    memset(error, 0, ERROR_BUFF_MAX);
    if(!(json_data = json_parse_ex(&settings, (const char*)f_data, f_len, error)))
        return error;
    else if (munmap(f_data, f_len) == -1)
        return "Unmapping your file failed";
    else if(close(fd) < 0)
        return "Unable to close your input file";
    
    
    json_filters = json_data->u.array.values;
    len_array = json_data->u.array.length; 


    arr=(struct pci_filter*)calloc(len_array, sizeof(struct pci_filter));
    memset(arr, -1, len_array*sizeof(struct pci_filter));
    for(i = 0; i < len_array; i++){
        // Parse data here
        num_of_fields = json_filters[i]->u.object.length;
        // condition could be && with values
        for(j=0; j < num_of_fields && j < PCI_FILTER_FIELDS; j++){
            field = json_filters[i]->u.object.values[j].value;
            field_name = json_filters[i]->u.object.values[j].name;
            
            field_int = strtol(field->u.string.ptr, &endptr, 16);
            if(field_int == 0 && field->u.string.ptr == endptr)
                return "Invalid field format";

            
            if (!strcmp(field_name,"vendor")){
                if(field_int < 0 || field_int > 0xffff) return "Infield_intid vendor ID";
                arr[i].vendor =  field_int; // #TODO: conversion to decimal
            }

            else if(!strcmp(field_name,"device")){
                if(field_int < 0 || field_int > 0xffff) return "Infield_intid device ID";
                arr[i].device = field_int; 
            }

            else if(!strcmp(field_name, "super_class")){
                if(field_int < 0 || field_int > 0xff) return "Infield_intid super class";
                arr[i].super_class = field_int;
            }
            else if(!strcmp(field_name, "sub_class")){
                if(field_int < 0 || field_int > 0xff) return "Infield_intid sub class";
                arr[i].sub_class = field_int; 
            }

        }
    }
    fa->filters = arr;
    fa->len =  len_array;
    return NULL;
}

        

int main(int argc, char **argv){

    struct pci_filter_array filters;
    char *msg = NULL;

    if(argc != 2){
        printf("Usage: %s <file_filter>", argv[0]);
        return EXIT_FAILURE;
    }
   
   pci_filter_array_init(NULL, &filters);
   if ((msg=pci_filter_array_parse_file(argv[1], &filters))){
       printf("%s",msg);
   }
   struct pci_filter *f;
   f=filters.filters;
   if(f){
        int i;
        for(i=0; i < filters.len; i++)
            printf(
                "(vID,dID,superC,subC) = (%d,%d,%d,%d)",
                f[i].vendor, f[i].device, f[i].super_class, f[i].sub_class
            );
   }
        
    return 0;
}