#include "lib.h"
#include "stdint.h"

int main(void){
    int64_t counter = 0;

    while(1){
        printf("process2 %d\n",counter);
        sleepu(100);//roughly 1 second becuase of 10ms int timer
    }
    return 0;
}