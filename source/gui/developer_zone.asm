;BleskOS

%define DEVELOPER_ZONE_COLOR 0xD380

developer_zone:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 ;text
 PRINT 'Developer zone', dz_up_str, LINE(1), COLUMN(1)
 PRINT '[a] Hardware info', dz_hw_str, LINE(3), COLUMN(1)

 call redraw_screen

 .developer_zone_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je main_window

  cmp byte [key_code], KEY_A
  je hardware_info
 jmp .developer_zone_halt
