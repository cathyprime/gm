section .data
    msg db "Hello, World!", 0xA
    len equ $ - msg

section .start
    mov ebx, 1
    mov ecx, msg
    mov edx, len
    int 1
    hlt
