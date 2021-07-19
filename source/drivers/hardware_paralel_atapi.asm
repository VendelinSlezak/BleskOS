;BleskOS

atapi_base dw 0
atapi_status dd 0
atapi_transfer_lenght dw 0

atapi_sector dd 0
atapi_memory dd 0

atapi_select_master:
 BASE_OUTB atapi_base, 6, 0xE0
 ret

atapi_select_slave:
 BASE_OUTB atapi_base, 6, 0xF0
 ret

atapi_send_packet_command:
 BASE_OUTB atapi_base, 1, 0x0
 BASE_OUTB atapi_base, 2, 0x0
 BASE_OUTB atapi_base, 3, 0x0
 mov al, byte [atapi_transfer_lenght]
 BASE_OUTB atapi_base, 4, al
 mov al, byte [atapi_transfer_lenght+1]
 BASE_OUTB atapi_base, 5, al
 BASE_OUTB atapi_base, 7, 0xA0

 mov ecx, 100
 .wait_for_atapi:
  BASE_INB atapi_base, 7
  and al, 0x88
  cmp al, 0x08
  je .atapi_is_ready
 loop .wait_for_atapi
 mov dword [atapi_status], IDE_ERROR
 ret
 .atapi_is_ready:
 mov dword [atapi_status], IDE_OK
 ret

atapi_eject_drive:
 mov word [atapi_transfer_lenght], 0
 call atapi_send_packet_command

 BASE_OUTW atapi_base, 0, 0x1B ;eject command
 BASE_OUTW atapi_base, 0, 0x0
 BASE_OUTW atapi_base, 0, 0x0002
 BASE_OUTW atapi_base, 0, 0x0
 BASE_OUTW atapi_base, 0, 0x0
 BASE_OUTW atapi_base, 0, 0x0

 ret

atapi_read:
 mov word [atapi_transfer_lenght], 2048
 call atapi_send_packet_command

 BASE_OUTW atapi_base, 0, 0xA8 ;read command
 mov al, byte [atapi_sector+3]
 mov ah, byte [atapi_sector+2]
 BASE_OUTW atapi_base, 0, ax
 mov al, byte [atapi_sector+1]
 mov ah, byte [atapi_sector]
 BASE_OUTW atapi_base, 0, ax
 BASE_OUTW atapi_base, 0, 0x0
 BASE_OUTW atapi_base, 0, 0x0100 ;one sector
 BASE_OUTW atapi_base, 0, 0x0

 mov ecx, 1000
 .wait_for_atapi:
  BASE_INB atapi_base, 7
  and al, 0x88
  cmp al, 0x08
  je .atapi_is_ready
 loop .wait_for_atapi
 mov dword [atapi_status], IDE_ERROR
 ret

 .atapi_is_ready:
 mov esi, dword [atapi_memory]
 mov ecx, 1024
 .read_sector:
  BASE_INW atapi_base, 0
  mov word [esi], ax
  add esi, 2
 loop .read_sector

 mov dword [atapi_status], IDE_OK
 ret
