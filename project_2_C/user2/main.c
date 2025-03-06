#include "lib.h"
#include "stdint.h"

int main(void){
    printf("process2\n");
    sleepu(100);//roughly 1 second becuase of 10ms int timer
    return 0;
}