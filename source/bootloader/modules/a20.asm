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

; BOOTLOADER FUNCTION
%macro TRY_IF_A20_IS_ENABLED 0
    call try_if_a20_is_enabled
    cmp ax, 1
    je .a20_enabled
%endmacro
enable_a20_line:
    ; ENABLE A20 LINE
    ; check if A20 is already enabled
    TRY_IF_A20_IS_ENABLED

    ; try to activate A20 through BIOS
    PRINT_STRING 'Trying to enable A20 line through BIOS...'
    mov ax, 0x2401
    int 15h
    TRY_IF_A20_IS_ENABLED

    ; try fast A20 (should work everywhere)
    PRINT_STRING 'Trying to enable A20 line through fast A20...'
    in al, 0x92
    cmp al, 0xFF
    je .if_fast_a20_exist
        or al, (1 << 1)     ; activate A20 bit
        and al, 0xFE        ; be sure to deactivate system reset bit
        out 0x92, al

        TRY_IF_A20_IS_ENABLED
    .if_fast_a20_exist:

    ; try to activate A20 through 8042 controller
    PRINT_STRING 'Trying to enable A20 line through 8042 controller...'
    in al, 0x64
    cmp al, 0xFF
    je .if_8042_controller_exist
        CONTROLLER_8042_SEND_CMD 0xAD   ; disable first PS/2 port
        CONTROLLER_8042_SEND_CMD 0xA7   ; disable second PS/2 port
        .flush_loop:                    ; flush output buffer if there is anything left
            call wait_output_full       ; wait until data available or timeout
            jc .flush_done              ; if timeout, stop flushing
            in al, 0x60                 ; read and discard data
            jmp .flush_loop
        .flush_done:

        CONTROLLER_8042_SEND_CMD 0xD0   ; read output port command
        CONTROLLER_8042_READ_DATA al    ; get controller output port value
        push ax                         ; save it on stack
        CONTROLLER_8042_SEND_CMD 0xD1   ; write output port command
        pop ax                          ; restore value
        or al, (1 << 1) | (1 << 0)      ; set A20 enable bit and disable System Reset
        CONTROLLER_8042_SEND_DATA al    ; write modified value back HERE VIRTUALBOX CRASHES

        CONTROLLER_8042_SEND_CMD 0xAE   ; re-enable first PS/2 port
        CONTROLLER_8042_SEND_CMD 0xA8   ; re-enable second PS/2 port

        TRY_IF_A20_IS_ENABLED
    .if_8042_controller_exist:

    ; A20 was not enabled
    BOOTLOADER_PANIC jmp, 'A20 line was not enabled'

    .a20_enabled:
    PRINT_STRING 'A20 line is enabled'

    ret

; input: nothing
; output: ax = 0 (A20 not enabled), ax = 1 (A20 enabled)
try_if_a20_is_enabled:
    ; set segment registers
    mov ax, 0x0000
    mov fs, ax
    not ax ; ax = 0xFFFF
    mov gs, ax

    ; try to modify lower address, and check if higher address was modified
    mov dword [fs:0x500], 0x00000000 ; physical memory 0x000500
    cmp dword [gs:0x510], 0x00000000 ; with A20 physical memory 0x100500, without A20 physical memory 0x000500
    jne .enabled
    mov dword [fs:0x500], 0xFFFFFFFF ; physical memory 0x000500
    cmp dword [gs:0x510], 0xFFFFFFFF ; with A20 physical memory 0x100500, without A20 physical memory 0x000500
    jne .enabled

    ; A20 is not enabled
    mov ax, 0
    ret

    ; A20 is enabled
    .enabled:
    mov ax, 1
    ret