;BleskOS

usb_msd_bulk_in times 16 db 0
usb_msd_bulk_out times 16 db 0
usb_selected_msd dd 0
usb_selected_msd_bulk_in dd 0
usb_selected_msd_bulk_out dd 0

usb_select_msd:
 mov eax, dword [usb_selected_msd]
 inc eax ;port 0 means address 1
 mov dword [ehci_address], eax

 mov eax, dword [usb_selected_msd]
 mov esi, usb_msd_bulk_in
 add esi, eax
 mov ebx, 0
 mov bl, byte [esi]
 mov dword [usb_selected_msd_bulk_in], ebx

 mov eax, dword [usb_selected_msd]
 mov esi, usb_msd_bulk_out
 add esi, eax
 mov ebx, 0
 mov bl, byte [esi]
 mov dword [usb_selected_msd_bulk_out], ebx

 ret
