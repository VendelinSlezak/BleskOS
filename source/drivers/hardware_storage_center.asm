;BleskOS

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

serial_ata_ahci_enabled dd 0

read_hdd:
 call pata_read
 ret

write_hdd:
 call pata_write
 ret

delete_hdd:
 call pata_delete
 ret
