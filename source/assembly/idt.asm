;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023-2024 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

global load_idt
global irq_handlers
extern isr_handler

global stack_of_interrupt
stack_of_interrupt dd 0

%define EMPTY_IDT_ENTRY dq 0

%macro IDT_ENTRY 1
 dw ((%1 - $$ + 0x100000) & 0xFFFF)
 dw 0x0008
 dw 0x8E00
 dw 0x0010
%endmacro

%macro EOI_MASTER_PIC 0
 mov al, 0x20
 out 0x20, al
%endmacro

%macro EOI_SLAVE_PIC 0
 mov al, 0x20
 out 0xA0, al
 mov al, 0x20
 out 0x20, al
%endmacro

idt:
 IDT_ENTRY isr0_handler
 IDT_ENTRY isr1_handler
 IDT_ENTRY isr2_handler
 IDT_ENTRY isr3_handler
 IDT_ENTRY isr4_handler
 IDT_ENTRY isr5_handler
 IDT_ENTRY isr6_handler
 IDT_ENTRY isr7_handler
 IDT_ENTRY isr8_handler
 IDT_ENTRY isr9_handler
 IDT_ENTRY isr10_handler
 IDT_ENTRY isr11_handler
 IDT_ENTRY isr12_handler
 IDT_ENTRY isr13_handler
 IDT_ENTRY isr14_handler
 IDT_ENTRY isr15_handler
 IDT_ENTRY isr16_handler
 IDT_ENTRY isr17_handler
 IDT_ENTRY isr18_handler
 IDT_ENTRY isr19_handler
 IDT_ENTRY isr20_handler
 IDT_ENTRY isr21_handler
 IDT_ENTRY isr22_handler
 IDT_ENTRY isr23_handler
 IDT_ENTRY isr24_handler
 IDT_ENTRY isr25_handler
 IDT_ENTRY isr26_handler
 IDT_ENTRY isr27_handler
 IDT_ENTRY isr28_handler
 IDT_ENTRY isr29_handler
 IDT_ENTRY isr30_handler
 IDT_ENTRY isr31_handler
 IDT_ENTRY irq0_handler
 IDT_ENTRY irq1_handler
 IDT_ENTRY irq2_handler
 IDT_ENTRY irq3_handler
 IDT_ENTRY irq4_handler
 IDT_ENTRY irq5_handler
 IDT_ENTRY irq6_handler
 IDT_ENTRY irq7_handler
 IDT_ENTRY irq8_handler
 IDT_ENTRY irq9_handler
 IDT_ENTRY irq10_handler
 IDT_ENTRY irq11_handler
 IDT_ENTRY irq12_handler
 IDT_ENTRY irq13_handler
 IDT_ENTRY irq14_handler
 IDT_ENTRY irq15_handler
 times 256-48 EMPTY_IDT_ENTRY
 
isr0_handler:
 pusha
 push 0
 call isr_handler
 
isr1_handler:
 pusha
 push 1
 call isr_handler
 
isr2_handler:
 pusha
 push 2
 call isr_handler
 
isr3_handler:
 pusha
 push 3
 call isr_handler
 
isr4_handler:
 pusha
 push 4
 call isr_handler
 
isr5_handler:
 pusha
 push 5
 call isr_handler
 
isr6_handler:
 pusha
 push 6
 call isr_handler
 
isr7_handler:
 pusha
 push 7
 call isr_handler
 
isr8_handler:
 pusha
 push 8
 call isr_handler
 
isr9_handler:
 pusha
 push 9
 call isr_handler
 
isr10_handler:
 pusha
 push 10
 call isr_handler
 
isr11_handler:
 pusha
 push 11
 call isr_handler
 
isr12_handler:
 pusha
 push 12
 call isr_handler
 
isr13_handler:
 pusha
 push 13
 call isr_handler
 
isr14_handler:
 pusha
 push 14
 call isr_handler
 
isr15_handler:
 pusha
 push 15
 call isr_handler
 
isr16_handler:
 pusha
 push 16
 call isr_handler
 
isr17_handler:
 pusha
 push 17
 call isr_handler
 
isr18_handler:
 pusha
 push 18
 call isr_handler
 
isr19_handler:
 pusha
 push 19
 call isr_handler
 
isr20_handler:
 pusha
 push 20
 call isr_handler
 
isr21_handler:
 pusha
 push 21
 call isr_handler
 
isr22_handler:
 pusha
 push 22
 call isr_handler
 
isr23_handler:
 pusha
 push 23
 call isr_handler
 
isr24_handler:
 pusha
 push 24
 call isr_handler
 
isr25_handler:
 pusha
 push 25
 call isr_handler
 
isr26_handler:
 pusha
 push 26
 call isr_handler
 
isr27_handler:
 pusha
 push 27
 call isr_handler
 
isr28_handler:
 pusha
 push 28
 call isr_handler
 
isr29_handler:
 pusha
 push 29
 call isr_handler
 
isr30_handler:
 pusha
 push 30
 call isr_handler
 
isr31_handler:
 pusha
 push 31
 call isr_handler

idt_wrap:
 dw 2047 ;lenght
 dd idt ;address
 
irq_handlers:
 times 16 dd irq_basic_handler
 
irq_basic_handler:
 ret

irq0_handler:
 mov dword [stack_of_interrupt], esp

 pusha
 mov eax, dword [irq_handlers+(4*0)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq1_handler:
 pusha
 mov eax, dword [irq_handlers+(4*1)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq2_handler:
 pusha
 mov eax, dword [irq_handlers+(4*2)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq3_handler:
 pusha
 mov eax, dword [irq_handlers+(4*3)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq4_handler:
 pusha
 mov eax, dword [irq_handlers+(4*4)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq5_handler:
 pusha
 mov eax, dword [irq_handlers+(4*5)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq6_handler:
 pusha
 mov eax, dword [irq_handlers+(4*6)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq7_handler:
 pusha

 ;check if this is not spurious interrupt
 mov dx, 0x20
 mov al, 0xB
 out dx, al
 in al, dx
 test al, 0x80
 jnz .irq7_not_spurious_interrupt

 ;we do not do anything with spurious interrupt
 popa
 iret

 .irq7_not_spurious_interrupt:
 mov eax, dword [irq_handlers+(4*7)]
 call eax
 EOI_MASTER_PIC
 popa
 iret
 
irq8_handler:
 pusha
 mov eax, dword [irq_handlers+(4*8)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq9_handler:
 pusha
 mov eax, dword [irq_handlers+(4*9)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq10_handler:
 pusha
 mov eax, dword [irq_handlers+(4*10)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq11_handler:
 pusha
 mov eax, dword [irq_handlers+(4*11)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq12_handler:
 pusha
 mov eax, dword [irq_handlers+(4*12)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq13_handler:
 pusha
 mov eax, dword [irq_handlers+(4*13)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq14_handler:
 pusha
 mov eax, dword [irq_handlers+(4*14)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret
 
irq15_handler:
 pusha

 ;check if this is not spurious interrupt
 mov dx, 0xA0
 mov al, 0xB
 out dx, al
 in al, dx
 test al, 0x80
 jnz .irq15_not_spurious_interrupt

 ;we do not do anything with spurious interrupt, but because we are on slave PIC, we need to send EOI to master PIC because of connection to IRQ2
 EOI_MASTER_PIC
 popa
 iret

 .irq15_not_spurious_interrupt:
 mov eax, dword [irq_handlers+(4*15)]
 call eax
 EOI_SLAVE_PIC
 popa
 iret

load_idt:
 cli
 mov edx, idt_wrap
 lidt [edx]
 sti

 ret
