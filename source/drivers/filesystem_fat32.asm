;BleskOS

%macro MBR_PARTITION_ENTRY 1
 dd 0
 %1_partition_type db 0
 db 0, 0, 0
 %1_partition_lba dd 0
 %1_partition_size dd 0
%endmacro

mbr times 0x1BE db 0
 MBR_PARTITION_ENTRY first
 MBR_PARTITION_ENTRY second
 MBR_PARTITION_ENTRY third
 MBR_PARTITION_ENTRY fourth
 mbr_boot_signature dw 0

fat_bpb db 0, 0, 0
 fat_oem times 8 db 0
 fat_bps dw 0
 fat_sectors_per_cluster db 0
 fat_reserved_sectors dw 0
 fat_tables db 0
 times 15 db 0
 fat_sector_number dd 0
 fat_sectors_per_table dd 0
 times 4 db 0
 fat_root_dir_cluster dd 0
 times 23 db 0
 fat_label times 11 db 0
 times 430 db 0

fat_table times 512 db 0
fat_table_loaded_sector dd 0

fat_present dd 0
fat_base_sector dd 0
fat_table_sector dd 0
fat_data_sector dd 0

fat_entry dd 0
fat_entry_value dd 0
fat_cluster dd 0
fat_memory dd 0
fat_file_length dd 0
fat_file_clusters dd 0
fat_first_file_cluster dd 0

fat_lfn_file_name times 52 db 0

msd_read_mbr:
 mov esi, mbr
 mov ecx, 512
 .clear_mbr:
  mov byte [esi], 0
  inc esi
 loop .clear_mbr

 mov dword [msd_transfer_memory], mbr
 mov dword [msd_sector], 0
 call msd_read

 ret

init_fat:
 mov dword [fat_present], 0

 mov dword [msd_transfer_memory], fat_bpb
 mov eax, dword [fat_base_sector]
 mov dword [msd_sector], eax
 call msd_read
 cmp dword [msd_status], MSD_ERROR
 je .done

 cmp byte [fat_bpb], 0xEB
 jne .done

 mov dword [fat_present], 1

 mov eax, 0
 mov ax, word [fat_reserved_sectors]
 mov dword [fat_table_sector], eax
 mov eax, dword [fat_base_sector]
 add dword [fat_table_sector], eax

 mov eax, 0
 mov al, byte [fat_tables]
 mov ebx, dword [fat_sectors_per_table]
 mul ebx
 mov ebx, 0
 mov bx, word [fat_reserved_sectors]
 add eax, ebx
 add eax, dword [fat_base_sector]
 mov dword [fat_data_sector], eax

 mov dword [fat_table_loaded_sector], 0

 .done:
 ret

fat_get_entry:
 mov eax, dword [fat_entry]
 mov ebx, 128
 mov edx, 0
 div ebx
 push edx ;offset
 add eax, dword [fat_table_sector] ;sector of FAT table

 cmp dword [fat_table_loaded_sector], eax ;this sector is already loaded
 je .get_entry_value

 mov dword [msd_sector], eax
 mov dword [msd_transfer_memory], fat_table
 call msd_read
 cmp dword [msd_status], MSD_ERROR
 je .done
 mov dword [fat_table_loaded_sector], eax ;save loaded sector number

 .get_entry_value:
 pop edx
 mov eax, dword [fat_table+(edx*4)]
 mov dword [fat_entry_value], eax

 .done:
 ret

fat_set_entry:
 mov eax, dword [fat_entry]
 mov ebx, 128
 mov edx, 0
 div ebx
 add eax, dword [fat_table_sector]
 push eax ;sector of FAT table
 push edx ;offset

 cmp dword [fat_table_loaded_sector], eax ;this sector is already loaded
 je .set_entry_value

 mov dword [msd_sector], eax
 mov dword [msd_transfer_memory], fat_table
 call msd_read ;load sector
 cmp dword [msd_status], MSD_ERROR
 je .done
 mov dword [fat_table_loaded_sector], eax ;save loaded sector number

 .set_entry_value:
 pop edx
 mov eax, dword [fat_entry_value]
 mov dword [fat_table+(edx*4)], eax

 pop eax
 mov dword [msd_sector], eax
 mov dword [msd_transfer_memory], fat_table
 call msd_write ;rewrite sector

 .done:
 ret

fat_read_cluster:
 mov eax, dword [fat_cluster]
 sub eax, 2
 mov ebx, 0
 mov bl, byte [fat_sectors_per_cluster]
 mul ebx
 add eax, dword [fat_data_sector]

 mov dword [msd_sector], eax
 mov eax, dword [fat_memory]
 mov dword [msd_transfer_memory], eax

 mov ecx, 0
 mov cl, byte [fat_sectors_per_cluster]
 .read_cluster:
 push ecx
  call msd_read
  cmp dword [msd_status], MSD_ERROR
  je .error
  inc dword [msd_sector]
  add dword [msd_transfer_memory], 0x200 ;skip 
  add dword [fat_memory], 0x200 ;skip 
 pop ecx
 loop .read_cluster

 ret

 .error:
 pop ecx
 ret

fat_write_cluster:
 mov eax, dword [fat_cluster]
 sub eax, 2
 mov ebx, 0
 mov bl, byte [fat_sectors_per_cluster]
 mul ebx
 add eax, dword [fat_data_sector]

 mov dword [msd_sector], eax
 mov eax, dword [fat_memory]
 mov dword [msd_transfer_memory], eax

 mov ecx, 0
 mov cl, byte [fat_sectors_per_cluster]
 .read_cluster:
 push ecx
  call msd_write
  cmp dword [msd_status], MSD_ERROR
  je .error
  inc dword [msd_sector]
  add dword [msd_transfer_memory], 0x200 ;skip
  add dword [fat_memory], 0x200 ;skip
 pop ecx
 loop .read_cluster

 ret

 .error:
 pop ecx
 ret
 
fat_read_folder:
 cmp eax, 0
 jne .read_folder
 mov eax, dword [fat_root_dir_cluster]
 .read_folder:
 mov dword [fat_cluster], eax
 mov dword [fat_memory], MEMORY_FAT32_FOLDER
 call fat_read_cluster
 call convert_fat_folder_to_jus_folder
 
 ret
 
fat_rewrite_folder:
 cmp eax, 0
 jne .rewrite_folder
 mov eax, dword [fat_root_dir_cluster]
 .rewrite_folder:
 call convert_jus_folder_to_fat_folder
 mov dword [fat_cluster], eax
 mov dword [fat_memory], MEMORY_FAT32_FOLDER
 call fat_write_cluster
 
 ret
 
;TODO write folder

fat_read_file:
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, 10000
 .clear:
  mov dword [esi], 0
  add esi, 4
 loop .clear

 mov esi, MEMORY_FILE_DESCRIPTOR
 mov eax, dword [fat_entry]
 mov dword [esi], eax ;save first cluster

 mov eax, dword [fat_entry]
 push esi
 call fat_get_entry ;get value of first cluster
 pop esi

 mov ecx, 10000 ;max 10000 clusters
 .load_cluster_values:
 push ecx
  cmp dword [fat_entry_value], 0x0FFFFFF7
  ja .read_file ;we found last entry
  je .error ;bad cluster
  cmp dword [fat_entry_value], 0x00000000
  je .error ;free cluster

  mov eax, dword [fat_entry_value]
  mov dword [fat_entry], eax ;point to next cluster
  add esi, 4
  mov dword [esi], eax ;save cluster value
  push esi
  call fat_get_entry
  pop esi
 pop ecx
 loop .load_cluster_values
 ret

 .error:
 PSTR 'error', errorstre
 WAIT 3000
 pop ecx
 ret

 .read_file:
 pop ecx
 mov esi, MEMORY_FILE_DESCRIPTOR
 .read_cluster:
  cmp dword [esi], 0
  je .done

  mov eax, dword [esi]
  mov dword [fat_cluster], eax
  push esi
  call fat_read_cluster
  pop esi
  add esi, 4
 jmp .read_cluster

 .done:
 ret

fat_delete_file:
 mov ecx, 10000 ;max 10000 clusters
 .load_cluster_values:
  mov eax, dword [fat_entry]
  push ecx
  call fat_get_entry
  pop ecx
  push dword [fat_entry_value] ;save pointer to next cluster

  mov dword [fat_entry_value], 0x00000000
  push ecx
  call fat_set_entry ;delete cluster
  pop ecx
  cmp dword [msd_status], MSD_ERROR
  je .done

  pop dword [fat_entry_value]
  cmp dword [fat_entry_value], 0x0FFFFFF6
  jg .done
  cmp dword [fat_entry_value], 0x00000000
  je .done

  mov eax, dword [fat_entry_value]
  mov dword [fat_entry], eax ;point to next cluster
 loop .load_cluster_values

 .done:
 ret

fat_write_file:
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, 10000
 .clear:
  mov dword [esi], 0
  add esi, 4
 loop .clear

 ;calculate length of FAT table
 mov eax, dword [fat_sectors_per_table]
 mov ebx, 128
 mul ebx
 mov ecx, eax

 ;calculate how many clusters
 mov eax, dword [fat_file_length]
 mov ebx, 2
 mul ebx ;number of sectors of file
 mov ebx, 0
 mov bl, byte [fat_sectors_per_cluster]
 mov edx, 0
 div ebx
 inc eax ;number of clusters
 mov dword [fat_file_clusters], eax

 ;save first file cluster
 mov esi, MEMORY_FILE_DESCRIPTOR
 cmp dword [fat_first_file_cluster], 0
 je .skip_first_cluster
  mov eax, dword [fat_first_file_cluster]
  mov dword [esi], eax
  add esi, 4

  mov dword [fat_entry], eax
  mov dword [fat_entry_value], 0x0FFFFFFF
  push esi
  push ecx
  call fat_set_entry
  pop ecx
  pop esi
  cmp dword [msd_status], MSD_ERROR
  je .done

  dec dword [fat_file_clusters]
  cmp dword [fat_file_clusters], 0
  je .write_clusters
 .skip_first_cluster:

 mov dword [fat_entry], 2
 mov eax, dword [fat_file_clusters]
 ;ecx was calculated above
 .find_free_clusters:
  push eax
  push ecx
  push esi
  call fat_get_entry
  pop esi
  pop ecx
  pop eax
  cmp dword [msd_status], MSD_ERROR
  je .done
  cmp dword [fat_entry_value], 0x00000000
  jne .next_entry

  mov ebx, dword [fat_entry]
  mov dword [esi], ebx
  add esi, 4
  dec eax
  cmp eax, 0
  je .write_clusters
 .next_entry:
 inc dword [fat_entry]
 loop .find_free_clusters

 .write_clusters:
 mov dword [esi], 0x0FFFFFFF ;last pointer
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, dword [fat_file_clusters]
 .write_cluster:
  mov eax, dword [esi]
  mov dword [fat_cluster], eax
  push esi
  push ecx
  call fat_write_cluster
  pop ecx
  pop esi
  cmp dword [msd_status], MSD_ERROR
  je .done
  add esi, 4
 loop .write_cluster

 ;update fat table
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, dword [fat_file_clusters]
 .update_fat_table:
  mov eax, dword [esi]
  mov dword [fat_entry], eax
  mov eax, dword [esi+4]
  mov dword [fat_entry_value], eax
  push ecx
  push esi
  call fat_set_entry
  pop esi
  pop ecx
  cmp dword [msd_status], MSD_ERROR
  je .done
  add esi, 4
 loop .update_fat_table

 mov esi, MEMORY_FILE_DESCRIPTOR
 mov eax, dword [esi]
 mov dword [fat_first_file_cluster], eax

 .done:
 ret
 
convert_fat_folder_to_jus_folder:
 mov edi, MEMORY_FOLDER
 mov eax, 0
 mov ecx, 0x10000
 rep stosb
 
 mov esi, MEMORY_FAT32_FOLDER
 mov edi, MEMORY_FOLDER
 mov eax, fat_lfn_file_name 
 mov ecx, 2048 ;convert max 2048 items
 .convert_item:
  cmp byte [esi+11], 0
  je .done ;we are on last item
  cmp byte [esi+11], 0xF
  je .lfn_entry
  mov bl, byte [esi+11]
  test bl, 0xE ;hidden, system, volume id
  jnz .next_item
  cmp byte [esi], 0xE5 ;not used item
  je .next_item
  
  ;name
  push esi
  push edi
  push ecx
  
  mov eax, fat_lfn_file_name
  cmp word [eax], 0
  je .short_name
  
  add edi, 16
  mov eax, edi
  push eax
  
  mov esi, fat_lfn_file_name
  mov ecx, 50
  rep movsw ;copy name
  
  pop eax
  mov ecx, 50
  .find_comma:
   cmp word [eax], '.'
   jne .next_loop
   
   mov word [eax], 0
   jmp .comma_founded
  .next_loop:
  add eax, 2
  loop .find_comma
  .comma_founded:
  
  mov edi, fat_lfn_file_name
  mov eax, 0
  mov ecx, 52
  rep stosw ;clear file name
  jmp .long_name_end
  
  .short_name:
  mov bx, 0
  mov bl, byte [esi+0]
  mov word [edi+16], bx
  mov bl, byte [esi+1]
  mov word [edi+18], bx
  mov bl, byte [esi+2]
  mov word [edi+20], bx
  mov bl, byte [esi+3]
  mov word [edi+22], bx
  mov bl, byte [esi+4]
  mov word [edi+24], bx
  mov bl, byte [esi+5]
  mov word [edi+26], bx
  mov bl, byte [esi+6]
  mov word [edi+28], bx
  mov bl, byte [esi+7]
  mov word [edi+30], bx
  mov word [edi+32], 0

  .long_name_end:
  pop ecx
  pop edi
  pop esi
  
  ;type
  mov word [edi+14], 1 ;folder
  test byte [esi+11], 0x10
  jnz .if_not_directory
   mov word [edi+14], 2 ;normal file
   
   mov ebx, dword [esi+8]
   and ebx, 0x00FFFFFF
   mov dword [edi+116], ebx
  .if_not_directory:
  
  ;year
  mov ax, word [esi+16]
  shr ax, 9
  add ax, 1980
  mov word [edi+8], ax

  ;month
  mov ax, word [esi+16]
  shr ax, 5
  and ax, 0xF
  mov byte [edi+10], al

  ;day
  mov ax, word [esi+16]
  and ax, 0x1F
  mov byte [edi+11], al

  ;hour
  mov ax, word [esi+14]
  shr ax, 11
  mov byte [edi+12], al

  ;minute
  mov ax, word [esi+14]
  shr ax, 5
  and ax, 0x3F
  mov byte [edi+13], al
  
  ;cluster
  mov ax, word [esi+20]
  shl eax, 16
  mov ax, word [esi+26]
  mov dword [edi+0], eax
  
  ;size
  mov eax, dword [esi+28]
  mov ebx, 1024
  mov edx, 0
  div ebx ;convert to KB
  mov dword [edi+4], eax
  
  mov eax, fat_lfn_file_name
  add edi, 128 ;next jus item
  
  jmp .next_item
  
  .lfn_entry:
  mov ebx, 0
  mov bl, byte [esi+0]
  and bl, 0xF
  cmp bl, 4
  jg .next_item
  mov eax, 26
  dec ebx
  mul ebx
  add eax, fat_lfn_file_name
  mov bx, word [esi+1]
  mov word [eax+0], bx
  mov bx, word [esi+3]
  mov word [eax+2], bx
  mov bx, word [esi+5]
  mov word [eax+4], bx
  mov bx, word [esi+7]
  mov word [eax+6], bx
  mov bx, word [esi+9]
  mov word [eax+8], bx
  mov bx, word [esi+14]
  mov word [eax+10], bx
  mov bx, word [esi+16]
  mov word [eax+12], bx
  mov bx, word [esi+18]
  mov word [eax+14], bx
  mov bx, word [esi+20]
  mov word [eax+16], bx
  mov bx, word [esi+22]
  mov word [eax+18], bx
  mov bx, word [esi+24]
  mov word [eax+20], bx
  mov bx, word [esi+28]
  mov word [eax+22], bx
  mov bx, word [esi+30]
  mov word [eax+24], bx
  
 .next_item:
 add esi, 32 ;next FAT32 item
 dec ecx
 cmp ecx, 0
 jne .convert_item
 
 .done:
 ret

convert_jus_folder_to_fat_folder:
 mov edi, MEMORY_FOLDER
 mov ecx, 2048 ;convert max 2048 items
 .convert_item:
  cmp dword [edi], 0
  je .done

  mov eax, dword [edi+116]
  mov ebx, 32
  mul ebx
  add eax, MEMORY_FAT32_FOLDER ;pointer to right item
  mov esi, eax

  ;name
  mov al, byte [edi+16]
  mov byte [esi], al
  mov al, byte [edi+18]
  mov byte [esi+1], al
  mov al, byte [edi+20]
  mov byte [esi+2], al
  mov al, byte [edi+22]
  mov byte [esi+3], al
  mov al, byte [edi+24]
  mov byte [esi+4], al
  mov al, byte [edi+26]
  mov byte [esi+5], al
  mov al, byte [edi+28]
  mov byte [esi+6], al
  mov al, byte [edi+30]
  mov byte [esi+7], al

  ;type
  mov al, byte [edi+120]
  mov byte [esi+8], al
  mov al, byte [edi+121]
  mov byte [esi+9], al
  mov al, byte [edi+122]
  mov byte [esi+10], al

  ;size
  mov eax, dword [edi+4]
  mov ebx, 1024
  mul ebx
  mov dword [esi+28], eax

  add edi, 128
 .next_item:
 dec ecx
 cmp ecx, 0
 jne .convert_item

 .done:
 ret

create_new_fat_entry:
 mov esi, MEMORY_FAT32_FOLDER
 mov ecx, 2048
 .find_free_entry:
  cmp byte [esi], 0
  je .create_entry
  add esi, 32
 loop .find_free_entry
 ret

 .create_entry:
 ;name
 mov al, byte [edi+16]
 mov byte [esi], al
 mov al, byte [edi+18]
 mov byte [esi+1], al
 mov al, byte [edi+20]
 mov byte [esi+2], al
 mov al, byte [edi+22]
 mov byte [esi+3], al
 mov al, byte [edi+24]
 mov byte [esi+4], al
 mov al, byte [edi+26]
 mov byte [esi+5], al
 mov al, byte [edi+28]
 mov byte [esi+6], al
 mov al, byte [edi+30]
 mov byte [esi+7], al

 ;type
 mov al, byte [edi+120]
 mov byte [esi+8], al
 mov al, byte [edi+121]
 mov byte [esi+9], al
 mov al, byte [edi+122]
 mov byte [esi+10], al

 ;attribute
 mov al, 0x10
 cmp word [edi+14], 1
 je .write_attribute
 mov al, 0x01
 .write_attribute:
 mov byte [esi+11], al

 ;time
 mov byte [esi+12], 0
 mov byte [esi+13], 0
 mov word [esi+14], 0x0821
 mov word [esi+16], 0x0F01
 mov word [esi+18], 0x0821
 mov word [esi+22], 0x0821
 mov word [esi+24], 0x0821

 ;cluster
 mov eax, dword [edi]
 shr eax, 16
 mov word [esi+20], ax
 mov eax, dword [esi]
 mov word [esi+26], ax

 ;size
 mov eax, dword [edi+4]
 mov ebx, 1024
 mul ebx
 mov dword [esi+28], eax

 call convert_fat_folder_to_jus_folder
 ret
