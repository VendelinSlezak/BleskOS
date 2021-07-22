;BleskOS

ticks dd 0
al_save_state db 0

%macro WAIT 1
 mov eax, %1+1
 call wait_pit
%endmacro

set_pit:
 OUTB 0x43, 0x36
 OUTB 0x40, 1024 & 0xFF
 OUTB 0x40, 1024 >> 8

 ret

timer_irq:
 inc dword [ticks]
 
 mov byte [al_save_state], al
 mov al, 0x20
 out 0x20, al
 mov al, byte [al_save_state]

 iret

wait_pit:
 mov dword [ticks], 0
 .wait:
  hlt
 cmp dword [ticks], eax
 jl .wait

 ret
