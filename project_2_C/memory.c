#include "memory.h"
#include "print.h"
#include "debug.h"

static struct FreeMemRegion free_mem_region[50]; //assure 50 blocks of free regions of memory

void init_memory(void){
    int32_t count = *(int32_t*)0x9000; //number of memory regions stored here (done in loader file)
    uint64_t total_mem = 0;
    struct E820 *mem_map = (struct E820*)0x9008; //holds map retrieved by BIOS service E820
    int free_region_count = 0;

        //make assumption that number of memory regions is less than 50
    ASSERT(count <= 50);

    for(int i = 0; i < count; i++){
        //if memoryu type is 1 then that is a free memory region
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

    printk("Total memory is %uMB",total_mem/1024/1024);
}