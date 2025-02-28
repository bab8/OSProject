;[BITS 64]
;[ORG 0x200000], address location set by linker script link.lds
section .data; label defined for linker script
Gdt64:
    dq 0; first entry empty
    dq 0x0020980000000000; second entry code has attribute D=0 if long bit is set-L(long bit)=1 so we are in 64-bit mode not compatability mode-P(present bit)=1(else exception)-DPL(set privilege level)=0-1-1(descriptor is code segment)-C(conforming bit)=0
    dq 0x0020f80000000000; 01111110 dpl is changed from 0 to 3 from  first entry
    dq 0x0000f20000000000;this is data segment descriptor used in 64-bit for switching privilege level, 11110010 P(need present bit)-DPL(run level 3)-10(means data segment descriptor)-0-W(we want it to be writeable)-0
TssDesc:;Tss selector
    dw TssLen -1
    dw 0; base address set to 0 for now
    db 0
    db 0x89; attribute field, p-dpl-type 1-00-01001, means is present, has ring 0, and is the tss selector
    db 0
    db 0
    dq 0;set all remaining fields to 0

Gdt64Len: equ $-Gdt64

Gdt64Ptr: dw Gdt64Len-1; limit loaded with double word
          dq Gdt64; table loaded with quad word since its 8 bytes in 64-bit mode

;base definition for idt, no longer used, using C code instead
;Idt:
;    %rep 256
;         dw 0
;         dw 0x8; same as code segment descriptor in use(can only jump to same or higher privilege code segment)
;         db 0
;         db 0x8e; 6th byte is for attributes p-dpl-type in this case 1-00-0110, 0110 means interrupt gate descriptor
;         dw 0
;         dd 0
;         dd 0
;    %endrep

;IdtLen: equ $$-Idt

;IdtPtr: dw IdtLen
;        dq Idt

Tss:;task state segment
    dd 0; first bytes reserved
    dq 0x150000; rsp0
    times 88 db 0; io permission bitmap(not use so we assign it the size of TSS)
    dd TssLen

TssLen: equ $-Tss

section .text;label defined for linker script
extern KMain; set KMain to external function to call
global start

start:
    ;code used to set idt entries moved to C, only code in use is loading gdt, setting PIT and PIC
    ;mov rdi,Idt
    ;divide by zero interrupt IRQ0
    ;mov rax,handler0
    ;call SetHandler

    ;timer interrupt
    ;mov rax,Timer
    ;add rdi,32*16;make it point to timer entry
    ;call SetHandler

    ;spurious interrupt
    ;mov rdi,Idt+32*16+7*16;dealing with IRQ7 of parent chip, so vector number is 32+7 and each entry is 16 bytes
    ;mov rax,SIRQ
    ;call SetHandler
    
    lgdt[Gdt64Ptr]
    ;lidt[IdtPtr]

SetTss:
    mov rax,Tss
    mov [TssDesc+2],ax;lower third bytes contain first part of address
    shr rax,16; fifth bytes contain next part of address
    mov [TssDesc+4],al
    shr rax,8
    mov [TssDesc+7],al
    shr rax,8; eax now holds the rest of the address
    mov [TssDesc+8],eax; Tss selector now set

    mov ax,0x20; 0x20 is is the selector we want, since it is the 5 entry in the gdt
    ltr ax;load task register, Tss setup


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

    ;(commented out to disable interrupt)sti; enable interrupt

    ;push 0x18|3; ss selector with dpl at ring 3
    ;push 0x7c00; stack ptr
    ;push 0x202; set rflag bit 9 and 1 to 1, indicating interrupt is enabled
    ;push 0x10|3; cs selector at ring 3
    ;push UserEntry
    ;iretq; will return to user entry and load cs and ss selector as set here
    
    push 8; since code is first entry
    push KerenelEntry; save address of location we want ot enter
    db 0x48; change operand size to 64-bit
    retf; set cs register with code

KerenelEntry:
    ;xor ax,ax
    ;mov ss,ax
    mov rsp,0x200000; point stack to kernel
    call KMain
    ;sti; enable interrupt

End:        
    hlt
    jmp End


;previous code used fro idt in assembly, moved to C
;SetHandler:;code to set idt entries
;    mov [rdi],ax; lower 16-bit of offset
;    shr rax,16; bits 32-16 in ax
;    mov [rdi+6],ax
;    shr rax,16; bits 32-63 in eax now
;    mov [rdi+8],eax
;    ret

;UserEntry:
    ;mov ax,cs
    ;and al,11b; preserve lower 2 bits of al adn clear other bits, if val of al is 3 we are at ring 3
    ;cmp al,3
    ;jne UEnd; cannot excute hlt at ring3

;    inc byte[0xb8010]; running is user mode
;    mov byte[0xb8011],0xF; white text

;UEnd:
 ;   jmp UserEntry; infinite loop

;handler0:; divide by 0 interrupt handler
;    push rax
;    push rbx
;    push rcx
;    push rdx
;    push rsi
;    push rdi
;    push rbp
;    push r8
;    push r9
;    push r10
;    push r11
;    push r12
;    push r13
;    push r14
;    push r15; save state of cpu when interrupt occurs

;    mov byte[0xb8000],'D'
 ;   mov byte[0xb8001], 0xc;light red text

  ;  jmp End

;    pop r15
;    pop r14
;    pop r13
 ;   pop r12
;    pop r11
;    pop r10
;    pop r9
;    pop r8
;    pop rbp
;    pop rdi
;    pop rsi
;    pop rdx
;    pop rcx
;    pop rbx
;    pop rax;restore registers to values before interrupt occurred

;    iretq; interrupt return(pops mopre data and can return to different privilege level)

;Timer:; timer interrupt handler
;    push rax
;    push rbx
;    push rcx
;    push rdx
;    push rsi
;    push rdi
;    push rbp
;    push r8
 ;   push r9
;    push r10
;    push r11
;    push r12
;    push r13
;    push r14
;    push r15; save state of cpu when interrupt occurs

;    inc byte[0xb8020]
;    mov byte[0xb8021], 0xe; yellow

;    mov al,0x20; acknowledge interrupt so it can be used again, set bit 5 to 1 to send non-specific end of interrupt
;    out 0x20,al; write to command of parent

 ;   pop r15
 ;   pop r14
 ;   pop r13
 ;   pop r12
;    pop r11
;    pop r10
;    pop r9
;    pop r8
;    pop rbp
;    pop rdi
;    pop rsi
;    pop rdx
;    pop rcx
;    pop rbx
;    pop rax;restore registers to values before interrupt occurred

 ;   iretq

;SIRQ:; spuroious interrupt handler
;    push rax
;    push rbx
;    push rcx
;    push rdx
;    push rsi
;    push rdi
;    push rbp
;    push r8
;    push r9
;    push r10
;    push r11
;    push r12
;    push r13
;    push r14
;    push r15; save state of cpu when interrupt occurs

;    mov al,11; 11 = 00001011 bit 3 is 1 to identify command to read IRR or ISR register, bits 0 and 1 are set to indicate reading ISR register
;    out 0x20,al; write command to register of parent chip
;    in al,0x20; read command 

;    test al,(1<<7); test bit 7 
;    jz .end; if bit 7 is zero jump, interrupt is not regular interrupt

 ;   mov al,0x20
 ;   out 0x,20,al; signal end of interrupt

;.end:
;    pop r15
;    pop r14
;    pop r13
;    pop r12
;    pop r11
;    pop r10
;    pop r9
;    pop r8
;    pop rbp
;    pop rdi
;    pop rsi
;    pop rdx
;    pop rcx
;    pop rbx
;    pop rax;restore registers to values before interrupt occurred
