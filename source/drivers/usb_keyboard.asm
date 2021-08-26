;BleskOS

usb_keyboard_controller dd 0
usb_keyboard_base dd 0
usb_keyboard_controller_number dd 0
usb_keyboard_address db 0
usb_keyboard_speed dd 0
usb_keyboard_endpoint dd 0

usb_keyboard_data dd 0
usb_keyboard_wait dd 0

usb_keyboard_uhci_remove:
 mov ax, word [usb_keyboard_base]
 cmp word [uhci_base], ax
 jne .done
 mov al, byte [usb_keyboard_address]
 cmp byte [uhci_address], al
 jne .done

 mov dword [usb_keyboard_controller], 0
 mov dword [usb_keyboard_base], 0
 mov dword [usb_keyboard_controller_number], 0
 mov dword [usb_keyboard_speed], 0
 mov dword [usb_keyboard_endpoint], 0

 .done:
 ret

read_usb_keyboard:
 cmp dword [usb_keyboard_controller], UHCI
 je .uhci
 cmp dword [usb_keyboard_controller], OHCI
 je .ohci
 ret

 .uhci:
 mov ax, word [usb_keyboard_base]
 mov word [uhci_base], ax
 mov eax, dword [usb_keyboard_controller_number]
 mov dword [uhci_controller_number], eax
 mov eax, dword [usb_keyboard_speed]
 mov dword [uhci_device_speed], eax
 mov eax, dword [usb_keyboard_endpoint]
 mov dword [uhci_endpoint], eax
 mov al, byte [usb_keyboard_address]
 mov byte [uhci_address], al

 call uhci_read_hid
 mov eax, dword [MEMORY_UHCI+0x10200]
 mov dword [usb_keyboard_data], eax
 jmp .translate

 .ohci:
 mov ax, word [usb_keyboard_base]
 mov word [ohci_base], ax
 mov eax, dword [usb_keyboard_speed]
 mov dword [ohci_device_speed], eax
 mov eax, dword [usb_keyboard_endpoint]
 mov dword [ohci_endpoint], eax

 mov dword [MEMORY_OHCI+0x300], 0
 mov dword [MEMORY_OHCI+0x300+4], 0
 call ohci_read_hid
 mov eax, dword [MEMORY_OHCI+0x300]
 mov dword [usb_keyboard_data], eax

 ret

 .translate:
 mov eax, 0
 mov al, byte [usb_keyboard_data+2]
 add eax, usb_keyboard_layout
 mov bl, byte [eax]
 mov byte [key_code], bl

 cmp bl, KEY_LEFT_SHIFT
 je .shift
 cmp bl, KEY_RIGHT_SHIFT
 je .shift
 mov dword [keyboard_shift], 0
 test byte [usb_keyboard_data], 0x22
 jz .if_shift
  mov dword [keyboard_shift], 1
 .if_shift:

 .convert:
 mov al, bl
 call keyboard_convert_to_unicode

 ret

 .shift:
 mov eax, 0
 mov al, byte [usb_keyboard_data+3]
 add eax, usb_keyboard_layout
 mov bl, byte [eax]
 mov byte [key_code], bl
 mov dword [keyboard_shift], 1
 jmp .convert

wait_for_usb_keyboard:
 mov dword [keyboard_wait], 1 ;PS/2 keyboard
 mov byte [key_code], 0
 mov word [key_unicode], 0
 mov dword [ticks], 0

 .wait:
  hlt

  cmp dword [keyboard_wait], 0
  je .done

  cmp dword [ticks], 8
  jl .wait
  mov dword [usb_keyboard_data], 0
  call read_usb_keyboard
  cmp dword [key_code], 0x0
  jne .done
  mov dword [ticks], 0
 jmp .wait

 .done:
 ret
