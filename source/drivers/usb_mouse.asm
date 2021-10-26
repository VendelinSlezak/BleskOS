;BleskOS

%macro TEST_CLICK_ZONE 5
 cmp dword [ge_mouse_line], %2
 jb .%1_not_pass
 cmp dword [ge_mouse_line], %3
 ja .%1_not_pass
 cmp dword [ge_mouse_column], %4
 jb .%1_not_pass
 cmp dword [ge_mouse_column], %5
 ja .%1_not_pass
 mov eax, 1
 jmp .%1_over
 .%1_not_pass:
 mov eax, 0
 .%1_over:
%endmacro

usb_mouse_controller dd 0
usb_mouse_base dd 0
usb_mouse_controller_number dd 0
usb_mouse_address db 0
usb_mouse_speed dd 0
usb_mouse_endpoint dd 0
usb_mouse_toggle dd 0
usb_mouse_interval dd 0

usb_mouse_data dd 0, 0
old_usb_mouse_data dd 0
usb_mouse_dnd dd 0 ;drag and drop

usb_mouse_ohci_remove:
 mov ax, word [usb_mouse_base]
 cmp word [ohci_base], ax
 jne .done
 mov al, byte [usb_mouse_address]
 cmp byte [ohci_address], al
 jne .done

 mov dword [usb_mouse_controller], 0
 mov dword [usb_mouse_base], 0
 mov dword [usb_mouse_controller_number], 0
 mov dword [usb_mouse_speed], 0
 mov dword [usb_mouse_endpoint], 0

 .done:
 ret

usb_mouse_uhci_remove:
 mov ax, word [usb_mouse_base]
 cmp word [uhci_base], ax
 jne .done
 mov al, byte [usb_mouse_address]
 cmp byte [uhci_address], al
 jne .done

 mov dword [usb_mouse_controller], 0
 mov dword [usb_mouse_base], 0
 mov dword [usb_mouse_controller_number], 0
 mov dword [usb_mouse_speed], 0
 mov dword [usb_mouse_endpoint], 0

 .done:
 ret

read_usb_mouse:
 mov dword [usb_mouse_data], 0
 cmp dword [usb_mouse_controller], UHCI
 je .uhci
 cmp dword [usb_mouse_controller], OHCI
 je .ohci
 ret

 .uhci:
 mov ax, word [usb_mouse_base]
 mov word [uhci_base], ax
 mov eax, dword [usb_mouse_controller_number]
 mov dword [uhci_controller_number], eax
 mov eax, dword [usb_mouse_speed]
 mov dword [uhci_device_speed], eax
 mov eax, dword [usb_mouse_endpoint]
 mov dword [uhci_endpoint], eax
 mov al, byte [usb_mouse_address]
 mov byte [uhci_address], al
 mov eax, dword [usb_mouse_toggle]
 mov dword [uhci_toggle], eax
 
 call uhci_read_hid

 or dword [usb_mouse_toggle], 0xFFFFFFFE
 not dword [usb_mouse_toggle] ;reverse first bit
 
 mov ebx, dword [MEMORY_UHCI+0x10200]
 mov dword [usb_mouse_data], ebx
 mov ebx, dword [MEMORY_UHCI+0x10204]
 mov dword [usb_mouse_data+4], ebx

 jmp .parse_data

 .ohci:
 mov ax, word [usb_mouse_base]
 mov word [ohci_base], ax
 mov eax, dword [usb_mouse_speed]
 mov dword [ohci_device_speed], eax
 mov eax, dword [usb_mouse_endpoint]
 mov dword [ohci_endpoint], eax
 mov al, byte [usb_mouse_address]
 mov byte [ohci_address], al
 mov eax, dword [usb_mouse_toggle]
 mov dword [ohci_toggle], eax
 
 call ohci_read_hid
 mov ebx, dword [MEMORY_OHCI+0x300]
 mov dword [usb_mouse_data], ebx
 mov ebx, dword [MEMORY_OHCI+0x304]
 mov dword [usb_mouse_data+4], ebx
 
 or dword [usb_mouse_toggle], 0xFFFFFFFE
 not dword [usb_mouse_toggle] ;reverse first bit
 
 jmp .parse_data
 
 .parse_data:
 cmp al, 3
 je .usb_mouse_3_bytes
 cmp al, 4
 je .usb_mouse_4_bytes
 cmp al, 7
 je .usb_mouse_7_bytes
 ret
 
 .usb_mouse_3_bytes:
 and dword [usb_mouse_data], 0x00FFFFFF
 ret

 .usb_mouse_4_bytes:
 ret

 .usb_mouse_7_bytes:
 mov eax, dword [usb_mouse_data+1]
 and eax, 0x00FFFFFF
 mov bl, byte [usb_mouse_data+5]
 shl ebx, 24
 or eax, ebx
 mov dword [usb_mouse_data], eax
 ret

wait_for_usb_mouse:
 mov eax, dword [usb_mouse_data]
 mov dword [old_usb_mouse_data], eax
 
 mov dword [usb_mouse_data], 0
 mov dword [ps2_mouse_wait], 1 ;PS/2 mouse
 mov dword [ps2_mouse_data_pointer], ps2_mouse_data
 mov dword [keyboard_wait], 1 ;PS/2 keyboard
 mov byte [key_code], 0
 mov word [key_unicode], 0
 mov dword [usb_keyboard_wait], 0

 .wait:
  hlt

  cmp dword [keyboard_wait], 0
  je .done

  cmp dword [ps2_mouse_wait], 0
  je .ps2_mouse
  
  mov eax, dword [ticks]
  cmp eax, dword [usb_mouse_interval]
  jl .wait
  call read_usb_mouse
  mov dword [ticks], 0
  cmp dword [usb_mouse_data], 0
  je .wait
  
  mov eax, dword [old_usb_mouse_data]
  cmp dword [usb_mouse_data], eax
  je .wait
 jmp .test_drag_and_drop

 .ps2_mouse:
 mov eax, dword [ps2_mouse_data]
 mov dword [usb_mouse_data], eax
 mov al, 0xFF
 sub al, byte [usb_mouse_data+2]
 mov byte [usb_mouse_data+2], al ;touchpad have reversed up/down byte, so we reverse it back

 .test_drag_and_drop:
 test byte [usb_mouse_data], 0x1
 jz .no_dnd
 cmp dword [usb_mouse_dnd], 2
 je .done
 inc dword [usb_mouse_dnd]
 jmp .done
 .no_dnd:
 mov dword [usb_mouse_dnd], 0
 
 .done:
 ret
