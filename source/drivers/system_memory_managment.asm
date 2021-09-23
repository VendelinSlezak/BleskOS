;BleskOS

memory_size dd 0
memory times 4096 db 0

allocated_size dd 0 ;in MB
allocated_memory_pointer dd 0

read_memory_size:
 mov dword [memory_size], 0
 
 ;deallocate all memory
 mov edi, memory
 mov eax, 1
 mov ecx, 4096
 rep stosb
 
 mov esi, 0x80000 ;here bootloader load info about memory
 mov ecx, 20
 .find_entry:
  cmp dword [esi+16], 1 ;free memory
  jne .next_loop
  cmp dword [esi+12], 0
  jne .64_bit
  cmp dword [esi+8], 1024*1024*31 ;system need min first 32 MB of RAM (1 MB is low memory)
  jg .32_bit
  
 .next_loop:
 add esi, 24
 loop .find_entry

 ret
 
 .64_bit:
 mov dword [memory_size], 0xFFFFFFFF
 jmp .free_memory
 
 .32_bit:
 mov eax, dword [esi+8]
 mov dword [memory_size], eax
 
 .free_memory:
 mov ebx, 1024*1024
 mov edx, 0
 div ebx ;convert to MB
 mov dword [memory_size], eax
 
 mov edi, memory
 mov ecx, eax
 sub ecx, 31 ;system memory
 mov eax, 0
 rep stosb ;clear memory
 
 ret

allocate_memory:
 mov dword [allocated_memory_pointer], 0

 mov esi, memory
 mov ecx, 4096
 mov edi, 0x02000000 ;first MB
 .search_for_free_memory:
  cmp byte [esi], 0
  jne .next_mb

  mov eax, dword [allocated_size]
  mov ebx, esi
  .check_if_enough:
   cmp byte [ebx], 0
   jne .next_mb
   inc ebx
  dec eax
  cmp eax, 0
  jne .check_if_enough

  ;we find enough free memory block
  jmp .free_memory_found
 .next_mb:
 inc esi
 add edi, 0x100000 ;next MB
 loop .search_for_free_memory

 ret ;memory is not free

 .free_memory_found:
 mov dword [allocated_memory_pointer], edi
 mov ecx, dword [allocated_size]
 .mark_blocks:
  mov byte [esi], 0x1
  inc esi
 loop .mark_blocks

 ret

release_memory:
 mov eax, dword [allocated_memory_pointer]
 cmp eax, 0x10000000
 jg .done ;invalid pointer
 sub eax, 0x02000000
 mov ebx, 0x100000
 mov edx, 0
 div ebx

 add eax, memory
 mov ecx, dword [allocated_size]
 .mark_blocks:
  mov byte [eax], 0
  inc eax
 loop .mark_blocks

 .done:
 ret
