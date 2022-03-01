;BleskOS

%define DEVELOPER_ZONE_COLOR 0xD380

developer_zone:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 ;text
 mov dword [color], BLACK
 PRINT 'Developer zone', dz_up_str, LINE(1), COLUMN(1)
 PRINT '[a] Hardware info', dz_hw_str, LINE(3), COLUMN(1)
 PRINT '[b] Intel HD Audio', dz_hda_str, LINE(5), COLUMN(1)
 PRINT '[c] IDE', dz_ide_str, LINE(7), COLUMN(1)
 PRINT '[d] Mouse', dz_mouse_str, LINE(9), COLUMN(1)
 PRINT '[e] Acpi', dz_acpi_str, LINE(11), COLUMN(1)

 call redraw_screen

 .developer_zone_halt:
  call wait_for_usb_keyboard

  cmp byte [key_code], KEY_ESC
  je main_window

  cmp byte [key_code], KEY_A
  je hardware_info

  cmp byte [key_code], KEY_B
  je dp_hda

  cmp byte [key_code], KEY_C
  je dp_ide

  cmp byte [key_code], KEY_D
  je dp_mouse

  cmp byte [key_code], KEY_E
  je dp_acpi
 jmp .developer_zone_halt
