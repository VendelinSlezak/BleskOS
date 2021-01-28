;BleskOS

%macro PATA_SELECT_SECTOR 0
 ;number of sectors >> 8
 mov ax, word [ata_number_of_sectors]
 shr ax, 8
 OUTB 0x1F2, al

 ;sector >> 24
 mov eax, dword [ata_sector]
 shr eax, 24
 OUTB 0x1F3, al

 ;sector >> 32
 mov eax, dword [ata_sector+4]
 OUTB 0x1F4, al

 ;sector >> 40
 mov eax, dword [ata_sector+4]
 shr eax, 8
 OUTB 0x1F5, al

 ;number of sectors >> 0
 mov ax, word [ata_number_of_sectors]
 OUTB 0x1F2, al

 ;sector >> 0
 mov al, byte [ata_sector]
 OUTB 0x1F3, al

 ;sector >> 8
 mov eax, dword [ata_sector]
 shr eax, 8
 OUTB 0x1F4, al

 ;sector >> 16
 mov eax, dword [ata_sector]
 shr eax, 16
 OUTB 0x1F5, al
%endmacro

%macro PATA_WAIT 1
 mov ecx, %1
 .ata_wait_%1:
  INB 0x1F7
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
 OUTB 0x1F7, 0xC4 ;read command
 PATA_WAIT 100000

 ;read sectors
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   INW 0x1F0
   mov ebx, dword [ata_memory]
   mov word [ebx], ax ;write into memory
   add dword [ata_memory], 2
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  INB 0x1F7
  INB 0x1F7
  INB 0x1F7
  INB 0x1F7
 ENDFOR ata_nos_cycle

 ret

pata_write:
 mov dword [ata_status], ATA_OK

 PATA_SELECT_SECTOR
 OUTB 0x1F7, 0xC5 ;write command
 PATA_WAIT 100000

 ;write sectors
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   mov ebx, dword [ata_memory]
   mov ax, word [ebx]
   OUTW 0x1F0, ax ;write into hard disk
   add dword [ata_memory], 2
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  INB 0x1F7
  INB 0x1F7
  INB 0x1F7
  INB 0x1F7
 ENDFOR ata_nos_cycle

 ret

pata_delete:
 mov dword [ata_status], ATA_OK

 PATA_SELECT_SECTOR
 OUTB 0x1F7, 0xC5 ;write command
 PATA_WAIT 100000

 ;write sectors
 FOR dword [ata_number_of_sectors], ata_nos_cycle
  FOR 256, ata_sector_cycle
   OUTW 0x1F0, 0 ;write into hard disk
  ENDFOR ata_sector_cycle

  ;wait for prepare transfer
  INB 0x1F7
  INB 0x1F7
  INB 0x1F7
  INB 0x1F7
 ENDFOR ata_nos_cycle

 ret
