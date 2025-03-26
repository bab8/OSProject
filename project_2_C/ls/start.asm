section .text
global start
extern main
extern exitu

start:
    call main
    call exitu
    jmp $; no kernel return yet, cant use hlt bc we are in user mode