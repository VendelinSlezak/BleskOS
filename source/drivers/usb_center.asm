;BleskOS

ohci_num_of_ports dd 0
uhci_num_of_ports dd 0
ehci_num_of_ports dd 0
xhci_num_of_ports dd 0

init_usb_ports:
 call init_ehci

 ret
