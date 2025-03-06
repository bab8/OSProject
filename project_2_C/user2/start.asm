section .text
global start
extern main

start:
    call main
    jmp $; no kernel return yet, cant use hlt bc we are in user mode