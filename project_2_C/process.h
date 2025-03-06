#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "trap.h"
#include "lib.h"

struct Process { //PCB: process control block, save in kernel space
    struct List* next;
    int pid; //process id
    int state; //status of process
    int wait;
    uint64_t context;//used to save rsp val when processess are switched
    uint64_t page_map; //save page_map level 4 table
    uint64_t stack; //stack used for kernel mode
    struct TrapFrame* tf; //
};

//structure used to set up stack ptr for ring 0
struct TSS {
    uint32_t res0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t res1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t res2;
    uint64_t res3;
    uint16_t iopb;
} __attribute__((packed));

struct ProcessControl {
    struct Process* current_process;
    struct HeadList ready_list;
    struct HeadList wait_list;
};

#define STACK_SIZE (2*1024*1024) //size of kernel stack 2mb
#define NUM_PROC 10 
//process states
#define PROC_UNUSED 0 
#define PROC_INIT 1
#define PROC_RUNNING 2
#define PROC_READY 3
#define PROC_SLEEP 4

void init_process(void);
void launch(void);
void pstart(struct TrapFrame* tf);
void swap(uint64_t* prev, uint64_t next);
void yield(void);
void sleep(int wait);
void wake_up(int wait);

#endif