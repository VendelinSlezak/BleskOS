;BleskOS

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
  OUTB 0x21, 0x0
  OUTB 0xA1, 0x0

  ;load idt
  mov edx, idt_wrap
  lidt [edx]

  sti

  ret

base_irq_master:
  pusha
  OUTB 0x20, 0x20
  popa
  iret

base_irq_slave:
  pusha
  OUTB 0xA0, 0x20
  OUTB 0x20, 0x20
  popa
  iret

%macro SET_IDT 2
  pusha

  mov eax, 0
  mov al, %1
  add al, 32 ;first irq
  mov ebx, 8
  mul ebx
  add eax, idt

  mov word [eax], %2

  popa
%endmacro
   

idt times 256 db 0 ;irs routines

idt_irq0: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq1: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq2: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq3: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq4: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq5: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq6: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq7: 
dw base_irq_master
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq8: 
dw base_irq_slave
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq9: 
dw base_irq_slave
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq10: 
dw base_irq_slave
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq11: 
dw base_irq_slave
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq12: 
dw base_irq_slave
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq13: 
dw base_irq_slave
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq14: 
dw ata_irq
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_irq15: 
dw atapi_irq
dw 0x0008
db 0, 0x8E
dw 0x0001

idt_zero times 1664 db 0 ;other interrupts

idt_wrap:
  dw 2048-1 ;lenght
  dd idt ;address
