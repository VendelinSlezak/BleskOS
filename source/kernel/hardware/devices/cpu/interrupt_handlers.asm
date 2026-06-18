;;;;;
;;
;; BleskOS
;;
;; MIT License
;; Copyright (c) 2023-2026 BleskOS developers
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;
;;;;;

bits 32

section .text
global interrupt_handler
extern global_interrupt_handler

global interrupt_error_code
interrupt_error_code dd 0

%macro INTERRUPT_STUB 1
global interrupt_stub%1
interrupt_stub%1:
    push 0 ; push dummy error code for interrupts without error code
    push %1 ; push interrupt number
    jmp interrupt_handler
%endmacro
%macro INTERRUPT_STUB_WITH_ERROR_CODE 1
global interrupt_stub%1
interrupt_stub%1:
    push %1 ; push interrupt number
    jmp interrupt_handler
%endmacro
%assign i 0
%rep 256
    %if i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21 || i == 30
        INTERRUPT_STUB_WITH_ERROR_CODE i
    %else
        INTERRUPT_STUB i
    %endif
    %assign i i+1
%endrep

interrupt_handler:
    pushad
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10 ; GDT selector for kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp ; push current stack pointer
    push dword [esp+52] ; push interrupt number
    call global_interrupt_handler
    add esp, 8 ; clean up stack

    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8 ; pop interrupt number and error code
    iretd