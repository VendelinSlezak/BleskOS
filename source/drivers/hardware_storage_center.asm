;BleskOS

%define IDE_MODE 1
%define AHCI_MODE 2

%define IDE_OK 1
%define IDE_ERROR 0
%define IDE_MASTER 1
%define IDE_SLAVE 2

%define NO_DISK 0
%define UNKNOWN_DISK_FORMAT 1
%define ISO9660_DISK 2

hard_disk_mode dd 0
cdrom_mode dd 0
disk_state dd 0

hard_disk_base dd 0
hard_disk_drive dd 0
hard_disk_size dd 0
cdrom_base dd 0
cdrom_drive dd 0

ata_sector dq 0
ata_sectors_num dd 0
ata_number_of_sectors dd 0
ata_memory dd 0
ata_status dd 0

atapi_status dd 0
atapi_sector dd 0
atapi_memory dd 0

select_hdd:
 cmp dword [hard_disk_mode], IDE_MODE
 je .ide
 cmp dword [hard_disk_mode], AHCI_MODE
 je .ahci
 ret
 
 .ide:
 mov eax, 0
 mov ax, word [hard_disk_base]
 mov word [pata_base], ax

 cmp dword [hard_disk_drive], IDE_MASTER
 jne .if_master
  call pata_select_master
  ret
 .if_master:
 
 cmp dword [hard_disk_drive], IDE_SLAVE
 jne .if_slave
  call pata_select_slave
  ret
 .if_slave:
 ret
 
 .ahci:
 mov eax, dword [hard_disk_base]
 mov dword [ahci_port_base], eax
 ret

read_hdd:
 cmp dword [hard_disk_mode], IDE_MODE
 je .ide
 cmp dword [hard_disk_mode], AHCI_MODE
 je .ahci
 ret
 
 .ide:
 call pata_read
 ret
 
 .ahci:
 call sata_read
 ret
 
read_hdd_sectors:
 mov ecx, dword [ata_sectors_num]
 .read_sector:
  mov dword [ata_number_of_sectors], 1
  push ecx
  call read_hdd
  pop ecx
  cmp dword [ata_status], 0
  je .done
  inc dword [ata_sector]
 loop .read_sector
 
 .done:
 ret

write_hdd:
 cmp dword [hard_disk_mode], IDE_MODE
 je .ide
 cmp dword [hard_disk_mode], AHCI_MODE
 je .ahci
 ret
 
 .ide:
 call pata_write
 ret
 
 .ahci:
 call sata_write
 ret
 
write_hdd_sectors:
 mov ecx, dword [ata_sectors_num]
 .write_sector:
  mov dword [ata_number_of_sectors], 1
  push ecx
  call write_hdd
  pop ecx
  cmp dword [ata_status], 0
  je .done
  inc dword [ata_sector]
 loop .write_sector
 
 .done:
 ret

delete_hdd:
 cmp dword [hard_disk_mode], IDE_MODE
 je .ide
 cmp dword [hard_disk_mode], AHCI_MODE
 je .ahci
 ret
 
 .ide:
 call pata_delete
 ret
 
 .ahci:
 call sata_delete
 ret
 
delete_hdd_sectors:
 mov ecx, dword [ata_sectors_num]
 .write_sector:
  mov dword [ata_number_of_sectors], 1
  push ecx
  call delete_hdd
  pop ecx
  cmp dword [ata_status], 0
  je .done
  inc dword [ata_sector]
 loop .write_sector
 
 .done:
 ret
 
select_optical_disk:
 cmp dword [cdrom_mode], IDE_MODE
 je .ide
 cmp dword [cdrom_mode], AHCI_MODE
 je .ahci
 ret
 
 .ide:
 mov ax, word [cdrom_base]
 mov word [patapi_base], ax
 IF_E dword [cdrom_drive], IDE_MASTER, if_master
  call patapi_select_master
 ENDIF if_master
 IF_E dword [cdrom_drive], IDE_SLAVE, if_slave
  call patapi_select_slave
 ENDIF if_slave
 ret
 
 .ahci:
 mov eax, dword [cdrom_base]
 mov dword [ahci_port_base], eax
 ret
 
detect_optical_disk:
 mov dword [disk_state], NO_DISK
 call select_optical_disk
 
 call patapi_read_capabilites
 cmp word [disk_size], 0
 je .done
 
 mov dword [atapi_sector], 0
 mov dword [atapi_memory], MEMORY_ISO9660_FOLDER
 call patapi_read
 cmp dword [atapi_status], IDE_ERROR
 je .done
 
 mov dword [disk_state], UNKNOWN_DISK_FORMAT
 
 call init_iso9660
 cmp dword [iso9660_present], 1
 jne .if_iso9660
  mov dword [disk_state], ISO9660_DISK
 .if_iso9660:
 
 .done:
 ret
