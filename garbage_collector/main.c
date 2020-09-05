#include <stdio.h>
#include <stdlib.h>
#include "free_guard.h"

int main(){
     void *test[4] = {malloc(1),malloc(1),malloc(1),NULL};
     int i=0;
     test[3] = test[0];
     struct free_guard fg;
     fg_setup(&fg);
     for(i=0; i < 4; i++){
        fg_free(&fg, test[i]);
     }
     fg_cleanup(&fg);

}