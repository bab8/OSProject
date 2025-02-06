[org 0x00]
[bits 16]

section code

.init:
    mov eax, 0x07c0; where code is stored on computer
    mov ds, eax; data segment register
    mov eax, 0xb800
    mov es, eax
    mov eax, 0; use for loop counter
    mov ebx, 0; index of char in string being printed
    mov ecx, 0; actual address of char on screen
    mov dl, 0; store teh actual value that we are printing

.clear:
    mov byte [es:eax], 0; move blank char to current text address
    inc eax
    mov byte [es:eax], 0x30; move background color and char color to next address
    inc eax

    cmp eax, 2 * 25 * 80 ;2x25x80 addresses based on screen size

    jl .clear; jump less than 2*80*25

mov eax, text
push .end; store end of instruction address

.print:
    mov dl, byte[eax + ebx]

    cmp dl, 0; cmp to blank char
    je .print_end

    mov byte [es:ecx], dl

    inc ebx
    inc ecx
    inc ecx; each char is two bytes, 1 for text, 1 for background color

    jmp .print

.print_end:
    ret; return control to top of stack

.end:
    jmp $; infinite loop

text: db 'Hello, World!', 0 ; string to be printed
text1: db 'This is text1', 0

times 510 - ($ - $$) db 0x00; Pads the file with 0s to make it the right size

db 0x55
db 0xaa; make executable