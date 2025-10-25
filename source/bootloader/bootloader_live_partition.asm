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

org 0x7C00

bits 16

start:
    ; reset registers
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x7C00

    ; save boot device number
    push dx

    ; ensure that monitor has 80x25 text mode
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    ; clear screen and move cursor to top left
    mov ah, 0x02
    mov dx, 0x0000
    mov bh, 0
    int 10h

    mov ah, 0x09
    mov al, ' '
    mov bh, 0
    mov bl, 0x07
    mov cx, 80*25
    int 10h

    ; read extended bootloader of BleskOS partition from LBA sector 2
    mov ah, 0x2 ; read
    mov al, 8 ; eight sectors
    mov ch, 0 ; cylinder
    mov dh, 0 ; head
    mov cl, 3 ; sector
    pop dx
    push dx
    ; dl contains number of boot device
    mov bx, 0x1000 ; memory offset
    int 13h
    jc .error
    cmp ah, 0
    jne .error

    ; execute code of extended bootloader
    pop dx
    jmp 0x0000:0x1000
    
    ; error during loading extended bootloader, print error message independently from BIOS
    .error:
    mov ax, 0xB800
    mov es, ax
    mov dword [es:0], 0x40314045 ; 'E1' on position 0, 0 with black text and red background

    ; halt forever
    cli
    .halt:
        hlt
    jmp .halt

times 0x1FE-($-$$) db 0
dw 0xAA55 ; signature of bootable partition