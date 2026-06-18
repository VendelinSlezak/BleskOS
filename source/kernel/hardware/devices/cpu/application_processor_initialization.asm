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

extern application_processor_entry_point

bits 16

global application_processor_initialization_sequence
application_processor_initialization_sequence:
    ; switch to protected mode
    cli ; we do not want to be interrupted

    lgdt [(0x1000 + (init_ap_gdt_wrap - application_processor_initialization_sequence))] ; load GDT table that allows using 4 GB of memory

    mov eax, cr0
    or eax, 0x00000001 ; set protected mode bit
    mov cr0, eax

    jmp 0x0008:(0x1000 + (protected_mode_code - application_processor_initialization_sequence)) ; set cs register to GDT code segment 0x0008
    bits 32
    protected_mode_code:
    mov ax, 0x0010 ; set all other segment registers to GDT data segment 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; enable paging
    mov eax, 0x4000 ; physical page of Page Directory
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000 ; paging bit
    mov cr0, eax

    ; set stack pointer
    mov esp, dword [(0x1000 + (ap_stack_pointer - application_processor_initialization_sequence))]

    ; start execution of application processor main code
    mov eax, dword [0x1FF8]
    call eax

    halt:
        hlt
    jmp halt

; Protected mode Global Descriptor Table
align 16
init_ap_protected_mode_gdt:
    ; 0x00 segment must be null
    dq 0

    ; 0x08 segment is code segment
    ; it grants access to whole memory from 0x00000000 to 0xFFFFFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10011011b    ; access: present, privilege level ring 0, code segment, readable, accessed
    db 11001111b    ; flags: 4 KB blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)

    ; 0x10 segment is data segment
    ; it grants access to whole memory from 0x00000000 to 0xFFFFFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10010011b    ; access: present, privilege level ring 0, data segment, readable, accessed
    db 11001111b    ; flags: 4 KB blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)
init_ap_protected_mode_gdt_end:

init_ap_gdt_wrap:
    dw init_ap_protected_mode_gdt_end - init_ap_protected_mode_gdt - 1 ; limit
    dd (0x1000 + (init_ap_protected_mode_gdt - application_processor_initialization_sequence)) ; base

global ap_stack_pointer
ap_stack_pointer dd 0

global application_processor_initialization_sequence_end
application_processor_initialization_sequence_end: