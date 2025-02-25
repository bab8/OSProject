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
    int 0x10; each screen position is two bytes, first byte is for ascii code, second byte is for attributes, lower half is foreground color and the other half is for background color           

    cli; clear interrupt flag, disables interrupts(except nonmaskable) so that we can switch to modes
    lgdt [Gdt32Ptr]; load gdt register
    lidt [Idt32Ptr]; load Idt register, will be 0 since idt is not being set in 32-bit mode for this project
    
    mov eax,cr0
    or eax,1
    mov cr0,eax; cr0 is cpu control register used to enable rpotected mode, so we set it to 1 to enable

    jmp 8:PMEntry; init cs for PE use 8 since code is second entry and thus 8 bytes after beginning of gdt

ReadError:
NotSupport:
End:
    hlt
    jmp End

[BITS 32]
PMEntry:
    mov ax,0x10; data is third entry so we use 16 for index
    mov ds,ax
    mov es,ax
    mov ss,ax;init data segment registers with gdt data 
    mov esp,0x7c00; set stack ptr

    mov byte[0xb8000],'P'; 0xb8000 is first byte on screen in PE
    mov byte[0xb8001], 0xa; color will be light green

PEnd:
    hlt
    jmp PEnd

DriveId:       db 0
ReadPacket: times 16 db 0; 16 byte structure (0[first word] size, 2 number of sectors, 4 offset, 6 segement, 8 address low, address high)

Gdt32:
    dq 0; first entry empty dp is quad word allocates 8 bytes
Code32:
    dw 0xffff; first two bytes defines segement size, we want code to be max size
    dw 0; lower 24 bits of base address occupy next three bytes
    db 0; code segment starts from 0
    db 0x9a; 0x9a = 10011010b 1-00-1-1010 last bit is for present which needs to be there so cpu wont throw exception when accdesssing segement, 
           ; 00 is dpl used to assign privilage level for segment, 1 is for flagging whether this is code or data descriptor, 
           ; 1010 is for type which determines if code is conforming or non-conforming which determines if cpl is changed when transferred to higher privilege conforming code segment, 1010 is non-conforming
    db 0xcf; 0xcf = 1101111b 1-1-0-0-1111 G-D-0-A-LIMIT, is used for segment size and attributtes,
           ;1 for G for granularity size of field will be scaled by 4kb, 
           ;1 for D which is default operand size, we set so it is 32 bit otherwise defaults to 16 bit, 
           ;0 ignored bit, 0 for A determines if segment can be used by system software, 1111 for LIMIT for max size limit for segment
    db 0   ;last byte is upper8 bits of base address
Data32:
    dw 0xffff; same structure as code segement
    dw 0; 
    db 0; 
    db 0x92; changed type field to 0010 to mark as writeable segment
    db 0xcf; 
    db 0  

Gdt32Len: equ $-Gdt32

Gdt32Ptr: dw Gdt32Len-1
          dd Gdt32

Idt32Ptr: dw 0
          dd 0

