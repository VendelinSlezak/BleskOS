;BleskOS

%define OHCI 1
%define UHCI 2
%define EHCI 3
%define XHCI 4

ohci_num_of_ports dd 0
uhci_num_of_ports dd 0
ehci_num_of_ports dd 0
xhci_num_of_ports dd 0

uhci_controllers_base times 10 dw 0
uhci_pointer dd uhci_controllers_base

ehci_controllers_base times 10 dd 0
ehci_pointer dd ehci_controllers_base

usb_conf_number db 0
usb_descriptor times 100 db 0

init_usb_ports:
 mov esi, ehci_controllers_base
 mov ecx, dword [ehci_num_of_ports]
 cmp ecx, 0
 je .skip_ehci
 .init_ehci:
 push ecx
  mov eax, dword [esi]
  mov dword [ehci_base], eax
  push esi
  call init_ehci
  call ehci_detect_devices
  pop esi
  add esi, 4
 pop ecx
 loop .init_ehci
 .skip_ehci:

 mov dword [uhci_controller_number], 0
 mov esi, uhci_controllers_base
 mov ecx, dword [uhci_num_of_ports]
 cmp ecx, 0
 je .skip_uhci
 .init_uhci:
 push ecx
  mov ax, word [esi]
  mov word [uhci_base], ax
  push esi
  call init_uhci_controller
  call uhci_detect_devices
  pop esi
  inc dword [uhci_controller_number]
  add esi, 2
 pop ecx
 loop .init_uhci
 .skip_uhci:

 ret

detect_usb_devices:
 mov esi, ehci_controllers_base
 mov ecx, dword [ehci_num_of_ports]
 cmp ecx, 0
 je .skip_ehci
 .detect_ehci:
 push ecx
  mov eax, dword [esi]
  mov dword [ehci_base], eax
  push esi
  call ehci_detect_devices
  pop esi
  add esi, 4
 pop ecx
 loop .detect_ehci
 .skip_ehci:

 mov dword [uhci_controller_number], 0
 mov esi, uhci_controllers_base
 mov ecx, dword [uhci_num_of_ports]
 cmp ecx, 0
 je .skip_uhci
 .init_uhci:
 push ecx
  mov ax, word [esi]
  mov word [uhci_base], ax
  push esi
  call uhci_detect_devices
  pop esi
  inc dword [uhci_controller_number]
  add esi, 2
 pop ecx
 loop .init_uhci
 .skip_uhci:

 ret

parse_usb_descriptor:
 mov eax, usb_descriptor
 mov ecx, 100
 .clear_descriptor:
  mov byte [eax], 0
  inc eax
 loop .clear_descriptor

 mov edi, usb_descriptor

 .parse_item:
  cmp byte [esi], 0
  je .done
  cmp byte [esi+1], 0x02
  je .configuration
  cmp byte [esi+1], 0x04
  je .interface
  cmp byte [esi+1], 0x05
  je .endpoint
  jmp .next_item

  .configuration:
  mov al, byte [esi+5]
  mov byte [usb_conf_number], al
  jmp .next_item

  .interface:
  add edi, 10 ;new interface
  mov al, byte [usb_conf_number] ;configuration number
  mov byte [edi], al
  mov al, byte [esi+2] ;interface number
  mov byte [edi+1], al
  mov al, byte [esi+3] ;alternate interface number
  mov byte [edi+2], al
  mov al, byte [esi+5] ;class
  mov byte [edi+3], al
  mov al, byte [esi+6] ;subclass
  mov byte [edi+4], al
  mov al, byte [esi+7] ;progif
  mov byte [edi+5], al
  jmp .next_item

  .endpoint:
  mov ax, word [esi+2]
  and ax, 0x0380 ;endpoint type
  mov bl, byte [esi+2]
  and bl, 0xF ;endpoint number
  cmp ax, 0x0200
  je .bulk_out_endpoint
  cmp ax, 0x0280
  je .bulk_in_endpoint
  cmp ah, 0x3
  je .interrupt_endpoint
  cmp ah, 0x1
  je .isynchronous_endpoint
  jmp .next_item

  .bulk_out_endpoint:
  cmp byte [edi+7], 0
  jne .next_item
  mov byte [edi+7], bl
  jmp .next_item

  .bulk_in_endpoint:
  cmp byte [edi+8], 0
  jne .next_item
  mov byte [edi+8], bl
  jmp .next_item

  .interrupt_endpoint:
  cmp byte [edi+9], 0
  jne .next_item
  mov byte [edi+9], bl
  jmp .next_item

  .isynchronous_endpoint:
  cmp byte [edi+10], 0
  jne .next_item
  mov byte [edi+10], bl

 .next_item:
 mov eax, 0
 mov al, byte [esi] ;item length
 add esi, eax
 jmp .parse_item

 .done:
 ret
