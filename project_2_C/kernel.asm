[BITS 64]
[ORG 0x200000]

start:
    mov rdi,Idt
    mov rax,handler0

    mov [rdi],ax; lower 16-bit of offset
    shr rax,16; bits 32-16 in ax
    mov [rdi+6],ax
    shr rax,16; bits 32-63 in eax now
    mov [rdi+8],eax

    mov rax,Timer
    add rdi,32*16;make it point to timer entry
    mov [rdi],ax; lower 16-bit of offset
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


InitPIT:                  ; bits : 76(select PIT channel[0-2])-54(access mode)-321(operating mode)-0(indicates valuve PIT uses in Binary)
    mov al,(1<<2) | (3<<4); 00110100 -> 00(channel 0)-11(write low byte first then high byte)-010(mode 2, rate generator used for reoccuring interrupt)-0(indicates binary mode)
    out 0x43,al; 43 is address of command mode register

    mov ax,11931; 11931782(fires per second)/100(rate of fires we want) = 11931(value to decrement each time)
    out 0x40,al; 0x40 is address of channel 0, send low bytes
    mov al,ah; holds high byte of value
    out 0x40,al; send high bytes

;set command words
InitPIC:        ; bits: 765-4(init command followed by another 3)-3-2-1-0(use last init command word)
    mov al, 0x11; 00010001-> 000-1-0-0-0-1
    out 0x20,al; 0x20 address of parent chip
    out 0xa0,al; 0xa0 address of child chip

    mov al,32; first vector number of parent
    out 0x21,al;
    mov al,40; first vector number of child
    out 0xa1,al

    mov al,4;init command word 3, sets irq2 to child chip use binary 00000100=4 since 2nd index bit is set
    out 0x21,al
    mov al,2;word for child is 2
    out 0xa1,al

    mov al,1;init command word 4(selects mode) 
    out 0x21,al; bits: 76-54(fully nested mode)-32(buffered mode)-1(auto interrupt end)-0(mode) 
    out 0xa1,al; 00-00-0-1(x86 system used) -> 00000001

    mov al,11111110b; mask all parent IRQs except 0 so that only 0 will fire
    out 0x21,al
    mov al,11111111b; mask all child IQS
    out 0xa1,al

    sti; enable interrupt

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

Timer:
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

    mov byte[0xb8010], 'T'
    mov byte[0xb8011], 0xe; yellow

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

    iretq

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
