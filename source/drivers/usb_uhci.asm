;BleskOS

%define UHCI_LOW_SPEED 0x24800000
%define UHCI_FULL_SPEED 0x20800000

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

uhci_base dw 0
uhci_controller_number dd 0
uhci_device_speed dd 0
uhci_toggle dd 0
uhci_endpoint dd 0
uhci_td_pointer dd 0

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

 ;detect device on second port
 BASE_INW uhci_base, 0x12
 and ax, 0x3

 cmp ax, 0x0 ;no device
 je .no_device_second_port
 cmp ax, 0x2 ;no device
 je .no_device_second_port
 cmp ax, 0x1 ;initalized device
 je .done

 ;initalize device
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

 .no_device_second_port:

 .done:
 ret

uhci_init_device:
 ;SET ADDRESS
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, 0x00E0002D, MEMORY_UHCI+0x10300 ;setup
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, 0xFFE80069, MEMORY_UHCI+0x10400 ;in
 mov dword [MEMORY_UHCI+0x10300], 0x00010500
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 call uhci_transfer_queue_head

 ;GET DESCRIPTOR
 UHCI_CREATE_QH MEMORY_UHCI+0x10A00, MEMORY_UHCI+0x10100
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
 mov dword [MEMORY_UHCI+0x10B00+4], 0x00180000
 mov dword [MEMORY_UHCI+0x10C00+0], 0 ;clear
 mov dword [MEMORY_UHCI+0x10C00+14], 0 ;clear
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10A00+4
 call uhci_transfer_queue_head

 ;PARSE RECEIVED DATA
 cmp byte [MEMORY_UHCI+0x10C00+4], 0x1
 jne .more_interfaces

 mov eax, dword [MEMORY_UHCI+0x10C00+14]
 and eax, 0x00FFFFFF

 cmp eax, 0x00010103
 je .usb_keyboard

 cmp eax, 0x00020103
 je .usb_mouse

 ret

 .more_interfaces:
 PSTR 'USB device with more interfaces', usb_mi_str
 jmp .done

 .usb_keyboard:
 PSTR 'USB keyboard', usb_keyboard_str
 jmp .done

 .usb_mouse:
 PSTR 'USB mouse', usb_mouse_str

 ;SET IDLE - needed for some mouses
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, 0x00E0012D, MEMORY_UHCI+0x10300 ;setup
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, 0xFFE80169, MEMORY_UHCI+0x10300+8 ;in
 mov dword [MEMORY_UHCI+0x10300], 0x00000A21
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 call uhci_transfer_queue_head

 ;SET PROTOCOL - enable scroll on mouse
 UHCI_CREATE_QH 0x1, MEMORY_UHCI+0x10100
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, 0x00E0012D, MEMORY_UHCI+0x10300 ;setup
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, 0xFFE80169, MEMORY_UHCI+0x10300+8 ;in
 mov dword [MEMORY_UHCI+0x10300], 0x00010B21
 mov dword [MEMORY_UHCI+0x10300+4], 0x00000000
 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 call uhci_transfer_queue_head

 ;save mouse
 mov dword [usb_mouse_controller], UHCI

 mov ax, word [uhci_base]
 mov word [usb_mouse_base], ax

 mov eax, dword [uhci_controller_number]
 mov dword [usb_mouse_controller_number], eax

 mov eax, dword [uhci_device_speed]
 mov dword [usb_mouse_speed], eax

 mov eax, 0
 mov al, byte [MEMORY_UHCI+0x10C00+20]
 and eax, 0xF
 mov dword [usb_mouse_endpoint], eax
 PVAR eax

 .done:
 ret

uhci_read_hid:
 UHCI_CREATE_QH MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10100

 mov ebx, dword [uhci_endpoint]
 shl ebx, 15
 or ebx, 0x00E00169
 UHCI_CREATE_TD MEMORY_UHCI+0x10100, MEMORY_UHCI+0x10200 | 0x4, ebx, MEMORY_UHCI+0x10300
 mov eax, dword [uhci_endpoint]
 shl eax, 15
 or eax, 0x00E80169
 UHCI_CREATE_TD MEMORY_UHCI+0x10200, 0x1, ebx, MEMORY_UHCI+0x10300+8

 mov dword [uhci_td_pointer], MEMORY_UHCI+0x10200+4
 call uhci_transfer_queue_head

 ret

uhci_transfer_queue_head:
 ;set frame list pointer
 UHCI_FILL_FRAME_LIST

 ;transfer
 mov edi, dword [uhci_td_pointer]
 mov dword [ticks], 0
 .wait_for_transfer:
  mov ebx, dword [edi]
  and ebx, 0x00800000
  cmp ebx, 0
  je .transfer_is_done
 cmp dword [ticks], 10
 jl .wait_for_transfer

 .transfer_is_done:

 UHCI_CLEAR_FRAME_LIST

 ret
