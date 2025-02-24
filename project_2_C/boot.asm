[BITS 16]; running in real mode
[ORG 0x7c00]; indicates where code runs(MBR in this case)

start:
    xor ax,ax; clear ax(make 0)
    mov ds,ax; init data segement
    mov es,ax; init extra segement
    mov ss,ax; init stack segement
    mov sp,0x7c00; give stack pointer address(stack will be from this address to zero, remember stack grows down, sp decrements as it grows )

TestDiskExstention:
    mov [DriveId], dl; dl holds drive id when bios transfers control to boot code, square brackets access memory location DriveId represents
    mov ah,0x41
    mov bx,0x55aa
    int 0x13; if service is not supported carry flag is set
    jc NotSupport; jump if carry flag set
    cmp bx, 0xaa55
    jne NotSupport

LoadLoader:
    mov si, ReadPacket; si(source index register)
    mov word[si], 0x10; size = 16 bytes
    mov word[si+2], 5; sectors = 5(small loader file)
    mov word[si + 4],0x7e00; offset = 0x7e00, location of loader file, which is right after mbr which starts at 0x7c00 and is 512 bytes
    mov word[si + 6], 0; segement piece of address (0x7e00 + 16*0[segement] = 0x7e00)
    mov dword[si + 8],1; low address (LBA is zero based address so sector 0 is the first sector, sector 1 is the second sector and so on)
    mov dword[si + 12], 0; high address 
    mov dl, [DriveId]
    mov ah, 0x42; we want to use disk extension service
    int 0x13
    jc ReadError; carry flag will be set if sectors cannot be read

    mov dl, [DriveId]; save drive ID
    jmp 0x7e00

ReadError:
NotSupport:
    mov ah, 0x13; holds function code
    mov al, 1; specifies mode
    mov bx, 0xa; represents page number, 0xa means green color, holds info about character attributes
    xor dx,dx; dh is rows, dl is columns, so we set dx to zero to print at 0 position on screen
    mov bp, Message; copies address of message
    mov cx, MessageLength;specifies number of characters to print
    int 0x10; interrupt for print function

End:
    hlt; put processor in halt state
    jmp End; infinite loop

DriveId:        db 0
Message:        db "We have an error in the boot process"
MessageLength: equ $-Message; $ is current asm position so $ - Message gives number of char to print for message by using equ instruction
ReadPacket: times 16 db 0; 16 byte structure (0[first word] size, 2 number of sectors, 4 offset, 6 segement, 8 address low, address high)

times (0x1be-($-$$)) db 0; repeat command number of times, $$ means start of section, $-$$ means from start to end of msg, instruction fills from end of msg ($) to 0x1be with zeros

    db 0x80; boot indicator
    db 0,2,0; starting chs(cylinder,head,sector)
    db 0x0f0; type
    db 0xff, 0xff, 0xff; ending chs
    dd 1; starting sector logical block address
    dd (20*16*63-1); size

    times (16*3) db 0

    db 0x55
    db 0xaa; signature 
    ; sector size is assumed to be 512 bytes for this project