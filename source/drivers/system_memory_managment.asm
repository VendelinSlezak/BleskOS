;BleskOS

%define MEMORY_1_MB 1
%define MEMORY_4_MB 4
%define MEMORY_16_MB 16

memory times 4098 db 0
memory_block_pointer dd 0

allocate_free_memory:
 mov esi, memory
 mov edx, 0

 cmp eax, MEMORY_1_MB
 je .mem_1_mb

 cmp eax, MEMORY_4_MB
 je .mem_4_mb

 mov eax, 0 ;non succesful
 ret

 .mem_1_mb:
 mov ecx, 512
 .find_free_1_mb_block:
  cmp byte [esi], 0
  je .1_mb_block_found

  inc edx
  inc esi
 loop .find_free_1_mb_block

 mov eax, 0 ;non succesful
 ret

 .1_mb_block_found:
  mov byte [esi], MEMORY_1_MB ;set memory block

  mov eax, edx
  mov ebx, 0x100000
  mul ebx
  add eax, 0x1000000 ;skip BleskOS memory
  mov dword [memory_block_pointer], eax
 ret

 .mem_4_mb:
 mov ecx, 512
 .find_free_4_mb_block:
  cmp dword [esi], 0
  je .4_mb_block_found

  inc edx
  inc esi
 loop .find_free_4_mb_block

 mov eax, 0 ;non succesful
 ret

 .4_mb_block_found:
  mov dword [esi], 0x04040404 ;set memory block

  mov eax, edx
  mov ebx, 0x100000
  mul ebx
  add eax, 0x1000000 ;skip BleskOS memory
  mov dword [memory_block_pointer], eax
 ret

deallocate_memory:
 cmp eax, MEMORY_1_MB
 je .deallocate_1_mb

 cmp eax, MEMORY_4_MB
 je .deallocate_4_mb

 ret

 .deallocate_1_mb:
  sub dword [memory_block_pointer], 0x1000000 ;skip BleskOS memory
  mov eax, dword [memory_block_pointer]
  mov ebx, 0x100000
  mov edx, 0
  div ebx

  add eax, memory
  mov byte [eax], 0
 ret

 .deallocate_4_mb:
  sub dword [memory_block_pointer], 0x1000000 ;skip BleskOS memory
  mov eax, dword [memory_block_pointer]
  mov ebx, 0x100000
  mov edx, 0
  div ebx

  add eax, memory
  mov dword [eax], 0
 ret
