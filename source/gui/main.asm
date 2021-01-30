;BleskOS

%define BLESKOS_GREEN 0x0600

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
 PRINT '[F1] Developer zone', main_te_str, LINE(6), COLUMN(2)
 SCREEN_Y_SUB eax, LINESZ*3
 PRINT 'You can shutdown computer with press power button', main_down_str, eax, COLUMN(2)

 call redraw_screen

 .main_gui_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_F1
  je developer_zone
 jmp .main_gui_halt
