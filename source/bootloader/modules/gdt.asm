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

; Unreal mode Global Descriptor Table
align 16
unreal_mode_gdt:
    ; 0x00 segment must be null
    dq 0

    ; 0x08 segment is code segment
    ; it grants access to 4 GB of memory
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10011011b    ; access: present, privilege level ring 0, code segment, readable, accessed
    db 10001111b    ; flags: 1 byte blocks, 16-bit real mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)

    ; 0x10 segment is data segment
    ; it grants access to 4 GB of memory
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10010011b    ; access: present, privilege level ring 0, data segment, readable, accessed
    db 10001111b    ; flags: 1 byte blocks, 16-bit real mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)
unreal_mode_gdt_end:

unreal_mode_gdt_wrap:
    dw (unreal_mode_gdt_end - unreal_mode_gdt - 1)  ; size of GDT
    dd unreal_mode_gdt                              ; physical memory position of GDT

; Real mode Global Descriptor Table
align 16
real_mode_gdt:
    ; 0x00 segment must be null
    dq 0

    ; 0x08 segment is code segment
    ; it grants access to first MB of memory from 0x0000000 to 0x000FFFFF
    dw 0xFFFF       ; limit (0x0????)
    dw 0x0000       ; base (0x0000????)
    db 0x00         ; base (0x00??0000)
    db 10011011b    ; access: present, privilege level ring 0, code segment, readable, accessed
    db 00001111b    ; flags: 1 byte blocks, 16-bit real mode segment; limit (0x?0000)
    db 0x00         ; base (0x??000000)
real_mode_gdt_end:

real_mode_gdt_wrap:
    dw (real_mode_gdt - real_mode_gdt_end - 1)  ; size of GDT
    dd real_mode_gdt                            ; physical memory position of GDT

; Protected mode Global Descriptor Table
align 16
protected_mode_gdt:
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
protected_mode_gdt_end:

protected_mode_gdt_wrap:
    dw (protected_mode_gdt_end - protected_mode_gdt - 1)    ; size of GDT
    dd protected_mode_gdt                                   ; physical memory position of GDT