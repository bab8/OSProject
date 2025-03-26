section .text

global writeu
global sleepu
global exitu
global waitu
global keyboard_readu
global get_total_memoryu
global open_file
global read_file
global get_file_size
global close_file
global fork

writeu:
    sub rsp,16 ;allocate 16 bytes for arguements
    xor eax,eax ; holds index of system call function, index for write screen function is 0

    mov [rsp],rdi; move arguements to stack
    mov [rsp+8],rsi

    mov rdi,2; rdi holds # of args passed to kernel, in this case 2
    mov rsi,rsp; rsi now pts to address of arguements
    int 0x80; software int

    add rsp,16; restore stack 
    ret

sleepu:
    sub rsp,8; one param
    mov eax,1; index 1 for sleep in kernel

    mov [rsp],rdi
    mov rdi,1; num args
    mov rsi,rsp; address of args

    int 0x80

    add rsp,8
    ret

exitu:
    mov eax,2; index number 2
    mov rdi,0

    int 0x80

    ret

waitu:
    sub rsp,8
    mov eax,3
    mov [rsp],rdi
    mov rdi,1
    mov rsi,rsp

    int 0x80

    add rsp,8
    ret

keyboard_readu:
    mov eax,4
    xor edi,edi

    int 0x80

    ret

get_total_memoryu:
    mov eax,5
    xor edi,edi

    int 0x80

    ret

open_file:
    sub rsp,8; allocate space for 1 param
    mov eax,6; system call 6

    mov [rsp],rdi; retrieve of first param
    mov rdi,1
    mov rsi,rsp; save address of arguements

    int 0x80; call int

    add rsp,8;restore satck
    ret

read_file:
    sub rsp,24; three param
    mov eax,7; syscall 7

    mov [rsp],rdi; param 1
    mov [rsp+8],rsi; param 2
    mov [rsp+16],rdx; param 3

    mov rdi,3; num args
    mov rsi,rsp; address of args

    int 0x80

    add rsp,24
    ret

get_file_size:
    sub rsp,8; allocate space for 1 param
    mov eax,8; system call 6

    mov [rsp],rdi; retrieve of first param
    mov rdi,1
    mov rsi,rsp; save address of arguements

    int 0x80; call int

    add rsp,8;restore satck
    ret

close_file:
    sub rsp,8; allocate space for 1 param
    mov eax,9; system call 6

    mov [rsp],rdi; retrieve of first param
    mov rdi,1
    mov rsi,rsp; save address of arguements

    int 0x80; call int

    add rsp,8;restore satck
    ret

fork:
    mov eax,10
    xor edi,edi
    int 0x80
    
    ret