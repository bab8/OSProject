#include "lib.h"
#include "stdint.h"

int main(void){


    //attempt to acces kernel space to test exception
    char *p = (char*)0xffff800000200200;

    *p = 1;
    printf("process2\n");
    sleepu(100);//roughly 1 second becuase of 10ms int timer
    return 0;
}