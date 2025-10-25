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

; Protected mode Global Descriptor Table
global protected_mode_gdt
global protected_mode_gdt_end
protected_mode_gdt:
    ; 0x00 segment must be null
    dq 0

    ; 0x08 segment is kernel code segment
    ; it grants access to whole memory from 0x00000000 to 0xFFFFFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10011011b    ; access: present, privilege level ring 0, code segment, readable, accessed
    db 11001111b    ; flags: 4 KB blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)

    ; 0x10 segment is kernel data segment
    ; it grants access to whole memory from 0x00000000 to 0xFFFFFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10010011b    ; access: present, privilege level ring 0, data segment, readable, accessed
    db 11001111b    ; flags: 4 KB blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)

    ; 0x18 segment is user code segment
    ; it grants access to whole memory from 0x00000000 to 0xFFFFFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 11111011b    ; access: present, privilege level ring 3, code segment, readable, accessed
    db 11001111b    ; flags: 4 KB blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)

    ; 0x20 segment is user data segment
    ; it grants access to whole memory from 0x00000000 to 0xFFFFFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 11110011b    ; access: present, privilege level ring 3, data segment, readable, accessed
    db 11001111b    ; flags: 4 KB blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)

    ; 0x28 segment is task state segment
    ; it sets TSS to page 0xFF402000
    dw 0x0067       ; limit (0x0????)
    dw 0x2000       ; base (0x0000????)
    db 0x40         ; base (0x00??0000)
    db 10001001b    ; access: present, privilege level ring 0, system segment, 32-bit available TSS
    db 00000000b    ; flags: 1 B blocks, 32-bit protected mode segment; limit (0x?0000)
    db 0xFF         ; base (0x??000000)
protected_mode_gdt_end:

global protected_mode_gdt_wrap
protected_mode_gdt_wrap:
    dw (protected_mode_gdt_end - protected_mode_gdt - 1)    ; size of GDT
    dd 0xFF400000                                           ; virtual memory position of GDT