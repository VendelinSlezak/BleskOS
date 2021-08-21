;BleskOS

last_second dd 0

%macro WRITE_TIME 2
 cmp dword [%1], 9
 jg .%1_10
  PRINT_CHAR '0', LINE(4), COLUMN(%2)
  mov eax, dword [%1]
  PRINT_VAR eax, LINE(4), COLUMN( (%2+1) )
  jmp .%1_end
 .%1_10:
  mov eax, dword [%1]
  PRINT_VAR eax, LINE(4), COLUMN(%2)
 .%1_end:
%endmacro

hardware_centrum:
 CLEAR_SCREEN 0x00C000

 ;border
 mov eax, dword [screen_x]
 DRAW_SQUARE 0, 0, eax, 10, BLACK
 mov ebx, dword [screen_y]
 DRAW_SQUARE 0, 0, 10, ebx, BLACK
 SCREEN_X_SUB eax, LINESZ
 mov ebx, dword [screen_y]
 DRAW_SQUARE 0, eax, 10, ebx, BLACK
 mov eax, dword [screen_x]
 SCREEN_Y_SUB ebx, LINESZ
 DRAW_SQUARE ebx, 0, eax, 10, BLACK

 ;text
 PRINT 'Hardware centrum', hc_str, LINE(2), COLUMN(2)
 PRINT '[enter] Detect USB devices', detect_usb_devices_str, LINE(6), COLUMN(2)
 PRINT '[F12] Shutdown', shutdown_str, LINE(8), COLUMN(2)
 ;connected USB devices
 IF_NE dword [usb_mouse_controller], 0, if_usb_mouse
  PRINT 'USB mouse is connected', usb_mouse_is_str, LINE(10), COLUMN(2)
 ENDIF if_usb_mouse
 IF_E dword [usb_mouse_controller], 0, if_usb_mouse_is_not
  PRINT 'USB mouse is not connected', usb_mouse_is_not_str, LINE(10), COLUMN(2)
 ENDIF if_usb_mouse_is_not
 mov eax, 0
 mov esi, mass_storage_devices
 mov ecx, 5
 .find_number_of_msd:
  cmp byte [esi+7], 0
  je .next_item
  inc eax
 .next_item:
 add esi, 16
 loop .find_number_of_msd
 PRINT_VAR eax, LINE(12), COLUMN(2)
 PRINT 'USB mass storage devices connected', usb_msd_num_of_str, LINE(12), COLUMN(4)
 call redraw_screen

 ;time
 .update_time:
 call read_time
 DRAW_SQUARE LINE(4), COLUMN(2), COLUMNSZ*32, LINESZ, 0x00C000 ;erase time from screen
 PRINT '    /  /     :  :', time_str, LINE(4), COLUMN(2)
 mov eax, dword [year]
 PRINT_VAR eax, LINE(4), COLUMN(2)
 WRITE_TIME month, 7
 WRITE_TIME day, 10
 WRITE_TIME hour, 13
 WRITE_TIME minute, 16
 WRITE_TIME second, 19
 mov eax, dword [second]
 mov dword [last_second], eax

 REDRAW_LINES_SCREEN LINESZ*4, LINESZ

 mov dword [keyboard_wait], 1
 .hardware_halt:
  hlt ;wait for next interrupt
  cmp dword [keyboard_wait], 0
  je .keyboard_event

  call read_time_second
  mov eax, dword [last_second]
  cmp eax, dword [second]
  je .hardware_halt
  jmp .update_time

 .keyboard_event:
  cmp byte [key_code], KEY_ESC
  je main_window

  cmp byte [key_code], KEY_ENTER
  je .detect_usb_devices

  cmp byte [key_code], KEY_F12
  je .shutdown

  mov dword [keyboard_wait], 1
 jmp .hardware_halt

 .detect_usb_devices:
  DRAW_SQUARE LINE(10), COLUMN(2), COLUMNSZ*36, LINESZ*10, 0x00C000 ;erase messages from screen
  PRINT 'Detecting USB devices...', detecting_usb_str, LINE(10), COLUMN(2)
  call redraw_screen
  WAIT 100
  call detect_usb_devices
 jmp hardware_centrum

 .shutdown:
  CLEAR_SCREEN 0x00C000
  PRINT 'Do you really want to shutdown computer? [enter] Yes [other] No', shutdown_ask_str, LINE(1), COLUMN(1)
  call redraw_screen
  WAIT 200
  call wait_for_keyboard
  cmp byte [key_code], KEY_ENTER
  jne hardware_centrum
  call shutdown
  WAIT 4000
  CLEAR_SCREEN 0x00C000
  PRINT 'We are sorry, some problem occured, you can shutdown with pressing power button', shutdown_problem_str, LINE(1), COLUMN(1)
  call redraw_screen
 .halt:
  hlt
 jmp .halt
