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

 IF_E word [sound_card_type], 0, if_sc_none
  PRINT 'Sound card: none', dz_hw_sc_none_str, LINE(11), COLUMN(1)
 ENDIF if_sc_none

 IF_E word [sound_card_type], SOUND_CARD_AC97, if_sc_ac97
  PRINT 'Sound card: Audio Codec 97', dz_hw_sc_ac97_str, LINE(11), COLUMN(1)
 ENDIF if_sc_ac97

 PRINT 'Primary IDE:', dz_hw_str5, LINE(13), COLUMN(1)
 mov eax, 0
 INB 0x1F7
 PRINT_HEX eax, LINE(13), COLUMN(15)

 PRINT 'Secondary IDE:', dz_hw_str6, LINE(15), COLUMN(1)
 mov eax, 0
 INB 0x177
 PRINT_HEX eax, LINE(15), COLUMN(17)

 PRINT 'Number of OHCI ports:', dz_hw_str_ohci, LINE(17), COLUMN(1)
 mov eax, dword [ohci_num_of_ports]
 PRINT_VAR eax, LINE(17), COLUMN(23)

 PRINT 'Number of UHCI ports:', dz_hw_str_uhci, LINE(19), COLUMN(1)
 mov eax, dword [uhci_num_of_ports]
 PRINT_VAR eax, LINE(19), COLUMN(23)

 PRINT 'Number of EHCI ports:', dz_hw_str_ehci, LINE(21), COLUMN(1)
 mov eax, dword [ehci_num_of_ports]
 PRINT_VAR eax, LINE(21), COLUMN(23)

 PRINT 'Number of xHCI ports:', dz_hw_str_xhci, LINE(23), COLUMN(1)
 mov eax, dword [xhci_num_of_ports]
 PRINT_VAR eax, LINE(23), COLUMN(23)

 PRINT 'Ethernet card type:', dz_hw_str7, LINE(25), COLUMN(1)
 mov eax, dword [ethernet_card_id]
 PRINT_HEX eax, LINE(25), COLUMN(21)

 call redraw_screen

 .hardware_info_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .hardware_info_halt
