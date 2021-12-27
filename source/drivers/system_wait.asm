;BleskOS

ticks dd 0
ticks2 dd 0
ticks3 dd 0
enable_ticks3 dd 0
al_save_state db 0

%macro WAIT 1
 %if (%1 & 0x1)==0x0
 mov eax, (%1/2+1)
 %endif
 %if (%1 & 0x1)==0x1
 mov eax, (%1/2+2)
 %endif
 call wait_pit
%endmacro

set_pit:
 OUTB 0x43, 0x36
 OUTB 0x40, 2386 & 0xFF ;500 Hz = every 2 miliseconds
 OUTB 0x40, 2386 >> 8

 ret

timer_irq:
 inc dword [ticks]
 inc dword [ticks2]
 cmp dword [enable_ticks3], 0
 je .if_enable_ticks3
  inc dword [ticks3]
 .if_enable_ticks3:
 
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
