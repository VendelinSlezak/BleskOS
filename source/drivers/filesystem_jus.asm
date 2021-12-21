;BleskOS

%define JUS_BN_PART 2000
%define JUS_DATA 4000

%define JUS_NOT_ENOUGH_MEMORY 0
%define JUS_WRITING_SUCCESFUL 1
%define JUS_ERROR 2

jus_block dd 0
jus_memory dd 0
jus_file_size dd 0
jus_file_sector dd 0
jus_status dd 0
jus_folder_number dd 0

jus_bn_mem dd 0
jus_num_of_blocks dd 0

init_jus:
 cmp dword [hard_disk_base], 0
 je .done
 call select_hdd
 
 ;allocate memory
 mov dword [allocated_size], 1 ;we need one MB
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov dword [jus_bn_mem], eax
 
 ;read bn part
 mov dword [ata_memory], eax
 mov dword [ata_sectors_num], 20
 mov dword [ata_sector], JUS_BN_PART
 call read_hdd_sectors
 cmp dword [ata_status], IDE_ERROR
 je .error

 .done:
 ret
 
 .error:
 PSTR 'Error during reading JUS block numbers', error_str
 ret
 
jus_save_bn_part:
 mov eax, dword [jus_bn_mem]
 mov dword [ata_memory], eax
 mov dword [ata_sectors_num], 200
 mov dword [ata_sector], JUS_BN_PART
 call read_hdd_sectors
 
 ret
 
jus_read_file:
 mov eax, dword [jus_memory]
 mov dword [ata_memory], eax
 mov eax, dword [jus_file_sector]
 mov dword [ata_sector], eax
 mov eax, dword [jus_file_size]
 shl eax, 1 ;mul 2 - jus_file_size is in KB
 mov dword [ata_sectors_num], eax
 call read_hdd_sectors
 
 cmp dword [ata_status], IDE_ERROR
 je .error
 
 ret
 
 .error:
 mov dword [jus_status], JUS_ERROR
 PSTR 'error', errors_stresterter
 ret
 
jus_write_file:
 ;find enough free space for file
 mov eax, dword [jus_bn_mem]
 mov esi, eax
 mov eax, JUS_DATA ;sector of block
 mov ecx, 200*512
 .find_free_block:
  cmp byte [esi], 0
  je .free_block
  inc esi
  add eax, 256
 loop .find_free_block
 ret ;no free block founded
 
 .free_block:
 mov ebx, dword [jus_memory]
 mov dword [ata_memory], ebx
 mov byte [esi], 1 ;now is this block full
 mov dword [ata_sector], eax
 mov dword [jus_file_sector], eax
 mov dword [ata_sectors_num], 256
 call write_hdd_sectors ;save file
 cmp dword [ata_status], IDE_ERROR
 je .error
 
 ;save block numbers
 call jus_save_bn_part
 
 ret
 
 .error:
 mov dword [jus_status], JUS_ERROR
 mov byte [esi], 0
 PSTR 'error', errors_stresterter
 ret

jus_read_folder:
 cmp eax, 0
 jne .read_folder
 mov eax, JUS_DATA ;root folder
 .read_folder:
 mov dword [ata_memory], MEMORY_FOLDER
 mov dword [ata_sector], eax
 mov dword [ata_sectors_num], 256
 call read_hdd_sectors
 
 ret
 
jus_rewrite_folder:
 cmp eax, 0
 jne .rewrite_folder
 mov eax, JUS_DATA ;root folder
 .rewrite_folder:
 mov dword [ata_sector], eax
 mov dword [ata_memory], MEMORY_FOLDER
 mov dword [ata_sectors_num], 256
 call write_hdd_sectors
 
 ret
 
jus_write_folder:
 mov dword [jus_memory], MEMORY_FOLDER
 mov dword [jus_file_size], 128
 call jus_write_file
 
 ret
