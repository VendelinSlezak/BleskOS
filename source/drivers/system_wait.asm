;BleskOS

ticks dd 0

%macro WAIT 1
 mov eax, %1
 call wait_pit
%endmacro

set_pit:
 OUTB 0x43, 0x36
 OUTB 0x40, 10000 & 0xFF
 OUTB 0x40, 10000 >> 8

 ret

timer_irq:
 push eax
 push edx
 inc dword [ticks]
 EOI_MASTER_PIC
 pop edx
 pop eax
 iret

wait_pit:
 mov dword [ticks], 0
 .wait:
  hlt
  cmp dword [ticks], eax
  je .done
 jmp .wait

 .done:
 ret
