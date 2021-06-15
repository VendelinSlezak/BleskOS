;BleskOS

%define DP_VESA_RED 0
%define DP_VESA_GREEN 1
%define DP_VESA_BLUE 2

dp_vesa_red db 0
dp_vesa_green db 0
dp_vesa_blue db 0

dp_vesa_choosed_color dd 0

dp_vesa:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 ;text
 PRINT 'Red:', dz_vesa_red_str, LINE(1), COLUMN(1)
 mov eax, 0
 mov al, byte [dp_vesa_red]
 PRINT_VAR eax, LINE(1), COLUMN(6)

 PRINT 'Green:', dz_vesa_green_str, LINE(3), COLUMN(1)
 mov eax, 0
 mov al, byte [dp_vesa_green]
 PRINT_VAR eax, LINE(3), COLUMN(8)

 PRINT 'Blue:', dz_vesa_blue_str, LINE(5), COLUMN(1)
 mov eax, 0
 mov al, byte [dp_vesa_blue]
 PRINT_VAR eax, LINE(5), COLUMN(7)

 PRINT '[arrow right/left/up/down] +1/-1/+10/-10', dz_vesa_arrows_str, LINE(7), COLUMN(1)
 PRINT '[R] Set red [G] Set green [B] Set blue', dz_vesa_colors_str, LINE(9), COLUMN(1)

 ;square of color
 mov bx, 0
 mov bl, byte [dp_vesa_red]
 and bl, 0xF8
 shl bx, 8

 mov cx, 0
 mov cl, byte [dp_vesa_green]
 and cl, 0xFC
 shl cx, 3

 mov dx, 0
 mov dl, byte [dp_vesa_blue]
 and dl, 0xF8
 shr dx, 3

 mov eax, 0
 mov ax, bx
 or ax, cx
 or ax, dx

 DRAW_SQUARE 200, 300, 200, 200, eax

 call redraw_screen

 .vesa_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone

  cmp byte [key_code], KEY_R
  je .key_r

  cmp byte [key_code], KEY_G
  je .key_g

  cmp byte [key_code], KEY_B
  je .key_b

  cmp byte [key_code], KEY_RIGHT
  je .key_right

  cmp byte [key_code], KEY_LEFT
  je .key_left

  cmp byte [key_code], KEY_UP
  je .key_up

  cmp byte [key_code], KEY_DOWN
  je .key_down
 jmp .vesa_halt

 .key_r:
  mov dword [dp_vesa_choosed_color], DP_VESA_RED
 jmp .vesa_halt

 .key_g:
  mov dword [dp_vesa_choosed_color], DP_VESA_GREEN
 jmp .vesa_halt

 .key_b:
  mov dword [dp_vesa_choosed_color], DP_VESA_BLUE
 jmp .vesa_halt

 .key_right:
  mov esi, dp_vesa_red
  add esi, dword [dp_vesa_choosed_color]

  inc byte [esi]
 jmp dp_vesa

 .key_left:
  mov esi, dp_vesa_red
  add esi, dword [dp_vesa_choosed_color]

  dec byte [esi]
 jmp dp_vesa

 .key_up:
  mov esi, dp_vesa_red
  add esi, dword [dp_vesa_choosed_color]

  add byte [esi], 10
 jmp dp_vesa

 .key_down:
  mov esi, dp_vesa_red
  add esi, dword [dp_vesa_choosed_color]

  sub byte [esi], 10
 jmp dp_vesa
