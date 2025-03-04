#include "memory.h"
#include "print.h"
#include "debug.h"
#include "stddef.h"

static void free_region(uint64_t v, uint64_t r);

static struct FreeMemRegion free_mem_region[50]; //assure 50 blocks of free regions of memory
static struct Page free_memory;
static uint64_t memory_end;
extern char end; //provided by linker not defined here

void init_memory(void){
    int32_t count = *(int32_t*)0x9000; //number of memory regions stored here (done in loader file)
    uint64_t total_mem = 0;
    struct E820 *mem_map = (struct E820*)0x9008; //holds map retrieved by BIOS service E820
    int free_region_count = 0;

        //make assumption that number of memory regions is less than 50
    ASSERT(count <= 50);

    for(int i = 0; i < count; i++){
        //if memory type is 1 then that is a free memory region
        if(mem_map[i].type == 1){
            //free_mem_region struct acts as array of free memory regions
            free_mem_region[free_region_count].address = mem_map[i].address;
            free_mem_region[free_region_count].length = mem_map[i].length;
            //total mem will record how much memory the system can use
            total_mem += mem_map[i].length;
            free_region_count++;
        }

        printk("%x %uKB %u\n", mem_map[i].address,mem_map[i].length/1024,(uint64_t)mem_map[i].type);
    }

    for (int i = 0; i < free_region_count; i++){
        uint64_t vstart = P2V(free_mem_region[i].address);
        uint64_t vend = vstart + free_mem_region[i].length;

        //start of memory is larger than kernel
        if(vstart > (uint64_t)&end){
            free_region(vstart,vend);
        } 
        else if(vend > (uint64_t)&end){
            free_region((uint64_t)&end, vend);
        }
    }

    memory_end = (uint64_t)free_memory.next+PAGE_SIZE; //pts to last page we collect
    printk("%x\n",memory_end);
}

//divides region into 2mb pages and collects
static void free_region(uint64_t v, uint64_t e){
    //align page, compare with end, if in range call free function
    for(uint64_t start = PA_UP(v);start+PAGE_SIZE <= e; start += PAGE_SIZE){
        //check 1gb of base of kernel to see if page being initialized is beyond our 1gb of ram
        if(v+PAGE_SIZE <= 0xffff800040000000){
            kfree(start);
        }
    }
}

void kfree(uint64_t v){
    //make sure virtual addr is page aligned
    ASSERT(v % PAGE_SIZE == 0);
    //make sure virtual address is above kernel end
    ASSERT(v >= (uint64_t)&end);
    //make sure page address is within allowed ram
    ASSERT(v+PAGE_SIZE <= 0xffff800040000000);

    //free_memory is head of linked list,make head of listpt to current page
    struct Page *page_address = (struct Page*) v;
    page_address->next = free_memory.next;
    free_memory.next = page_address;
}

void* kalloc(void){
    struct Page *page_address = free_memory.next;

    //make sure page is valid
    if(page_address != NULL){
        //make sure virtual addr is page aligned
        ASSERT((uint64_t)page_address % PAGE_SIZE == 0);
        //make sure virtual address is above kernel end
        ASSERT((uint64_t)page_address >= (uint64_t)&end);
        //make sure page address is within allowed ram 
        ASSERT((uint64_t)page_address+PAGE_SIZE <= 0xffff800040000000);

        //move head of list pt to next page address
        free_memory.next = page_address->next;
    }

    //return page to caller
    return (void*)page_address;
}