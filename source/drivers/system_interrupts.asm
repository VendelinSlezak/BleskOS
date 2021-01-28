;BleskOS

%define EMPTY_IDT_ENTRY dq 0

%macro IDT_ENTRY 1
 dw %1
 dw 0x0008
 db 0, 0x8E
 dw 0x0001
%endmacro

%macro EOI_MASTER_PIC 0
 OUTB 0x20, 0x20
%endmacro

%macro EOI_SLAVE_PIC 0
 OUTB 0xA0, 0x20
 OUTB 0x20, 0x20
%endmacro

idt:
 times 32 EMPTY_IDT_ENTRY
 IDT_ENTRY timer_irq
 IDT_ENTRY keyboard_irq
 times 10 EMPTY_IDT_ENTRY
 IDT_ENTRY mouse_irq
 times 256-24 EMPTY_IDT_ENTRY

idt_wrap:
 dw 2047 ;lenght
 dd idt ;address

init_idt:
 cli

 ;remapping PIC
 OUTB 0x20, 0x11
 OUTB 0xA0, 0x11
 OUTB 0x21, 0x20
 OUTB 0xA1, 40
 OUTB 0x21, 0x04
 OUTB 0xA1, 0x02
 OUTB 0x21, 0x01
 OUTB 0xA1, 0x01
 OUTB 0x21, 0xF8 ;irq 0, 1, 2
 OUTB 0xA1, 0xEF ;irq 12

 ;load idt
 mov edx, idt_wrap
 lidt [edx]

 sti

 ret
