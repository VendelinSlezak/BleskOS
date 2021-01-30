;BleskOS

mouse_packet_bytes dd 0
mouse_data_pointer dd 0
mouse_data db 0, 0, 0, 0
mouse_wait dd 0

%macro WRITE_PS2_MOUSE 1
 WRITE_PS2_COMMAND 0xD4
 WRITE_PS2_DATA %1
%endmacro

%macro READ_PS2_MOUSE 0
 READ_PS2_DATA
%endmacro

init_ps2_mouse:
 ;normal settings
 WRITE_PS2_MOUSE 0xF6
 READ_PS2_MOUSE
 mov dword [mouse_packet_bytes], 3

 ;enable sending packets
 WRITE_PS2_MOUSE 0xF4
 READ_PS2_MOUSE
 mov dword [mouse_data_pointer], 0

 ret

mouse_irq:
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
