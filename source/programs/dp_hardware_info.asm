;BleskOS

%define HARDWARE_INFO_COLOR 0xF580

hardware_info:
 ;orange background
 CLEAR_SCREEN HARDWARE_INFO_COLOR

 ;text
 PRINT 'Hardware info', dz_up_str, LINE(1), COLUMN(1)

 PRINT 'Vesa x:', dz_hw_vesa_x_str, LINE(3), COLUMN(1)
 mov eax, dword [screen_x]
 PRINT_VAR eax, LINE(3), COLUMN(9)

 PRINT 'Vesa y:', dz_hw_vesa_y_str, LINE(5), COLUMN(1)
 mov eax, dword [screen_y]
 PRINT_VAR eax, LINE(5), COLUMN(9)

 PRINT 'Vesa bpp:', dz_hw_vesa_bpp_str, LINE(7), COLUMN(1)
 mov eax, dword [screen_bites_per_pixel]
 PRINT_VAR eax, LINE(7), COLUMN(11)

 PRINT 'Vesa linear frame buffer:', dz_hw_vesa_lfb_str, LINE(9), COLUMN(1)
 mov eax, dword [screen_lfb]
 PRINT_HEX eax, LINE(9), COLUMN(27)

 PRINT 'Number of OHCI controllers:', dz_hw_str_ohci, LINE(13), COLUMN(1)
 mov eax, dword [ohci_num_of_controllers]
 PRINT_VAR eax, LINE(13), COLUMN(30)

 PRINT 'Number of UHCI controllers:', dz_hw_str_uhci, LINE(15), COLUMN(1)
 mov eax, dword [uhci_num_of_controllers]
 PRINT_VAR eax, LINE(15), COLUMN(30)

 PRINT 'Number of EHCI contollers:', dz_hw_str_ehci, LINE(17), COLUMN(1)
 mov eax, dword [ehci_num_of_controllers]
 PRINT_VAR eax, LINE(17), COLUMN(30)

 PRINT 'Number of xHCI controllers:', dz_hw_str_xhci, LINE(19), COLUMN(1)
 mov eax, dword [xhci_num_of_controllers]
 PRINT_VAR eax, LINE(19), COLUMN(30)

 PRINT 'Ethernet card type:', dz_hw_str7, LINE(21), COLUMN(1)
 mov eax, dword [ethernet_card_id]
 PRINT_HEX eax, LINE(21), COLUMN(21)

 call redraw_screen

 .hardware_info_halt:
  call wait_for_usb_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .hardware_info_halt
