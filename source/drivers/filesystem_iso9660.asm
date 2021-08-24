;BleskOS

%define NO_DISK 0
%define UNKNOWN_DISK_FORMAT 1
%define ISO9660_DISK 2

disk_state dd 0
iso9660_root_dir_lba dd 0
iso9660_root_dir_length dd 0
iso9660_file_lba dd 0
iso9660_file_length dd 0
iso9660_file_memory dd 0

init_iso9660:
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

 mov dword [disk_state], NO_DISK
 call patapi_read_capabilites
 cmp dword [disk_size], 0
 je .done

 mov dword [patapi_sector], 0x10
 mov dword [patapi_memory], MEMORY_ISO9660_FOLDER
 call patapi_read
 cmp dword [patapi_status], IDE_ERROR
 je .done

 mov dword [disk_state], UNKNOWN_DISK_FORMAT
 cmp dword [MEMORY_ISO9660_FOLDER+1], 0x30304443
 jne .done

 mov ecx, 0x10
 .find_primary_volume_record:
 push ecx
  cmp byte [MEMORY_ISO9660_FOLDER], 1
  je .read_primary_volume_record

  mov dword [patapi_memory], MEMORY_ISO9660_FOLDER
  inc dword [patapi_sector]
  call patapi_read
 pop ecx
 cmp dword [patapi_status], IDE_ERROR
 je .done
 loop .find_primary_volume_record

 .done:
 ret

 .read_primary_volume_record:
 pop ecx
 mov dword [disk_state], ISO9660_DISK

 mov al, byte [MEMORY_ISO9660_FOLDER+162]
 mov byte [iso9660_root_dir_lba+3], al
 mov al, byte [MEMORY_ISO9660_FOLDER+163]
 mov byte [iso9660_root_dir_lba+2], al
 mov al, byte [MEMORY_ISO9660_FOLDER+164]
 mov byte [iso9660_root_dir_lba+1], al
 mov al, byte [MEMORY_ISO9660_FOLDER+165]
 mov byte [iso9660_root_dir_lba], al

 mov al, byte [MEMORY_ISO9660_FOLDER+170]
 mov byte [iso9660_root_dir_length+3], al
 mov al, byte [MEMORY_ISO9660_FOLDER+171]
 mov byte [iso9660_root_dir_length+2], al
 mov al, byte [MEMORY_ISO9660_FOLDER+172]
 mov byte [iso9660_root_dir_length+1], al
 mov al, byte [MEMORY_ISO9660_FOLDER+173]
 mov byte [iso9660_root_dir_length], al

 mov eax, dword [iso9660_root_dir_length]
 mov ebx, 2048
 mov edx, 0
 div ebx
 mov dword [iso9660_root_dir_length], eax

 ret

iso9660_read_file:
 mov eax, dword [iso9660_file_memory]
 mov dword [patapi_memory], eax
 mov eax, dword [iso9660_file_lba]
 mov dword [patapi_sector], eax

 mov ecx, dword [iso9660_file_length]
 .load_file:
 push ecx
  call patapi_read
  inc dword [patapi_sector]
 pop ecx
 loop .load_file

 ret

convert_iso9660_folder_to_jus_folder:
 mov esi, MEMORY_ISO9660_FOLDER
 mov edi, MEMORY_FOLDER
 mov ecx, 2048 ;max 2048 entries
 .convert_entry:
  cmp dword [esi+33], 0
  je .done

  ;LBA
  mov al, byte [esi+6]
  mov byte [edi+3], al
  mov al, byte [esi+7]
  mov byte [edi+2], al
  mov al, byte [esi+8]
  mov byte [edi+1], al
  mov al, byte [esi+9]
  mov byte [edi+0], al

  ;size
  mov al, byte [esi+14]
  mov byte [edi+7], al
  mov al, byte [esi+15]
  mov byte [edi+6], al
  mov al, byte [esi+16]
  mov byte [edi+5], al
  mov al, byte [esi+17]
  mov byte [edi+4], al

  ;year
  mov ax, 0
  mov al, byte [esi+18]
  add ax, 1900
  mov word [edi+8], ax

  ;month
  mov al, byte [esi+19]
  mov byte [edi+10], al

  ;day
  mov al, byte [esi+20]
  mov byte [edi+11], al

  ;hour
  mov al, byte [esi+21]
  mov byte [edi+12], al

  ;minute
  mov al, byte [esi+22]
  mov byte [edi+13], al

  ;type
  mov word [edi+14], 2
  mov al, byte [esi+25]
  and al, 0x2
  cmp al, 0x2
  jne .if_folder
   mov word [edi+14], 1
  .if_folder:

  ;name
  push ecx
  mov eax, esi
  add eax, 33
  mov edx, edi
  add edx, 16
  mov ecx, 0
  mov cl, byte [esi+32]
  .convert_name_byte:
   inc eax
   mov bx, 0
   mov bl, byte [eax]
   mov word [edx], bx
   inc eax
   add edx, 2
  loop .convert_name_byte
  pop ecx

  ;next item
  mov eax, 0
  mov al, byte [esi]
  add esi, eax
  add edi, 128
 dec ecx
 cmp ecx, 0
 jne .convert_entry

 .done:
 ret
