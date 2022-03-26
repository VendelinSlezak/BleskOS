;BleskOS

%define EMPTY_IDT_ENTRY dq 0

%macro IDT_ENTRY 1
 dw %1
 dw 0x0008
 dw 0x8E00
 dw (%1 - $$ + 0x10000) >> 16
%endmacro

%macro EOI_MASTER_PIC 0
 push eax
 mov al, 0x20
 out 0x20, al
 pop eax
%endmacro

%macro EOI_SLAVE_PIC 0
 push eax
 mov al, 0x20
 out 0xA0, al
 mov al, 0x20
 out 0x20, al
 pop eax
%endmacro

idt:
 times 32 EMPTY_IDT_ENTRY
 IDT_ENTRY timer_irq
 IDT_ENTRY keyboard_irq
 times 10 EMPTY_IDT_ENTRY
 IDT_ENTRY ps2_mouse_irq
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
 OUTB 0x21, 0x00 ;all irq
 OUTB 0xA1, 0xE0 ;all irq

 ;load idt
 mov edx, idt_wrap
 lidt [edx]

 sti

 ret
