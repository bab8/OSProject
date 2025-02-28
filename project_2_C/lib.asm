section .text
global memset
global memmove
global memcpy
global memcmp

memset:
    cld; clear direction flag
    mov ecx,edx; rdi(buffer) first param, rsi(value), second param, rdx(size) third param
    mov al,sil
    rep stosb; copies value sotred in al to address stored in rdi
    ret

memcmp:
    cld
    xor eax,eax
    mov ecx,edx
    repe cmpsb; cmp mem and set flags accordingly
    setnz al; result not equal
    ret

memmove:
memcpy:
    cld
    cmp rsi,rdi; cmp src to dest
    jae .copy; jump if src is greater than dest
    mov r8,rsi
    add r8,rdx
    cmp r8,rdi
    jbe .copy
.overlap:; there is overlap between src and dest
    std; set direction flag copy high mem address to low, backwards direction
    add rdi,rdx
    add rsi,rdx
    sub rdi,1
    sub rsi,1

.copy:
    mov ecx,edx
    rep movsb; will copy memory address from rsi to rdi
    cld
    ret