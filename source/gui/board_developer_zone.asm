;BleskOS

%define DEVELOPER_ZONE_COLOR 0xD380

developer_zone:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 ;text
 mov dword [color], BLACK
 PRINT 'Developer zone', dz_up_str, LINE(1), COLUMN(1)
 PRINT '[a] Log viewer', dz_log_viewer_str, LINE(3), COLUMN(1)
 PRINT '[b] Mouse', dz_mouse_str, LINE(5), COLUMN(1)

 call redraw_screen

 .developer_zone_halt:
  call wait_for_usb_keyboard

  cmp byte [key_code], KEY_ESC
  je main_window

  cmp byte [key_code], KEY_A
  je log_viewer

  cmp byte [key_code], KEY_B
  je dp_mouse
 jmp .developer_zone_halt
