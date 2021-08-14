;BleskOS

%define NO_MSD 0x0
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

mass_storage_devices times 5 dd 0, 0
msd_number dd 0
msd_pointer dd 0
msd_usb_controller_base dd 0
msd_usb_controller_address db 0

select_msd:
 mov eax, dword [msd_number]
 mov ebx, 8
 mul ebx
 add eax, mass_storage_devices
 mov esi, eax ;pointer to device info
 mov dword [msd_pointer], eax
 cmp byte [esi+7], 0 ;no device
 je .done

 mov eax, dword [esi]
 mov dword [ehci_base], eax ;set base
 mov al, byte [esi+4]
 mov dword [ehci_address], 0
 mov byte [ehci_address], al ;set address

 mov dword [ehci_endpoint_bulk_out], 0
 mov al, byte [esi+5]
 mov byte [ehci_endpoint_bulk_out], al ;set out endpoint
 mov dword [ehci_endpoint_bulk_in], 0
 mov al, byte [esi+6]
 mov byte [ehci_endpoint_bulk_in], al ;set in endpoint

 .done:
 ret

msd_remove_device:
 mov eax, dword [msd_usb_controller_base]
 mov bl, byte [msd_usb_controller_address]
 mov esi, mass_storage_devices
 mov ecx, 5
 .remove_device:
  cmp dword [esi], eax
  jne .next_loop
  cmp byte [esi+4], bl
  jne .next_loop

  mov dword [esi], 0
  mov dword [esi+4], 0
  jmp .done
 .next_loop:
 add esi, 8
 loop .remove_device

 .done:
 ret

msd_set_data_toggle_out:
 mov eax, dword [msd_pointer]
 mov ebx, 0
 mov bl, byte [eax+5]
 and bl, 0x80
 cmp bl, 0x0
 je .data_toggle_0
 jmp .data_toggle_1

 .data_toggle_0:
 mov dword [ehci_data_toggle], DATA_TOGGLE_0
 mov bl, byte [eax+5]
 or bl, 0x80
 mov byte [eax+5], bl
 jmp .done

 .data_toggle_1:
 mov dword [ehci_data_toggle], DATA_TOGGLE_1
 mov bl, byte [eax+5]
 and bl, 0xF
 mov byte [eax+5], bl

 .done:
 ret

msd_set_data_toggle_in:
 mov eax, dword [msd_pointer]
 mov ebx, 0
 mov bl, byte [eax+6]
 and bl, 0x80
 cmp bl, 0x0
 je .data_toggle_0
 jmp .data_toggle_1

 .data_toggle_0:
 mov dword [ehci_data_toggle], DATA_TOGGLE_0
 mov bl, byte [eax+6]
 or bl, 0x80
 mov byte [eax+6], bl
 jmp .done

 .data_toggle_1:
 mov dword [ehci_data_toggle], DATA_TOGGLE_1
 mov bl, byte [eax+6]
 and bl, 0xF
 mov byte [eax+6], bl

 .done:
 ret

msd_init:
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
 call msd_set_data_toggle_out
 call ehci_transfer_bulk_out

 ;READ MSD RESPONSE
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x600
 mov dword [ehci_transfer_length], 36
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 ;READ CSW
 ;mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x700
 ;mov dword [ehci_transfer_length], 13
 ;call ehci_transfer_bulk_in

 ret
