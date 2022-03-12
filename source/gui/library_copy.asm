;BleskOS

copy_mem_pointer dd 0
copy_length dd 0
copy_source dd 0
copy_destination dd 0

copy_to_memory:
 mov esi, dword [copy_source]
 mov edi, dword [copy_mem_pointer]
 mov ecx, dword [copy_length]
 cld
 rep movsb
 
 ret
 
copy_from_memory:
 cmp dword [copy_length], 0
 je .done
 
 mov esi, dword [copy_mem_pointer]
 mov edi, dword [copy_destination]
 mov ecx, dword [copy_length]
 cld
 rep movsb
 
 .done:
 ret
