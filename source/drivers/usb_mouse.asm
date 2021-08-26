;BleskOS

usb_mouse_controller dd 0
usb_mouse_base dd 0
usb_mouse_controller_number dd 0
usb_mouse_address db 0
usb_mouse_speed dd 0
usb_mouse_endpoint dd 0

usb_mouse_data dd 0
usb_mouse_wait dd 0

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
 cmp dword [usb_mouse_controller], UHCI
 je .uhci
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

 call uhci_read_hid
 cmp al, 3
 je .usb_mouse_3_bytes
 cmp al, 4
 je .usb_mouse_4_bytes
 cmp al, 7
 je .usb_mouse_7_bytes

 ret

 .usb_mouse_3_bytes:
 mov eax, dword [MEMORY_UHCI+0x10200]
 and eax, 0x00FFFFFF
 mov dword [usb_mouse_data], eax
 ret

 .usb_mouse_4_bytes:
 mov eax, dword [MEMORY_UHCI+0x10200]
 mov dword [usb_mouse_data], eax
 ret

 .usb_mouse_7_bytes:
 mov eax, dword [MEMORY_UHCI+0x10201]
 and eax, 0x00FFFFFF
 mov bl, byte [MEMORY_UHCI+0x10205]
 shl ebx, 24
 or eax, ebx
 mov dword [usb_mouse_data], eax
 ret

wait_for_usb_mouse:
 mov dword [usb_mouse_wait], 0
 mov dword [ps2_mouse_wait], 1 ;PS/2 mouse
 mov dword [ps2_mouse_data_pointer], ps2_mouse_data
 mov dword [keyboard_wait], 1 ;PS/2 keyboard

 .wait:
  hlt

  cmp dword [keyboard_wait], 0
  je .done

  cmp dword [ps2_mouse_wait], 0
  je .ps2_mouse

  inc dword [usb_mouse_wait]
  cmp dword [usb_mouse_wait], 8
  jl .wait
  mov dword [usb_mouse_data], 0
  call read_usb_mouse
  cmp dword [usb_mouse_data], 0x0
  jne .done
  mov dword [usb_mouse_wait], 0
 jmp .wait

 .ps2_mouse:
 mov eax, dword [ps2_mouse_data]
 mov dword [usb_mouse_data], eax

 .done:
 ret
