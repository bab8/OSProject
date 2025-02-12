[org 0x7c00]
[bits 16]

section code

;.init:
    ;mov eax, 0x07c0; where code is stored on computer
    ;mov ds, eax; data segment register
   ; mov eax, 0xb800
   ; mov es, eax
   ; mov eax, 0; use for loop counter
   ; mov ebx, 0; index of char in string being printed
  ;  mov ecx, 0; actual address of char on screen
 ;   mov dl, 0; store teh actual value that we are printing

;.clear:
    ;mov byte [es:eax], 0; move blank char to current text address
   ; inc eax
  ;  mov byte [es:eax], 0xB0; move background color and char color to next address
 ;   inc eax

;    cmp eax, 2 * 25 * 80;2x25x80 addresses based on screen size

;    jl .clear; jump less than 2*80*25

;mov eax, welcome
;mov ecx, 0 * 2 * 80; num lines * num bytes per char * num char per line(starting line to print on starts with 0 index)
;push .end store end of instruction address
;call .print

;jmp .switch; go to 32-bit mode

;.end:
 ;   mov byte [es:0x00], 'L'
  ;  jmp $; infinite loop

;.print: ;16-bit mode function
;    mov dl, byte[eax + ebx]

;    cmp dl, 0; cmp to blank char
;    je .print_end

;    mov byte [es:ecx], dl

;    inc ebx
;    inc ecx
;    inc ecx; each char is two bytes, 1 for text, 1 for background color

;    jmp .print

;.print_end:
 ;   ret; return control to top of stack

.switch:
    mov ax, 0x4f01; querying the VBE
    mov cx, 0x111; mode we want(see vbe-modex file)
    mov bx, 0x0800; offset for the vbe infrastructure
    mov es, bx
    mov di, 0x00
    int 0x10; graphiccs interupt

    ; make switch to graphics mmode
    mov ax, 0x4f02
    mov bx, 0x111
    int 0x10

    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, 0x1000; This is the location where code is loaded from hard disk
    mov ah, 0x02
    mov al, 21 ; number of sectors to read from hard disk(if you specify more sectors than are actually present on physical device it will crash w/o err)
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13

    cli; turn off interrupts
    lgdt [gdt_descriptor]; load the GDT table

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax; make the switch

    jmp code_seg:protected_start ; offset to ensure code runs first

;text: db 'Hello, World!', 0 ; string to be printed
;text1: db 'This is text1', 0
welcome: db 'Welcome to Sapphire OS!', 0

[bits 32];code to switch to 32 bit mode
protected_start:
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ;Update stack pointer
    mov ebp, 0x90000
    mov esp, ebp

    call 0x1000
    jmp $

gdt_begin:
gdt_null_descriptor:
    dd 0x00
    dd 0x00
gdt_code_seg:
    dw 0xeeee
    dw 0x00
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data_seg:
    dw 0xeeee
    dw 0x00
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_begin - 1
    dd gdt_begin

code_seg equ gdt_code_seg - gdt_begin
data_seg equ gdt_data_seg - gdt_begin

times 510 - ($ - $$) db 0x00; Pads the file with 0s to make it the right size

db 0x55
db 0xaa; make executable