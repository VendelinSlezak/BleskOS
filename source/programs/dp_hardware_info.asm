;BleskOS

%define HARDWARE_INFO_COLOR 0xF580

hardware_info:
 ;orange background
 CLEAR_SCREEN HARDWARE_INFO_COLOR

 ;text
 PRINT 'Hardware info', dz_up_str, LINE(1), COLUMN(1)

 PRINT 'Vesa x:', dz_hw_str1, LINE(3), COLUMN(1)
 mov eax, dword [screen_x]
 PRINT_VAR eax, LINE(3), COLUMN(9)

 PRINT 'Vesa y:', dz_hw_str2, LINE(5), COLUMN(1)
 mov eax, dword [screen_y]
 PRINT_VAR eax, LINE(5), COLUMN(9)

 PRINT 'Vesa bpp:', dz_hw_str3, LINE(7), COLUMN(1)
 mov eax, dword [screen_bites_per_pixel]
 PRINT_VAR eax, LINE(7), COLUMN(11)

 PRINT 'Vesa linear frame buffer:', dz_hw_str4, LINE(9), COLUMN(1)
 mov eax, dword [screen_lfb]
 PRINT_HEX eax, LINE(9), COLUMN(27)

 call redraw_screen

 .hardware_info_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .hardware_info_halt
