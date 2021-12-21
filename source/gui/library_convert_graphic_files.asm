;BleskOS

lc_image_width dd 0
lc_image_heigth dd 0
lc_image_bpp dd 0

convert_bmp_file:
 CLEAR_SCREEN WHITE
 
 mov eax, dword [file_memory] 
 mov ebx, dword [eax+18]
 mov dword [lc_image_width], ebx
 PVAR ebx
 mov ebx, dword [eax+22]
 mov dword [lc_image_heigth], ebx
 PVAR ebx
 
 mov bx, word [eax+28]
 shr bx, 3 ;div 8 - convert from bits to bytes
 mov word [lc_image_bpp], bx
 PVAR ebx
 
 mov esi, dword [eax+10]
 add esi, eax ;pointer to start of image data
 
 ;allocate memory for image
 mov eax, dword [lc_image_width]
 mov ebx, dword [lc_image_heigth]
 mul ebx
 mov ebx, 4
 mul ebx ;number of pixels
 
 mov ebx, 1024*1024
 mov edx, 0
 div ebx
 inc eax
 mov dword [allocated_size], eax
 push esi
 call allocate_memory
 pop esi
 cmp dword [allocated_memory_pointer], 0
 je .done
 mov edi, dword [allocated_memory_pointer] ;memory for converted image
 
 ;convert image 
 cmp word [lc_image_bpp], 4
 je .four_bytes_bmp
 cmp word [lc_image_bpp], 3
 je .three_bytes_bmp

 .done:
 ret
 
 .four_bytes_bmp:
 mov dword [file_memory], edi
 
 mov eax, dword [lc_image_width]
 mov word [edi], ax
 mov eax, dword [lc_image_heigth]
 mov word [edi+2], ax
 add edi, 8 ;skip header
 
 mov eax, dword [lc_image_width]
 mov ebx, dword [lc_image_heigth]
 mul ebx
 mov ebx, 4
 mul ebx
 add edi, eax
 
 mov edx, dword [lc_image_width]
 shl edx, 2 ;mul 4 - pixels per line
 
 mov ecx, dword [lc_image_heigth]
 .convert_pixel_four_bytes_bmp:
 push ecx
  mov ecx, dword [lc_image_width]
  sub edi, edx
  .convert_line_four_bytes_bmp:
   mov eax, dword [esi]
   and eax, 0x00FFFFFF
   mov dword [edi], eax
   add esi, 4
   add edi, 4
  loop .convert_line_four_bytes_bmp
  sub edi, edx
 pop ecx
 loop .convert_pixel_four_bytes_bmp
 
 ret
 
 .three_bytes_bmp:
 mov dword [file_memory], edi
 
 mov eax, dword [lc_image_width]
 mov word [edi], ax
 mov eax, dword [lc_image_heigth]
 mov word [edi+2], ax
 add edi, 8 ;skip header
 
 mov eax, dword [lc_image_width]
 mov ebx, dword [lc_image_heigth]
 mul ebx
 mov ebx, 4
 mul ebx
 add edi, eax
 
 mov edx, dword [lc_image_width]
 shl edx, 2 ;mul 4 - pixels per line
 
 mov ecx, dword [lc_image_heigth]
 .convert_pixel_three_bytes_bmp:
 push ecx
  mov ecx, dword [lc_image_width]
  sub edi, edx
  .convert_line_three_bytes_bmp:
   mov eax, dword [esi]
   and eax, 0x00FFFFFF
   mov dword [edi], eax
   add esi, 3
   add edi, 4
  loop .convert_line_three_bytes_bmp
  sub edi, edx
 pop ecx
 loop .convert_pixel_three_bytes_bmp
 
 ret
