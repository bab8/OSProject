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

;loads kernel file
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

;loads user file
LoadUser1:
    mov si, ReadPacket; si(source index register)
    mov word[si], 0x10; size = 16 bytes
    mov word[si+2], 10; sectors = 10
    mov word[si + 4],0; offset = 0, 0x20000 will overflow in a single word so we set offset to 0 and tehn use the segment to calculate 0x20000, where the kernel will be placed
    mov word[si + 6], 0x2000; segement piece of address (0 + 16*0x2000[segement] = 0x20000), physical address for user will be 0x20000
    mov dword[si + 8],106; sector start for user file
    mov dword[si + 12], 0; high address 
    mov dl, [DriveId]
    mov ah, 0x42; we want to use disk extension service
    int 0x13
    jc ReadError; carry flag will be set if sectors cannot be read

LoadUser2:
    mov si, ReadPacket; si(source index register)
    mov word[si], 0x10; size = 16 bytes
    mov word[si+2], 10; sectors = 10
    mov word[si + 4],0; 
    mov word[si + 6], 0x3000; s
    mov dword[si + 8],116; sector start for user file
    mov dword[si + 12], 0; high address 
    mov dl, [DriveId]
    mov ah, 0x42; we want to use disk extension service
    int 0x13
    jc ReadError; carry flag will be set if sectors cannot be read

LoadUser3:
    mov si, ReadPacket; si(source index register)
    mov word[si], 0x10; size = 16 bytes
    mov word[si+2], 10; sectors = 10
    mov word[si + 4],0; 
    mov word[si + 6], 0x4000; s
    mov dword[si + 8],126; sector start for user file
    mov dword[si + 12], 0; high address 
    mov dl, [DriveId]
    mov ah, 0x42; we want to use disk extension service
    int 0x13
    jc ReadError; carry flag will be set if sectors cannot be read

GetMemInfoStart:;use to get info on memory address block to see what memory is avaible to be used
    mov eax,0xe820
    mov edx,0x534d4150; ascii code for smap
    mov ecx,20; length of memory block
    mov dword[0x9000],0;struture
    mov edi,0x9008; save the memory address
    xor ebx,ebx
    int 0x15
    jc NotSupport; service 0xe280 not available

GetMemInfo:
    add edi,20; move to next memory address
    inc dword[0x9000]; inc count
    test ebx,ebx
    jz GetMemDone; loop while not 0

    mov eax,0xe820
    mov edx,0x534d4150
    mov ecx,20
    int 0x15
    jnc GetMemInfo; carry flag means end of memory blocks has been reached


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

    jmp 8:PMEntry; init cs for PE use 8 since code is second entry and thus 8 bytes after beginning of gdt, have to use jmp instead of mov for cs register

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

    cld;            the addresses 0x80000 - 0x90000 may be used for BIOS data instead we can use 0x70000 to 0x80000      
    mov edi,0x70000; this code uses a free memory area to initialize the paging structure, translates virtual address to physical address
    xor eax,eax;
    mov ecx,0x10000/4;
    rep stosd;

    mov dword[0x70000],0x71003 ;readable, writeable, only accessable by kernel, indicated by first 3 bits or 3
    mov dword[0x71000],10000011b; base address set to 0 indicated by bits 3 to 7, first 2 bits set to 1 to indicate same properties described in prev comment

    mov eax,(0xffff800000000000>>39); retrieve 9 bit page map level 4 value located at bit 39 for kernel
    and eax,0x1ff; zero all but the 9 bits
    mov dword[0x70000+eax*8],0x72003; each entry is 8 bytes, accessing page table
    mov dword[0x72000],10000011b; set 1gb page to physcial page of kernel location

    lgdt[Gdt64Ptr]
    
    mov eax,cr4
    or eax,(1<<5); bit 5 in cr4 needs to be set for Physical address extension whihc is necessary for 64-bit mode
    mov cr4,eax

    mov eax,0x70000; cr3 needs address of paging structure for 64-bit mode
    mov cr3,eax; from here addresses need to be mapped to physical before being used, but any addresses loaded to cr3 will still require physcial address

    mov ecx,0xc0000080
    rdmsr; read msr
    or eax,(1<<8); registers 8th bit needs to be set to enable long mode
    wrmsr; write msr

    mov eax,cr0
    or eax,(1<<31)
    mov cr0,eax;enabling paging, long mode enabled

    jmp 8:LMEntry; code is at first index or at 8 bytes

PEnd:
    hlt
    jmp PEnd

[BITS 64];placed after PEnd
LMEntry:
    mov rsp,0x7c00; set stack ptr

    cld;clear direction flag(allows processing of low memory address to high memory address)
    mov rdi,0x200000; destination address stored in rdi, 0x200000 is where we want kernel
    mov rsi,0x10000; source address stored in rsi, current location of kernel is 0x10000
    mov rcx,51200/8; rcx acts as counter,512000 is equal to 100 sectors, divide by 8 because of quad word
    rep movsq; repeat mov quad word rcx times

    mov rax, 0xffff800000200000; virtual address of kernel
    jmp rax; transfers exectuion to kernel
    
LEnd:
    hlt
    jmp LEnd

DriveId:       db 0
ReadPacket: times 16 db 0; 16 byte structure (0[first word] size, 2 number of sectors, 4 offset, 6 segement, 8 address low, address high)

Gdt32:
    dq 0; first entry empty dp is quad word allocates 8 bytes
Code32:
    dw 0xffff; first two bytes defines segement size, we want code to be max size
    dw 0; lower 24 bits of base address occupy next three bytes
    db 0; code segment starts from 0
    db 0x9a; 0x9a = 10011010b 1-00-1-1010 last bit is for present which needs to be there so cpu wont throw exception when accesssing segement, 
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

Gdt64:
    dq 0; first entry empty
    dq 0x0020980000000000; second entry code has attribute D=0 if long bit is set-L(long bit)=1 so we are in 64-bit mode not compatability mode-P(present bit)=1(else exception)-DPL(set privilege level)=0-1-1(descriptor is code segment)-C(conforming bit)=0
                         ;third entry is data but in 64-bit mode switching privilege level is only use for data segement so there is no need to define it in the loader file for this project
Gdt64Len: equ $-Gdt64

Gdt64Ptr: dw Gdt64Len-1
          dd Gdt64
    

