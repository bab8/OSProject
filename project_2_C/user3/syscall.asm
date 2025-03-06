section .text

global writeu
global sleepu
global exitu
global waitu

writeu:
    sub rsp,16 ;allocate 16 bytes for arguements
    xor rax,rax ; holds index of system call function, index for write screen function is 0

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
    mov eax,3
    mov rdi,0

    int 0x80

    ret