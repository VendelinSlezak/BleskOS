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
 times 24 db 0
 fat_label times 11 db 0
 times 430 db 0

fat_table times 512 db 0

fat_base_sector dd 0
fat_table_sector dd 0
fat_data_sector dd 0

fat_entry dd 0
fat_entry_value dd 0
fat_cluster dd 0
fat_memory dd 0

fat_present dd 0

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

 .done:
 ret

fat_get_entry:
 mov eax, dword [fat_entry]
 mov ebx, 128
 mov edx, 0
 div ebx
 push edx ;offset
 add eax, dword [fat_table_sector] ;sector of FAT table

 mov dword [msd_sector], eax
 mov dword [msd_transfer_memory], fat_table
 call msd_read
 cmp dword [msd_status], MSD_ERROR
 je .done

 pop edx
 sub edx, 2
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

 mov dword [msd_sector], eax
 mov dword [msd_transfer_memory], fat_table
 call msd_read ;load sector
 cmp dword [msd_status], MSD_ERROR
 je .done

 pop edx
 dec edx
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
 dec eax
 dec eax
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
 pop ecx
 loop .read_cluster

 ret

 .error:
 pop ecx
 ret

fat_write_cluster:
 mov eax, dword [fat_cluster]
 dec eax
 dec eax
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
 mov eax, dword [fat_entry]
 call fat_get_entry

 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, 10000
 .clear:
  mov dword [esi], 0
  add esi, 4
 loop .clear

 mov esi, MEMORY_FILE_DESCRIPTOR
 mov eax, dword [fat_entry]
 mov dword [esi], eax ;save first cluster

 mov ecx, 10000 ;max 10000 clusters
 .load_cluster_values:
 push ecx
  cmp dword [fat_entry_value], 0x0FFFFFF7
  jg .read_file ;we found last entry
  je .error ;bad cluster
  cmp dword [fat_entry_value], 0x00000000
  je .error ;free cluster

  mov eax, dword [fat_entry_value]
  mov dword [fat_entry], eax
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
