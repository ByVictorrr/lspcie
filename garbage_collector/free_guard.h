#ifndef FREE_GUARD_H_
#define FREE_GUARD_H_
#include <stdio.h>
#include <stdlib.h>


struct free_guard{
    struct address_set *set;
};
void fg_setup(struct free_guard *g);
void fg_cleanup(struct free_guard *g);
void fg_free(struct free_guard *g, void *ptr);

#endif