;BleskOS

%define MSD_NON_INITALIZED 0x1
%define MSD_FLASH 0x2
%define MSD_CDROM 0x3

%macro MSD_CLEAR_REQUEST_MEMORY 0
 mov edi, MEMORY_EHCI+0x500
 mov ecx, 31
 .clear_mem:
  mov byte [edi], 0
  inc edi
 loop .clear_mem
%endmacro

mass_storage_devices times 16 dd 0, 0

usb_select_msd:
 ;set address
 mov dword [ehci_address], eax
 inc dword [ehci_address] ;port 0 is address 1

 mov esi, mass_storage_devices
 mov ebx, 8
 mul ebx
 add esi, eax ;pointer to msd item

 cmp word [esi], 0 ;no device
 je .done

 ;set endpoints
 mov dword [ehci_endpoint_bulk_out], 0
 mov al, byte [esi+2]
 mov byte [ehci_endpoint_bulk_out], al
 mov dword [ehci_endpoint_bulk_in], 0
 mov al, byte [esi+3]
 mov byte [ehci_endpoint_bulk_in], al

 ;write
 mov eax, dword [ehci_address]
 PHEX eax
 mov eax, dword [ehci_endpoint_bulk_in]
 PHEX eax
 mov eax, dword [ehci_endpoint_bulk_out]
 PHEX eax

 .done:
 ret

usb_msd_init:
 MSD_CLEAR_REQUEST_MEMORY

 ;write CBW
 mov dword [MEMORY_EHCI+0x500], 0x43425355
 mov dword [MEMORY_EHCI+0x500+4], 0x00000001
 mov dword [MEMORY_EHCI+0x500+8], 36
 mov byte [MEMORY_EHCI+0x500+9], 0x80 ;read command
 mov byte [MEMORY_EHCI+0x500+10], 0x00 ;LUN 0
 mov byte [MEMORY_EHCI+0x500+11], 6 ;command lenght

 mov byte [MEMORY_EHCI+0x500+12], 0x12 ;inqury
 mov byte [MEMORY_EHCI+0x500+13], 0x0
 mov byte [MEMORY_EHCI+0x500+14], 0x0
 mov byte [MEMORY_EHCI+0x500+15], 0x0
 mov byte [MEMORY_EHCI+0x500+16], 36 ;transfer 36 bytes
 mov byte [MEMORY_EHCI+0x500+17], 0x0

 ;SEND CBW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x500
 mov dword [ehci_transfer_length], 31
 call ehci_transfer_bulk_out

 ;READ MSD RESPONSE
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x600
 mov dword [ehci_transfer_length], 36
 call ehci_transfer_bulk_in

 ;READ CSW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x700
 mov dword [ehci_transfer_length], 13
 call ehci_transfer_bulk_in

 mov eax, dword [MEMORY_EHCI+0x600]
 PHEX eax
 mov eax, dword [MEMORY_EHCI+0x700]
 PHEX eax

 ret
