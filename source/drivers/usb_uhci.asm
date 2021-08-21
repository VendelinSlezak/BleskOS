;BleskOS

%define UHCI_LOW_SPEED 0x04800000
%define UHCI_FULL_SPEED 0x00800000

%macro UHCI_CLEAR_FRAME_LIST 0
 mov eax, dword [uhci_controller_number]
 mov ebx, 4096
 mul ebx
 mov esi, MEMORY_UHCI
 add esi, eax
 mov ecx, 1024
 .clear_list:
  mov dword [esi], 0x1 ;invalid pointer
  add esi, 4
 loop .clear_list
%endmacro

%macro UHCI_FILL_FRAME_LIST 0
 mov eax, dword [uhci_controller_number]
 mov ebx, 4096
 mul ebx
 mov esi, MEMORY_UHCI
 add esi, eax
 mov ecx, 1024
 .fill_list:
  mov dword [esi], MEMORY_UHCI+0x10002 ;pointer
  add esi, 4
 loop .fill_list
%endmacro

%macro UHCI_CREATE_QH 2
 mov dword [MEMORY_UHCI+0x10000], %1
 mov dword [MEMORY_UHCI+0x10000+4], %2
%endmacro

%macro UHCI_CREATE_TD 4
 mov dword [%1], %2
 mov eax, dword [uhci_device_speed]
 mov dword [%1+4], eax
 mov dword [%1+8], %3
 mov dword [%1+12], %4
%endmacro

%macro UHCI_SETUP_TRANSFER 1
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, 0x00E0012D, %1 ;setup
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, 0xFFE80169, MEMORY_UHCI+0x10300+8 ;in
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 mov dword [uhci_time_for_transfer], 100
 call uhci_transfer_queue_head
%endmacro

uhci_base dw 0
uhci_controller_number dd 0
uhci_device_speed dd 0
uhci_toggle dd 0
uhci_endpoint dd 0
uhci_td_pointer dd 0
uhci_time_for_transfer dd 0

uhci_conf_num db 0
uhci_interface_num db 0
uhci_alt_interface_num db 0

init_uhci_controller:
 cmp word [uhci_base], 0
 je .done

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
 UHCI_CLEAR_FRAME_LIST
 BASE_OUTW uhci_base, 0x0, 0x1 ;run controller

 .done:
 ret

uhci_detect_devices:
 ;detect device on first port
 BASE_INW uhci_base, 0x10
 and ax, 0x3
 cmp ax, 0x0 ;no device
 je .no_device_first_port
 cmp ax, 0x2 ;no device
 je .no_device_first_port
 cmp ax, 0x1 ;initalized device
 je .done

 ;initalize device
 call usb_mouse_uhci_remove
 BASE_OUTW uhci_base, 0x10, 0x200 ;reset
 WAIT 50
 BASE_OUTW uhci_base, 0x10, 0x0
 WAIT 10
 BASE_INW uhci_base, 0x10
 mov dword [uhci_device_speed], UHCI_FULL_SPEED
 and ax, 0x100
 cmp ax, 0x100
 jne .if_low_speed
  mov dword [uhci_device_speed], UHCI_LOW_SPEED
 .if_low_speed:
 BASE_OUTW uhci_base, 0x10, 0x6 ;enable device and clear status change
 WAIT 50
 call uhci_init_device
 jmp .done

 .no_device_first_port:
 mov ax, word [usb_mouse_base]
 cmp word [uhci_base], ax
 jne .detect_second_port
 call usb_mouse_uhci_remove

 ;detect device on second port
 .detect_second_port:
 BASE_INW uhci_base, 0x12
 and ax, 0x3
 cmp ax, 0x0 ;no device
 je .no_device
 cmp ax, 0x2 ;no device
 je .done
 cmp ax, 0x1 ;initalized device
 je .done

 ;initalize device
 call usb_mouse_uhci_remove
 BASE_OUTW uhci_base, 0x12, 0x200 ;reset
 WAIT 50
 BASE_OUTW uhci_base, 0x12, 0x0
 WAIT 10
 BASE_INW uhci_base, 0x12
 mov dword [uhci_device_speed], UHCI_FULL_SPEED
 and ax, 0x100
 cmp ax, 0x100
 jne .if_low_speed_2
  mov dword [uhci_device_speed], UHCI_LOW_SPEED
 .if_low_speed_2:
 BASE_OUTW uhci_base, 0x12, 0x6 ;enable device and clear status change
 WAIT 50
 call uhci_init_device
 jmp .done

 .no_device:
 mov ax, word [usb_mouse_base]
 cmp word [uhci_base], ax
 jne .done
 call usb_mouse_uhci_remove

 .done:
 ret

uhci_init_device:
 ;SET ADDRESS
 UHCI_SETUP_TRANSFER MEMORY_UHCI+0x10300
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, 0x00E0002D, MEMORY_UHCI+0x10300 ;setup
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, 0xFFE80069, MEMORY_UHCI+0x10400 ;in
 mov dword [MEMORY_UHCI+0x10300], 0x00010500
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 mov dword [uhci_time_for_transfer], 100
 call uhci_transfer_queue_head

 ;GET DESCRIPTOR
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, 0x00E0012D, MEMORY_UHCI+0x10B00 ;setup
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, MEMORY_UHCI+0x10300 | 0x4, 0x00E80169, MEMORY_UHCI+0x10C00+0 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10300, MEMORY_UHCI+0x10400 | 0x4, 0x00E00169, MEMORY_UHCI+0x10C00+8 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10400, MEMORY_UHCI+0x10500 | 0x4, 0x00E80169, MEMORY_UHCI+0x10C00+16 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10500, MEMORY_UHCI+0x10600 | 0x4, 0x00E00169, MEMORY_UHCI+0x10C00+24 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10600, MEMORY_UHCI+0x10700 | 0x4, 0x00E80169, MEMORY_UHCI+0x10C00+32 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10700, MEMORY_UHCI+0x10800 | 0x4, 0x00E00169, MEMORY_UHCI+0x10C00+40 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10800, MEMORY_UHCI+0x10900 | 0x4, 0x00E80169, MEMORY_UHCI+0x10C00+48 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10900, MEMORY_UHCI+0x10A00 | 0x4, 0x00E00169, MEMORY_UHCI+0x10C00+56 ;in
 UHCI_CREATE_TD MEMORY_UHCI+0x10A00, 0x1, 0xFFE801E1, MEMORY_UHCI+0x10700+24 ;out
 mov dword [MEMORY_UHCI+0x10B00], 0x02000680
 mov dword [MEMORY_UHCI+0x10B00+4], 0x00400000
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10A00+4
 mov dword [uhci_time_for_transfer], 100
 call uhci_transfer_queue_head

 ;PARSE RECEIVED DATA
 mov esi, MEMORY_UHCI+0x10C00
 call parse_usb_descriptor

 mov al, byte [usb_descriptor+20]
 mov byte [uhci_conf_num], al
 mov al, byte [usb_descriptor+21]
 mov byte [uhci_interface_num], al
 mov al, byte [usb_descriptor+22]
 mov byte [uhci_alt_interface_num], al
 cmp dword [usb_descriptor+23], 0x00020103
 je .usb_mouse_2

 mov al, byte [usb_descriptor+10]
 mov byte [uhci_conf_num], al
 mov al, byte [usb_descriptor+11]
 mov byte [uhci_interface_num], al
 mov al, byte [usb_descriptor+12]
 mov byte [uhci_alt_interface_num], al
 cmp dword [usb_descriptor+13], 0x00010103
 je .usb_keyboard
 cmp dword [usb_descriptor+13], 0x00020103
 je .usb_mouse

 ret

 .usb_keyboard:
 jmp .done

 .usb_mouse:
 ;SET CONFIGURATION
 mov dword [MEMORY_UHCI+0x10300], 0x00000900
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 mov al, byte [uhci_conf_num]
 mov byte [MEMORY_UHCI+0x10300+2], al
 UHCI_SETUP_TRANSFER MEMORY_UHCI+0x10300

 ;SET INTERFACE
 mov dword [MEMORY_UHCI+0x10300], 0x00000B01
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 mov al, byte [uhci_interface_num]
 mov byte [MEMORY_UHCI+0x10300+4], al
 mov al, byte [uhci_alt_interface_num]
 mov byte [MEMORY_UHCI+0x10300+2], al
 UHCI_SETUP_TRANSFER MEMORY_UHCI+0x10300

 ;SET IDLE - needed for some mouses
 mov dword [MEMORY_UHCI+0x10300], 0x00000A21
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 UHCI_SETUP_TRANSFER MEMORY_UHCI+0x10300

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

 .done:
 ret

 .usb_mouse_2:
 call .usb_mouse

 mov eax, 0
 mov al, byte [usb_descriptor+29]
 mov dword [usb_mouse_endpoint], eax
 PVAR eax

 ret

uhci_read_hid:
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100

 mov ebx, dword [uhci_endpoint]
 shl ebx, 15
 or ebx, 0x00E00169
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200, ebx, MEMORY_UHCI+0x10300
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, ebx, MEMORY_UHCI+0x10300+8

 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 mov dword [uhci_time_for_transfer], 3
 call uhci_transfer_queue_head

 ret

uhci_transfer_queue_head:
 UHCI_FILL_FRAME_LIST

 ;transfer
 mov eax, dword [uhci_time_for_transfer]
 mov ecx, dword [uhci_td_pointer]
 mov dword [ticks], 0
 .wait_for_transfer:
  mov ebx, dword [ecx]
  and ebx, 0x00800000
  cmp ebx, 0
  je .transfer_is_done
 cmp dword [ticks], eax
 jnge .wait_for_transfer

 .transfer_is_done:
 UHCI_CLEAR_FRAME_LIST

 ret
