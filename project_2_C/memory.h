#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "stdint.h"

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

#define PAGE_SIZE (2*1024*1024) //2mb pages
#define PA_UP(v) ((((uint64_t)v+PAGE_SIZE-1)>>21)<<21)//align address to 2 next mb boundary
#define PA_DOWN(v) (((uint64_t)v>>21)<<21)//align address to previous 2mb boundary
#define P2V(p) ((uint64_t)(p) + 0xffff800000000000)//physical to virtual(this works because our kernel space lines up the same way it did when it was physcial address so we just have to add an offset of the virtual address)
#define V2P(v) ((uint64_t)(v) - 0xffff800000000000)//virtual to physical

void init_memory(void);
void* kalloc(void);
void kfree(uint64_t v);

#endif