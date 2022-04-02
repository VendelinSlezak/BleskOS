;BleskOS

table_editor_up_str db 'Table editor', 0
table_editor_down_str db '[F4/5] Change column size [F6/7] Change line size', 0
be_message_new_file_up db 'Are you sure you want to erase all table?', 0
be_message_new_file_down db '[enter] Yes [esc] Cancel', 0
be_message_color_up db 'Please select color:', 0
be_message_color_down db '[a] Black [r] Red [g] Green [b] Blue [o] Orange [w] White', 0

table_editor_file_pointer dd 0

be_columns_length times 26 db 9
be_lines_length times 1000 db 1
be_lines_on_screen dd 0
be_first_show_line dd 1
be_first_show_column dd 0
be_selected_cell_line dd 1
be_selected_cell_column dd 0

be_cursor_offset dd 0

be_mouse_line dd 20
be_mouse_column dd 0
 
table_editor: 
 call be_draw_table
 call redraw_screen
 
 mov dword [mcursor_up_side], 20
 mov dword [mcursor_left_side], 0
 SCREEN_Y_SUB eax, 20
 mov dword [mcursor_down_side], eax
 mov eax, dword [screen_x]
 mov dword [mcursor_right_side], eax
 
 .be_halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F4
  je .change_column_length_down
  cmp byte [key_code], KEY_F5
  je .change_column_length_up
  cmp byte [key_code], KEY_F6
  je .change_line_heigth_down
  cmp byte [key_code], KEY_F7
  je .change_line_heigth_up
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  cmp byte [key_code], KEY_ENTER
  je .key_down
  
  cmp byte [key_code], KEY_LEFT
  je .key_left
  
  cmp byte [key_code], KEY_RIGHT
  je .key_right
  
  cmp byte [key_code], KEY_BACKSPACE
  je .key_backspace
  
  cmp byte [key_code], KEY_DELETE
  je .key_delete
  
  cmp word [key_unicode], 0
  jne .draw_char
  
  cmp dword [usb_mouse_data], 0
  jne .mouse_event
 jmp .be_halt
 
 .change_column_length_down:
  mov esi, be_columns_length
  add esi, dword [be_selected_cell_column]
  cmp byte [esi], 1
  je .be_halt
  dec byte [esi]
 jmp table_editor
 
 .change_column_length_up:
  mov esi, be_columns_length
  add esi, dword [be_selected_cell_column]
  cmp byte [esi], 40
  je .be_halt
  inc byte [esi]
 jmp table_editor
 
 .change_line_heigth_down:
  mov esi, be_lines_length
  add esi, dword [be_selected_cell_line]
  dec esi
  cmp byte [esi], 1
  je .be_halt
  dec byte [esi]
 jmp table_editor
 
 .change_line_heigth_up:
  mov esi, be_lines_length
  add esi, dword [be_selected_cell_line]
  dec esi
  cmp byte [esi], 20
  je .be_halt
  inc byte [esi]
 jmp table_editor
 
 .key_up:
  cmp dword [be_selected_cell_line], 1
  je .be_halt
  dec dword [be_selected_cell_line]
  mov dword [be_cursor_offset], 0
  
  mov eax, dword [be_selected_cell_line]
  cmp eax, dword [be_first_show_line]
  ja table_editor
  je table_editor
  mov dword [be_first_show_line], eax
 jmp table_editor
 
 .key_down:
  cmp dword [be_selected_cell_line], 999
  je .be_halt
  inc dword [be_selected_cell_line]
  mov dword [be_cursor_offset], 0
  
  mov esi, be_lines_length
  add esi, dword [be_first_show_line]
  dec esi
  mov dword [cursor_line], 48
  mov ecx, dword [be_first_show_line]
  .key_down_test_end_of_screen:
   mov eax, 0
   mov al, byte [esi]
   mov ebx, 13
   mul ebx
   add dword [cursor_line], eax
   inc esi
   inc ecx
   
   cmp ecx, dword [be_selected_cell_line]
   je .key_down_move_down
  jmp .key_down_test_end_of_screen
  
  .key_down_move_down:
  mov eax, dword [screen_y]
  sub eax, 20
  cmp dword [cursor_line], eax
  jb table_editor

  mov esi, be_lines_length
  add esi, dword [be_selected_cell_line]
  dec esi
  mov ecx, 48
  mov eax, dword [be_selected_cell_line]
  mov dword [be_first_show_line], eax
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add ecx, eax
  dec esi
  .find_first_line_for_show_all_of_last_line:
   mov eax, 0
   mov al, byte [esi]
   mov ebx, 13
   mul ebx
   add ecx, eax
   
   cmp ecx, dword [screen_y]
   ja .select_first_line
   
   dec esi
   dec dword [be_first_show_line]
  jmp .find_first_line_for_show_all_of_last_line
  
  .select_first_line:
  inc dword [be_first_show_line]
 jmp table_editor
 
 .key_left:
  cmp byte [be_cursor_offset], 0
  je .key_left_move_cell
  dec dword [be_cursor_offset]
  call be_redraw_input
  REDRAW_LINES_SCREEN 21, 13
 jmp .be_halt
  .key_left_move_cell:
  cmp dword [be_selected_cell_column], 0
  je .be_halt
  dec dword [be_selected_cell_column]
  mov eax, dword [be_selected_cell_column]
  cmp dword [be_first_show_column], eax
  jb .if_move_first_column_left
   mov dword [be_first_show_column], eax
  .if_move_first_column_left:
 jmp table_editor
 
 .key_right:
  call be_selected_cell_pointer
  add esi, 8
  mov eax, dword [be_cursor_offset]
  shl eax, 1 ;mul 2
  add esi, eax
  cmp word [esi], 0
  je .key_right_move_cell
  inc dword [be_cursor_offset]
  call be_redraw_input
  REDRAW_LINES_SCREEN 21, 13
 jmp .be_halt
  .key_right_move_cell:
  cmp dword [be_selected_cell_column], 25
  je .be_halt
  inc dword [be_selected_cell_column]
  
  call be_selected_cell_pointer
  add esi, 8
  mov eax, 0
  .key_right_find_cursor:
   cmp word [esi], 0
   je .key_right_cursor_founded
   add esi, 2
   inc eax
  jmp .key_right_find_cursor
  .key_right_cursor_founded:
  mov dword [be_cursor_offset], eax
  
  mov esi, be_columns_length
  add esi, dword [be_first_show_column]
  mov eax, 27
  mov ebx, dword [be_first_show_column]
  .key_right_test_end_of_screen:
   cmp eax, dword [screen_x]
   ja .key_right_move_right
   mov ecx, 0
   mov cl, byte [esi]
   shl ecx, 3 ;mul 8
   add ecx, 3
   add eax, ecx
   inc esi
   inc ebx ;actual column
  jmp .key_right_test_end_of_screen
  
  .key_right_move_right:
  dec ebx
  cmp dword [be_selected_cell_column], ebx ;ebx is column that is not showed all on screen
  jb table_editor

  ;find first column to show all of last column
  mov eax, 27
  mov ebx, dword [be_selected_cell_column]
  mov esi, be_columns_length
  add esi, dword [be_selected_cell_column]
  .find_first_column_for_show_all_of_last_column:
   mov ecx, 0
   mov cl, byte [esi]
   shl ecx, 3 ;mul 8
   add ecx, 3
   add eax, ecx
   cmp eax, dword [screen_x]
   ja .select_first_column
   je .select_first_column
   cmp ebx, 0
   je .select_first_column
   
   dec esi
   dec ebx
  jmp .find_first_column_for_show_all_of_last_column
  
  .select_first_column:
  inc ebx
  mov dword [be_first_show_column], ebx
 jmp table_editor
 
 .key_backspace:
  cmp dword [be_cursor_offset], 0
  je .be_halt
  dec dword [be_cursor_offset]
  
  call be_selected_cell_pointer
  add esi, 8
  mov eax, dword [be_cursor_offset]
  shl eax, 1 ;mul 2
  add esi, eax
  mov edi, esi
  add esi, 2
  mov ecx, 98
  sub ecx, dword [be_cursor_offset]
  cld
  rep movsw
  
  call be_draw_table
  REDRAW_LINES_SCREEN 21, 13
  call be_redraw_cell
 jmp .be_halt
 
 .key_delete:
  call be_selected_cell_pointer
  add esi, 8
  mov eax, dword [be_cursor_offset]
  shl eax, 1 ;mul 2
  add esi, eax
  mov edi, esi
  add esi, 2
  mov ecx, 98
  sub ecx, dword [be_cursor_offset]
  cld
  rep movsw
  
  call be_draw_table
  REDRAW_LINES_SCREEN 21, 13
  call be_redraw_cell
 jmp .be_halt
 
 .draw_char:
  cmp byte [be_cursor_offset], 40
  je .be_halt
  call be_selected_cell_pointer
  
  ;move all chars in cell
  push esi
  add esi, 204
  mov edi, esi
  add edi, 2
  mov ecx, 98
  sub ecx, dword [be_cursor_offset]
  inc ecx
  std
  rep movsw
  cld
    
  ;insert char
  pop esi
  add esi, 8
  mov eax, dword [be_cursor_offset]
  shl eax, 1 ;mul 2
  add esi, eax
  mov ax, word [key_unicode]
  mov word [esi], ax
  inc dword [be_cursor_offset]
  
  call be_draw_table
  REDRAW_LINES_SCREEN 21, 13
  call be_redraw_cell
 jmp .be_halt
 
 ;MOUSE
 .mouse_event:
  mov eax, dword [be_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [be_mouse_column]
  mov dword [cursor_column], eax
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [be_mouse_line], eax
  mov eax, dword [cursor_column]
  mov dword [be_mouse_column], eax
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  cmp dword [usb_mouse_dnd], 0x2
  je .mouse_click
 jmp .be_halt
 
 .mouse_click:
  SCREEN_Y_SUB eax, 20
  mov ebx, dword [screen_x]
  TEST_CLICK_ZONE_WITH_JUMP click_zone_table, be_mouse_line, be_mouse_column, 48, eax, 27, ebx, .select_cell_by_mouse
 
  SCREEN_X_SUB eax, 16+(13*14)
  mov ebx, eax
  add ebx, 24
  TEST_CLICK_ZONE_WITH_JUMP click_zone_bg_color, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_background_color
  SCREEN_X_SUB eax, 16+(13*12)
  mov ebx, eax
  add ebx, 24
  TEST_CLICK_ZONE_WITH_JUMP click_zone_tx_color, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_text_color
  SCREEN_X_SUB eax, 14+(13*10)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_bold, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_bold
  SCREEN_X_SUB eax, 11+(13*9)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_left_aligment, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_left_aligment
  SCREEN_X_SUB eax, 10+(13*8)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_middle_aligment, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_middle_aligment
  SCREEN_X_SUB eax, 9+(13*7)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_right_aligment, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_right_aligment
  SCREEN_X_SUB eax, 6+(13*6)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_border_no, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_border_no
  SCREEN_X_SUB eax, 5+(13*5)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_border_left, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_border_left
  SCREEN_X_SUB eax, 4+(13*4)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_border_up, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_border_up
  SCREEN_X_SUB eax, 3+(13*3)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_border_right, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_border_right
  SCREEN_X_SUB eax, 2+(13*2)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_border_down, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_border_down
  SCREEN_X_SUB eax, 1+(13*1)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_border_full, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_border_full
 jmp .be_halt
 
 .select_cell_by_mouse:
  mov eax, 27
  mov ecx, dword [be_first_show_column]
  mov esi, be_columns_length
  add esi, dword [be_first_show_column]
  .select_cell_by_mouse_column:
   mov ebx, 0
   mov bl, byte [esi]
   shl ebx, 3 ;mul 8
   add ebx, 3
   
   mov edx, ebx
   add edx, eax
   cmp edx, dword [screen_x]
   ja .be_halt
   
   cmp dword [be_mouse_column], eax
   jb .select_cell_by_mouse_next_column
   cmp dword [be_mouse_column], edx
   ja .select_cell_by_mouse_next_column
   
   mov dword [be_selected_cell_column], ecx
   jmp .select_cell_by_mouse_line
   
   .select_cell_by_mouse_next_column:
   add eax, ebx
   inc esi
   inc ecx
  jmp .select_cell_by_mouse_column
  
  .select_cell_by_mouse_line:
  mov esi, be_lines_length
  add esi, dword [be_first_show_line]
  dec esi
  mov eax, 48
  mov ecx, dword [be_first_show_line]
  .select_cell_by_mouse_line_loop:
   push eax
   mov eax, 0
   mov al, byte [esi]
   mov ebx, 13
   mul ebx
   mov ebx, eax
   pop eax
   add ebx, eax
   
   cmp dword [cursor_line], eax ;eax is first line of this line
   jb .select_cell_by_mouse_line_next_line
   cmp dword [cursor_line], ebx ;ebx is last line of this line
   jb .select_cell_by_mouse_line_founded
   
   .select_cell_by_mouse_line_next_line:
   mov eax, ebx
   inc esi
   inc ecx
   cmp ecx, 999
   je .select_cell_by_mouse_line_founded
  jmp .select_cell_by_mouse_line_loop
  
  .select_cell_by_mouse_line_founded:
  mov dword [be_selected_cell_line], ecx
  
  mov dword [be_cursor_offset], 0
 jmp table_editor
 
 .change_background_color:
  mov dword [message_window_key0], KEY_A
  mov dword [message_window_key1], KEY_R
  mov dword [message_window_key2], KEY_G
  mov dword [message_window_key3], KEY_B
  mov dword [message_window_key4], KEY_O
  mov dword [message_window_key5], KEY_W
  mov esi, be_message_color_up
  mov edi, be_message_color_down
  call show_message_window
  mov dword [message_window_key0], 0
  mov dword [message_window_key1], 0
  mov dword [message_window_key2], 0
  mov dword [message_window_key3], 0
  mov dword [message_window_key4], 0
  mov dword [message_window_key5], 0
  cmp byte [key_code], KEY_ESC
  je table_editor
  cmp byte [key_code], KEY_A
  jne .change_bg_color_black
   mov eax, BLACK
  .change_bg_color_black:
  cmp byte [key_code], KEY_R
  jne .change_bg_color_red
   mov eax, 0xFF0000
  .change_bg_color_red:
  cmp byte [key_code], KEY_G
  jne .change_bg_color_green
   mov eax, 0x00FF00
  .change_bg_color_green:
  cmp byte [key_code], KEY_B
  jne .change_bg_color_blue
   mov eax, 0x0000FF
  .change_bg_color_blue:
  cmp byte [key_code], KEY_O
  jne .change_bg_color_orange
   mov eax, 0xFF8000
  .change_bg_color_orange:
  cmp byte [key_code], KEY_W
  jne .change_bg_color_white
   mov eax, WHITE
  .change_bg_color_white:
  
  push eax
  call be_selected_cell_pointer
  pop eax
  mov byte [esi+2], al
  mov byte [esi+3], ah
  shr eax, 16
  mov byte [esi+4], al
 jmp table_editor
 
 .change_text_color:
  mov dword [message_window_key0], KEY_A
  mov dword [message_window_key1], KEY_R
  mov dword [message_window_key2], KEY_G
  mov dword [message_window_key3], KEY_B
  mov dword [message_window_key4], KEY_O
  mov dword [message_window_key5], KEY_W
  mov esi, be_message_color_up
  mov edi, be_message_color_down
  call show_message_window
  mov dword [message_window_key0], 0
  mov dword [message_window_key1], 0
  mov dword [message_window_key2], 0
  mov dword [message_window_key3], 0
  mov dword [message_window_key4], 0
  mov dword [message_window_key5], 0
  cmp byte [key_code], KEY_ESC
  je table_editor
  cmp byte [key_code], KEY_A
  jne .change_tx_color_black
   mov eax, BLACK
  .change_tx_color_black:
  cmp byte [key_code], KEY_R
  jne .change_tx_color_red
   mov eax, 0xFF0000
  .change_tx_color_red:
  cmp byte [key_code], KEY_G
  jne .change_tx_color_green
   mov eax, 0x00FF00
  .change_tx_color_green:
  cmp byte [key_code], KEY_B
  jne .change_tx_color_blue
   mov eax, 0x0000FF
  .change_tx_color_blue:
  cmp byte [key_code], KEY_O
  jne .change_tx_color_orange
   mov eax, 0xFF8000
  .change_tx_color_orange:
  cmp byte [key_code], KEY_W
  jne .change_tx_color_white
   mov eax, WHITE
  .change_tx_color_white:
  
  push eax
  call be_selected_cell_pointer
  pop eax
  mov byte [esi+5], al
  mov byte [esi+6], ah
  shr eax, 16
  mov byte [esi+7], al
 jmp table_editor
 
 .change_bold:
  call be_selected_cell_pointer
  mov al, byte [esi]
  or al, 0xFE
  not al
  and byte [esi], 0xFE
  or byte [esi], al
 jmp table_editor
 
 .change_left_aligment:
  call be_selected_cell_pointer
  and byte [esi], 0xF3
 jmp table_editor
 
 .change_middle_aligment:
  call be_selected_cell_pointer
  and byte [esi], 0xF3
  or byte [esi], 0b0100
 jmp table_editor
 
 .change_right_aligment:
  call be_selected_cell_pointer
  and byte [esi], 0xF3
  or byte [esi], 0b1000
 jmp table_editor
 
 .change_border_no:
  call be_selected_cell_pointer
  and byte [esi], 0x0F
 jmp table_editor
 
 .change_border_up:
  call be_selected_cell_pointer
  mov al, byte [esi]
  or al, 0b01111111
  not al
  and byte [esi], 0b01111111
  or byte [esi], al
 jmp table_editor
 
 .change_border_down:
  call be_selected_cell_pointer
  mov al, byte [esi]
  or al, 0b10111111
  not al
  and byte [esi], 0b10111111
  or byte [esi], al
 jmp table_editor
 
 .change_border_left:
  call be_selected_cell_pointer
  mov al, byte [esi]
  or al, 0b11011111
  not al
  and byte [esi], 0b11011111
  or byte [esi], al
 jmp table_editor
 
 .change_border_right:
  call be_selected_cell_pointer
  mov al, byte [esi]
  or al, 0b11101111
  not al
  and byte [esi], 0b11101111
  or byte [esi], al
 jmp table_editor
 
 .change_border_full:
  call be_selected_cell_pointer
  or byte [esi], 0xF0
 jmp table_editor

be_draw_table:
 DRAW_WINDOW table_editor_up_str, table_editor_down_str, 0x00FF00, WHITE
 mov dword [size_of_text], 1
 
 mov eax, dword [screen_x]
 DRAW_SQUARE 20, 0, eax, 15, 0xAAAAAA
 mov eax, dword [screen_y]
 sub eax, 40
 DRAW_SQUARE 35, 0, 27, eax, 0xFF8000
 mov eax, dword [screen_x]
 DRAW_SQUARE 35, 0, eax, 13, 0xFF8000
 
 ;draw table lines
 mov eax, dword [screen_x]
 DRAW_LINE 35, 0, eax, BLACK
 mov eax, dword [screen_x]
 DRAW_LINE 48, 0, eax, BLACK
 mov dword [cursor_line], 48
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 .draw_lines:
  push esi
  mov dword [cursor_column], 0
  mov dword [color], BLACK
  mov dword [line_length], 27
  call draw_line
  mov dword [cursor_column], 27
  mov dword [color], 0x888888
  SCREEN_X_SUB eax, 27
  mov dword [line_length], eax
  call draw_line
  pop esi
  
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [cursor_line], eax
  inc esi
  
  cmp esi, be_lines_length+1000
  je .table_lines_erase_place_after
  
  mov eax, dword [cursor_line]
  cmp eax, dword [screen_y]
  ja .table_lines_drawed
 jmp .draw_lines
 
 ;erase place after last line
 .table_lines_erase_place_after:
  inc dword [cursor_line]
  mov dword [cursor_column], 0
  mov dword [color], WHITE
  mov eax, dword [screen_x]
  mov dword [square_length], eax
  mov eax, dword [screen_y]
  cmp eax, dword [cursor_line]
  jb .table_lines_drawed
  je .table_lines_drawed
  sub eax, dword [cursor_line]
  call draw_square 
 .table_lines_drawed:
 
 ;draw table columns
 mov dword [cursor_line], 35
 mov dword [cursor_column], 0
 mov dword [color], BLACK
 mov eax, dword [screen_y]
 sub eax, 40
 mov dword [column_heigth], eax
 call draw_column
 add dword [cursor_column], 27
 call draw_column
 mov dword [color], 0x888888
 mov edx, 27 ;drawed columns
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .draw_columns:
  mov eax, 0
  mov al, byte [esi]
  shl eax, 3 ;mul 8
  add eax, 3 ;number of columns to next drawed column
  mov ebx, edx
  add ebx, eax
  cmp ebx, dword [screen_x]
  ja .columns_drawed
  
  add dword [cursor_column], eax
  add edx, eax
  
  push esi
  push edx
  push ecx
  mov dword [cursor_line], 35
  mov dword [column_heigth], 13
  mov dword [color], BLACK
  call draw_column
  mov dword [cursor_line], 49
  mov eax, dword [screen_y]
  sub eax, 44
  mov dword [column_heigth], eax
  mov dword [color], 0x888888
  call draw_column
  pop ecx
  pop edx
  pop esi
  
  inc esi
 loop .draw_columns
 
 ;all columns are on screen, so this will erase place after them
 mov eax, 0
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .count_columns:
  mov ebx, 0
  mov bl, byte [esi]
  shl ebx, 3 ;mul 8
  add ebx, 3
  add eax, ebx
  inc esi
 loop .count_columns
 add eax, 28
 mov dword [cursor_column], eax
 mov dword [cursor_line], 35
 mov ebx, dword [screen_x]
 sub ebx, eax
 mov dword [square_length], ebx
 SCREEN_Y_SUB eax, 40
 mov dword [square_heigth], eax
 mov dword [color], WHITE
 call draw_square
 
 .columns_drawed:
 
 ;print numbers of lines
 mov eax, dword [be_first_show_line]
 mov dword [var_print_value], eax
 mov dword [cursor_line], 48
 mov dword [color], BLACK
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 .draw_lines_numbers:
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  shr eax, 1 ;div 2
  sub eax, 2
  add dword [cursor_line], eax ;middle of line
 
  mov dword [cursor_column], 11
  cmp dword [var_print_value], 10
  jb .print_line_number
  mov dword [cursor_column], 6
  cmp dword [var_print_value], 100
  jb .print_line_number
  mov dword [cursor_column], 2
  .print_line_number:
  push eax
  push esi
  call print_var
  pop esi
  pop eax
  sub dword [cursor_line], eax
  
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [cursor_line], eax
  inc esi
  
  cmp esi, be_lines_length+1000
  je .table_lines_numbers_drawed
  
  mov eax, dword [cursor_line]
  cmp eax, dword [screen_y]
  ja .table_lines_numbers_drawed
  inc dword [var_print_value]
 jmp .draw_lines_numbers
 .table_lines_numbers_drawed:
 
 ;print chars of columns
 mov eax, dword [be_first_show_column]
 add eax, 'A'
 mov dword [char_for_print], eax
 mov dword [cursor_line], 39
 mov dword [cursor_column], 27
 mov dword [color], BLACK
 mov edx, 27 ;drawed columns
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .draw_columns_chars:
  mov eax, 0
  mov al, byte [esi]
  shl eax, 3 ;mul 8
  add eax, 3 ;number of columns to next drawed column
  mov edi, eax
  mov ebx, edx
  add ebx, eax
  
  add dword [cursor_column], eax
  add edx, eax
  
  push ebx
  push esi
  push edx
  push ecx
  push dword [cursor_column]
  shr edi, 1 ;div 2
  add edi, 4
  sub dword [cursor_column], edi
  call print_char
  pop dword [cursor_column]
  pop ecx
  pop edx
  pop esi
  pop ebx
  
  inc esi
  inc dword [char_for_print]
  
  cmp ebx, dword [screen_x]
  ja .columns_chars_drawed
 loop .draw_columns_chars
 .columns_chars_drawed:
 
 ;PRINT CELLS CONTENT
 
 ; byte 0: bit 0 - bold
 ;         bit 1 - italic
 ;         bit 2:3 - aligment 0=left 1=middle 2=right
 ;         bit 4 - right border
 ;         bit 5 - left border
 ;         bit 6 - down border
 ;         bit 7 - up border
 ; byte 1: bit 0:1 - aligment 0=up 1=middle 2=right
 ; byte 2-5: color of byckground
 ; byte 6-8: color of text
 ; byte 9-208: cell text in unicode
 
 mov ebp, dword [table_editor_file_pointer]
 mov eax, dword [be_first_show_line]
 dec eax
 mov ebx, 208*26 ;bytes per line
 mul ebx
 add ebp, eax
 mov eax, dword [be_first_show_column]
 mov ebx, 208
 mul ebx
 add ebp, eax ;pointer to first showed cell content
 mov dword [cursor_line], 20+15+14
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 .draw_cells_lines_cycle:
 push esi
  push ebp
  
  mov esi, be_columns_length
  add esi, dword [be_first_show_column]
  mov eax, 27
  .draw_cells_columns_cycle:
   mov dword [cursor_column], eax
   inc dword [cursor_column]
   
   mov ebx, 0
   mov bl, byte [esi]
   shl ebx, 3 ;mul 8
   add ebx, 3
   add ebx, eax
   
   ;here we have cursor_line and cursor_column on start of cell and ebp points to it's content
   pusha
    ;draw background
    mov eax, dword [ebp+2]
    and eax, 0x00FFFFFF
    cmp eax, WHITE
    je .skip_drawing_background
    mov dword [color], eax
    mov eax, 0
    mov al, byte [esi]
    shl eax, 3 ;mul 8
    add eax, 3
    mov dword [square_length], eax
    dec dword [square_length]
    mov dword [square_heigth], 12
    call draw_square
    .skip_drawing_background:
    
    ;draw border
    dec dword [cursor_line]
    dec dword [cursor_column]
    mov al, byte [ebp+0]
    test al, 0x80 ;up border
    jz .if_up_border
     mov eax, 0
     mov al, byte [esi]
     shl eax, 3 ;mul 8
     add eax, 3
     mov dword [line_length], eax
     mov dword [color], BLACK
     call draw_line
    .if_up_border:
    
    mov al, byte [ebp+0]
    test al, 0x40 ;down border
    jz .if_down_border
     add dword [cursor_line], 13
     mov eax, 0
     mov al, byte [esi]
     shl eax, 3 ;mul 8
     add eax, 3
     mov dword [line_length], eax
     mov dword [color], BLACK
     call draw_line
     sub dword [cursor_line], 13
    .if_down_border:
    
    mov al, byte [ebp+0]
    test al, 0x20 ;left border
    jz .if_left_border
     mov dword [column_heigth], 14
     mov dword [color], BLACK
     call draw_column
    .if_left_border:
    
    mov al, byte [ebp+0]
    test al, 0x10 ;right border
    jz .if_right_border
     push dword [cursor_column]
     mov eax, 0
     mov al, byte [esi]
     shl eax, 3 ;mul 8
     add eax, 3
     add dword [cursor_column], eax
     mov dword [column_heigth], 14
     mov dword [color], BLACK
     call draw_column
     pop dword [cursor_column]
    .if_right_border:
    inc dword [cursor_line]
    inc dword [cursor_column]
    
    ;set text setting
    mov eax, dword [ebp+5]
    and eax, 0x00FFFFFF
    mov dword [color], eax
    mov dword [type_of_text], PLAIN
    test byte [ebp], 0x1
    jz .if_bold_text
     mov dword [type_of_text], BOLD
    .if_bold_text
    
    ;set text aligment
    push dword [cursor_line]
    push dword [cursor_column]
    
    add dword [cursor_line], 2
    
    mov al, byte [ebp+0]
    and al, 0x0C
    cmp al, 0x00
    jne .if_aligment_left
     inc dword [cursor_column]
     jmp .print_cell_string
    .if_aligment_left:
    
    cmp al, 0x04
    jne .if_aligment_middle
     mov edi, ebp
     add edi, 8
     mov eax, 0 ;number of chars in string
     mov bl, byte [esi] ;number of showed chars in column
     .aligment_middle_count_chars:
      cmp word [edi], 0
      je .aligment_middle_calculate
      cmp al, bl
      je .aligment_middle_calculate
      inc eax
      add edi, 2
     jmp .aligment_middle_count_chars
     
     .aligment_middle_calculate:
     shl eax, 2 ;mul 4 ;this is half of length of string
     mov ebx, 0
     mov bl, byte [esi]
     shl ebx, 3 ;mul 8
     add ebx, 3 ;this is length of column
     shr ebx, 1 ;div 2 
     sub ebx, eax ;this is first draw column for aligmented string
     add dword [cursor_column], ebx
     jmp .print_cell_string
    .if_aligment_middle:
    
    cmp al, 0x08
    jne .if_aligment_right
     mov edi, ebp
     add edi, 8
     mov eax, 0 ;number of chars in string
     mov bl, byte [esi] ;number of showed chars in column
     .aligment_right_count_chars:
      cmp word [edi], 0
      je .aligment_right_calculate
      cmp al, bl
      je .aligment_right_calculate
      inc eax
      add edi, 2
     jmp .aligment_right_count_chars
     
     .aligment_right_calculate:
     shl eax, 3 ;mul 8 
     inc eax ;this is length of string
     mov ebx, 0
     mov bl, byte [esi]
     shl ebx, 3 ;mul 8
     add ebx, 3 ;this is length of column
     sub ebx, eax ;this is first draw column for aligmented string
     add dword [cursor_column], ebx
     jmp .print_cell_string
    .if_aligment_right:
    
    ;print string  
    .print_cell_string:  
    mov eax, ebp
    add eax, 8 ;text offset
    mov ecx, 0
    mov cl, byte [esi] ;number of showed chars
    test byte [ebp], 0x1
    jz .if_bold_text_2
     dec cl
    .if_bold_text_2
    .print_char_from_cell:
     mov ebx, 0
     mov bx, word [eax]
     cmp bx, 0
     je .end_of_printing_cell
     mov dword [char_for_print], ebx
     push ecx
     push eax
     call print_char
     pop eax
     pop ecx
     add eax, 2
     add dword [cursor_column], COLUMNSZ
    loop .print_char_from_cell
    .end_of_printing_cell:
    
    pop dword [cursor_column]
    pop dword [cursor_line]
   popa
   
   cmp ebx, dword [screen_x]
   ja .draw_cells_next_line
   
   mov eax, ebx
   inc esi
   add ebp, 208
  jmp .draw_cells_columns_cycle
  
  .draw_cells_next_line:
  pop ebp
  add ebp, 208*26 ;bytes per line
 pop esi
 mov eax, 0
 mov al, byte [esi]
 mov ebx, 13
 mul ebx
 add dword [cursor_line], eax
 inc esi
 
 cmp esi, be_lines_length+1000
 je .cell_content_drawed
 
 mov eax, dword [cursor_line]
 cmp eax, dword [screen_y]
 jb .draw_cells_lines_cycle
 .cell_content_drawed:

 ;highlight selected cell
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 mov dword [cursor_line], 48
 mov ecx, dword [be_first_show_line]
 .find_selected_cell_line:
  cmp ecx, dword [be_selected_cell_line]
  je .selected_cell_line_founded
  
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [cursor_line], eax
  inc ecx
  inc esi
 jmp .find_selected_cell_line 
 .selected_cell_line_founded:
 mov eax, 0
 mov al, byte [esi]
 mov ebx, 13
 mul ebx
 mov dword [square_heigth], eax
 
 mov eax, 27 ;column
 mov ebx, dword [be_first_show_column] ;skiped columns
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .find_selected_cell_column:
  cmp ebx, dword [be_selected_cell_column]
  je .selected_cell_column_founded
  
  mov ecx, 0
  mov cl, byte [esi]
  shl ecx, 3 ;mul 8
  add ecx, 3
  add eax, ecx
  inc ebx
  inc esi
 loop .find_selected_cell_column
 
 .selected_cell_column_founded:
 mov dword [cursor_column], eax
 mov esi, be_columns_length
 add esi, dword [be_selected_cell_column]
 mov eax, 0
 mov al, byte [esi]
 shl eax, 3 ;mul 8
 add eax, 3
 mov dword [square_length], eax
 mov dword [color], BLACK
 call draw_empty_square
 dec dword [cursor_line]
 dec dword [cursor_column]
 add dword [square_length], 2
 add dword [square_heigth], 2
 call draw_empty_square
 
 ;DRAW TOOLS
 call be_redraw_input
 
 call be_selected_cell_pointer
 push esi
 push esi
 push esi
 push esi
 
 ;full border
 mov al, byte [esi]
 and al, 0xF0
 cmp al, 0xF0
 jne .if_show_full_border
  SCREEN_X_SUB eax, 1+13
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_full_border:
 SCREEN_X_SUB eax, 1+13
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 1+13-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, BLACK
 
 ;down border
 pop esi
 test byte [esi], 0x40
 jz .if_show_down_border
  SCREEN_X_SUB eax, 2+(13*2)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_down_border:
 SCREEN_X_SUB eax, 2+(13*2)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 2+(13*2)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 2+(13*2)-2
 DRAW_LINE 23+8, eax, 9, BLACK
 
 ;right border
 pop esi
 test byte [esi], 0x10
 jz .if_show_right_border
  SCREEN_X_SUB eax, 3+(13*3)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_right_border:
 SCREEN_X_SUB eax, 3+(13*3)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 3+(13*3)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 3+(13*3)-10
 DRAW_COLUMN 23, eax, 9, BLACK
 
 ;up border
 pop esi
 test byte [esi], 0x80
 jz .if_show_up_border
  SCREEN_X_SUB eax, 4+(13*4)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_up_border:
 SCREEN_X_SUB eax, 4+(13*4)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 4+(13*4)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 4+(13*4)-2
 DRAW_LINE 23, eax, 9, BLACK
 
 ;left border
 pop esi
 test byte [esi], 0x20
 jz .if_show_left_border
  SCREEN_X_SUB eax, 5+(13*5)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_left_border:
 SCREEN_X_SUB eax, 5+(13*5)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 5+(13*5)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 5+(13*5)-2
 DRAW_COLUMN 23, eax, 9, BLACK
 
 ;no border
 mov al, byte [esi]
 and al, 0xF0
 cmp al, 0x00
 jne .if_show_no_border
  SCREEN_X_SUB eax, 6+(13*6)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_no_border:
 SCREEN_X_SUB eax, 6+(13*6)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 6+(13*6)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 
 ;show aligment
 mov al, byte [esi]
 and al, 0x0C
 cmp al, 0x08
 jne .if_show_right_aligment
  SCREEN_X_SUB eax, 9+(13*7)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_right_aligment:
 SCREEN_X_SUB eax, 9+(13*7)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 9+(13*7)-2
 PRINT_CHAR 'R', 24, eax
 
 mov al, byte [esi]
 and al, 0x0C
 cmp al, 0x04
 jne .if_show_middle_aligment
  SCREEN_X_SUB eax, 10+(13*8)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_middle_aligment:
 SCREEN_X_SUB eax, 10+(13*8)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 10+(13*8)-2
 PRINT_CHAR 'M', 24, eax
 
 mov al, byte [esi]
 and al, 0x0C
 cmp al, 0x00
 jne .if_show_left_aligment
  SCREEN_X_SUB eax, 11+(13*9)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_left_aligment:
 SCREEN_X_SUB eax, 11+(13*9)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 11+(13*9)-2
 PRINT_CHAR 'L', 24, eax
 
 ;show type of text
 test byte [esi], 0x1
 jz .if_show_bold
  SCREEN_X_SUB eax, 14+(13*10)
  DRAW_SQUARE 21, eax, 12, 12, 0x0088FF
 .if_show_bold:
 SCREEN_X_SUB eax, 14+(13*10)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 14+(13*10)-2
 PRINT_CHAR 'B', 24, eax
 
 ;color of text
 call be_selected_cell_pointer
 mov eax, dword [esi+5]
 and eax, 0x00FFFFFF
 mov dword [color], eax
 SCREEN_X_SUB eax, 16+(13*12)-2
 push eax
 push eax
 PRINT_CHAR 'A', 24, eax
 pop eax
 add eax, COLUMNSZ
 PRINT_CHAR 'B', 24, eax
 pop eax
 add eax, COLUMNSZ*2
 PRINT_CHAR 'C', 24, eax
 
 ;color of background
 call be_selected_cell_pointer
 mov ebx, dword [esi+2]
 and ebx, 0x00FFFFFF
 mov dword [color], ebx
 SCREEN_X_SUB eax, 16+(13*14)
 DRAW_SQUARE 21, eax, 24, 12, ebx
 SCREEN_X_SUB eax, 16+(13*14)
 DRAW_EMPTY_SQUARE 21, eax, 24, 12, BLACK
 SCREEN_X_SUB eax, 16+(13*14)-1
 DRAW_EMPTY_SQUARE 22, eax, 22, 10, BLACK
 
 mov dword [type_of_text], PLAIN
 DRAW_WINDOW_BORDERS table_editor_up_str, table_editor_down_str, 0x00FF00
 
 mov eax, dword [be_mouse_line]
 mov dword [cursor_line], eax
 mov eax, dword [be_mouse_column]
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 ret
 
be_redraw_input:
 SCREEN_X_SUB eax, 27+18+(13*14)
 DRAW_SQUARE 21, 27, eax, 13, WHITE
 SCREEN_X_SUB eax, 27+18+(13*14)
 DRAW_EMPTY_SQUARE 21, 27, eax, 12, BLACK
 call be_selected_cell_pointer
 add esi, 8 ;text offset
 
 mov dword [cursor_line], 23
 mov dword [cursor_column], 29
 mov ecx, 60 ;show max 60 chars
 .draw_char:
  mov eax, 0
  mov ax, word [esi]
  cmp ax, 0
  je .draw_cursor
  mov dword [char_for_print], eax
  push esi
  push ecx
  call print_char
  pop ecx
  pop esi
  add dword [cursor_column], COLUMNSZ
  add esi, 2
 loop .draw_char
 
 .draw_cursor:
 mov eax, dword [be_cursor_offset]
 shl eax, 3 ;mul 8
 add eax, 29
 cmp dword [be_cursor_offset], 0
 jne .if_zero
  dec eax
 .if_zero:
 mov dword [cursor_column], eax
 mov dword [cursor_line], 22
 mov dword [column_heigth], 11
 mov dword [color], 0x444444
 call draw_column
 ret
 
be_redraw_cell:
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 mov dword [first_redraw_line], 48
 mov ecx, dword [be_first_show_line]
 .skip_line:
  cmp ecx, dword [be_selected_cell_line]
  je .redraw_cell
 
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [first_redraw_line], eax
  inc esi
  inc ecx
 jmp .skip_line
 
 .redraw_cell:
 mov eax, 0
 mov al, byte [esi]
 mov ebx, 13
 mul ebx
 mov dword [how_much_lines_redraw], eax
 call redraw_lines_screen
 
 ret

be_selected_cell_pointer: ;return pointer to cell in esi
 mov eax, dword [be_selected_cell_line]
 dec eax
 mov ebx, 208*26
 mul ebx
 mov ecx, eax
 mov eax, dword [be_selected_cell_column]
 mov ebx, 208
 mul ebx
 add ecx, eax
 mov esi, ecx
 add esi, dword [table_editor_file_pointer]
 
 ret
