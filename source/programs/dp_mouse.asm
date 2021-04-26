;BleskOS

%define DP_MOUSE_COLOR 0xF580

dp_mouse:
 ;orange background
 CLEAR_SCREEN DP_MOUSE_COLOR
 call redraw_screen

 ;text
 .draw:
 DRAW_SQUARE LINE(1), COLUMN(1), COLUMNSZ*27, LINESZ*5, DP_MOUSE_COLOR

 PRINT 'Mouse x:', dz_mouse_str1, LINE(1), COLUMN(1)
 mov eax, 0
 mov al, byte [mouse_data+1]
 PRINT_HEX eax, LINE(1), COLUMN(10)

 PRINT 'Mouse y:', dz_mouse_str2, LINE(3), COLUMN(1)
 mov eax, 0
 mov al, byte [mouse_data+2]
 PRINT_HEX eax, LINE(3), COLUMN(10)

 PRINT 'Mouse button:', dz_mouse_str3, LINE(5), COLUMN(1)
 mov al, byte [mouse_data]
 and al, 0x7
 IF_E al, 0x1, if_left_button
  PRINT 'LEFT BUTTON', dz_mouse_left, LINE(5), COLUMN(15)
 ENDIF if_left_button
 IF_E al, 0x2, if_right_button
  PRINT 'RIGHT BUTTON', dz_mouse_right, LINE(5), COLUMN(15)
 ENDIF if_right_button
 IF_E al, 0x4, if_middle_button
  PRINT 'MIDDLE BUTTON', dz_mouse_middle, LINE(5), COLUMN(15)
 ENDIF if_middle_button

 REDRAW_LINES_SCREEN LINE(1), LINESZ*5

 .mouse_halt:
  call wait_for_mouse_or_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .draw
