;BleskOS

%define ATA_ERROR 0
%define ATA_OK 1

pata_base dw 0
pata_info times 512 db 0
pata_size dq 0

pata_select_master:
 BASE_OUTB pata_base, 6, 0x40
 ret

pata_select_slave:
 BASE_OUTB pata_base, 6, 0x50
 ret

pata_detect_drive:
 mov dword [pata_size], 0
 mov dword [pata_size+4], 0
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
 mov dword [pata_size], eax

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
 mov eax, dword [ata_sector+4]
 BASE_OUTB pata_base, 4, al

 ;sector >> 40
 mov eax, dword [ata_sector+4]
 shr eax, 8
 BASE_OUTB pata_base, 5, al

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

%macro PATA_WAIT 1
 mov ecx, %1
 .ata_wait_%1:
  BASE_OUTB pata_base, 7, al
  and al, 0x88
  cmp al, 0x08 ;transfer is ready
  je .transfer_%1
 dec ecx
 cmp ecx, 0
 jne .ata_wait_%1

 ;something fail
 mov dword [ata_status], ATA_ERROR
 ret

 .transfer_%1:
%endmacro

pata_read:
 mov dword [ata_status], ATA_OK

 call pata_select_sector
 BASE_OUTB pata_base, 7, 0xC4 ;read command
 PATA_WAIT 100000

 ;read sectors
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
 mov dword [ata_status], ATA_OK

 call pata_select_sector
 BASE_OUTB pata_base, 7, 0xC5 ;read command
 PATA_WAIT 100000

 ;write sectors
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
 mov dword [ata_status], ATA_OK

 call pata_select_sector
 BASE_OUTB pata_base, 7, 0xC5 ;read command
 PATA_WAIT 100000

 ;write sectors
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
