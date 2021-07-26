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

init_usb_ports:
 call init_ehci

 mov dword [uhci_controller_number], 0
 mov esi, uhci_controllers_base
 mov ecx, dword [uhci_num_of_ports]
 cmp ecx, 0
 je .done
 .init_uhci:
 push ecx
  mov ax, word [esi]
  mov word [uhci_base], ax
  push esi
  call init_uhci_controller
  pop esi
  inc dword [uhci_controller_number]
  add esi, 2
 pop ecx
 loop .init_uhci

 .done:
 ret
