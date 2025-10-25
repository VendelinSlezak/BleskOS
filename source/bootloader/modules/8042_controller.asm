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

%define CONTROLLER_8042_DATA_PORT 0x60
%define CONTROLLER_8042_CMD_PORT 0x64
%define CONTROLLER_8042_STATUS_PORT 0x64
%define CONTROLLER_8042_TIMEOUT_COUNT 100

; input: nothing
; output: nothing
wait_input_clear:
    mov  cx, CONTROLLER_8042_TIMEOUT_COUNT
.wait_loop:
    in   al, CONTROLLER_8042_STATUS_PORT
    test al, 00000010b
    jz   .ready
    loop .wait_loop
    stc
    ret
.ready:
    clc
    ret

; input: nothing
; output: nothing
wait_output_full:
    mov cx, CONTROLLER_8042_TIMEOUT_COUNT
.wait_loop2:
    in  al, CONTROLLER_8042_STATUS_PORT
    test al, 00000001b
    jnz .ready
    loop .wait_loop2
    stc
    ret
.ready:
    clc
    ret

; input: al = data
; output: cf on timeout
controller_8042_send_command:
    push ax
    call wait_input_clear
    jc .timeout
    pop ax
    out CONTROLLER_8042_CMD_PORT, al
    ret
.timeout:
    pop ax
    ret

; input: al = data
; output: cf on timeout
controller_8042_send_data:
    push ax
    call wait_input_clear
    jc .timeout
    pop ax
    out CONTROLLER_8042_DATA_PORT, al
    ret
.timeout:
    pop ax
    ret

; input: nothing
; output: al = data, cf on timeout
controller_8042_read_data:
    call wait_output_full
    jc .timeout
    in al, CONTROLLER_8042_DATA_PORT
    ret
.timeout:
    xor  al, al
    ret