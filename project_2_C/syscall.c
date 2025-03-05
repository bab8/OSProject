#include "syscall.h"
#include "print.h"
#include "debug.h"
#include "stddef.h"

static SYSTEMCALL system_calls[10];

static int sys_write(int64_t *argptr){
    //argptr references data on stack in user mode, 0xe for yellow color text
    write_screen((char*)argptr[0], (int)argptr[1], 0xe);
    return (int)argptr[1];
}

void init_system_call(void){
    system_calls[0] = sys_write;
}

void system_call(struct TrapFrame* tf){
    //rax holds index of system call
    int64_t i = tf->rax;
    //rdi hols param count
    int64_t param_count = tf->rdi;
    //rsi holds params passed to function
    int64_t* argptr = (int64_t*)tf->rsi;

    //make sure requests are valid
    if(param_count < 0 || i != 0){
        tf->rax = -1;
        return;
    }

    ASSERT(system_calls[i] != NULL);
    tf->rax = system_calls[i](argptr);
}