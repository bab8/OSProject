#include "process.h"
#include "trap.h"
#include "memory.h"
#include "print.h"
#include "lib.h"
#include "debug.h"

extern struct TSS Tss;
static struct Process process_table[NUM_PROC]; //define process_table with max number processess
static int pid_num = 1; // used to allocate new process with process id
static struct ProcessControl pc;

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

static struct Process* alloc_new_process(void){
    uint64_t stack_top;
    struct Process* proc;

    proc = find_unused_process();
    //no process available
    if(proc == NULL){
        return NULL;
    }
    //set process state to intialized and give process an id
    proc->state = PROC_INIT;
    proc->pid = pid_num++;

    //give process a kernel stack in mem and make sure the memory reserved is not 0
    proc->stack = (uint64_t)kalloc();
    if(proc->stack == 0){
        return NULL;
    }

    //zero teh stack memory and make stack_top pt to top of new process stack
    memset((void*)proc->stack, 0 , PAGE_SIZE);
    stack_top = proc->stack + STACK_SIZE;

    //init the context for the process so first switch returns to correct location
    proc->context = stack_top - sizeof(struct TrapFrame) - 7*8;
    //6*8, represents the amount of registers saved
    *(uint64_t*)(proc->context + 6*8) = (uint64_t)TrapReturn;

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
    if(proc->page_map == 0){
        kfree(proc->stack);
        memset(proc, 0, sizeof(struct Process));
        return NULL;
    }
    return proc;
}

struct ProcessControl* get_pc(void){
    return &pc;
}

static void init_idle_process(void){
    struct ProcessControl* process_control;
    struct Process* process;

    process = find_unused_process();
    ASSERT(process == &process_table[0]);

    process->pid = 0;
    process->page_map = P2V(read_cr3());
    process->state = PROC_RUNNING;

    process_control = get_pc();
    process_control->current_process = process;
}

static void init_user_process(void){
    struct ProcessControl* process_control;
    struct Process* process;
    struct HeadList* list;

    process_control = get_pc();
    list = &process_control->ready_list;

    process = alloc_new_process();
    ASSERT(process != NULL);

    ASSERT(setup_uvm(process->page_map, P2V(0x30000), 5120));

    process->state = PROC_READY;
    append_list_tail(list, (struct List*)process);
}


void init_process(void){
    //OLD CODE FROM BEFORE THE FILE SYSTEM WAS SETUP AND KERNEL WAS MOVED INTO HARD DISK
    // struct ProcessControl* process_control;
    // struct Process* process;
    // struct HeadList* list;
    // //set addresses of processess
    // uint64_t addr[3] = {0x20000, 0x30000, 0x40000};

    // process_control = get_pc();
    // list = &process_control->ready_list;

    // //set processess to ready state and add them to list
    // for(int i = 0; i < 3; i++){
    //     process = find_unused_process();
    //     set_process_entry(process,addr[i]);
    //     append_list_tail(list, (struct List*)process);
    // }

    init_idle_process();
    init_user_process();
}

//unused function when we have the idel process
// void launch(void){
//     struct ProcessControl *process_control;
//     struct Process* process;

//     process_control = get_pc();
//     process = (struct Process*)remove_list_head(&process_control->ready_list);
//     process->state = PROC_RUNNING;
//     process_control->current_process = process;


//     set_tss(process);
//     switch_vm(process->page_map);
//     pstart(process->tf);
// }


static void switch_process(struct Process* prev, struct Process* current){
    set_tss(current);
    switch_vm(current->page_map);
    swap(&prev->context, current->context);
}

static void schedule(void){
    struct Process* prev_proc;
    struct Process* current_proc;
    struct ProcessControl* process_control;
    struct HeadList* list;

    process_control = get_pc();
    prev_proc = process_control->current_process;
    list = &process_control->ready_list;
    if(is_list_empty(list)){
        //if there are no ready processess switch to idle process
        ASSERT(process_control->current_process->pid != 0);
        current_proc = &process_table[0];
    } else{
        //else remove the next process from ready list
        current_proc = (struct Process*)remove_list_head(list);
        
    }
    current_proc->state = PROC_RUNNING;
    process_control->current_process = current_proc;
    
    
    switch_process(prev_proc, current_proc);
}

void yield(void){
    struct ProcessControl* process_control;
    struct Process* process;
    struct HeadList* list;
    
    process_control = get_pc();
    list = &process_control->ready_list;

    if(is_list_empty(list)){
        return;
    }

    process = process_control->current_process;
    process->state = PROC_READY;

    if(process->pid != 0){
        append_list_tail(list, (struct List*)process);
    }
    schedule();
}

void sleep(int wait){
    struct ProcessControl* process_control;
    struct Process* process;

    process_control = get_pc();
    process = process_control->current_process;
    process->state = PROC_SLEEP;
    process->wait = wait;

    append_list_tail(&process_control->wait_list, (struct List*)process);
    schedule();
}

void wake_up(int wait){
    struct ProcessControl* process_control;
    struct Process* process;
    struct HeadList* ready_list;
    struct HeadList* wait_list;

    process_control = get_pc();
    ready_list = &process_control->ready_list;
    wait_list = &process_control->wait_list;
    process = (struct Process*)remove_list(wait_list,wait);

    while(process != NULL){
        process->state = PROC_READY;
        append_list_tail(ready_list, (struct List*)process);
        process = (struct Process*)remove_list(wait_list,wait);
    }
}

void exit(void){
    struct ProcessControl* process_control;
    struct Process* process;
    struct HeadList* list;

    process_control = get_pc();
    process = process_control->current_process;
    process->state = PROC_KILLED;

    list = &process_control->kill_list;
    append_list_tail(list,(struct List*)process);

    //process 1 will kill processes and could be asleep so we wake it up with its pid
    wake_up(1);
    schedule();
}

void wait(void){
    struct ProcessControl* process_control;
    struct Process* process;
    struct HeadList* list;

    process_control = get_pc();
    list = &process_control->kill_list;

    while(1){
        if(!is_list_empty(list)){
            process = (struct Process*)remove_list_head(list);
            ASSERT(process->state == PROC_KILLED);

            kfree(process->stack);
            free_vm(process->page_map);
            memset(process,0,sizeof(struct Process));
        }else{
            //1 is pid of the process
            sleep(1);
        }
    }
}