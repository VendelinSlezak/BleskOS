;BleskOS

%define ATA_ERROR 0
%define ATA_OK 1

ata_sector dq 0
ata_number_of_sectors dw 0
ata_memory dd 0
ata_status dd 0

%macro READ_HARD_DISK 3
 mov dword [ata_sector], %1
 mov dword [ata_number_of_sectors], %2
 mov dword [ata_memory], %3
 call read_hdd
%endmacro

%macro WRITE_HARD_DISK 3
 mov dword [ata_sector], %1
 mov dword [ata_number_of_sectors], %2
 mov dword [ata_memory], %3
 call write_hdd
%endmacro

init_ide_devices:
 OUTB 0x1F6, 0x40 ;master drive
 WAIT 2 ;wait for enable drive
 OUTB 0x3F6, 0x2 ;disable interrupt

 OUTB 0x176, 0xE0 ;master drive for CDROM
 WAIT 2 ;wait for enable drive
 OUTB 0x376, 0x2 ;disable interrupt

 ret

read_hdd:
 ;when SATA driver will be written here come code if hard disk is SATA type

 call pata_read

 ret

write_hdd:
 ;when SATA driver will be written here come code if hard disk is SATA type

 call pata_write

 ret
