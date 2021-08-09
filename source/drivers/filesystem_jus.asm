;BleskOS

;Description of JUS filesystem:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  1 MB  ; BleskOS code and other BleskOS things;
; 16 MB  ; Block Numbers (BN part)              ;
; 128 KB ; root directory (1 block)             ;
; to end ; Data of blocks                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Every block is 128 KB length
;; There are three types of blocks: descriptor block, folder block or data block
;; Every file has one descriptor block. In this block are numbers of data blocks of the file.
;; Folder block has file entries. Every file entry has this format:

; dd ; number of descriptor block of this file
; dd ; size of file in KB
; dw ; year
; db ; month
; db ; day
; db ; hour
; db ; minute
; db ; second
; dw ; type of file 0=free entry 1=folder 2=pure text 0xFFFE=defined in name
; db ; name - max 50 chars

;In BN part we can found if is some block free or not. Every block has one byte. 
;0 mean free block, 1 mean used block and 0xFF mean that this sector of BN part
;was not readed from hard disk.

jus_block_number dd 0
jus_number_of_bn_sectors dd 0

jus_file_length dd 0
jus_file_blocks dd 0
jus_file_memory dd 0

jus_cycle dd 0

jus_load_bn_part:
 cmp dword [hard_disk_size], 0
 je .done

 ;select hard disk
 mov ax, word [hard_disk_base]
 mov word [pata_base], ax
 cmp dword [hard_disk_drive], IDE_MASTER
 jne .if_master
  call pata_select_master
 .if_master:
 cmp dword [hard_disk_drive], IDE_SLAVE
 jne .if_slave
  call pata_select_slave
 .if_slave:

 mov esi, MEMORY_JUS_BN
 mov ecx, 0x100000
 .clear_bn:
  mov byte [esi], 0xFF
  inc esi
 loop .clear_bn

 ;calculate number of BN sectors
 mov eax, dword [hard_disk_size]
 mov ebx, 131072 ;every sector describes 64 MB
 mov edx, 0
 div ebx
 mov ecx, eax
 inc ecx
 mov dword [jus_number_of_bn_sectors], ecx

 mov dword [ata_sector], 2000
 mov dword [ata_memory], MEMORY_JUS_BN
 .load_bn:
 push ecx
  mov dword [ata_number_of_sectors], 1
  call read_hdd
  inc dword [ata_sector]
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .load_bn

 .done:
 ret

 .error:
 pop ecx
 PSTR 'Error while reading BN', error_bn_str
 mov eax, dword [ata_sector]
 PVAR eax
 ret

jus_read_block:
 mov eax, dword [jus_block_number]
 mov ebx, 256 ;128 KB
 mul ebx
 add eax, 4000

 mov dword [ata_sector], eax ;first sector of block
 mov dword [ata_number_of_sectors], 256
 ;ata memory is already set
 call read_hdd
 cmp dword [ata_status], ATA_OK
 je .done
 call read_hdd ;try again
 cmp dword [ata_status], ATA_OK
 je .done
 WAIT 20
 call read_hdd ;try again

 .done:
 ret

jus_write_block:
 mov eax, dword [jus_block_number]
 mov ebx, 256 ;128 KB
 mul ebx
 add eax, 4000

 mov dword [ata_sector], eax ;first sector of block
 mov dword [ata_number_of_sectors], 256
 ;ata memory is already set
 call write_hdd
 cmp dword [ata_status], ATA_OK
 je .done
 call write_hdd ;try again
 cmp dword [ata_status], ATA_OK
 je .done
 WAIT 20
 call write_hdd ;try again

 .done:
 ret

jus_read_file:
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, 800 ;for testing max 100 MB files
 .read_file:
  mov eax, dword [esi]
  cmp eax, 0
  je .done

  mov dword [jus_block_number], eax
  push esi
  push ecx
  call jus_read_block
  pop ecx
  pop esi

  cmp dword [ata_status], ATA_ERROR
  je .done

  add esi, 4
 loop .read_file

 .done:
 ret

jus_write_file:
 ;clear descriptor memory
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, 0x100000
 .clear_descriptor:
  mov byte [esi], 0
  inc esi
 loop .clear_descriptor

 ;calculate number of file blocks
 mov eax, dword [jus_file_length]
 mov ebx, 128
 mov edx, 0
 div ebx
 add eax, 2
 mov dword [jus_file_blocks], eax

 ;find blocks
 mov esi, MEMORY_JUS_BN+1 ;skip root folder
 mov edi, MEMORY_FILE_DESCRIPTOR
 mov eax, 1 ;skip root folder
 mov ecx, 1000 ;value for testing
 .find_blocks:
  cmp byte [esi], 0
  jne .next_loop

  ;free block
  mov byte [esi], 2 ;we work with this block
  mov dword [edi], eax ;save number of this block
  add edi, 4

  dec dword [jus_file_blocks]
  cmp dword [jus_file_blocks], 0
  je .write_descriptor_block

 .next_loop:
 inc esi
 inc eax
 loop .find_blocks

 .write_descriptor_block:
 sub edi, 4
 mov eax, dword [edi] ;last block will be descriptor block
 mov dword [edi], 0
 mov dword [jus_block_number], eax
 mov dword [ata_memory], MEMORY_FILE_DESCRIPTOR
 call jus_write_block
 cmp dword [ata_status], ATA_ERROR
 je .done

 ;write file to disk
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov eax, dword [jus_file_memory]
 mov dword [ata_memory], eax
 mov dword [jus_cycle], 80 ;value for testing
 .write_file_block:
  mov eax, dword [esi]
  mov dword [jus_block_number], eax
  cmp eax, 0
  je .write_bn

  call jus_write_block
  cmp dword [ata_status], ATA_ERROR
  je .done

  add esi, 4
 dec dword [jus_cycle]
 cmp dword [jus_cycle], 0
 jne .write_file_block

 ;update values in BN
 .write_bn:
 mov dword [ata_sector], 2000
 mov dword [ata_memory], MEMORY_JUS_BN
 mov esi, MEMORY_JUS_BN

 mov ecx, dword [jus_number_of_bn_sectors]
 .update_bn_sector:
 push ecx
  mov edx, 0 ;do not rewrite this sector

  mov ecx, 512
  .read_bn_sector:
   cmp byte [esi], 0x2
   jne .next_byte

   mov byte [esi], 0x1
   mov edx, 1 ;rewrite this sector
  .next_byte:
  inc esi
  loop .read_bn_sector

  cmp edx, 0
  je .read_next_bn_sector

  ;write this sector
  mov dword [ata_number_of_sectors], 1
  call write_hdd
  cmp dword [ata_status], ATA_ERROR
  je .error

  sub dword [ata_memory], 0x200 ;write_hdd perform add 0x2000, but we will do it few lines bottom

 .read_next_bn_sector:
 inc dword [ata_sector]
 add dword [ata_memory], 0x200 ;skip sector
 pop ecx
 loop .update_bn_sector

 .done:
 ret

 .error:
 pop ecx
 ret

jus_delete_block:
 mov esi, MEMORY_FILE_DESCRIPTOR
 mov ecx, 800 ;value for testing
 .delete_block_from_bn:
  cmp dword [esi], 0
  je .write_bn

  mov eax, dword [esi] ;number of block
  mov byte [MEMORY_JUS_BN+eax], 0x2 ;delete block from bn

  add esi, 4
 loop .delete_block_from_bn

 .write_bn:
 mov dword [ata_sector], 2000
 mov dword [ata_memory], MEMORY_JUS_BN
 mov esi, MEMORY_JUS_BN

 mov ecx, dword [jus_number_of_bn_sectors]
 .update_bn_sector:
 push ecx
  mov edx, 0 ;do not rewrite this sector

  mov ecx, 512
  .read_bn_sector:
   cmp byte [esi], 0x2
   jne .next_byte

   mov byte [esi], 0x0
   mov edx, 1 ;rewrite this sector
  .next_byte:
  inc esi
  loop .read_bn_sector

  cmp edx, 0
  je .read_next_bn_sector

  ;write this sector
  mov dword [ata_number_of_sectors], 1
  call write_hdd
  cmp dword [ata_status], ATA_ERROR
  je .error

  sub dword [ata_memory], 0x200 ;write_hdd perform add 0x2000, but we will do it few lines bottom

 .read_next_bn_sector:
 inc dword [ata_sector]
 add dword [ata_memory], 0x200 ;skip sector
 pop ecx
 loop .update_bn_sector

 .done:
 ret

 .error:
 pop ecx
 ret
