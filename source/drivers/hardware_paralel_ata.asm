;BleskOS

%define ATA_ERROR 0
%define ATA_OK 1

pata_base dw 0
pata_info times 512 db 0
pata_size dd 0

pata_select_master:
 BASE_OUTB pata_base, 6, 0xE0
 ret

pata_select_slave:
 BASE_OUTB pata_base, 6, 0xF0
 ret

pata_detect_drive:
 mov dword [pata_size], 0
 mov dword [ata_status], ATA_OK

 BASE_OUTB pata_base, 1, 0
 BASE_OUTB pata_base, 2, 0
 BASE_OUTB pata_base, 3, 0
 BASE_OUTB pata_base, 4, 0
 BASE_OUTB pata_base, 5, 0
 BASE_OUTB pata_base, 7, 0xEC

 mov ecx, 1000
 .wait_for_drive:
  BASE_INB pata_base, 7
  and al, 0x88
  cmp al, 0x08
  je .data_are_ready
 loop .wait_for_drive
 mov dword [ata_status], ATA_ERROR
 ret

 .data_are_ready:
 mov esi, pata_info
 mov ecx, 256
 .read_info:
  BASE_INW pata_base, 0
  mov word [esi], ax
  add esi, 2
 loop .read_info

 mov eax, dword [pata_info+200]
 mov ebx, dword [pata_info+120]
 mov dword [pata_size], ebx ;size in 28 bits
 cmp eax, 0
 je .done
 mov dword [pata_size], eax ;size in 48 bits

 .done:
 ret

pata_select_sector:
 ;number of sectors >> 8
 mov ax, word [ata_number_of_sectors]
 shr ax, 8
 BASE_OUTB pata_base, 2, al

 ;sector >> 24
 mov eax, dword [ata_sector]
 shr eax, 24
 BASE_OUTB pata_base, 3, al

 ;sector >> 32
 BASE_OUTB pata_base, 4, 0

 ;sector >> 40
 BASE_OUTB pata_base, 5, 0

 ;number of sectors >> 0
 mov ax, word [ata_number_of_sectors]
 BASE_OUTB pata_base, 2, al

 ;sector >> 0
 mov al, byte [ata_sector]
 BASE_OUTB pata_base, 3, al

 ;sector >> 8
 mov eax, dword [ata_sector]
 shr eax, 8
 BASE_OUTB pata_base, 4, al

 ;sector >> 16
 mov eax, dword [ata_sector]
 shr eax, 16
 BASE_OUTB pata_base, 5, al

 ret

pata_wait:
 mov dword [ata_status], ATA_OK

 mov ecx, 10000
 .wait_for_drive:
  BASE_INB pata_base, 7
  and al, 0x88
  cmp al, 0x08
  je .done
 loop .wait_for_drive

 mov dword [ticks], 0
 .wait_for_drive_longer:
  BASE_INB pata_base, 7
  and al, 0x88
  cmp al, 0x08
  je .done
 cmp dword [ticks], 500
 jl .wait_for_drive_longer

 mov dword [ata_status], ATA_ERROR
 add dword [ata_memory], 0x200

 .done:
 ret

pata_read:
 call pata_select_sector
 BASE_OUTB pata_base, 7, 0x24 ;read command

 call pata_wait
 cmp dword [ata_status], ATA_OK
 je .data_are_ready

 ret

 ;read sectors
 .data_are_ready:
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   BASE_INW pata_base, 0
   mov ebx, dword [ata_memory]
   mov word [ebx], ax ;write into memory
   add dword [ata_memory], 2
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
 ENDFOR ata_nos_cycle

 ret

pata_write:
 call pata_select_sector
 BASE_OUTB pata_base, 7, 0x25 ;write command

 call pata_wait
 cmp dword [ata_status], ATA_OK
 je .data_are_ready

 ret

 ;write sectors
 .data_are_ready:
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   mov ebx, dword [ata_memory]
   mov ax, word [ebx]
   BASE_OUTW pata_base, 0, ax ;write into hard disk
   add dword [ata_memory], 2
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
 ENDFOR ata_nos_cycle

 ret

pata_delete:
 call pata_select_sector
 BASE_OUTB pata_base, 7, 0xC5 ;read command

 call pata_wait
 cmp dword [ata_status], ATA_OK
 je .data_are_ready

 ret

 ;write sectors
 .data_are_ready:
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   BASE_OUTW pata_base, 0, 0 ;write into hard disk
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
  BASE_INB pata_base, 0
 ENDFOR ata_nos_cycle

 ret
