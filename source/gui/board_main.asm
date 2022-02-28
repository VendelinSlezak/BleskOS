;BleskOS

%define BLESKOS_GREEN 0x00C000

main_window:
 ;green background
 CLEAR_SCREEN BLESKOS_GREEN

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
 PRINT 'Welcome in BleskOS', main_up_str, LINE(2), COLUMN(2)
 PRINT 'You can open programs with keyboard:', main_second_str, LINE(4), COLUMN(2)
 PRINT '[a] Text editor', main_text_editor_str, LINE(6), COLUMN(2)
 PRINT '    Graphic editor (not in this version)', main_graphic_editor_str, LINE(8), COLUMN(2)
 PRINT '[c] Media viewer', main_media_viewer_str, LINE(10), COLUMN(2)
 PRINT '[d] Document editor', main_document_editor_str, LINE(12), COLUMN(2)
 PRINT '[e] Internet browser', main_internet_browser_str, LINE(14), COLUMN(2)
 PRINT '[F1] Hardware centrum', main_hardware_centrum_str, LINE(16), COLUMN(2)
 PRINT '[F2] Developer zone', main_te_str, LINE(18), COLUMN(2)

 call redraw_screen

 .main_gui_halt:
  call wait_for_usb_keyboard
  
  cmp byte [key_code], KEY_A
  je text_editor
  
  cmp byte [key_code], KEY_C
  je media_viewer
  
  cmp byte [key_code], KEY_D
  je document_editor
  
  cmp byte [key_code], KEY_E
  je internet_browser

  cmp byte [key_code], KEY_F1
  je hardware_centrum

  cmp byte [key_code], KEY_F2
  je developer_zone
 jmp .main_gui_halt
