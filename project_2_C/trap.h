#ifndef _TRAP_H_//guard so that header file is only defined once
#define _TRAP_H_

#include "stdint.h"

struct IdtEntry {
    uint16_t low; //first 2 bytes is lower 16 bits of offset
    uint16_t selector; //selector stored in next 2 bytes
    uint8_t res0;//reserved field, 1 byte
    uint8_t attr;//attribute, 1 byte
    uint16_t mid;//2 bytes for middle of offset
    uint32_t high;//4 bytes for high part of offset
    uint32_t res1;// 4 bytes reserved space
};

struct IdtPtr{
    uint16_t limit;//IDT limit
    uint64_t addr;//IDTaddress
} __attribute__((packed));//add packed so it is natural alignment and ensure no padding is added

struct TrapFrame {
    int64_t r15;
    int64_t r14;
    int64_t r13;
    int64_t r12;
    int64_t r11;
    int64_t r10;
    int64_t r9;
    int64_t r8;
    int64_t rbp;
    int64_t rdi;
    int64_t rsi;
    int64_t rdx;
    int64_t rcx;
    int64_t rbx;
    int64_t rax;//registers
    int64_t trapno;
    int64_t errorcode;
    int64_t rip;
    int64_t cs;
    int64_t rflags;
    int64_t rsp;
    int64_t ss;
};

void vector0(void);
void vector1(void);
void vector2(void);
void vector3(void);
void vector4(void);
void vector5(void);
void vector6(void);
void vector7(void);
void vector8(void);
void vector10(void);
void vector11(void);
void vector12(void);
void vector13(void);
void vector14(void);
void vector16(void);
void vector17(void);
void vector18(void);
void vector19(void);
void vector32(void);
void vector39(void);
void sysint(void);
void init_idt(void);
void eoi(void);
void load_idt(struct IdtPtr *ptr);
unsigned char read_isr(void);
uint64_t read_cr2(void);
void TrapReturn(void);
uint64_t get_ticks(void);

#endif