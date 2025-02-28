#include "trap.h"
#include "print.h"
#include "debug.h"

void KMain(void){
    char* string = "Hello World!";
    int64_t value = 0x123456789ABCDE;

    init_idt();

    printk("%s\n", string);
    printk("This value is equal to %x", value);
    ASSERT(0);
}