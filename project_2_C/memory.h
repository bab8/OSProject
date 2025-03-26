#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

struct E820 {
    uint64_t address;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));//no padding

struct FreeMemRegion {
    uint64_t address;
    uint64_t length;
};

struct Page {
    struct Page* next;
};

typedef uint64_t PDE;// pts to page directory entry
typedef PDE* PD;//holds page directory
typedef PD* PDPTR;//holds page directory ptr

//attributes of table entries, set bits 7,2,1,0 in table entries
#define PTE_P 1//present
#define PTE_W 2//write
#define PTE_U 4//user
#define PTE_ENTRY 0x80//mark 2mb page 

#define KERNEL_BASE 0xffff800000000000// kernel virtual addr


#define PAGE_SIZE (2*1024*1024) //2mb pages
#define PA_UP(v) ((((uint64_t)v+PAGE_SIZE-1)>>21)<<21)//align address to 2 next mb boundary
#define PA_DOWN(v) (((uint64_t)v>>21)<<21)//align address to previous 2mb boundary
#define P2V(p) ((uint64_t)(p) + KERNEL_BASE)//physical to virtual(this works because our kernel space lines up the same way it did when it was physcial address so we just have to add an offset of the virtual address)
#define V2P(v) ((uint64_t)(v) - KERNEL_BASE)//virtual to physical
#define PDE_ADDR(p) (((uint64_t)p >> 12) << 12)//achieve next level page addr
#define PTE_ADDR(p) (((uint64_t)p >> 21) << 21)//achieve physical page addr

void* kalloc(void);
void kfree(uint64_t v);
void init_memory(void);
void init_kvm(void);
bool map_pages(uint64_t map, uint64_t v, uint64_t e, uint64_t pa, uint32_t attribute);
void load_cr3(uint64_t map);
void switch_vm(uint64_t map);
void free_vm(uint64_t map);
void free_pages(uint64_t map, uint64_t vstart, uint64_t vend);
bool setup_uvm(uint64_t map, uint64_t start, int size);
uint64_t setup_kvm(void);
uint64_t get_total_memory(void);
bool copy_uvm(uint64_t dst_map, uint64_t src_map, int size);


#endif