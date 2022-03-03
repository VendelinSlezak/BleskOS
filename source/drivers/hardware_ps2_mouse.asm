;BleskOS

;Touchpad is emulating PS/2 mouse

ps2_mouse_present dd 0
ps2_mouse_data_pointer dd ps2_mouse_data
ps2_mouse_data dd 0
ps2_mouse_wait dd 0
ps2_mouse_ack dd 0

%macro WRITE_PS2_MOUSE 1
 mov byte [ps2_command], 0xD4
 call write_command_ps2_controller
 mov byte [ps2_command], %1
 call write_ps2_controller
%endmacro

%macro READ_PS2_MOUSE 0
 call read_ps2_controller
%endmacro

enable_touchpad:
 cmp dword [ps2_mouse_present], 0
 je .done

 ;enable sending packets
 mov dword [ps2_mouse_ack], 1
 WRITE_PS2_MOUSE 0xF6
 READ_PS2_MOUSE
 WRITE_PS2_MOUSE 0xF4
 READ_PS2_MOUSE
 mov dword [ps2_mouse_data_pointer], ps2_mouse_data
 mov dword [ps2_mouse_ack], 0

 .done:
 ret

disable_touchpad:
 cmp dword [ps2_mouse_present], 0
 je .done

 ;disable sending packets
 mov dword [ps2_mouse_ack], 1
 WRITE_PS2_MOUSE 0xF5
 READ_PS2_MOUSE

 .done:
 ret

ps2_mouse_irq:
 push eax
 push ebx

 in al, 0x60

 ;ACK
 cmp dword [ps2_mouse_ack], 1
 je .done

 ;save data
 mov ebx, dword [ps2_mouse_data_pointer]
 mov byte [ebx], al
 inc dword [ps2_mouse_data_pointer]

 ;packet is received
 cmp dword [ps2_mouse_data_pointer], ps2_mouse_data+3
 jl .done

 mov dword [ps2_mouse_data_pointer], ps2_mouse_data
 mov dword [ps2_mouse_wait], 0

 .done:
 EOI_SLAVE_PIC
 pop ebx
 pop eax
 iret
