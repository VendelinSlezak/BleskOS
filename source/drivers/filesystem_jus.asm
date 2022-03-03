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
 call write_hdd_sectors
 
 ret
 
jus_read_file:
 mov eax, dword [jus_memory]
 mov dword [ata_memory], eax
 mov eax, dword [jus_file_sector]
 mov dword [ata_sector], eax
 mov eax, dword [jus_file_size]
 shl eax, 1 ;mul 2 - jus_file_size is in KB
 mov ecx, eax
 mov dword [fd_file_length], eax
 mov eax, 0
 .read_sector:
 push ecx
  push eax
  mov dword [ata_number_of_sectors], 1
  call read_hdd
  pop eax
  inc dword [ata_sector]
  inc dword [fd_file_loaded_length]
  inc eax
  cmp eax, 200
  jb .next_sector
  call file_dialog_draw_percents_of_file
  mov eax, 0
 .next_sector:
 pop ecx
 loop .read_sector
  
 cmp dword [ata_status], IDE_ERROR
 je .error
 
 ret
 
 .error:
 mov dword [jus_status], JUS_ERROR
 PSTR 'error with reading file', error_str
 ret
 
jus_write_file:
 ;calculate how many blocks we need
 mov eax, dword [jus_file_size]
 mov ebx, 128
 mov edx, 0
 div ebx
 inc eax
 mov edx, eax
 mov ebx, 0
 
 ;find enough free space for file
 mov eax, dword [jus_bn_mem]
 mov esi, eax
 mov eax, JUS_DATA+256 ;sector of block
 mov ecx, 200*512
 .find_free_block:
  cmp byte [esi], 1
  je .full_block
  
  cmp ebx, 0
  jne .if_first_zero_block
   mov edi, eax
  .if_first_zero_block:
  inc ebx
  cmp ebx, edx
  je .write_file ;we found enough free memory
  jmp .next_item
  
  .full_block:
  mov ebx, 0
  
  .next_item:
  inc esi
  add eax, 256
 loop .find_free_block
 ret ;no free block founded
 
 .write_file:
 sub esi, edx ;pointer to first block
 inc esi
 mov eax, dword [jus_memory]
 mov dword [ata_memory], eax
 mov dword [ata_sector], edi
 mov dword [jus_file_sector], edi
 mov eax, dword [jus_file_size]
 mov ebx, 128
 mov edx, 0
 div ebx
 inc eax
 mov ebx, 256
 mul ebx
 mov dword [ata_sectors_num], eax
 push esi
 call write_hdd_sectors ;save file
 pop esi
 cmp dword [ata_status], IDE_ERROR
 je .error
 
 ;fill block numbers
 mov edi, esi
 mov eax, dword [jus_file_size]
 mov ebx, 128
 mov edx, 0
 div ebx
 inc eax
 mov ecx, eax
 mov eax, 1
 rep stosb ;fill blocks
 
 ;save block numbers
 call jus_save_bn_part
 
 ret
 
 .error:
 mov dword [jus_status], JUS_ERROR
 mov byte [esi], 0
 PSTR 'error with writing file', error_str
 ret
 
jus_delete_file:
 mov eax, dword [jus_file_sector]
 sub eax, JUS_DATA+256
 mov ebx, 256
 mov edx, 0
 div ebx
 add eax, dword [jus_bn_mem]
 mov edi, eax ;pointer to first block of file
 
 mov eax, dword [jus_file_size]
 shr eax, 7 ;div 128 - convert from KB to blocks
 inc eax
 mov ebx, 256
 mov edx, 0
 div ebx
 inc eax
 mov ecx, eax
 
 mov eax, 0
 rep stosd ;clear all blocks of file
 
 ;save block numbers
 call jus_save_bn_part
 
 ret

jus_read_folder:
 push eax
 mov edi, MEMORY_FOLDER
 mov eax, 0
 mov ecx, 1024*128
 rep stosb ;clear memory
 pop eax

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
