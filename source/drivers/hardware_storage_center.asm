;BleskOS

%define NO_DISK 0
%define UNKNOWN_DISK_FORMAT 1
%define ISO9660_DISK 2

serial_ata_ahci_enabled dd 0
disk_state dd 0

read_hdd:
 call pata_read
 ret

write_hdd:
 call pata_write
 ret

delete_hdd:
 call pata_delete
 ret
 
detect_optical_disk:
 mov dword [disk_state], NO_DISK
 cmp word [cdrom_base], 0
 je .done
 cmp word [cdrom_base], 0xFFFF
 je .done

 mov ax, word [cdrom_base]
 mov word [patapi_base], ax
 IF_E dword [cdrom_drive], IDE_MASTER, if_master
  call patapi_select_master
 ENDIF if_master
 IF_E dword [cdrom_drive], IDE_SLAVE, if_slave
  call patapi_select_slave
 ENDIF if_slave
 
 call patapi_read_capabilites
 cmp word [disk_size], 0
 je .done
 
 mov dword [patapi_sector], 0
 mov dword [patapi_memory], MEMORY_ISO9660_FOLDER
 call patapi_read
 cmp dword [patapi_status], IDE_ERROR
 je .done
 
 mov dword [disk_state], UNKNOWN_DISK_FORMAT
 
 call init_iso9660
 cmp dword [iso9660_present], 1
 jne .if_iso9660
  mov dword [disk_state], ISO9660_DISK
 .if_iso9660:
 
 .done:
 ret
