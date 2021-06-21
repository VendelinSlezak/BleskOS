;BleskOS

%define ATA_ERROR 0
%define ATA_OK 1

pata_base dw 0

%macro PATA_SELECT_SECTOR 0
 ;number of sectors >> 8
 mov ax, word [ata_number_of_sectors]
 shr ax, 8
 mov dx, word [pata_base]
 add dx, 2
 OUTB dx, al

 ;sector >> 24
 mov eax, dword [ata_sector]
 shr eax, 24
 mov dx, word [pata_base]
 add dx, 3
 OUTB dx, al

 ;sector >> 32
 mov eax, dword [ata_sector+4]
 mov dx, word [pata_base]
 add dx, 4
 OUTB dx, al

 ;sector >> 40
 mov eax, dword [ata_sector+4]
 shr eax, 8
 mov dx, word [pata_base]
 add dx, 5
 OUTB dx, al

 ;number of sectors >> 0
 mov ax, word [ata_number_of_sectors]
 mov dx, word [pata_base]
 add dx, 2
 OUTB dx, al

 ;sector >> 0
 mov al, byte [ata_sector]
 mov dx, word [pata_base]
 add dx, 3
 OUTB dx, al

 ;sector >> 8
 mov eax, dword [ata_sector]
 shr eax, 8
 mov dx, word [pata_base]
 add dx, 4
 OUTB dx, al

 ;sector >> 16
 mov eax, dword [ata_sector]
 shr eax, 16
 mov dx, word [pata_base]
 add dx, 5
 OUTB dx, al
%endmacro

%macro PATA_WAIT 1
 mov ecx, %1
 .ata_wait_%1:
  mov dx, word [pata_base]
  add dx, 7
  INB dx
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

 PATA_SELECT_SECTOR
 mov dx, word [pata_base]
 add dx, 7
 OUTB dx, 0xC4 ;read command
 PATA_WAIT 100000

 ;read sectors
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   mov dx, word [pata_base]
   INW dx
   mov ebx, dword [ata_memory]
   mov word [ebx], ax ;write into memory
   add dword [ata_memory], 2
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  mov dx, word [pata_base]
  INB dx
  INB dx
  INB dx
  INB dx
 ENDFOR ata_nos_cycle

 ret

pata_write:
 mov dword [ata_status], ATA_OK

 PATA_SELECT_SECTOR
 mov dx, word [pata_base]
 add dx, 7
 OUTB dx, 0xC5 ;write command
 PATA_WAIT 100000

 ;write sectors
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   mov ebx, dword [ata_memory]
   mov ax, word [ebx]
   mov dx, word [pata_base]
   OUTW dx, ax ;write into hard disk
   add dword [ata_memory], 2
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  mov dx, word [pata_base]
  INB dx
  INB dx
  INB dx
  INB dx
 ENDFOR ata_nos_cycle

 ret

pata_delete:
 mov dword [ata_status], ATA_OK

 PATA_SELECT_SECTOR
 mov dx, word [pata_base]
 add dx, 7
 OUTB dx, 0xC5 ;write command
 PATA_WAIT 100000

 ;write sectors
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   mov dx, word [pata_base]
   OUTW dx, 0 ;write into hard disk
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  mov dx, word [pata_base]
  INB dx
  INB dx
  INB dx
  INB dx
 ENDFOR ata_nos_cycle

 ret
