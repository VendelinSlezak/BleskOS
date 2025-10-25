;;;;;
;;
;; BleskOS
;;
;; MIT License
;; Copyright (c) 2023-2025 BleskOS developers
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;
;;;;;

section .text
global interrupt_handler
global setup_idt
global stack_of_interrupt
global protected_mode_idt
global protected_mode_idt_wrap
extern global_interrupt_handler

%macro INTERRUPT_STUB 1
global interrupt_stub%1
interrupt_stub%1:
    push %1
    jmp interrupt_handler
%endmacro
%assign i 0
%rep 256
    INTERRUPT_STUB i
    %assign i i+1
%endrep

interrupt_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov dword [stack_of_interrupt], esp

    push dword [esp+48]
    call global_interrupt_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 4
    iretd

%macro IDT_ENTRY_STUB 1
    mov eax, interrupt_stub%1
    mov word [edi], ax          ; offset low
    mov word [edi+2], 0x08      ; kernel code segment
    mov byte [edi+4], 0         ; always 0
    mov byte [edi+5], 0x8E      ; P=1, DPL=0, 32-bit interrupt gate
    shr eax, 16
    mov word [edi+6], ax        ; offset high
    add edi, 8
%endmacro

setup_idt:
    mov edi, protected_mode_idt

    %assign i 0
    %rep 256
        IDT_ENTRY_STUB i
        %assign i i+1
    %endrep

    ret

stack_of_interrupt dd 0
align 8
protected_mode_idt times 256*8 db 0
protected_mode_idt_wrap:
    dw 0
    dd 0