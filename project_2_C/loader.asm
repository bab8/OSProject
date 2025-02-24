[BITS 16]
[ORG 0x7e00]

start:
    mov [DriveId],dl; save drive id
    mov eax,0x80000000
    cpuid
    cmp eax,0x80000001; test if value is supported
    jb NotSupport
    mov eax,0x80000001
    cpuid
    test edx,(1<<29); 29th bit in edx being set means long mode is supported
    jz NotSupport
    test edx,(1<<26); check for 1gb support on 26 bit
    jz NotSupport

LoadKernel:
    mov si, ReadPacket; si(source index register)
    mov word[si], 0x10; size = 16 bytes
    mov word[si+2], 100; sectors = 100
    mov word[si + 4],0; offset = 0, 0x10000 will overflow in a single word so we set offset to 0 and tehn use the segment to calculate 0x10000, where the kernel will be placed
    mov word[si + 6], 0x1000; segement piece of address (0 + 16*0x1000[segement] = 0x10000), physical address for kernel will be 0x10000
    mov dword[si + 8],6; low address first sector is mbr, next five is loader file, so kernel starts at sector 7
    mov dword[si + 12], 0; high address 
    mov dl, [DriveId]
    mov ah, 0x42; we want to use disk extension service
    int 0x13
    jc ReadError; carry flag will be set if sectors cannot be read

    mov ah, 0x13; holds function code
    mov al, 1; specifies mode
    mov bx, 0xa; represents page number, 0xa means green color, holds info about character attributes
    xor dx,dx; dh is rows, dl is columns, so we set dx to zero to print at 0 position on screen
    mov bp, Message; copies address of message
    mov cx, MessageLength;specifies number of characters to print
    int 0x10; interrupt for print function

ReadError:
NotSupport:
End:
    hlt
    jmp End

DriveId:       db 0
Message:       db "Kernel Loaded"
MessageLength: equ $-Message; $ is current asm position so $ - Message gives number of char to print for message by using equ instruction
ReadPacket: times 16 db 0; 16 byte structure (0[first word] size, 2 number of sectors, 4 offset, 6 segement, 8 address low, address high)