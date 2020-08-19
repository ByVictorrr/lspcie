#include <stdlib.h>
#include <string.h>

struct dev{
    char *src;
    struct dev *next;
};
void set_src(struct dev *d){
    d->src=strdup("hi");
}

int main(){
    struct dev *head, *curr;
    head=malloc(sizeof(struct dev));
    head->next=malloc(sizeof(struct dev));
    for(curr=head; curr; curr=curr->next)
        set_src(curr);
    return 0;
}