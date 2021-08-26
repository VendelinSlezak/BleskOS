;BleskOS

%macro UHCI_FILL_FRAME_LIST 1
 mov eax, dword [uhci_controller_number]
 mov ebx, 4096
 mul ebx
 add eax, MEMORY_UHCI

 mov edi, eax
 mov eax, %1
 mov ecx, 1024
 rep stosd
%endmacro

%macro UHCI_CREATE_QH 0
 mov dword [MEMORY_UHCI+0x10000], 0x1
 mov dword [MEMORY_UHCI+0x10004], MEMORY_UHCI+0x10100
%endmacro

%macro UHCI_CREATE_TD_ZERO_DEVICE 4
 mov dword [%1+0], %2
 mov eax, dword [uhci_device_speed]
 mov dword [%1+4], eax
 mov dword [%1+8], %3
 mov dword [%1+12], %4
%endmacro

%macro UHCI_CREATE_TD 4
 mov dword [%1+0], %2
 mov eax, dword [uhci_device_speed]
 mov dword [%1+4], eax
 mov ebx, 0
 mov bh, byte [uhci_address]
 or ebx, %3
 mov dword [%1+8], ebx
 mov dword [%1+12], %4
%endmacro

%define UHCI_LOW_SPEED 0x04800000
%define UHCI_FULL_SPEED 0x00800000
%define UHCI_SETUP 0x2D
%define UHCI_IN 0x69
%define UHCI_OUT 0xE1
%define UHCI_DATA_TOGGLE_0 0x00000000
%define UHCI_DATA_TOGGLE_1 0x00080000
%define UHCI_TRANSFER_8_BYTES 0x00E00000
%define UHCI_TRANSFER_0_BYTES 0xFFE00000

uhci_base dw 0
uhci_port_base dw 0
uhci_address db 0
uhci_controller_number dd 0
uhci_conf_number dd 0
uhci_interface_number dd 0
uhci_alt_interface_number dd 0

uhci_device_speed dd 0
uhci_endpoint dd 0
uhci_td dd 0
uhci_wait dd 0

uhci_reset_every_device dd 0

init_uhci:
 ;reset controller
 BASE_OUTW uhci_base, 0x0, 0x4
 WAIT 10
 BASE_OUTW uhci_base, 0x0, 0x0

 ;init other things
 BASE_OUTW uhci_base, 0x2, 0x3F ;clear status
 BASE_OUTW uhci_base, 0x4, 0x0 ;disable interrupts
 BASE_OUTB uhci_base, 0xC, 0x40 ;set SOF
 BASE_OUTW uhci_base, 0x6, 0 ;frame number 0
 mov eax, dword [uhci_controller_number]
 mov ebx, 4096
 mul ebx
 add eax, MEMORY_UHCI
 BASE_OUTD uhci_base, 0x8, eax ;set frame address pointer

 ;clear frame list
 UHCI_FILL_FRAME_LIST 0x1
 BASE_OUTW uhci_base, 0x0, 0x1 ;run controller

 ret

uhci_detect_devices:
 mov ax, word [uhci_base]
 add ax, 0x10
 mov word [uhci_port_base], ax
 mov byte [uhci_address], 1

 mov ecx, 10 ;max ten ports
 .detect_device:
  BASE_INW uhci_port_base, 0
  cmp ax, 0xFFFF
  je .done
  test ax, 0x80
  jz .done
  BASE_INW uhci_port_base, 0

  push ecx
  call uhci_detect_device
  pop ecx

  add word [uhci_port_base], 0x2
  inc byte [uhci_address]
 loop .detect_device

 .done:
 ret

uhci_detect_device:
 ;is device connected?
 BASE_INW uhci_port_base, 0
 test ax, 0x1
 jz .no_device

 cmp dword [uhci_reset_every_device], 1
 je .initalize_device

 ;is device initalized?
 BASE_INW uhci_port_base, 0
 test ax, 0x2
 jz .initalized_device

 .initalize_device:
 call usb_keyboard_uhci_remove
 call usb_mouse_uhci_remove
 BASE_OUTW uhci_port_base, 0, 0x200 ;reset device
 WAIT 50
 BASE_OUTW uhci_port_base, 0, 0x0
 WAIT 30

 BASE_INW uhci_port_base, 0
 mov dword [uhci_device_speed], UHCI_FULL_SPEED
 test ax, 0x100
 jz .if_low_speed
  mov dword [uhci_device_speed], UHCI_LOW_SPEED
 .if_low_speed:

 BASE_OUTW uhci_port_base, 0, 0x6 ;enable device and clear status change
 WAIT 50
 BASE_INW uhci_port_base, 0
 test ax, 0x4
 jz .no_device ;device is not enabled

 call uhci_set_address
 call uhci_read_descriptor
 ret

 .no_device:
 call usb_keyboard_uhci_remove
 call usb_mouse_uhci_remove
 .initalized_device:
 ret

uhci_set_address:
 UHCI_CREATE_QH
 UHCI_CREATE_TD_ZERO_DEVICE MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10110 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_SETUP), MEMORY_UHCI+0x10200
 UHCI_CREATE_TD_ZERO_DEVICE MEMORY_UHCI+0x10110, 0x1, (UHCI_TRANSFER_0_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), 0x0
 mov dword [MEMORY_UHCI+0x10200], 0x00000500
 mov dword [MEMORY_UHCI+0x10204], 0x00000000
 mov al, byte [uhci_address]
 mov byte [MEMORY_UHCI+0x10202], al

 mov dword [uhci_td], MEMORY_UHCI+0x10110+4
 mov dword [uhci_wait], 100
 call uhci_transfer_queue_head

 ret

uhci_read_descriptor:
 UHCI_CREATE_QH
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10110 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_SETUP), MEMORY_UHCI+0x10200
 UHCI_CREATE_TD MEMORY_UHCI+0x10110, MEMORY_UHCI+0x10120 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), MEMORY_UHCI+0x10300
 UHCI_CREATE_TD MEMORY_UHCI+0x10120, MEMORY_UHCI+0x10130 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_IN), MEMORY_UHCI+0x10300+8
 UHCI_CREATE_TD MEMORY_UHCI+0x10130, MEMORY_UHCI+0x10140 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), MEMORY_UHCI+0x10300+16
 UHCI_CREATE_TD MEMORY_UHCI+0x10140, MEMORY_UHCI+0x10150 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_IN), MEMORY_UHCI+0x10300+24
 UHCI_CREATE_TD MEMORY_UHCI+0x10150, MEMORY_UHCI+0x10160 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), MEMORY_UHCI+0x10300+32
 UHCI_CREATE_TD MEMORY_UHCI+0x10160, MEMORY_UHCI+0x10170 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_IN), MEMORY_UHCI+0x10300+40
 UHCI_CREATE_TD MEMORY_UHCI+0x10170, MEMORY_UHCI+0x10180 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), MEMORY_UHCI+0x10300+48
 UHCI_CREATE_TD MEMORY_UHCI+0x10180, 0x1, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_IN), MEMORY_UHCI+0x10300+56
 mov dword [MEMORY_UHCI+0x10200], 0x02000680
 mov dword [MEMORY_UHCI+0x10204], 0x00400000

 mov dword [uhci_td], MEMORY_UHCI+0x10180+4
 mov dword [uhci_wait], 100
 call uhci_transfer_queue_head

 ;status stage
 UHCI_CREATE_QH
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, 0x1, (UHCI_TRANSFER_0_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_OUT), 0x0
 mov dword [uhci_td], MEMORY_UHCI+0x10100+4
 mov dword [uhci_wait], 100
 call uhci_transfer_queue_head

 ;parse descriptor
 mov esi, MEMORY_UHCI+0x10300
 call parse_usb_descriptor

 mov al, byte [usb_descriptor+20]
 mov byte [uhci_conf_number], al
 mov al, byte [usb_descriptor+21]
 mov byte [uhci_interface_number], al
 mov al, byte [usb_descriptor+22]
 mov byte [uhci_alt_interface_number], al
 cmp dword [usb_descriptor+23], 0x00020103
 je .usb_mouse_2

 mov al, byte [usb_descriptor+10]
 mov byte [uhci_conf_number], al
 mov al, byte [usb_descriptor+11]
 mov byte [uhci_interface_number], al
 mov al, byte [usb_descriptor+12]
 mov byte [uhci_alt_interface_number], al
 cmp dword [usb_descriptor+13], 0x00010103
 je .usb_keyboard
 cmp dword [usb_descriptor+13], 0x00020103
 je .usb_mouse
 ret

 .usb_keyboard:
 call uhci_set_configuration
 call uhci_set_interface
 call uhci_set_idle

 ;save keyboard
 mov dword [usb_keyboard_controller], UHCI
 mov ax, word [uhci_base]
 mov word [usb_keyboard_base], ax
 mov eax, dword [uhci_controller_number]
 mov dword [usb_keyboard_controller_number], eax
 mov eax, dword [uhci_device_speed]
 mov dword [usb_keyboard_speed], eax
 mov eax, 0
 mov al, byte [usb_descriptor+19]
 mov dword [usb_keyboard_endpoint], eax
 PVAR eax
 mov al, byte [uhci_address]
 mov byte [usb_keyboard_address], al

 ret

 .usb_mouse:
 call uhci_set_configuration
 call uhci_set_interface
 call uhci_set_idle

 ;save mouse
 mov dword [usb_mouse_controller], UHCI
 mov ax, word [uhci_base]
 mov word [usb_mouse_base], ax
 mov eax, dword [uhci_controller_number]
 mov dword [usb_mouse_controller_number], eax
 mov eax, dword [uhci_device_speed]
 mov dword [usb_mouse_speed], eax
 mov eax, 0
 mov al, byte [usb_descriptor+19]
 mov dword [usb_mouse_endpoint], eax
 PVAR eax
 mov al, byte [uhci_address]
 mov byte [usb_mouse_address], al

 ret

 .usb_mouse_2:
 call .usb_mouse
 mov eax, 0
 mov al, byte [usb_descriptor+29]
 mov dword [usb_mouse_endpoint], eax
 PVAR eax

 ret

uhci_set_configuration:
 UHCI_CREATE_QH
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10110 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_SETUP), MEMORY_UHCI+0x10200
 UHCI_CREATE_TD MEMORY_UHCI+0x10110, 0x1, (UHCI_TRANSFER_0_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), 0x0
 mov dword [MEMORY_UHCI+0x10200], 0x00000900
 mov dword [MEMORY_UHCI+0x10204], 0x00000000
 mov al, byte [uhci_conf_number]
 mov byte [MEMORY_UHCI+0x10202], al

 mov dword [uhci_td], MEMORY_UHCI+0x10110+4
 mov dword [uhci_wait], 100
 call uhci_transfer_queue_head

 ret

uhci_set_interface:
 UHCI_CREATE_QH
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10110 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_SETUP), MEMORY_UHCI+0x10200
 UHCI_CREATE_TD MEMORY_UHCI+0x10110, 0x1, (UHCI_TRANSFER_0_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), 0x0
 mov dword [MEMORY_UHCI+0x10200], 0x00000B01
 mov dword [MEMORY_UHCI+0x10204], 0x00000000
 mov al, byte [uhci_interface_number]
 mov byte [MEMORY_UHCI+0x10204], al
 mov al, byte [uhci_alt_interface_number]
 mov byte [MEMORY_UHCI+0x10202], al

 mov dword [uhci_td], MEMORY_UHCI+0x10110+4
 mov dword [uhci_wait], 100
 call uhci_transfer_queue_head

 ret

uhci_set_idle:
 UHCI_CREATE_QH
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10110 | 0x4, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_SETUP), MEMORY_UHCI+0x10200
 UHCI_CREATE_TD MEMORY_UHCI+0x10110, 0x1, (UHCI_TRANSFER_0_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN), 0x0
 mov dword [MEMORY_UHCI+0x10200], 0x00000A21
 mov dword [MEMORY_UHCI+0x10204], 0x00000000
 mov al, byte [uhci_interface_number]
 mov byte [MEMORY_UHCI+0x10204], al

 mov dword [uhci_td], MEMORY_UHCI+0x10110+4
 mov dword [uhci_wait], 100
 call uhci_transfer_queue_head

 ret

uhci_read_hid:
 UHCI_CREATE_QH
 mov ecx, dword [uhci_endpoint]
 shl ecx, 15
 or ecx, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_0 | UHCI_IN)
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10110 | 0x4, ecx, MEMORY_UHCI+0x10200
 mov ecx, dword [uhci_endpoint]
 shl ecx, 15
 or ecx, (UHCI_TRANSFER_8_BYTES | UHCI_DATA_TOGGLE_1 | UHCI_IN)
 UHCI_CREATE_TD MEMORY_UHCI+0x10110, 0x1, ecx, MEMORY_UHCI+0x10208

 mov dword [uhci_td], MEMORY_UHCI+0x10110+4
 mov dword [uhci_wait], 3
 call uhci_transfer_queue_head

 test dword [MEMORY_UHCI+0x10104], 0x007F0000
 jnz .error

 mov al, byte [MEMORY_UHCI+0x10104]
 inc al ;number of transferred bytes

 ret

 .error:
 mov dword [MEMORY_UHCI+0x10200], 0
 mov dword [MEMORY_UHCI+0x10200+4], 0
 ret

uhci_transfer_queue_head:
 UHCI_FILL_FRAME_LIST MEMORY_UHCI+0x10000 | 2

 mov eax, dword [uhci_td]
 mov ebx, dword [uhci_wait]
 mov dword [ticks], 0
 .wait:
  mov ecx, dword [eax]
  test ecx, 0x00800000
  jz .done
 cmp dword [ticks], ebx
 jl .wait

 .done:
 UHCI_FILL_FRAME_LIST 0x1
 ret
