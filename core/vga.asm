;BleskOS real mode edition

%macro SC 2
  mov ah, 0x02
  mov bh, 0
  mov dh, %1
  mov dl, %2
  int 10h
%endmacro

%macro GC 0
  mov ah, 0x03
  mov bh, 0
  int 10h
%endmacro

%macro SHOW_CURSOR 0
  mov ah, 0x01
  mov ch, 0x0E
  mov cl, 0x0F
  int 10h
%endmacro

%macro HIDE_CURSOR 0
  mov ah, 0x01
  mov ch, 0x0F
  mov cl, 0x0E
  int 10h
%endmacro

%macro DRAW_BACKGROUND 1
  mov bl, %1
  call draw_background
%endmacro

%macro DRAW_LINE 2
  mov bl, %2
  mov cx, %1
  call draw_line
%endmacro

%macro DRAW_COLUMN 2
  mov bl, %2
  mov cx, %1
  call draw_column
%endmacro

%macro DRAW_MEMORY 2
  mov si, %1
  mov cx, %2
  call draw_memory
%endmacro

%macro PRINT_CH 1
  mov ah, 0x0E
  mov al, %1
  mov bh, 0
  int 10h
%endmacro

%macro PRINT_R 3
  mov ah, 0x09
  mov al, %1
  mov bh, 0
  mov bl, %2
  mov cx, %3
  int 10h
%endmacro

%macro PRINT 1
  mov si, %1
  call print
%endmacro

%macro PRINT_V 1
  mov ax, %1
  call print_var
%endmacro

%macro PRINT_H 1
  mov ax, %1
  call print_hex
%endmacro

%macro PRINT_MEMORY 2
  mov si, %1
  mov cx, %2
  call print_memory
%endmacro

draw_background:
  mov ah, 0x02
  mov bh, 0
  mov dx, 0
  int 10h ;move cursor to 0, 0

  mov ah, 0x09
  mov al, ' '
  ;bl - color is defined
  mov cx, 2000
  int 10h ;draw background

  ret

draw_line:
  mov ah, 0x09
  mov al, ' '
  mov bh, 0
  ;bl - color is defined
  ;cx - lenght is defined
  int 10h

  ret

draw_column:
  push cx

  mov ah, 0x03
  mov bh, 0
  int 10h ;get cursor position

  mov al, ' ' ;this value isnt changed in cycle

  pop cx
  .draw:
  push cx
    mov ah, 0x09
    ;bl - color is defined
    mov cx, 1
    int 10h ;draw pixel

    mov ah, 0x02
    inc dh ;next row
    int 10h ;move cursor
  pop cx
  loop .draw

  ret

draw_memory:
  push cx

  mov ah, 0x03
  mov bh, 0
  int 10h ;get cursor position

  mov al, ' ' ;this value isnt changed in cycle

  pop cx
  .draw:
  push cx
    mov ah, 0x09
    mov al, ' '
    mov bl, byte [si]
    mov cx, 1
    int 10h

    mov ah, 0x02
    inc dl
    cmp dl, 80
    jne .move_cursor
      inc dh
      mov dl, 0
    .move_cursor:
    int 10h

    inc si
  pop cx
  loop .draw

  ret

print:
  mov ah, 0x0E
  mov bh, 0

  .print_char:
    mov al, byte [si]
    cmp al, 0 ;end of string?
    je .done

    int 10h ;print char
    inc si
  jmp .print_char

  .done:
  ret

print_var:
  cmp ax, 0
  jne .endif
    mov ah, 0x0E
    mov al, '0'
    mov bh, 0
    int 10h
    ret
  .endif:

  mov bx, 10 ;dividite by 10

  mov dx, 0
  div bx
  mov byte [print_var_string+4], dl ;zvysok

  mov dx, 0
  div bx
  mov byte [print_var_string+3], dl ;zvysok

  mov dx, 0
  div bx
  mov byte [print_var_string+2], dl ;zvysok

  mov dx, 0
  div bx
  mov byte [print_var_string+1], dl ;zvysok

  mov dx, 0
  div bx
  mov byte [print_var_string], dl ;zvysok

  add byte [print_var_string], '0'
  add byte [print_var_string+1], '0'
  add byte [print_var_string+2], '0'
  add byte [print_var_string+3], '0'
  add byte [print_var_string+4], '0'

  mov ah, 0x0E
  mov bh, 0

  cmp byte [print_var_string], '0'
  jne .draw_5
  cmp byte [print_var_string+1], '0'
  jne .draw_4
  cmp byte [print_var_string+2], '0'
  jne .draw_3
  cmp byte [print_var_string+3], '0'
  jne .draw_2
  jmp .draw_1

  .draw_5:
  mov al, byte [print_var_string]
  int 10h
  .draw_4:
  mov al, byte [print_var_string+1]
  int 10h
  .draw_3:
  mov al, byte [print_var_string+2]
  int 10h
  .draw_2:
  mov al, byte [print_var_string+3]
  int 10h
  .draw_1:
  mov al, byte [print_var_string+4]
  int 10h

  ret

print_hex:
  mov bx, ax
  shr bx, 12
  and bl, 0xF
  add bl, '0'
  cmp bl, ':'
  jl .endif1
    add bl, 7
  .endif1:
  mov byte [print_hex_string], bl

  mov bx, ax
  shr bx, 8
  and bl, 0xF
  add bl, '0'
  cmp bl, ':'
  jl .endif2
    add bl, 7
  .endif2:
  mov byte [print_hex_string+1], bl

  mov bx, ax
  shr bx, 4
  and bl, 0xF
  add bl, '0'
  cmp bl, ':'
  jl .endif3
    add bl, 7
  .endif3:
  mov byte [print_hex_string+2], bl

  mov bx, ax
  and bl, 0xF
  add bl, '0'
  cmp bl, ':'
  jl .endif4
    add bl, 7
  .endif4:
  mov byte [print_hex_string+3], bl

  mov ah, 0x0E
  mov bh, 0
  mov al, '0'
  int 10h
  mov al, 'x'
  int 10h
  mov al, byte [print_hex_string]
  int 10h
  mov al, byte [print_hex_string+1]
  int 10h
  mov al, byte [print_hex_string+2]
  int 10h
  mov al, byte [print_hex_string+3]
  int 10h

  ret

print_memory:
  mov ah, 0x0E
  mov bh, 0

  .print_char:
    mov al, byte [si]
    int 10h
    inc si
  loop .print_char

  ret

  print_var_string db 0, 0, 0, 0, 0
  print_hex_string db 0, 0, 0, 0, 0
