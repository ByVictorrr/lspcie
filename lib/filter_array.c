#include "json-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "pci.h"


#define PCI_FILTER_FIELDS 4


void pci_filter_array_init(struct pci_access *a, struct pci_filter_array *fa)
{
    fa->filters = NULL;
    fa->cmd_line_index = -1;
    fa->len = 0;
}
void pci_filter_array_delete(struct pci_filter_array *arr)
{
    int i;
    if(arr->len == 0)
        return;
    free(arr->filters);

}
char *pci_filter_array_parse_id(struct pci_filter_array *arr, char *str)
{
    struct pci_filter *cmd_line_f;
    if(arr->cmd_line_index != -1){
        cmd_line_f = &arr->filters[arr->cmd_line_index];
    }else if(arr->len == 0){
        arr->filters=malloc(sizeof(struct pci_filter));
        cmd_line_f = &arr->filters[arr->len];
        memset(cmd_line_f, -1, sizeof(struct pci_filter));
        arr->cmd_line_index = 0;
        arr->len++;
    }else{
        arr->filters=realloc(arr->filters, arr->len+1); 
        cmd_line_f = &arr->filters[arr->len];
        memset(cmd_line_f, -1, sizeof(struct pci_filter));
        arr->cmd_line_index = arr->len;
        arr->len++;
    }
    return pci_filter_parse_id(cmd_line_f, str);
}

char *pci_filter_array_parse_slot(struct pci_filter_array *arr, char *str)
{
    struct pci_filter *cmd_line_f;

    if(arr->cmd_line_index != -1){
        cmd_line_f = &arr->filters[arr->cmd_line_index];
    }else if(arr->len == 0){
        arr->filters=malloc(sizeof(struct pci_filter));
        cmd_line_f = &arr->filters[arr->len];
        memset(cmd_line_f, -1, sizeof(struct pci_filter));
        arr->cmd_line_index = arr->len;
        arr->len++;
    }else{
        arr->filters=realloc(arr->filters, arr->len+1); 
        cmd_line_f = &arr->filters[arr->len];
        memset(cmd_line_f, -1, sizeof(struct pci_filter));
        arr->cmd_line_index = arr->len;
        arr->len++;
    }
    return pci_filter_parse_slot(cmd_line_f, str);
}



// return error 
char *pci_filter_array_parse_file(struct pci_filter_array *fa, const char * filter_file)
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
    

    if((fd=open(filter_file, O_RDONLY)) < 0)
        return "Unable to open the file submitted";
    else if((f_len=lseek(fd, 0, SEEK_END)) < 0)
        return "Unable to read through the file submitted";
    else if ((f_data = (char*)mmap(0, f_len, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
        return "Not able to memory map your file";

    memset(error, 0, ERROR_BUFF_MAX);
    if(!(json_data = json_parse_ex(&settings, (const char*)f_data, f_len, error)))
        return error;
   
    
    json_filters = json_data->u.array.values;
    len_array = json_data->u.array.length; 


    arr=(struct pci_filter*)calloc(len_array, sizeof(struct pci_filter));
    memset(arr, -1, sizeof(struct pci_filter)*len_array);

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
            }else if(!strcmp(field_name,"device")){
                if(field_int < 0 || field_int > 0xffff) return "Infield_intid device ID";
                arr[i].device = field_int; 
            }else if(!strcmp(field_name, "super_class")){
                if(field_int < 0 || field_int > 0xff) return "Infield_intid super class";
                arr[i].super_class = field_int;
            }else if(!strcmp(field_name, "sub_class")){
                if(field_int < 0 || field_int > 0xff) return "Infield_intid sub class";
                arr[i].sub_class = field_int; 
            }else if(!strcmp(field_name, "domain")){
                if(field_int < 0 || field_int > 0xffff) return "Infield_int domain";
                arr[i].domain = field_int; 
            }else if(!strcmp(field_name, "bus")){
                if(field_int < 0 || field_int > 0xff) return "Infield_int bus";
                arr[i].bus = field_int; 
            }else if(!strcmp(field_name, "slot")){
                if(field_int < 0 || field_int > 32) return "Infield_int domain";
                arr[i].slot = field_int; 
            }else if(!strcmp(field_name, "func")){
                if(field_int < 0 || field_int > 8) return "Infield_int func";
                arr[i].func = field_int; 
            }

        }
    }
    fa->filters = arr;
    fa->len =  len_array;
    if (munmap(f_data, f_len) == -1)
        return "Unmapping your file failed";
    else if(close(fd) < 0)
        return "Unable to close your input file";
    
    return NULL;
}
int pci_filter_array_match(struct pci_filter_array *filters, struct pci_dev *p)
{
    int i;
    if(filters->len == 0)
        return 1;

    for(i=0; i < filters->len; i++)
        if(pci_filter_match(&filters->filters[i], p))
            return 1;

    return 0;
}        
int pci_filter_array_in(int val,
                        enum pci_filter_fields field, 
                        struct pci_filter_array *fils)
{
    int i, *cursor;
    cursor = fils->filters;
    struct pci_filter *filters = fils->filters;
    for(i=0; i< fils->len; i++){
        cursor = (int *)&filters[i];
        cursor += field;
        if(*cursor == val)
            return 1;
    }
    return 0;

}

/*
int main(int argc, char **argv){

    struct pci_filter_array filters;

    if(argc != 2){
        printf("Usage: %s <file_filter>", argv[0]);
        return EXIT_FAILURE;
    }
   
    
   pci_filter_array_parse_file(argv[1], &filters);

        
    return 0;
}
*/