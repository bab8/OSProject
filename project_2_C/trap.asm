section .text
extern handler; define interrupt vectors, some are reserved such as 9,15
global vector0; divide by zero exception
global vector1; debug interrupt
global vector2
global vector3
global vector4
global vector5
global vector6
global vector7
global vector8
global vector10
global vector11
global vector12
global vector13
global vector14
global vector16
global vector17
global vector18
global vector19
global vector32
global vector39
global eoi
global read_isr
global load_idt
global load_cr3

Trap:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    inc byte[0xb8010]
    mov byte[0xb8011], 0xe

    mov rdi,rsp; first argument passed, arguement is stack ptr
    call handler

TrapReturn:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp,16; adjust rsp to make it point to correct location, 16 bc of 2 push commands in vectors
    iretq

vector0:
    push 0; error code
    push 0; index value to know which interrupt
    jmp Trap

vector1:
    push 0; error code
    push 1; index value to know which interrupt
    jmp Trap

vector2:
    push 0; error code
    push 2; index value to know which interrupt
    jmp Trap

vector3:
    push 0; error code
    push 3; index value to know which interrupt
    jmp Trap

vector4:
    push 0; error code
    push 4; index value to know which interrupt
    jmp Trap

vector5:
    push 0; error code
    push 5; index value to know which interrupt
    jmp Trap

vector6:
    push 0; error code
    push 6; index value to know which interrupt
    jmp Trap

vector7:
    push 0; error code, zero signifes no err code so we need to push smth instead of the stack to represent it
    push 7; index value to know which interrupt
    jmp Trap

vector8:
    push 8; index value to know which interrupt
    jmp Trap

vector10:
    push 10; index value to know which interrupt
    jmp Trap

vector11:
    push 11; index value to know which interrupt
    jmp Trap

vector12:
    push 12; index value to know which interrupt
    jmp Trap

vector13:
    push 13; index value to know which interrupt
    jmp Trap

vector14:
    push 14; index value to know which interrupt
    jmp Trap

vector16:
    push 0; error code
    push 16; index value to know which interrupt
    jmp Trap

vector17:
    push 17; index value to know which interrupt
    jmp Trap

vector18:
    push 0; error code
    push 18; index value to know which interrupt
    jmp Trap

vector19:
    push 0; error code
    push 19; index value to know which interrupt
    jmp Trap

vector32:
    push 0; error code
    push 32; index value to know which interrupt
    jmp Trap

vector39:
    push 0; error code
    push 39; index value to know which interrupt
    jmp Trap

eoi:
    mov al, 0x20; set bit 5
    out 0x20,al; write command to parent chip
    ret

read_isr:
    mov al,11; 11 = 00001011 bit 3 is 1 to identify command to read IRR or ISR register, bits 0 and 1 are set to indicate reading ISR register
    out 0x20,al; write command to parent chip
    in al,0x20; read command 
    ret

load_idt:
    lidt [rdi]; load idt with passed parameter
    ret

load_cr3:
    mov rax,rdi
    mov cr3,rax
    ret