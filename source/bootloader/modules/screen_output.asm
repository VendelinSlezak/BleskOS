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

; input: si = string
; output: nothing
print_first_line_string:
    ; if 0xE9 hack is enabled, log output here
    push si
    in al, 0xE9
    cmp al, 0xE9
    jne .if_0xE9_hack_exist
        mov al, 0xA
        out 0xE9, al

        .log_char:
            mov al, byte [si]
            cmp al, 0
            je .if_0xE9_hack_exist
            out 0xE9, al
            inc si
        jmp .log_char
    .if_0xE9_hack_exist:
    pop si

    ; set cursor
    mov ah, 0x02
    mov bh, 0
    mov dx, 0x0000
    int 10h

    .print_string:
    mov ah, 0x0E
    .print_char:
        cmp byte [si], 0
        je .delete_rest_of_line

        mov al, byte [si]
        int 10h

        inc si
        inc dl ; column of cursor
    jmp .print_char

    .delete_rest_of_line:
    mov ah, 0x09
    mov al, ' '
    mov bh, 0
    mov bl, 0x07
    mov cx, 80
    sub cl, dl
    int 10h

    ret

; input: ax = number to be printed
; output: nothing
print_loaded_percent:
    push ax

    mov ah, 0x02
    mov bh, 0
    mov dx, 0x0000
    int 10h

    pop ax

    mov bx, 10
    mov dx, 0
    div bx ; get upper and lower digit

    mov ah, 0x0E

    cmp al, 0
    je .if_second_digit_present
        add al, '0'
        int 10h
    .if_second_digit_present:

    mov al, dl
    add al, '0'
    int 10h

    .print_string:
    mov si, loading_string
    jmp print_first_line_string.print_string

; input: si = string that will be printed out
bootloader_panic:
    cli

    mov ax, 0x0000
    mov ds, ax
    mov ax, 0xB800
    mov fs, ax
    mov di, 0

    mov bx, bootloader_panic_error_str
    .print_error_string:
        cmp byte [bx], 0
        je .print_char
        mov ah, 0x40 ; red background, black letter
        mov al, byte [bx]
        mov word [fs:di], ax
        inc bx
        add di, 2
    jmp .print_error_string

    .print_char:
        cmp byte [si], 0
        je .halt
        mov ah, 0x40 ; red background, black letter
        mov al, byte [si]
        mov word [fs:di], ax
        inc si
        add di, 2
    jmp .print_char

    .halt:
        hlt
    jmp .halt

bootloader_panic_error_str db 'ERROR: ', 0