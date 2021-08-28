;BleskOS

iso9660_present dd 0
iso9660_root_dir_lba dd 0
iso9660_root_dir_length dd 0
iso9660_file_lba dd 0
iso9660_file_length dd 0
iso9660_file_memory dd 0

init_iso9660:
 mov dword [iso9660_present], 0
 
 mov dword [patapi_sector], 0x10
 mov dword [patapi_memory], MEMORY_ISO9660_FOLDER
 call patapi_read
 cmp dword [patapi_status], IDE_ERROR
 je .done
 cmp dword [MEMORY_ISO9660_FOLDER+1], 0x30304443
 jne .done

 mov ecx, 0x10
 .find_primary_volume_record:
  cmp byte [MEMORY_ISO9660_FOLDER], 1
  je .read_primary_volume_record

  mov dword [patapi_memory], MEMORY_ISO9660_FOLDER
  inc dword [patapi_sector]
  push ecx
  call patapi_read
  pop ecx
 cmp dword [patapi_status], IDE_ERROR
 je .done
 loop .find_primary_volume_record

 .done:
 ret

 .read_primary_volume_record:
 mov dword [iso9660_present], 1

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
 cmp dword [patapi_status], IDE_ERROR
 je .done
 loop .load_file

 .done:
 ret

convert_iso9660_folder_to_jus_folder:
 ;clear memory
 mov edi, MEMORY_FOLDER
 mov eax, 0
 mov ecx, 10000
 stosd
 
 mov esi, MEMORY_ISO9660_FOLDER
 mov edi, MEMORY_FOLDER
 mov ecx, 2048 ;max 2048 entries
 .convert_entry:
  cmp byte [esi+0], 0
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
  
  mov eax, dword [edi+4]
  mov ebx, 1024
  mov edx, 0
  div ebx
  mov dword [edi+4], eax ;convert to KB

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
  mov eax, esi
  add eax, 33 ;start of name
  mov ebx, edi
  add ebx, 16 ;start of name
  mov ecx, 50
  .char:
   mov dl, byte [eax]
   cmp dl, ';'
   je .end_of_string
   cmp dl, 0
   je .end_of_string
   mov byte [ebx], dl
   inc eax
   add ebx, 2
  dec ecx
  cmp ecx, 0
  jne .char
  .end_of_string:

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
