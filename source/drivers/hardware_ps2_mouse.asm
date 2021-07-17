;BleskOS

;Touchpad is emulating PS/2 mouse

mouse_packet_bytes dd 0
mouse_data_pointer dd 0
mouse_data db 0, 0, 0, 0
mouse_wait dd 0

%macro WRITE_PS2_MOUSE 1
 OUTB PS2_COMMAND_PORT, 0xD4
 mov byte [ps2_command], %1
 call write_ps2_controller
%endmacro

%macro READ_PS2_MOUSE 0
 call read_ps2_controller
%endmacro

enable_touchpad:
 mov dword [mouse_packet_bytes], 3

 ;enable sending packets
 WRITE_PS2_MOUSE 0xF4
 READ_PS2_MOUSE
 mov dword [mouse_data_pointer], 0

 ret

disable_touchpad:
 ;disable sending packets
 WRITE_PS2_MOUSE 0xF5
 READ_PS2_MOUSE

 ret

mouse_irq:
 pusha

 mov eax, 0
 INB 0x60

 ;save data
 mov ebx, mouse_data
 add ebx, dword [mouse_data_pointer]
 mov byte [ebx], al
 inc dword [mouse_data_pointer]

 ;packet is received
 mov eax, dword [mouse_packet_bytes]
 IF_E dword [mouse_data_pointer], eax, if_new_cycle
  mov dword [mouse_data_pointer], 0
  mov dword [mouse_wait], 0
 ENDIF if_new_cycle

 EOI_SLAVE_PIC
 popa
 iret

wait_for_mouse_or_keyboard:
 mov dword [mouse_wait], 1
 mov dword [keyboard_wait], 1

 .wait:
  cmp dword [mouse_wait], 0
  je .done
  cmp dword [keyboard_wait], 0
  je .done
  hlt
 jmp .wait

 .done:
 ret
