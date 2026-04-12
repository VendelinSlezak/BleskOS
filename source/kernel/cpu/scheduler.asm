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

global idle_thread
idle_thread:
    pause
    jmp idle_thread

global move_to_floating_stack
move_to_floating_stack:
    mov eax, [esp + 8]  ; get pointer to function
    mov ebx, [esp + 12] ; get argument for function
    mov esp, [esp + 4]  ; move to floating stack
    push ebx            ; push argument
    push eax            ; push function pointer
    ret

global exit_interrupt_to_thread
exit_interrupt_to_thread:
    ; restore the thread's stack pointer
    mov esp, [esp + 4] ; get pointer from first argument

    ; restore segment registers
    pop ds
    pop es
    pop fs
    pop gs

    ; restore general-purpose registers
    popad

    ; pop interrupt number and error code
    add esp, 8

    ; return from interrupt
    iret