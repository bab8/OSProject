[BITS 64]
[ORG 0x200000]

start:
    mov rdi,Idt
    mov rax,handler0

    mov [rdi],ax; lower 16-bit fo offset
    shr rax,16; bits 32-16 in ax
    mov [rdi+6],ax
    shr rax,16; bits 32-63 in eax now
    mov [rdi+8],eax

    lgdt[Gdt64Ptr]
    lidt[IdtPtr]

    push 8; since code is first entry
    push KerenelEntry; save address of location we want ot enter
    db 0x48; change operand size to 64-bit
    retf; set cs register with code

KerenelEntry:
    mov byte[0xb8000],'K'
    mov byte[0xb8001], 0xa;light green text

    xor rbx,rbx
    div rbx; purposely throw divide by zero interrupt to test it

End:
    hlt
    jmp End

handler0:
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
    push r15; save state of cpu when interrupt occurs

    mov byte[0xb8000],'D'
    mov byte[0xb8001], 0xc;light red text

    jmp End

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
    pop rax;restore registers to values before interrupt occurred

    iretq; interrupt return(pops mopre data and can return to different privilege level)

Gdt64:
    dq 0; first entry empty
    dq 0x0020980000000000; second entry code has attribute D=0 if long bit is set-L(long bit)=1 so we are in 64-bit mode not compatability mode-P(present bit)=1(else exception)-DPL(set privilege level)=0-1-1(descriptor is code segment)-C(conforming bit)=0
                         ;third entry is data but in 64-bit mode switching privilege level is only use for data segement so there is no need to define it in the loader file for this project
Gdt64Len: equ $-Gdt64

Gdt64Ptr: dw Gdt64Len-1; limit loaded with double word
          dq Gdt64; table loaded with quad word since its 8 bytes in 64-bit mode

Idt:
    %rep 256
         dw 0
         dw 0x8; same as code segment descriptor in use(can only jump to same or higher privilege code segment)
         db 0
         db 0x8e; 6th byte is for attributes p-dpl-type in this case 1-00-0110, 0110 means interrupt gate descriptor
         dw 0
         dd 0
         dd 0
    %endrep

IdtLen: equ $$-Idt

IdtPtr: dw IdtLen
        dq Idt
