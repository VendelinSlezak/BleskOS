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

extern kernel_core_entry

bits 16

global core_initialization_sequence
core_initialization_sequence:
    ; switch to protected mode
    cli ; we do not want to be interrupted

    lgdt [0x1FF0] ; load GDT table that allows using 4 GB of memory

    mov eax, cr0
    or eax, 0x00000001 ; set protected mode bit
    mov cr0, eax

    jmp 0x0008:(0x1000 + (protected_mode_code - core_initialization_sequence)) ; set cs register to GDT code segment 0x0008
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

    ; set stack
    mov esp, dword [0x1E00]

    ; set signal that core is initalized
    mov dword [0x1E08], 1

    ; start execution
    mov eax, dword [0x1E04]
    call eax

    halt:
        hlt
    jmp halt

; Protected mode Global Descriptor Table
align 16
global init_core_protected_mode_gdt
global init_core_protected_mode_gdt_end
init_core_protected_mode_gdt:
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
init_core_protected_mode_gdt_end: