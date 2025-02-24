[BITS 16]
[ORG 0x7e00]

start:
    mov ah, 0x13; holds function code
    mov al, 1; specifies mode
    mov bx, 0xa; represents page number, 0xa means green color, holds info about character attributes
    xor dx,dx; dh is rows, dl is columns, so we set dx to zero to print at 0 position on screen
    mov bp, Message; copies address of message
    mov cx, MessageLength;specifies number of characters to print
    int 0x10; interrupt for print function

End:
    hlt
    jmp End

Message:       db "Loader Starts"
MessageLength: equ $-Message; $ is current asm position so $ - Message gives number of char to print for message by using equ instruction