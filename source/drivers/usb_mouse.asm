;BleskOS

usb_mouse_controller dd 0
usb_mouse_base dd 0
usb_mouse_controller_number dd 0
usb_mouse_speed dd 0

usb_mouse_data dd 0
usb_mouse_wait dd 0

read_usb_mouse:
 cmp dword [usb_mouse_base], 0
 je .done

 cmp dword [usb_mouse_controller], UHCI
 je .uhci

 jmp .done

 .uhci:
 mov ax, word [usb_mouse_base]
 mov word [uhci_base], ax
 mov eax, dword [usb_mouse_controller_number]
 mov dword [uhci_controller_number], eax
 mov eax, dword [usb_mouse_speed]
 mov dword [uhci_device_speed], eax

 mov dword [MEMORY_UHCI+0x10300], 0
 call uhci_read_hid

 mov eax, dword [MEMORY_UHCI+0x10300]
 mov dword [usb_mouse_data], eax

 .done:
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
