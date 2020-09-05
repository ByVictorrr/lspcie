#include <stdio.h>
#include <stdlib.h>

struct address{
  struct address *next;
  struct address *prev;
  void *addr;
};
struct address_set{
  struct address *head;
  void (*insert)(struct address_set *set, void *addr);
  int (*in_set)(struct address_set *set, void *addr);
  void (*clean_up)(struct address_set *set);
};
void clean_up(struct address_set *set){
    struct address *curr, *next;
    for(curr=set->head; curr; curr=next){
        next=curr->next;
        free(curr);
    }
}
int in_set(struct address_set *set, void *addr){
  struct address *head = set->head;
  struct address *curr;
  if(head == NULL)
    return 0;

  for(curr=head; curr; curr=curr->next){
    if(curr->addr == addr)
      return 1;
  }
  return 0;
}
void insert(struct address_set *set, void *addr){
  struct address *prev, *tail;
  // Case 0 - check to see if addr in set and parms
  if(!set || !addr || in_set(set, addr))
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
int main(){
     void *test[4] = {malloc(1),malloc(1),malloc(1),NULL};
     int i=0;
     test[3] = test[0];
     struct address_set set = {NULL, insert, in_set, clean_up};
     for(i=0; i < 4; i++){
        set.insert(&set, test[i]);
     }
     set.clean_up(&set);

}