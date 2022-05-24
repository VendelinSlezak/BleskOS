;BleskOS

%define DP_MOUSE_COLOR 0xF580

dp_cursor_line dd 0
dp_cursor_column dd 0
dp_now_mouse_data dd 0

dp_mouse:
 ;orange background
 CLEAR_SCREEN DP_MOUSE_COLOR
 call redraw_screen

 ;text
 .draw:
 mov dword [cursor_line], LINE(1)
 mov dword [cursor_column], COLUMN(1)
 mov dword [square_length], 100
 mov dword [square_heigth], 10
 mov dword [color], DP_MOUSE_COLOR
 call draw_square

 mov eax, dword [usb_mouse_data]
 mov dword [hex_print_value], eax
 mov dword [color], BLACK
 call print_hex

 REDRAW_LINES_SCREEN LINE(1), LINESZ

 .mouse_halt:
  call wait_for_usb_mouse

  cmp byte [key_code], KEY_ESC
  je developer_zone
  
  cmp byte [key_code], KEY_A
  je .normal_mode
  
  cmp byte [key_code], KEY_B
  je .slow_mode
  
  cmp byte [key_code], KEY_C
  je .double_slow_mode

  cmp dword [usb_mouse_data], 0
  je .mouse_halt
 jmp .draw
 
 .normal_mode:
  mov dword [usb_mouse_mode], USB_MOUSE_NORMAL_MODE
 jmp .mouse_halt
 
 .slow_mode:
  mov dword [usb_mouse_mode], USB_MOUSE_SLOW_MODE
 jmp .mouse_halt
 
 .double_slow_mode:
  mov dword [usb_mouse_mode], USB_MOUSE_DOUBLE_SLOW_MODE
 jmp .mouse_halt
