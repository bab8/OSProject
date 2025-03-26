#include "memory.h"
#include "print.h"
#include "debug.h"
#include "lib.h"
#include "stddef.h"
#include "stdbool.h"

static void free_region(uint64_t v, uint64_t r);

static struct FreeMemRegion free_mem_region[50]; //assure 50 blocks of free regions of memory
static struct Page free_memory;
static uint64_t memory_end;
uint64_t total_mem;
extern char end; //provided by linker not defined here

void init_memory(void){
    int32_t count = *(int32_t*)0x20000; //number of memory regions stored here (done in loader file)
    struct E820 *mem_map = (struct E820*)0x20008; //holds map retrieved by BIOS service E820
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

uint64_t get_total_memory(void){
    return total_mem/1024/1024;
}

//divides region into 2mb pages and collects
static void free_region(uint64_t v, uint64_t e){
    //align page, compare with end, if in range call free function
    for(uint64_t start = PA_UP(v);start+PAGE_SIZE <= e; start += PAGE_SIZE){
        //check 1gb of base of kernel to see if page being initialized is beyond our 1gb of ram
        if(start+PAGE_SIZE <= 0xffff800030000000){
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
    ASSERT(v+PAGE_SIZE <= 0xffff800030000000);

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
        ASSERT((uint64_t)page_address+PAGE_SIZE <= 0xffff800030000000);

        //move head of list pt to next page address
        free_memory.next = page_address->next;
    }

    //return page to caller
    return (void*)page_address;
}

//next 2 functions cover finding next table for page, page directory ptr and page directory
static PDPTR find_pml4t_entry(uint64_t map, uint64_t v, int alloc, uint32_t attribute){
    PDPTR* map_entry = (PDPTR*)map;
    PDPTR pdptr = NULL;
    unsigned int index = (v >> 39) & 0x1FF;

    if((uint64_t)map_entry[index] & PTE_P){
        pdptr = (PDPTR)P2V(PDE_ADDR(map_entry[index]));
    }
    else if(alloc == 1){
        pdptr = (PDPTR)kalloc();
        if(pdptr != NULL){
            memset(pdptr,0,PAGE_SIZE);
            map_entry[index] = (PDPTR)(V2P(pdptr) | attribute);
        }
    }

    return pdptr;
}

static PD find_pdpt_entry(uint64_t map, uint64_t v, int alloc, uint32_t attribute){
    PDPTR pdptr = NULL;
    PD pd = NULL;
    unsigned int index = (v >> 30) & 0x1FF;

    pdptr = find_pml4t_entry(map,v,alloc,attribute);
    if(pdptr == NULL){
        return NULL;
    }
    if((uint64_t)pdptr[index] & PTE_P){
        pd = (PD)P2V(PDE_ADDR(pdptr[index]));
    }
    else if(alloc == 1){ //create page if dne, denoted by no present bit
        pd = kalloc();
        if(pd != NULL){
            memset(pd,0,PAGE_SIZE);
            pdptr[index] = (PD)(V2P(pd) | attribute);
        }
    }

    return pd;
}

bool map_pages(uint64_t map, uint64_t v, uint64_t e, uint64_t pa, uint32_t attribute){
    //save aligned vaddr
    uint64_t vstart = PA_DOWN(v);
    uint64_t vend = PA_UP(e);
    //use to set page directory entry
    PD pd = NULL;
    unsigned int index;

    ASSERT(v < e);
    ASSERT(pa % PAGE_SIZE == 0);
    //check if end of physical addr is outside of 1gb of mem
    ASSERT(pa+vend-vstart <= 1024*1024*1024);

    do {
        //find page directory ptr table entry wwhich pts to page directory table
        pd = find_pdpt_entry(map, vstart,1,attribute);
        if(pd == NULL){
            return false;
        }

        //index locates correct page entry
        index = (vstart >> 21) & 0x1FF;
        //check present bit
        ASSERT(((uint64_t)pd[index] & PTE_P) == 0);

        //set entry w/ paddr and attributes
        pd[index] = (PDE)(pa | attribute | PTE_ENTRY);

        //move to next page until out of region
        vstart += PAGE_SIZE;
        pa += PAGE_SIZE;
    }while(vstart + PAGE_SIZE <= vend);

    return true;
}

//load cr3 with new translation table for paging
void switch_vm(uint64_t map){
    load_cr3(V2P(map));
}

//remap kernel with 2mb pages
uint64_t setup_kvm(void){
    uint64_t page_map = (uint64_t)kalloc();
    if(page_map != 0){
        memset((void*)page_map, 0, PAGE_SIZE);
        //pass page map, give start vaddr of KERNEL BASE, end vaddr of memory end, give physical adr of kernel, give attributes present and writable but not user
        if(!map_pages(page_map, KERNEL_BASE, P2V(0x40000000), V2P(KERNEL_BASE), PTE_P|PTE_W)){
            free_vm(page_map);
            page_map = 0;
        }
    }
    return page_map;
}

void init_kvm(void){
    uint64_t page_map = setup_kvm();
    ASSERT(page_map != 0);
    switch_vm(page_map);
    printk("memory manager is now working\n");
}

bool setup_uvm(uint64_t map, uint64_t start, int size){
    bool status = false;
    void* page = kalloc();

    if(page != NULL){
        memset(page, 0 , PAGE_SIZE);
        status = map_pages(map, 0x400000, 0x400000+PAGE_SIZE, V2P(page), PTE_P | PTE_W | PTE_U);
        if(status == true){
            memcpy(page, (void*)start, size);
        } else{
            kfree((uint64_t)page);
            free_vm(map);
        }
    }

    return status;
}

void free_pages(uint64_t map, uint64_t vstart, uint64_t vend){
    unsigned int index;

    ASSERT(vstart % PAGE_SIZE == 0);
    ASSERT(vend % PAGE_SIZE == 0);

    do{
        PD pd = find_pdpt_entry(map,vstart,0,0);

        if(pd != NULL){
            index = (vstart >> 21) & 0x1FF;
            if(pd[index] & PTE_P){
                kfree(P2V(PTE_ADDR(pd[index])));
                pd[index] = 0;
            }
        }

        vstart += PAGE_SIZE;
    }while(vstart+PAGE_SIZE <= vend);
}

static void free_pdt(uint64_t map){
    PDPTR* map_entry = (PDPTR*)map;

    for(int i = 0; i < 512; i++){
        if((uint64_t)map_entry[i] & PTE_P){
            PD* pdptr = (PD*)P2V(PDE_ADDR(map_entry[i]));

            for(int j = 0; j < 512; j++){
                if((uint64_t)pdptr[j] & PTE_P){
                    kfree(P2V(PDE_ADDR(pdptr[j])));
                    pdptr[j] = 0;
                }
            }
        }   
    }
}

static void free_pdpt(uint64_t map){
    PDPTR* map_entry = (PDPTR*)map;

    for(int i = 0; i < 512; i++){
        if((uint64_t)map_entry[i] & PTE_P){
            kfree(P2V(PDE_ADDR(map_entry[i])));
            map_entry[i] = 0;
        }
    }
}

static void free_pml4t(uint64_t map){
    kfree(map);
}

void free_vm(uint64_t map){ 
    free_pages(map,0x400000,0x400000+PAGE_SIZE);
    free_pdt(map);
    free_pdpt(map);
    free_pml4t(map);
}

bool copy_uvm(uint64_t dst_map, uint64_t src_map, int size){
    bool status = false;
    unsigned int index;
    PD pd = NULL;
    uint64_t start;

    void* page = kalloc();
    if(page != NULL){
        memset(page,0 , PAGE_SIZE);
        status = map_pages(dst_map,0x400000, 0x400000+PAGE_SIZE, V2P(page), PTE_P|PTE_W|PTE_U);

        if(status == true){
            pd = find_pdpt_entry(src_map, 0x400000, 0 ,0);
            if(pd == NULL){
                free_vm(dst_map);
                return false;
            }

            index = (0x400000U >> 21) & 0x1FF;
            ASSERT(((uint64_t)pd[index] & PTE_P) == 1);
            start = P2V(PTE_ADDR(pd[index]));
            memcpy(page,(void*)start, size);
        }else{
            kfree((uint64_t)page);
            free_vm(dst_map);
        }    
    }

    return status;
}