;BleskOS

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
 cmp word [sata_base], 0
 je .init_pata

 PSTR 'Serial ATA', serial_ata

 ;SATA_OUT 0x28, 0x2 ;disable BIOS ownership
 ;WAIT 2

 ;SATA_OUT 0x04, 0x80000000 ;enable AHCI mode

 ;SATA_OUT 0x118, 0 ;port 1 command register

 SATA_IN 0x10
 PHEX eax

 SATA_IN 0x124
 PHEX eax

 ret

 .init_pata:
 PSTR 'Paralel ATA', paralel_ata
 mov word [pata_base], 0x1F0
 OUTB 0x1F6, 0x40 ;master drive
 WAIT 2 ;wait for enable drive
 OUTB 0x3F6, 0x2 ;disable interrupt

 ret

read_hdd:
 cmp dword [sata_base], 0
 je .pata_read
 call sata_read
 ret

 .pata_read:
 call pata_read
 ret

write_hdd:
 cmp dword [sata_base], 0
 je .pata_write
 call sata_write
 ret

 .pata_write:
 call pata_write
 ret
