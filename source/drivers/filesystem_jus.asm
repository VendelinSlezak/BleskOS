;BleskOS

;Description of JUS filesystem:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  1 MB  ; BleskOS code and other BleskOS things;
;  1 MB  ; Block Numbers of Files (BNF part)    ;
; 128 KB ; root directory (1 block)             ;
; to end ; Data of blocks                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Every block have his dword number in BNF part of disk. This number is number of file whose is written in this block. Block 0 is reserved for root dir.
;Free block have number 0. New file number is by number of first free block. Every block have 128 KB lenght.

;Example BNF part:
; dword 0 ; 0 ; reserved for root dir
; dword 1 ; 0 ;
; dword 2 ; 2 ;
; dword 3 ; 2 ;
; dword 4 ; 4 ;
; dword 5 ; 0 ;
; dword 6 ; 4 ;
; dword 7 ; 4 ;
; dword 8 ; 0 ;
;If we want to read file 2, we read block 2 and after it block 3. If we want to read file 4, we read block 4, 6, 7.
;If we want to write some file with lenght 3 blocks, we will write it to blocks 1, 4 and 7. File number will be 1.

jus_file_number dd 0
jus_file_memory dd 0
jus_file_lenght dd 0

read_bfn_part:
 READ_HARD_DISK 2000, 10, MEMORY_JUS_BNF ;test lenght of BNF part 10 sectors

 ret

write_bfn_part:
 WRITE_HARD_DISK 2000, 10, MEMORY_JUS_BNF ;test lenght of BNF part 10 sectors

 ret

read_block:
 push ebx
 push ecx
 push edx

 mov ebx, 256
 mul ebx
 mov ecx, dword [jus_file_memory]

 READ_HARD_DISK eax, 256, ecx

 add dword [jus_file_memory], 256*512 ;sector is 512 bytes * 256 sectors per block

 pop edx
 pop ecx
 pop ebx

 ret

write_block:
 push ebx
 push ecx
 push edx

 mov ebx, 256
 mul ebx
 mov ecx, dword [jus_file_memory]

 WRITE_HARD_DISK eax, 256, ecx

 add dword [jus_file_memory], 256*512 ;sector is 512 bytes * 256 sectors per block

 pop edx
 pop ecx
 pop ebx

 ret

jus_read_file:
 ;clear file block numbers
 mov edi, MEMORY_FILE_BLOCK_NUMBERS
 mov ecx, 0x100000
 .clear_fbn:
  mov byte [edi], 0
  inc edi
 loop .clear_fbn
 
 ;read file block numbers
 mov eax, dword [jus_file_number]
 mov ebx, 1 ;skip root dir block
 mov esi, MEMORY_JUS_BNF+4 ;skip root dir block
 mov edi, MEMORY_FILE_BLOCK_NUMBERS
 mov ecx, 0x100000/4
 .read_fbn:
  cmp dword [esi], eax
  jne .next_loop

  mov dword [edi], ebx ;save block number
  add edi, 4

  .next_loop:
  inc ebx
  add esi, 4
 loop .read_fbn

 ;load file to memory
 mov ebx, 1
 mov edi, MEMORY_FILE_BLOCK_NUMBERS
 mov ecx, 0x100000/4
 .load_file:
  cmp dword [edi], 0
  je .next_loop2

  mov eax, ebx
  call read_block

  .next_loop2:
  inc ebx
  add edi, 4
 loop .load_file

 ret

jus_write_file:
 ;clear file block numbers
 mov edi, MEMORY_FILE_BLOCK_NUMBERS
 mov ecx, 0x100000
 .clear_fbn:
  mov byte [edi], 0
  inc edi
 loop .clear_fbn
 
 ;get free file block numbers
 mov ebx, 1 ;skip root dir block
 mov esi, MEMORY_JUS_BNF+4 ;skip root dir block
 mov edi, MEMORY_FILE_BLOCK_NUMBERS
 mov ecx, 0x100000/4
 .read_fbn:
  cmp dword [esi], 0 ;free block
  jne .next_loop

  mov dword [edi], ebx ;save block number
  add edi, 4

  .next_loop:
  inc ebx
  add esi, 4
 loop .read_fbn

 ;write file
 mov eax, dword [MEMORY_FILE_BLOCK_NUMBERS] ;file number
 mov dword [jus_file_number], eax
 mov edi, MEMORY_FILE_BLOCK_NUMBERS
 mov ecx, dword [jus_file_lenght]
 .write_file:
  mov eax, dword [edi]
  call write_block

  ;set block number
  mov esi, MEMORY_JUS_BNF
  mov eax, dword [edi]
  mov ebx, 4
  mul ebx
  add esi, eax
  mov eax, dword [jus_file_number]
  mov dword [esi], eax

  add edi, 4
 loop .write_file

 ;write bnf part
 call write_bnf_part

 ret
