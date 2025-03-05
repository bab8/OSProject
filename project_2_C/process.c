#include "process.h"
#include "trap.h"
#include "memory.h"
#include "print.h"
#include "lib.h"
#include "debug.h"

extern struct TSS Tss;
static struct Process process_table[NUM_PROC]; //define process_table with max number processess
static int pid_num = 1; // used to allocate new process with process id

static void set_tss(struct Process* proc){
    //assigns top of kernel stack to rsp0 in tss(jumps from ring3 to ring0)
    Tss.rsp0 = proc->stack + STACK_SIZE;
}

static struct Process* find_unused_process(void){
    struct Process* process = NULL;

    for(int i = 0; i < NUM_PROC; i++){
        if(process_table[i].state == PROC_UNUSED){
            process = &process_table[i];
            break;
        }
    }

    return process;
}

static void set_process_entry(struct Process* proc){
    uint64_t stack_top;

    //set process state to intialized and give process an id
    proc->state = PROC_INIT;
    proc->pid = pid_num++;

    //give process a kernel stack in mem and make sure the memory reserved is not 0
    proc->stack = (uint64_t)kalloc();
    ASSERT(proc->stack != 0);

    //zero teh stack memory and make stack_top pt to top of new process stack
    memset((void*)proc->stack, 0 , PAGE_SIZE);
    stack_top = proc->stack + STACK_SIZE;

    //init process trap frame, needed for jumping from ring3 to ring0 and returning
    //pts process tf to start of trapframe by utilizing stack_top
    proc->tf = (struct TrapFrame*)(stack_top - sizeof(struct TrapFrame));
    proc->tf->cs = 0x10|3;
    proc->tf->rip = 0x400000;
    proc->tf->ss = 0x18|3;
    proc->tf->rsp = 0x400000 + PAGE_SIZE;
    proc->tf->rflags = 0x202;

    //set up process pages for kernel and user mode
    proc->page_map = setup_kvm();
    ASSERT(proc->page_map != 0);
    ASSERT(setup_uvm(proc->page_map, (uint64_t)P2V(0x20000), 512*10));
}

void init_process(void){
    struct Process* proc = find_unused_process();
    ASSERT(proc == &process_table[0]);

    set_process_entry(proc);
}

void launch(void){
    set_tss(&process_table[0]);
    switch_vm(process_table[0].page_map);
    pstart(process_table[0].tf);
}
