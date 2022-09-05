;BleskOS

;every pixel is saved as 4 bytes
; ---------------------------------------------------
; | R | G | B | number of repetitions of this color |
; ---------------------------------------------------
;if last entry is 0xFF, then next 4 bytes are used for real number of repetitions

sic_image_pointer dd 0
sic_image_width dd 0
sic_image_heigth dd 0
sic_comprimed_image_pointer dd 0
sic_comprimed_image_length dd 0
sic_image_mb dd 0

sic_comprime_image:
 mov eax, dword [sic_image_width]
 mov ebx, dword [sic_image_heigth]
 mul ebx
 push eax ;number of pixels in image
 shr eax, 18 ;mul 4 and div 1000000
 inc eax
 mov dword [allocated_size], eax ;size of file in MB
 mov dword [sic_image_mb], eax
 call allocate_memory

 mov edi, dword [allocated_memory_pointer]
 mov dword [sic_comprimed_image_pointer], edi
 add edi, 8
 mov dword [sic_comprimed_image_length], 8+4 ;header+first pixel
 
 mov esi, dword [sic_image_pointer]
 
 ;create header
 sub edi, 8
 mov dword [edi], 'BIMG'
 mov eax, dword [sic_image_width]
 mov word [edi+4], ax
 mov eax, dword [sic_image_heigth]
 mov word [edi+6], ax
 add edi, 8
 
 ;put first pixel
 mov eax, dword [esi]
 mov dword [edi], eax
 mov byte [edi+3], 1
 add esi, 4
 mov edx, eax ;in edx is color of last pixel
 
 pop ecx ;number of pixels in image
 dec ecx ;first pixel is done
 .comprime_pixel: 
 push ecx
  mov eax, dword [esi]
  cmp eax, edx ;is this pixel same color as last pixel?
  mov edx, eax
  jne .other_color
  cmp byte [edi+3], 0xFF
  je .more_than_0xFF
  
  inc byte [edi+3] ;same color, increase number of repetition
  cmp byte [edi+3], 0xFF
  jne .next_pixel
  mov dword [edi+4], 0
  add dword [sic_comprimed_image_length], 4
  jmp .next_pixel
  
  .other_color:
  cmp byte [edi+3], 0xFF
  jne .if_0xFF
  add edi, 4
  .if_0xFF:
  add edi, 4
  mov dword [edi], eax
  mov byte [edi+3], 1
  add dword [sic_comprimed_image_length], 4
  jmp .next_pixel
  
  .more_than_0xFF:
  inc dword [edi+4]
  
 .next_pixel:
 add esi, 4 
 pop ecx
 loop .comprime_pixel
 
 ret

sic_decomprime_image:
 mov esi, dword [sic_comprimed_image_pointer]
 mov eax, 0
 mov ebx, 0
 mov ax, word [esi+4]
 mov bx, word [esi+6]
 mul ebx
 shr eax, 18 ;mul 4 and div 1000000
 inc eax
 mov dword [allocated_size], eax ;size of file in MB
 mov dword [sic_image_mb], eax
 call allocate_memory
 
 mov edi, dword [allocated_memory_pointer]
 mov dword [sic_image_pointer], edi
 mov esi, dword [sic_comprimed_image_pointer]
 add esi, 8 ;skip header
 
 .decomprime_pixel:
  cmp dword [esi], 0
  je .done
  cmp byte [esi+3], 0
  je .next_pixel
  
  mov eax, dword [esi]
  and eax, 0x00FFFFFF ;color
  
  mov ecx, 0
  mov cl, byte [esi+3]
  cmp ecx, 0xFF
  jne .if_0xFF
   add ecx, dword [esi+4]
   add esi, 4
  .if_0xFF:
  and ecx, 0x1FFFFFFF

  rep stosd ;write pixels through edi
  
 .next_pixel:
 add esi, 4
 jmp .decomprime_pixel
 
 .done:
 ret
