;BleskOS

memory times 256 db 0

allocated_size dd 0 ;in MB
allocated_memory_pointer dd 0

allocate_memory:
 mov dword [allocated_memory_pointer], 0

 mov esi, memory
 mov ecx, 256
 mov edi, 0x01000000 ;first MB
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
 sub eax, 0x01000000
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
