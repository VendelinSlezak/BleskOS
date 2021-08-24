;BleskOS

patapi_base dw 0
patapi_status dd 0
patapi_transfer_lenght dw 0

patapi_sector dd 0
patapi_memory dd 0

disk_size dd 0

patapi_select_master:
 BASE_OUTB patapi_base, 6, 0xE0
 ret

patapi_select_slave:
 BASE_OUTB patapi_base, 6, 0xF0
 ret

patapi_send_packet_command:
 BASE_OUTB patapi_base, 1, 0x0
 BASE_OUTB patapi_base, 2, 0x0
 BASE_OUTB patapi_base, 3, 0x0
 mov al, byte [patapi_transfer_lenght]
 BASE_OUTB patapi_base, 4, al
 mov al, byte [patapi_transfer_lenght+1]
 BASE_OUTB patapi_base, 5, al
 BASE_OUTB patapi_base, 7, 0xA0

 mov ecx, 10000
 .wait_for_patapi:
  BASE_INB patapi_base, 7
  and al, 0x88
  cmp al, 0x08
  je .patapi_is_ready
 loop .wait_for_patapi
 mov dword [patapi_status], IDE_ERROR
 ret
 .patapi_is_ready:
 mov dword [patapi_status], IDE_OK
 ret

patapi_eject_drive:
 mov word [patapi_transfer_lenght], 0
 call patapi_send_packet_command

 BASE_OUTW patapi_base, 0, 0x1B ;eject command
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0002
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0

 ret

patapi_read_capabilites:
 mov dword [disk_size], 0
 mov word [patapi_transfer_lenght], 8
 call patapi_send_packet_command

 BASE_OUTW patapi_base, 0, 0x25 ;read capabilites
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0

 mov ecx, 1000
 .wait_for_patapi:
  BASE_INB patapi_base, 7
  and al, 0x88
  cmp al, 0x08
  je .patapi_is_ready
 loop .wait_for_patapi
 mov dword [patapi_status], IDE_ERROR
 ret

 .patapi_is_ready:
 BASE_INW patapi_base, 0
 mov word [disk_size], ax
 BASE_INW patapi_base, 0
 mov word [disk_size+2], ax
 BASE_INW patapi_base, 0
 BASE_INW patapi_base, 0

 mov dword [patapi_status], IDE_OK
 ret

patapi_read:
 mov word [patapi_transfer_lenght], 2048
 call patapi_send_packet_command

 BASE_OUTW patapi_base, 0, 0xA8 ;read command
 mov al, byte [patapi_sector+3]
 mov ah, byte [patapi_sector+2]
 BASE_OUTW patapi_base, 0, ax
 mov al, byte [patapi_sector+1]
 mov ah, byte [patapi_sector]
 BASE_OUTW patapi_base, 0, ax
 BASE_OUTW patapi_base, 0, 0x0
 BASE_OUTW patapi_base, 0, 0x0100 ;one sector
 BASE_OUTW patapi_base, 0, 0x0

 mov ecx, 10000
 .wait_for_patapi:
  BASE_INB patapi_base, 7
  and al, 0x88
  cmp al, 0x08
  je .patapi_is_ready
 loop .wait_for_patapi

 mov dword [ticks], 0
 .wait_for_patapi_longer:
  BASE_INB patapi_base, 7
  and al, 0x88
  cmp al, 0x08
  je .patapi_is_ready
 cmp dword [ticks], 1000
 jl .wait_for_patapi_longer

 mov dword [patapi_status], IDE_ERROR
 ret

 .patapi_is_ready:
 mov esi, dword [patapi_memory]
 mov ecx, 1024
 .read_sector:
  BASE_INW patapi_base, 0
  mov word [esi], ax
  add esi, 2
 loop .read_sector
 add dword [patapi_memory], 2048

 mov dword [patapi_status], IDE_OK
 ret
