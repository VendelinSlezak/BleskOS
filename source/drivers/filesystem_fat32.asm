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
 sub eax, 2
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
 sub eax, 2
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
 call fat_get_entry ;get value of first cluster

 mov ecx, 10000 ;max 10000 clusters
 .load_cluster_values:
 push ecx
  cmp dword [fat_entry_value], 0x0FFFFFF7
  jg .read_file ;we found last entry
  je .error ;bad cluster
  cmp dword [fat_entry_value], 0x00000000
  je .error ;free cluster

  mov eax, dword [fat_entry_value]
  mov dword [fat_entry], eax ;point to next cluster
  add esi, 4
  mov esi, dword [fat_entry] ;save cluster value
  push esi
  call fat_get_entry
  pop esi
 pop ecx
 loop .load_cluster_values

 .error:
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

 mov dword [fat_entry], 2
 mov esi, MEMORY_FILE_DESCRIPTOR
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

 .done:
 ret

convert_fat_folder_to_jus_folder:
 mov edi, MEMORY_FOLDER
 mov ecx, 10000
 .clear:
  mov dword [edi], 0
  add edi, 4
 loop .clear

 mov esi, MEMORY_FAT32_FOLDER
 mov edi, MEMORY_FOLDER
 mov ebp, 0
 mov ecx, 2048 ;convert max 2048 items
 .convert_item:
  cmp byte [esi+11], 0
  je .done
  mov al, byte [esi+11]
  and al, 0x2
  cmp al, 0x2
  je .next_item
  mov al, byte [esi+11]
  and al, 0x4
  cmp al, 0x4
  je .next_item
  mov al, byte [esi+11]
  and al, 0x8
  cmp al, 0x8
  je .next_item
  cmp byte [esi+11], 0xF
  je .next_item

  ;name
  mov eax, dword [esi]
  mov dword [edi+16], eax
  mov eax, dword [esi+4]
  mov dword [edi+20], eax
  mov byte [edi+25], 0

  ;type
  mov eax, dword [esi+8]
  and eax, 0x00FFFFFF
  mov dword [edi+56], eax
  mov word [edi+14], 2

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

  ;size
  mov eax, dword [esi+28]
  mov ebx, 1024
  mov edx, 0
  div ebx ;convert to KB
  mov dword [edi+4], eax

  ;cluster
  mov ax, word [esi+20]
  shl ax, 16
  mov ax, word [esi+26]
  mov dword [edi+60], eax

  ;item offset
  mov eax, ebp
  mov dword [edi+50], eax

  add edi, 64
 .next_item:
 inc ebp
 add esi, 32
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

  mov eax, dword [edi+50]
  mov ebx, 32
  mul ebx
  add eax, MEMORY_FAT32_FOLDER ;pointer to right item
  mov esi, eax

  ;name
  mov eax, dword [edi+16]
  mov dword [esi], eax
  mov eax, dword [edi+20]
  mov dword [esi+4], eax

  ;type
  mov al, byte [edi+56]
  mov byte [esi+8], al
  mov al, byte [edi+57]
  mov byte [esi+9], al
  mov al, byte [edi+58]
  mov byte [esi+10], al

  add edi, 64
 .next_item:
 dec ecx
 cmp ecx, 0
 jne .convert_item

 .done:
 ret
