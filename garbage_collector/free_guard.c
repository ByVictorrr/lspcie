#include <stdio.h>
#include <stdlib.h>
#include "free_guard.h"

struct address{
  struct address *next;
  struct address *prev;
  void *addr;
};
struct address_set{
  struct address *head;
  void (*insert)(struct address_set *set, void *addr);
  int (*in_set)(struct address_set *set, void *addr);
  void (*cleanup)(struct address_set *set);
};
static void 
address_set_cleanup(struct address_set *set){
    struct address *curr, *next;
    for(curr=set->head; curr; curr=next){
        next=curr->next;
        free(curr);
    }
}
static int 
address_set_in_set(struct address_set *set, void *addr){
  struct address *curr;
  for(curr=set->head; curr; curr=curr->next)
    if(curr->addr == addr)
      return 1;
  return 0;
}
static void 
address_set_insert(struct address_set *set, void *addr){
  struct address *prev, *tail;
  // Case 0 - check to see if addr in set and parms
  if(!addr || set->in_set(set, addr))
    return;
  // Case 1 - head has not been initalized
  if(set->head == NULL){
    set->head = calloc(1, sizeof(struct address));
    set->head->addr = addr;
    set->head->prev=NULL;
    set->head->next=NULL;
    return;
  }
  // Case 2 - head has been initalized (but what about the tail)
  if(set->head->prev == NULL){
    tail = calloc(1, sizeof(struct address));
    set->head->prev = set->head->next = tail; //tail
    tail->prev = set->head;
    tail->next = NULL; 
    return;
  }
  // Case 3 - at least two nodes in the set
  /* This means that we can get the tail 
  tail = (*head) ->prev

  */
  tail = calloc(1, sizeof(struct address));
  prev = set->head->prev; /* prev tail */
  prev->next = tail;
  tail->addr = addr;
  tail->prev = prev;
  tail->next = NULL;
  set->head->prev = tail;

}
void fg_setup(struct free_guard *g){
    g->set = calloc(1, sizeof(struct address_set));
    g->set->head = NULL;
    g->set->insert = address_set_insert;
    g->set->in_set = address_set_in_set;
    g->set->cleanup = address_set_cleanup;
}

void fg_cleanup(struct free_guard *g){
    g->set->cleanup(g->set); 
    free(g->set);
}

void fg_free(struct free_guard *g, void *ptr){
    if(!g->set->in_set(g->set, ptr)){
        g->set->insert(g->set, ptr);
        free(ptr);
    }
}
