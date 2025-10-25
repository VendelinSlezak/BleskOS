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
    ; create space for BPB, some BIOSes will overwrite this area
    jmp .code
        times 0x3E db 0
    .code:

    ; reset registers
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x7C00

    ; copy rest of code to higher address because to 0x7C00 will be loaded bootloader of partition
    mov si, code_on_0x7E00
    mov di, 0x7E00
    mov cx, (end_of_code_on_0x7E00 - code_on_0x7E00)
    rep movsb
    jmp 0x0000:0x7E00

code_on_0x7E00:
    ; save boot device number
    push dx

    ; ensure that monitor has 80x25 text mode
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    ; read bootloader of BleskOS partition from LBA sector 1
    mov ah, 0x2 ; read
    mov al, 1 ; one sector
    mov ch, 0 ; cylinder
    mov dh, 0 ; head
    mov cl, 2 ; sector
    pop dx
    push dx
    ; dl contains number of boot device
    mov bx, 0x7C00 ; memory offset
    int 13h
    jc .error
    cmp ah, 0
    jne .error

    ; execute code of partition bootloader
    pop dx
    jmp 0x0000:0x7C00
    
    ; error during loading bootloader of partition, print error message independently from BIOS
    .error:
    mov ax, 0xB800
    mov es, ax
    mov dword [es:0], 0x40304045 ; 'E0' on position 0, 0 with black text and red background

    ; halt forever
    cli
    .halt:
        hlt
    jmp .halt
end_of_code_on_0x7E00:

    ; variables
    boot_device_number db 0
    
    ; fill rest of file with zeros
    times 0x1B4-($-$$) db 0
    
    ; MBR

    ; first entry
    times 10 db 0   ; Disk ID
    db 0x80         ; bootable partition
    db 0, 2, 0      ; first sector in CHS (head/sector/cylinder)
    db 0x01         ; partition type FAT12
    db 1, 18, 80    ; last sector in CHS (head/sector/cylinder)
    dd 1            ; LBA of first sector
    dd 2879         ; sectors of this partition

    times 16 db 0   ; second entry

    times 16 db 0   ; thrid entry

    times 16 db 0   ; fourth entry

dw 0xAA55 ; signature of bootable disk