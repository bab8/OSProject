section .text

global writeu

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