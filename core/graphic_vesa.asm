;BleskOS

read_vesa_info:
  mov ax, word [0x20012]
  mov word [vesa_x], ax
  mov ax, word [0x20014]
  mov word [vesa_y], ax
  mov al, byte [0x20019]
  mov byte [vesa_bpp], al
  mov eax, dword [0x20028]
  mov dword [vesa_lfb], eax

  ret

draw_background:
  mov eax, dword [vesa_lfb]

  mov ecx, 480000
  .cycle:
    mov word [eax], 0xFFFF
    inc eax
    inc eax
  loop .cycle

  ret

vesa_pixel:
  push edx

  ;pixel position in memory is ((y*pixels_per_line)+x)*2
  mov eax, dword [vesa_pixel_y]
  mov ebx, 800 ;we are in mode 800x600
  mul ebx
  add eax, dword [vesa_pixel_x]
  mov ebx, 2 ;bpp is 2 bytes
  mul ebx
  add eax, dword [vesa_lfb] ;graphic memory

  mov bx, word [vesa_color]
  mov word [eax], bx ;write pixel

  pop edx

  ret

draw_char:
  push eax
  push ebx
  push ecx
  push edx

  mov edx, 0
  mov dl, al
  mov eax, edx

  mov ebx, 8
  mul ebx
  mov edx, eax
  add edx, vesa_font ;pointer to memory
  push edx

  ;pointer to memory
  mov eax, dword [vesa_pixel_y]
  mov ebx, 800 ;we are in mode 800x600
  mul ebx
  add eax, dword [vesa_pixel_x]
  mov ebx, 2 ;bpp is 2 bytes
  mul ebx
  add eax, dword [vesa_lfb] ;graphic memory
  mov bx, word [vesa_color]

  pop edx

  mov ecx, 8
  .cycle:
  push ecx
    mov cl, byte [edx]
    and cl, 0x80
    cmp cl, 0x80
    jne .pixel1
      mov word [eax], bx
    .pixel1:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x40
    cmp cl, 0x40
    jne .pixel2
      mov word [eax], bx
    .pixel2:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x20
    cmp cl, 0x20
    jne .pixel3
      mov word [eax], bx
    .pixel3:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x10
    cmp cl, 0x10
    jne .pixel4
      mov word [eax], bx
    .pixel4:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x08
    cmp cl, 0x08
    jne .pixel5
      mov word [eax], bx
    .pixel5:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x04
    cmp cl, 0x04
    jne .pixel6
      mov word [eax], bx
    .pixel6:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x02
    cmp cl, 0x02
    jne .pixel7
      mov word [eax], bx
    .pixel7:
    add eax, 2

    mov cl, byte [edx]
    and cl, 0x01
    cmp cl, 0x01
    jne .pixel8
      mov word [eax], bx
    .pixel8:
    add eax, 2

    add eax, 1584 ;new line
    inc edx
  pop ecx
  dec ecx
  cmp ecx, 0
  jne .cycle

  pop edx
  pop ecx
  pop ebx
  pop eax

  ret

print:
  .cycle:
    mov al, byte [edx]
    cmp al, 0
    je .done
    call draw_char
    add dword [vesa_pixel_x], 8 ;next char position
    inc edx
  jmp .cycle

  .done:
  ret

print_var:
  mov eax, ecx
  mov ebx, 100000000
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string], al
  sub al, '0'
  mov ebx, 100000000
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 10000000
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+1], al
  sub al, '0'
  mov ebx, 10000000
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 1000000
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+2], al
  sub al, '0'
  mov ebx, 1000000
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 100000
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+3], al
  sub al, '0'
  mov ebx, 100000
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 10000
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+4], al
  sub al, '0'
  mov ebx, 10000
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 1000
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+5], al
  sub al, '0'
  mov ebx, 1000
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 100
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+6], al
  sub al, '0'
  mov ebx, 100
  mul ebx
  sub ecx, eax

  mov eax, ecx
  mov ebx, 10
  mov edx, 0
  div ebx
  add al, '0'
  mov byte [vesa_print_var_string+7], al
  sub al, '0'
  mov ebx, 10
  mul ebx
  sub ecx, eax

  mov eax, ecx
  add al, '0'
  mov byte [vesa_print_var_string+8], al

  mov edx, vesa_print_var_string
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print
  inc edx
  cmp byte [edx], '0'
  jne .print

  .print:
  call print

  ret

print_hex:
  push ecx

  mov al, '0'
  call draw_char
  add dword [vesa_pixel_x], 8
  mov al, 'x'
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 28
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif1
    add al, 7
  .endif1:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 24
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif2
    add al, 7
  .endif2:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 20
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif3
    add al, 7
  .endif3:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 16
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif4
    add al, 7
  .endif4:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 12
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif5
    add al, 7
  .endif5:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 8
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif6
    add al, 7
  .endif6:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  shr eax, 4
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif8
    add al, 7
  .endif8:
  push ecx
  call draw_char
  add dword [vesa_pixel_x], 8

  pop ecx
  mov eax, ecx
  and eax, 0xF
  add al, '0'
  cmp al, ':'
  jl .endif9
    add al, 7
  .endif9:
  call draw_char

  ret

draw_line:
  ;pointer to memory
  mov eax, dword [vesa_pixel_y]
  mov ebx, 800 ;we are in mode 800x600
  mul ebx
  add eax, dword [vesa_pixel_x]
  mov ebx, 2 ;bpp is 2 bytes
  mul ebx
  add eax, dword [vesa_lfb] ;graphic memory
  mov bx, word [vesa_color]

  mov ecx, dword [vesa_line_lenght]
  .cycle:
    mov word [eax], bx
    inc eax
    inc eax
  loop .cycle

  ret

draw_column:
  ;pointer to memory
  mov eax, dword [vesa_pixel_y]
  mov ebx, 800 ;we are in mode 800x600
  mul ebx
  add eax, dword [vesa_pixel_x]
  mov ebx, 2 ;bpp is 2 bytes
  mul ebx
  add eax, dword [vesa_lfb] ;graphic memory
  mov bx, word [vesa_color]

  mov ecx, dword [vesa_column_height]
  .cycle:
    mov word [eax], bx
    add eax, 1600
  loop .cycle

  ret

draw_square:
  mov eax, dword [vesa_square_lenght]
  mov dword [vesa_line_lenght], eax

  mov ecx, dword [vesa_square_height]
  .cycle:
  push ecx
    call draw_line
    inc dword [vesa_pixel_y]
  pop ecx
  loop .cycle

  ret

draw_empty_square:
  mov eax, dword [vesa_square_lenght]
  mov dword [vesa_line_lenght], eax
  call draw_line

  mov eax, dword [vesa_square_height]
  mov dword [vesa_column_height], eax
  call draw_column

  mov eax, dword [vesa_square_height]
  add dword [vesa_pixel_y], eax
  mov eax, dword [vesa_square_lenght]
  mov dword [vesa_line_lenght], eax
  call draw_line

  mov eax, dword [vesa_square_height]
  sub dword [vesa_pixel_y], eax
  mov eax, dword [vesa_square_lenght]
  add dword [vesa_pixel_x], eax
  mov eax, dword [vesa_square_height]
  inc eax
  mov dword [vesa_column_height], eax
  call draw_column

  ret
    

vesa_x dw 0
vesa_y dw 0
vesa_bpp db 0
vesa_lfb dd 0

vesa_pixel_x dd 0
vesa_pixel_y dd 0
vesa_color dw 0

vesa_line_lenght dd 0
vesa_column_height dd 0
vesa_square_lenght dd 0
vesa_square_height dd 0

vesa_print_var_string db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
