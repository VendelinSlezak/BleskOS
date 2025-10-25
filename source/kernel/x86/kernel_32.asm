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

; we need to place this code first in output .bin, therefore we place it in special section
section .start

; bootloader switched processor to protected mode
bits 32

; after bootloader jumps to 0xFF000000, this code is executed
global start
extern initialize_kernel
start:
    call initialize_kernel ; this function does everything

    ; if for some reason there is return from main function, halt processor forever
    cli
    .halt:
        hlt
    jmp .halt

; function to use breakpoints in Bochs
global magic_breakpoint
magic_breakpoint:
    xchg bx, bx

    ret