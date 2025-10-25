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

global change_pd_esp_and_jump_to_new_command
change_pd_esp_and_jump_to_new_command:
    ; read values
    mov eax, dword [esp+4]
    mov ebx, dword [esp+8]

    ; set stack pointer
    mov esp, eax

    ; change page directory
    mov cr3, ebx

    ; now we can exit interrupt to new task
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 4
    iretd

global change_esp_and_jump_to_new_command
change_esp_and_jump_to_new_command:
    ; set stack pointer
    mov eax, dword [esp+4]
    mov esp, eax

    ; now we can exit interrupt to new task
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 4
    iretd

global idle_command
idle_command:
    hlt
    jmp idle_command

extern kill_running_command
global jump_to_kill_running_command
jump_to_kill_running_command:
    ; copy stack to floating stack page
    mov esi, esp
    and esi, 0xFFFFF000
    mov edi, 0xFF000000 + 0x00105000
    mov ecx, 4096
    rep movsb

    ; move stack pointer to floating stack
    and esp, 0xFFF
    or esp, 0xFF000000 + 0x00105000

    ; call function
    call kill_running_command

global exit_command
exit_command:
    mov eax, 0
    mov ebx, 1
    int 0xA0