;BleskOS

%define NO_MSD 0x0
%define MSD_UNINITALIZED 0x1
%define MSD_INITALIZED 0x2
%define MSD_FAT32 0x3

%define MSD_READ 0x80
%define MSD_WRITE 0x00

%define MSD_OK 1
%define MSD_ERROR 0

%macro MSD_CREATE_CBW 3
 mov dword [MEMORY_EHCI+0x500], 0x43425355
 mov dword [MEMORY_EHCI+0x500+4], 0x12345678
 mov dword [MEMORY_EHCI+0x500+8], %2 ;command
 mov byte [MEMORY_EHCI+0x500+12], %1 ;direction
 mov byte [MEMORY_EHCI+0x500+13], 0x00 ;LUN 0
 mov byte [MEMORY_EHCI+0x500+14], %3 ;command lenght

 mov byte [MEMORY_EHCI+0x500+15], 0
 mov dword [MEMORY_EHCI+0x500+16], 0
 mov dword [MEMORY_EHCI+0x500+20], 0
 mov dword [MEMORY_EHCI+0x500+24], 0
 mov dword [MEMORY_EHCI+0x500+28], 0
%endmacro

mass_storage_devices times 5 dd 0, 0, 0, 0
mass_storage_device_label times 60 db 0
msd_number dd 0
msd_pointer dd 0
msd_usb_controller_base dd 0
msd_usb_controller_address db 0

msd_sector dd 0
msd_transfer_memory dd 0
msd_status dd 0

select_msd:
 mov eax, dword [msd_number]
 mov ebx, 16
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
 mov edi, mass_storage_device_label
 mov ecx, 5
 .remove_device:
  cmp dword [esi], eax
  jne .next_loop
  cmp byte [esi+4], bl
  jne .next_loop

  mov dword [esi], 0
  mov dword [esi+4], 0
  mov dword [esi+8], 0
  mov dword [esi+12], 0
  mov dword [edi], 0
  mov dword [edi+4], 0
  mov dword [edi+8], 0
  jmp .done
 .next_loop:
 add esi, 16
 add edi, 12
 loop .remove_device

 .done:
 ret

msd_set_data_toggle_out:
 mov eax, dword [msd_pointer]
 mov ebx, 0
 mov bl, byte [eax+5]
 and bl, 0x80
 cmp bl, 0x0
 je .EHCI_DATA_TOGGLE_0
 jmp .EHCI_DATA_TOGGLE_1

 .EHCI_DATA_TOGGLE_0:
 mov dword [ehci_data_toggle], EHCI_DATA_TOGGLE_0
 mov bl, byte [eax+5]
 or bl, 0x80
 mov byte [eax+5], bl
 jmp .done

 .EHCI_DATA_TOGGLE_1:
 mov dword [ehci_data_toggle], EHCI_DATA_TOGGLE_1
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
 je .EHCI_DATA_TOGGLE_0
 jmp .EHCI_DATA_TOGGLE_1

 .EHCI_DATA_TOGGLE_0:
 mov dword [ehci_data_toggle], EHCI_DATA_TOGGLE_0
 mov bl, byte [eax+6]
 or bl, 0x80
 mov byte [eax+6], bl
 jmp .done

 .EHCI_DATA_TOGGLE_1:
 mov dword [ehci_data_toggle], EHCI_DATA_TOGGLE_1
 mov bl, byte [eax+6]
 and bl, 0xF
 mov byte [eax+6], bl

 .done:
 ret

msd_init:
 MSD_CREATE_CBW MSD_READ, 36, 6
 mov byte [MEMORY_EHCI+0x500+15], 0x12 ;inquiry
 mov byte [MEMORY_EHCI+0x500+19], 36 ;transfer length

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
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x700
 mov dword [ehci_transfer_length], 13
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 call msd_request_sense

 MSD_CREATE_CBW MSD_READ, 8, 10
 mov byte [MEMORY_EHCI+0x500+15], 0x25 ;capacity

 ;SEND CBW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x500
 mov dword [ehci_transfer_length], 31
 call msd_set_data_toggle_out
 call ehci_transfer_bulk_out

 ;READ MSD RESPONSE
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x600
 mov dword [ehci_transfer_length], 8
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 ;READ CSW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x700
 mov dword [ehci_transfer_length], 13
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 mov eax, 0
 mov al, byte [MEMORY_EHCI+0x600+3]
 mov ah, byte [MEMORY_EHCI+0x600+2]
 mov ebx, 0
 mov bl, byte [MEMORY_EHCI+0x600+1]
 mov bh, byte [MEMORY_EHCI+0x600+0]
 shl ebx, 16
 or eax, ebx
 mov esi, dword [msd_pointer]
 dec eax
 mov dword [esi+8], eax ;save size of msd drive

 ;if we are here, it mean that MSD drive is initalized
 mov eax, dword [msd_pointer]
 mov byte [eax+7], MSD_INITALIZED

 ;read partitions numbers
 call msd_read_mbr
 cmp dword [first_partition_lba], 0
 je .without_partition

 ;is on first partition FAT32?
 mov eax, dword [first_partition_lba]
 mov dword [fat_base_sector], eax
 call init_fat
 cmp dword [fat_present], 0
 je .without_partition

 ;we found MSD drive with FAT32 partition
 mov eax, dword [msd_pointer]
 mov byte [eax+7], MSD_FAT32
 ;copy label of device
 mov eax, dword [msd_number]
 mov ebx, 12
 mul ebx
 add eax, mass_storage_device_label
 mov ebx, dword [fat_label]
 mov dword [eax], ebx
 mov ebx, dword [fat_label+4]
 mov dword [eax+4], ebx
 mov ebx, dword [fat_label+8]
 mov dword [eax+8], ebx
 mov byte [eax+11], 0

 .without_partition:
 ret

msd_request_sense:
 MSD_CREATE_CBW MSD_READ, 0x12, 6
 mov byte [MEMORY_EHCI+0x500+15], 0x03 ;request sense
 mov byte [MEMORY_EHCI+0x500+19], 0x12 ;transfer length

 ;SEND CBW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x500
 mov dword [ehci_transfer_length], 31
 call msd_set_data_toggle_out
 call ehci_transfer_bulk_out

 ;READ MSD RESPONSE
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x600
 mov dword [ehci_transfer_length], 0x12
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 ;READ CSW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x700
 mov dword [ehci_transfer_length], 13
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 ret

msd_read:
 mov dword [msd_status], MSD_ERROR
 mov dword [MEMORY_EHCI+0x600], 0

 MSD_CREATE_CBW MSD_READ, 0x200, 10
 mov byte [MEMORY_EHCI+0x500+15], 0x28 ;read
 mov al, byte [msd_sector+3]
 mov byte [MEMORY_EHCI+0x500+17], al
 mov al, byte [msd_sector+2]
 mov byte [MEMORY_EHCI+0x500+18], al
 mov al, byte [msd_sector+1]
 mov byte [MEMORY_EHCI+0x500+19], al
 mov al, byte [msd_sector+0]
 mov byte [MEMORY_EHCI+0x500+20], al
 mov byte [MEMORY_EHCI+0x500+23], 0x01 ;1 sector

 ;SEND CBW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x500
 mov dword [ehci_transfer_length], 31
 call msd_set_data_toggle_out
 call ehci_transfer_bulk_out

 ;READ MSD RESPONSE
 mov eax, dword [msd_transfer_memory]
 mov dword [ehci_transfer_pointer], eax
 mov dword [ehci_transfer_length], 0x200
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 ;READ CSW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x600
 mov dword [ehci_transfer_length], 13
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 cmp dword [MEMORY_EHCI+0x600], 0x53425355 ;CSW singature
 jne .error
 cmp dword [MEMORY_EHCI+0x600+8], 0
 jne .done
 mov dword [msd_status], MSD_OK
 .done:
 ret

 .error:
 call msd_request_sense
 ret

msd_write:
 mov dword [msd_status], MSD_ERROR
 mov dword [MEMORY_EHCI+0x600], 0

 MSD_CREATE_CBW MSD_WRITE, 0x200, 10
 mov byte [MEMORY_EHCI+0x500+15], 0x2A ;write
 mov al, byte [msd_sector+3]
 mov byte [MEMORY_EHCI+0x500+17], al
 mov al, byte [msd_sector+2]
 mov byte [MEMORY_EHCI+0x500+18], al
 mov al, byte [msd_sector+1]
 mov byte [MEMORY_EHCI+0x500+19], al
 mov al, byte [msd_sector+0]
 mov byte [MEMORY_EHCI+0x500+20], al
 mov byte [MEMORY_EHCI+0x500+23], 0x01 ;1 sector

 ;SEND CBW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x500
 mov dword [ehci_transfer_length], 31
 call msd_set_data_toggle_out
 call ehci_transfer_bulk_out

 ;READ MSD RESPONSE
 mov eax, dword [msd_transfer_memory]
 mov dword [ehci_transfer_pointer], eax
 mov dword [ehci_transfer_length], 0x200
 call msd_set_data_toggle_out
 call ehci_transfer_bulk_out

 ;READ CSW
 mov dword [ehci_transfer_pointer], MEMORY_EHCI+0x600
 mov dword [ehci_transfer_length], 13
 call msd_set_data_toggle_in
 call ehci_transfer_bulk_in

 cmp dword [MEMORY_EHCI+0x600], 0x53425355 ;CSW singature
 jne .error
 cmp dword [MEMORY_EHCI+0x600+8], 0
 jne .done
 mov dword [msd_status], MSD_OK
 .done:
 ret

 .error:
 call msd_request_sense
 ret
