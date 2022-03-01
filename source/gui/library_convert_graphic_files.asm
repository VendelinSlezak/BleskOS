;BleskOS

; BMP FILE

lc_image_width dd 0
lc_image_heigth dd 0
lc_image_bpp dd 0

convert_bmp_file:
 push dword [allocated_memory_pointer]
 push dword [allocated_size]

 mov eax, dword [file_memory] 
 mov ebx, dword [eax+18]
 mov dword [lc_image_width], ebx
 mov ebx, dword [eax+22]
 mov dword [lc_image_heigth], ebx
 mov bx, word [eax+28]
 shr bx, 3 ;div 8 - convert from bits to bytes
 mov word [lc_image_bpp], bx
 
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
 je .not_enough_memory
 mov edi, dword [allocated_memory_pointer] ;memory for converted image
 
 ;convert image 
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
 
 cmp word [lc_image_bpp], 4
 je .four_bytes_bmp
 cmp word [lc_image_bpp], 3
 je .three_bytes_bmp
 cmp word [lc_image_bpp], 2
 je .two_bytes_bmp

 .done:
 ;release memory with uncoded file
 pop dword [allocated_size]
 pop dword [allocated_memory_pointer]
 call release_memory
 ret
 
 .not_enough_memory:
 pop eax
 pop eax
 ret
 
 .four_bytes_bmp:
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
 
 jmp .done
 
 .three_bytes_bmp:
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
 
 jmp .done
 
 .two_bytes_bmp:
 mov ecx, dword [lc_image_heigth]
 .convert_pixel_two_bytes_bmp:
 push ecx
  mov ecx, dword [lc_image_width]
  sub edi, edx
  .convert_line_two_bytes_bmp:
   mov eax, 0
   mov ax, word [esi]
   mov bx, ax
   shl eax, 8
   mov ax, bx
   shl ax, 5
   mov al, bl
   shl al, 3
   and eax, 0x00F8FCF8
   mov dword [edi], eax
   add esi, 2
   add edi, 4
  loop .convert_line_two_bytes_bmp
  sub edi, edx
 pop ecx
 loop .convert_pixel_two_bytes_bmp
 
 jmp .done
 
create_bmp_file:
 mov eax, dword [lc_image_width]
 mov ebx, dword [lc_image_heigth]
 mul ebx
 shl eax, 2 ;mul 4
 mov ecx, eax ;length of image in bytes
 add eax, 54 ;size of header
 mov ebx, 0x100000
 mov edx, 0
 div ebx
 inc eax
 mov dword [allocated_size], eax ;size of BMP file in MB
 push ecx
 call allocate_memory
 pop ecx
 cmp dword [allocated_memory_pointer], 0
 je .done
 mov esi, dword [allocated_memory_pointer]

 ;create header
 mov byte [esi], 'B'
 mov byte [esi+1], 'M'
 mov dword [esi+2], ecx ;size of file
 mov dword [esi+6], 0
 mov dword [esi+10], 54 ;offset to image data
 mov dword [esi+14], 40 ;size of extended header
 mov eax, dword [lc_image_width]
 mov dword [esi+18], eax
 mov eax, dword [lc_image_heigth]
 mov dword [esi+22], eax
 mov word [esi+26], 1
 mov word [esi+28], 32 ;4 bytes per pixel
 mov dword [esi+30], 0 ;no compression
 mov dword [esi+34], ecx ;size of image
 mov eax, dword [lc_image_width]
 mov dword [esi+38], eax
 mov eax, dword [lc_image_heigth]
 mov dword [esi+42], eax
 mov dword [esi+46], 0 ;number of colors in pallete
 mov dword [esi+50], 0 ;number of important colors
 
 ;create image data
 push ecx
 mov edi, esi
 add edi, 54
 mov esi, dword [file_memory]
 add edi, ecx
 mov edx, dword [lc_image_width]
 shl edx, 2 ;mul 4 - number of bytes per line
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
 
 pop ecx
 add ecx, 54 ;length of file
 
 .done:
 ret
