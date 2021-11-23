;BleskOS

%define LI_MONITOR 0xFFFFFFFF

li_source_memory dd 0
li_source_heigth dd 0
li_source_width dd 0
li_source_line dd 0
li_source_column dd 0
li_destination_memory dd 0
li_destination_heigth dd 0
li_destination_width dd 0
li_destination_line dd 0
li_destination_column dd 0
li_image_width dd 0
li_image_heigth dd 0

li_source_bytes_per_line dd 0
li_destination_bytes_per_line dd 0

transfer_image:
 ;set destination values
 cmp dword [li_destination_memory], LI_MONITOR
 jne .if_destination_monitor
  mov eax, dword [screen_lfb]
  mov dword [li_destination_memory], MEMORY_RAM_SCREEN
  mov eax, dword [screen_x]
  mov dword [li_destination_width], eax
  mov eax, dword [screen_y]
  mov dword [li_destination_heigth], eax
 .if_destination_monitor:
 mov eax, dword [li_destination_width]
 mov ebx, 4
 mul ebx
 mov dword [li_destination_bytes_per_line], eax

 ;set source values
 cmp dword [li_source_memory], LI_MONITOR
 jne .if_source_monitor
  mov eax, dword [screen_lfb]
  mov dword [li_source_memory], MEMORY_RAM_SCREEN
  mov eax, dword [screen_x]
  mov dword [li_source_width], eax
  mov eax, dword [screen_y]
  mov dword [li_source_heigth], eax
 .if_source_monitor:
 mov eax, dword [li_source_width]
 mov ebx, 4
 mul ebx
 mov dword [li_source_bytes_per_line], eax
 
 ;set other values
 mov eax, dword [li_destination_line]
 mov ebx, dword [li_destination_bytes_per_line]
 mul ebx
 add eax, dword [li_destination_column]
 add eax, dword [li_destination_column]
 add eax, dword [li_destination_column]
 add eax, dword [li_destination_column]
 add eax, dword [li_destination_memory]
 push eax
 
 mov eax, dword [li_source_line]
 mov ebx, dword [li_source_bytes_per_line]
 mul ebx
 add eax, dword [li_source_column]
 add eax, dword [li_source_column]
 add eax, dword [li_source_column]
 add eax, dword [li_source_column]
 add eax, dword [li_source_memory]
 
 pop ebx
 
 ;transfer image
 mov ecx, dword [li_image_heigth]
 .transfer_line:
 push ecx
  mov esi, eax
  mov edi, ebx
  mov ecx, dword [li_image_width]
  rep movsd ;transfer one line
  
  ;move to next line
  add eax, dword [li_source_bytes_per_line]
  add ebx, dword [li_destination_bytes_per_line]
 pop ecx
 loop .transfer_line
 
 ret
