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

GetMemInfoStart:;use to get info on memory address block to see what memory is avaible to be used
    mov eax,0xe820
    mov edx,0x534d4150; ascii code for smap
    mov ecx,20; length of memory block
    mov edi,0x9000; save the memory address
    xor ebx,ebx
    int 0x15
    jc NotSupport; service 0xe280 not available

GetMemInfo:
    add edi,20; move to next memory address
    mov eax,0xe820
    mov edx,0x534d4150
    mov ecx,20
    int 0x15
    jc GetMemDone; carry flag means end of memory blocks has been reached

    test ebx,ebx
    jnz GetMemInfo; repeat function

GetMemDone:
TestA20:; test is a20line is enabled, deteremines is 20th bit is read or ignored
    mov ax, 0xffff
    mov es,ax
    mov word[ds:0x7c00], 0xa200
    cmp word[es:0x7c10], 0xa200; 0xffff:0x7c00 = 0xfff * 16 + 0x7c10 = 0x107c00
    jne SetA20LineDone
    mov word[0x7c00], 0xb200; second test
    cmp word[es:0x7c10],0xb200
    je End

SetA20LineDone:
    xor ax,ax
    mov es,ax

SetVideoMode:
    mov ax,3; 3 for text mode, screen is 80x25(80 char each line, 25 lines), first position is b8000 and increments by 2 for each position
    int 0x10; each screen position is two bytes, first byte is for ascii code, second byte is for attributes, lower half is foreground color and teh other half is for background color           
    mov si,Message ; 0 - Black, 1 - Blue, 2 - Green, 3 - Cyan, 4 - Red, 5 - Magenta, 6 - Brown, 7 - Light Gray, 8 - Dark gray, 9 - Light Blue, A - Light Green, B - Light Cyan, C - Light Red, D - Light Magenta, E - Yellow
    mov ax,0xb800
    mov es,ax
    xor di,di
    mov cx, MessageLength

PrintMessage:
    mov al,[si]
    mov [es:di],al; [es:di] is 0xb8000
    mov byte[es:di + 1], 0xa; make char green

    add di,2; char takes up 2 bytes
    add si,1; char stored takes up 1 byte
    loop PrintMessage; loops based on cx

ReadError:
NotSupport:
End:
    hlt
    jmp End

DriveId:       db 0
Message:       db "Text Mode is set"
MessageLength: equ $-Message; $ is current asm position so $ - Message gives number of char to print for message by using equ instruction
ReadPacket: times 16 db 0; 16 byte structure (0[first word] size, 2 number of sectors, 4 offset, 6 segement, 8 address low, address high)