#include "print.h"
#include "file.h"
#include "debug.h"

void EMain(void){
    //initialize file system
    init_fs();
    
    //load kernel file
    ASSERT(load_file("KERNEL.BIN", 0x200000) == 0);

    //load userfile
    ASSERT(load_file("USER.BIN", 0x30000) == 0);
    
}