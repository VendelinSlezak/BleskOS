;BleskOS

table_editor_up_str db 'Table editor', 0
table_editor_down_str db '[F1] Save [F2] Open [F3] New [F4/5] Column size [F6/7] Line size [F11] Function [F12] Recalculate functions', 0
be_message_new_file_up db 'Are you sure you want to erase all table?', 0
be_message_new_file_down db '[enter] Yes [esc] Cancel', 0
be_message_color_up db 'Please select color:', 0
be_message_color_down db '[a] Black [r] Red [g] Green [b] Blue [o] Orange [w] White', 0
be_message_recalculating db 'Recalculating cell functions...', 0
be_message_invalid_cell_range db 'Please enter valid cell range', 0

be_sum_str db 'SUM(', 0
be_sumif_str db 'SUMIF(', 0
be_count_str db 'COUNT(', 0
be_countif_str db 'COUNTIF(', 0
be_average_str db 'AVERAGE(', 0

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

be_function_cell_list_pointer dd 0
be_function_list_pointer dd 0
be_function_string times 100 dw 0

be_function_c1_line dd 1
be_function_c1_column dd 0
be_function_c2_line dd 1
be_function_c2_column dd 0
be_function_rule dd '='
be_function_rule_number dd 0
be_function_num_after_comma dd 0

be_function_c1_str dw 'A', '1', 0, 0, 0
be_function_c2_str dw 'A', '1', 0, 0, 0
be_function_cell_input_string times 5 dw 0
be_function_cell_number_input times 10 dw 0
be_function_cell_input_line dd 0
be_function_cell_input_column dd 0
 
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
  
  cmp byte [key_code], KEY_F1
  je .save_file
  cmp byte [key_code], KEY_F2
  je .open_file
  cmp byte [key_code], KEY_F3
  je .new_file
  
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
  
  cmp byte [key_code], KEY_F11
  je .function_dialog
  
  cmp byte [key_code], KEY_F12
  je .recalculate_functions
  
  cmp word [key_unicode], 0
  jne .draw_char
  
  cmp dword [usb_mouse_data], 0
  jne .mouse_event
 jmp .be_halt
 
 .save_file:
  mov dword [allocated_size], 6
  call allocate_memory
  cmp dword [allocated_memory_pointer], 0
  je .be_halt
  
  mov edi, dword [allocated_memory_pointer]
  push edi
  mov dword [file_memory], edi
  mov dword [edi], 'Bles'
  mov dword [edi+4], 'kOS ' ;signature
  add edi, 8
  mov esi, dword [table_editor_file_pointer]
  mov ecx, 26*999
  mov ebx, 0x00010000 ;line and column of actual processed cell
  mov edx, 8 ;length of file in bytes
  .convert_table_to_sd_file:
   cmp dword [esi], 0xFFFF0000
   jne .convert_cell_to_sd
   cmp dword [esi+4], 0x000000FF
   jne .convert_cell_to_sd
   cmp word [esi+8], 0
   je .skip_this_cell
   
   .convert_cell_to_sd:
   mov dword [edi], ebx ;save line and column of this cell
   add edi, 4
   push ecx
   mov ecx, 208
   rep movsb
   pop ecx
   add edx, 212

   jmp .next_cell
   
   .skip_this_cell:
   add esi, 208
  .next_cell:
  inc ebx ;next column
  cmp bx, 26
  jb .next_loop
  mov bx, 0
  add ebx, 0x00010000 ;next line
  .next_loop:
  loop .convert_table_to_sd_file
  
  mov eax, edx
  mov ebx, 1024
  mov edx, 0
  div ebx ;convert from bytes to KB
  inc eax
  mov dword [file_size], eax ;in KB
  mov dword [file_type], 'sd'
  call file_dialog_save
  
  pop edi
  mov dword [allocated_memory_pointer], edi
  mov dword [allocated_size], 6
  call release_memory
 jmp table_editor
 
 .open_file:
  mov dword [fd_file_type_1], 'sd'
  mov dword [fd_file_type_2], 'SD'
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je table_editor
  
  push dword [allocated_memory_pointer]
  push dword [allocated_size]
  mov esi, dword [allocated_memory_pointer]
  cmp dword [esi], 'Bles'
  jne .finish_sd_file
  cmp dword [esi+4], 'kOS '
  jne .finish_sd_file
  add esi, 8
  
  mov edi, dword [table_editor_file_pointer]
  mov ecx, 26*999
  .init_table_editor_cell:
   mov word [edi], 0x0000
   mov dword [edi+2], WHITE ;background
   mov dword [edi+5], BLACK ;text color
   mov word [edi+8], 0
   add edi, 208
  loop .init_table_editor_cell
  
  mov edi, dword [table_editor_file_pointer]
  .convert_cell_from_sd_to_table:
   cmp dword [esi], 0
   je .finish_sd_file
   
   mov eax, dword [esi]
   shr eax, 16
   dec eax
   mov ebx, 208*26
   mul ebx
   mov ecx, eax
   mov eax, dword [esi]
   and eax, 0xFFFF
   mov ebx, 208
   mul ebx
   add ecx, eax
   add ecx, dword [table_editor_file_pointer]
   
   add esi, 4
   mov edi, ecx
   mov ecx, 208
   rep movsb
  jmp .convert_cell_from_sd_to_table
   
  .finish_sd_file:
  pop dword [allocated_size]
  pop dword [allocated_memory_pointer]
  call release_memory
 jmp table_editor
 
 .new_file:
  mov esi, be_message_new_file_up
  mov edi, be_message_new_file_down
  call show_message_window
  cmp byte [key_code], KEY_ESC
  je table_editor
  
  mov edi, be_columns_length
  mov eax, 9
  mov ecx, 26
  rep stosb
  
  mov edi, be_lines_length
  mov eax, 1
  mov ecx, 1000
  rep stosb
  
  mov edi, dword [table_editor_file_pointer]
  mov ecx, 26*999
  .new_file_init_table_editor_cell:
   mov word [edi], 0x0000
   mov dword [edi+2], WHITE ;background
   mov dword [edi+5], BLACK ;text color
   mov word [edi+8], 0
   add edi, 208
  loop .new_file_init_table_editor_cell
  
  mov dword [be_first_show_line], 1
  mov dword [be_first_show_column], 0
  mov dword [be_selected_cell_line], 1
  mov dword [be_selected_cell_column], 0
  mov dword [be_cursor_offset], 0
 jmp table_editor
 
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
  cmp word [esi], 0xFFFF ;function
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
  cmp word [esi], 0xFFFF ;function
  je .be_halt
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
 
 .function_dialog:
  ;load actual cell function
  mov dword [be_function_list_pointer], 0
  call be_selected_cell_pointer
  cmp word [esi+8], 0xFFFF
  jne .function_if_cell_with_function
   mov ax, word [esi+10]
   and ax, 0xF
   mov word [be_function_list_pointer], ax
   
   ;load values from function
   cmp ax, 1
   je .function_with_cell_area
   cmp ax, 2
   je .function_with_cell_area
   cmp ax, 3
   je .function_with_cell_area
   cmp ax, 4
   je .function_with_cell_area
   cmp ax, 5
   je .function_with_cell_area
   jmp .function_if_cell_area
   .function_with_cell_area
    mov bx, word [esi+12]
    mov word [be_function_c1_line], bx
    mov bx, word [esi+14]
    mov word [be_function_c1_column], bx
    mov bx, word [esi+16]
    mov word [be_function_c2_line], bx
    mov bx, word [esi+18]
    mov word [be_function_c2_column], bx
   .function_if_cell_area:
   
   cmp ax, 2
   je .function_with_cell_rule
   cmp ax, 4
   je .function_with_cell_rule
   jmp .function_if_cell_rule
   .function_with_cell_rule:
    mov bx, word [esi+20]
    mov word [be_function_rule], bx
    mov ebx, dword [esi+22]
    mov dword [be_function_rule_number], ebx
   .function_if_cell_rule:
   
   cmp ax, 5
   jne .function_if_average
    mov bx, word [esi+20]
    mov word [be_function_num_after_comma], bx
   .function_if_average:
  .function_if_cell_with_function:
 
 .function: 
  ;draw background
  mov dword [message_window_length], 404
  mov dword [message_window_heigth], 204
  call show_empty_message_window
  
  ;draw list
  mov eax, dword [screen_y_center]
  sub eax, 100
  mov dword [cursor_line], eax
  push eax
  mov eax, dword [screen_x_center]
  sub eax, 200
  mov dword [cursor_column], eax
  push eax
  mov dword [square_length], 200
  mov dword [square_heigth], 201
  mov dword [color], WHITE
  call draw_square
  
  mov eax, dword [be_function_list_pointer]
  mov ebx, 10
  mul ebx
  add eax, dword [screen_y_center]
  sub eax, 100
  mov dword [cursor_line], eax
  mov eax, dword [screen_x_center]
  sub eax, 200
  mov dword [cursor_column], eax
  mov dword [square_length], 200
  mov dword [square_heigth], 10
  mov dword [color], 0xFF0000
  call draw_square
  
  mov dword [color], BLACK
  pop ebx
  pop eax
  inc eax
  push eax
  push ebx
  PRINT 'No function', no_func_str, eax, ebx
  pop ebx
  pop eax
  
  add eax, LINESZ
  push eax
  push ebx
  PRINT 'SUM', sum_function_str, eax, ebx
  pop ebx
  pop eax
  
  add eax, LINESZ
  push eax
  push ebx
  PRINT 'SUMIF', sumif_function_str, eax, ebx
  pop ebx
  pop eax
  
  add eax, LINESZ
  push eax
  push ebx
  PRINT 'COUNT', count_function_str, eax, ebx
  pop ebx
  pop eax
  
  add eax, LINESZ
  push eax
  push ebx
  PRINT 'COUNTIF', countif_function_str, eax, ebx
  pop ebx
  pop eax
  
  add eax, LINESZ
  push eax
  push ebx
  PRINT 'AVERAGE', average_function_str, eax, ebx
  pop ebx
  pop eax
  
  mov eax, dword [screen_y_center]
  add eax, 80
  mov ebx, dword [screen_x_center]
  add ebx, 10
  PRINT '[enter] Select function', enter_select_function_str, eax, ebx
  
  ;draw other stuff acording to function
  cmp dword [be_function_list_pointer], 0
  je .function_redraw
  
  cmp dword [be_function_list_pointer], 1 ;SUM
  je .function_draw_sum_count
  cmp dword [be_function_list_pointer], 3 ;COUNT
  je .function_draw_sum_count
  jmp .function_if_draw_sum
  .function_draw_sum_count:
   mov eax, dword [screen_y_center]
   sub eax, 90
   mov dword [cursor_line], eax
   mov eax, dword [screen_x_center]
   add eax, 10
   mov dword [cursor_column], eax
   mov dword [color], BLACK
   
   mov eax, dword [be_function_c1_column]
   add eax, 'A'
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c1_line]
   mov dword [var_print_value], eax
   call print_var
   mov dword [char_for_print], ':'
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c2_column]
   add eax, 'A'
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c2_line]
   mov dword [var_print_value], eax
   call print_var
   
   mov eax, dword [screen_y_center]
   sub eax, 80
   mov ebx, dword [screen_x_center]
   add ebx, 10
   PRINT '[a] Change first cell', first_cell_str, eax, ebx
   
   mov eax, dword [screen_y_center]
   sub eax, 70
   mov ebx, dword [screen_x_center]
   add ebx, 10
   PRINT '[b] Change second cell', second_cell_str, eax, ebx
  .function_if_draw_sum:
  
  cmp dword [be_function_list_pointer], 2 ;SUMIF
  je .function_draw_sumif_countif
  cmp dword [be_function_list_pointer], 4 ;COUNTIF
  je .function_draw_sumif_countif
  jmp .function_if_draw_sumif
  .function_draw_sumif_countif:
   mov eax, dword [screen_y_center]
   sub eax, 90
   mov dword [cursor_line], eax
   mov eax, dword [screen_x_center]
   add eax, 10
   mov dword [cursor_column], eax
   mov dword [color], BLACK
   
   mov eax, dword [be_function_c1_column]
   add eax, 'A'
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c1_line]
   mov dword [var_print_value], eax
   call print_var
   mov dword [char_for_print], ':'
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c2_column]
   add eax, 'A'
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c2_line]
   mov dword [var_print_value], eax
   call print_var
   
   mov eax, dword [screen_y_center]
   sub eax, 80
   mov ebx, dword [screen_x_center]
   add ebx, 10
   push eax
   push ebx
   PRINT 'Rule:', cell_rule_str, eax, ebx
   mov eax, dword [be_function_rule]
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_rule_number]
   mov dword [var_print_value], eax
   call print_var
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[a] Change first cell', first_cell_sumif_str, eax, ebx
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[b] Change second cell', second_cell_sumif_str, eax, ebx
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[c] Change rule', cell_rule_sumif_str, eax, ebx
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[d] Change rule number', cell_rule_number_sumif_str, eax, ebx
   pop ebx
   pop eax
  .function_if_draw_sumif:
  
  cmp dword [be_function_list_pointer], 5 ;AVERAGE
  jne .function_if_draw_average
   mov eax, dword [screen_y_center]
   sub eax, 90
   mov dword [cursor_line], eax
   mov eax, dword [screen_x_center]
   add eax, 10
   mov dword [cursor_column], eax
   mov dword [color], BLACK
   
   mov eax, dword [be_function_c1_column]
   add eax, 'A'
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c1_line]
   mov dword [var_print_value], eax
   call print_var
   mov dword [char_for_print], ':'
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c2_column]
   add eax, 'A'
   mov dword [char_for_print], eax
   call print_char
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [be_function_c2_line]
   mov dword [var_print_value], eax
   call print_var
   
   mov eax, dword [screen_y_center]
   sub eax, 80
   mov ebx, dword [screen_x_center]
   add ebx, 10
   push eax
   push ebx
   PRINT 'Numbers after comma: ', numbers_after_comma_str, eax, ebx
   mov eax, dword [be_function_num_after_comma]
   mov dword [var_print_value], eax
   call print_var
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[a] Change first cell', first_cell_average_str, eax, ebx
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[b] Change second cell', second_cell_average_str, eax, ebx
   pop ebx
   pop eax
   
   add eax, LINESZ
   push eax
   push ebx
   PRINT '[c] Change numbers', num_after_comma_average_str, eax, ebx
   pop ebx
   pop eax
  .function_if_draw_average:
  
  .function_redraw:
  call redraw_screen
  
  .function_halt:
   call wait_for_keyboard
   
   cmp byte [key_code], KEY_ESC
   je table_editor
   
   cmp byte [key_code], KEY_UP
   jne .function_if_key_up
    cmp dword [be_function_list_pointer], 0
    je .function_halt
    
    dec dword [be_function_list_pointer]
    
    jmp .function
   .function_if_key_up:
   
   cmp byte [key_code], KEY_DOWN
   jne .function_if_key_down
    cmp dword [be_function_list_pointer], 5
    je .function_halt
    
    inc dword [be_function_list_pointer]
    
    jmp .function
   .function_if_key_down:
   
   ;these keys are for all functions with area
   cmp dword [be_function_list_pointer], 1
   jb .if_function_sum
   cmp dword [be_function_list_pointer], 5
   ja .if_function_sum
    cmp byte [key_code], KEY_A
    jne .if_func_sum_key_a
     mov esi, be_function_c1_str
     mov edi, be_function_cell_input_string
     mov ecx, 5
     rep movsw
    
     call be_function_input_cell
     cmp byte [key_code], KEY_ESC
     je .function
     
     mov eax, dword [be_function_cell_input_line]
     mov dword [be_function_c1_line], eax
     mov eax, dword [be_function_cell_input_column]
     mov dword [be_function_c1_column], eax
     
     mov esi, be_function_cell_input_string
     mov edi, be_function_c1_str
     mov ecx, 5
     rep movsw
     jmp .function
    .if_func_sum_key_a:
    
    cmp byte [key_code], KEY_B
    jne .if_func_sum_key_b
     mov esi, be_function_c2_str
     mov edi, be_function_cell_input_string
     mov ecx, 5
     rep movsw
    
     call be_function_input_cell
     cmp byte [key_code], KEY_ESC
     je .function
     
     mov eax, dword [be_function_cell_input_line]
     mov dword [be_function_c2_line], eax
     mov eax, dword [be_function_cell_input_column]
     mov dword [be_function_c2_column], eax
     
     mov esi, be_function_cell_input_string
     mov edi, be_function_c2_str
     mov ecx, 5
     rep movsw
     jmp .function
    .if_func_sum_key_b:
   .if_function_sum:
   
   cmp dword [be_function_list_pointer], 2
   je .if_function_sumif_countif_keyboard
   cmp dword [be_function_list_pointer], 4
   je .if_function_sumif_countif_keyboard
   jmp .if_not_function_sumif_countif_keyboard
   .if_function_sumif_countif_keyboard:
    cmp byte [key_code], KEY_C
    jne .if_func_sumif_key_c
     .text_input_for_rule_sumif:
     mov dword [be_function_cell_input_string], 0
     
     mov eax, dword [screen_y_center]
     sub eax, 93
     mov dword [cursor_line], eax
     mov eax, dword [screen_x_center]
     add eax, 180
     mov dword [cursor_column], eax
     mov dword [text_input_pointer], be_function_cell_input_string
     mov dword [text_input_length], 1
     call text_input
     cmp byte [key_code], KEY_ESC
     je .function
     cmp word [be_function_cell_input_string], '='
     je .change_rule_sumif
     cmp word [be_function_cell_input_string], '<'
     je .change_rule_sumif
     cmp word [be_function_cell_input_string], '>'
     je .change_rule_sumif
     jmp .text_input_for_rule_sumif
     .change_rule_sumif:
     mov eax, 0
     mov ax, word [be_function_cell_input_string]
     mov dword [be_function_rule], eax
     jmp .function
    .if_func_sumif_key_c:
    
    cmp byte [key_code], KEY_D
    jne .if_func_sumif_key_d
     mov edi, be_function_cell_number_input
     mov eax, 0
     mov ecx, 10
     rep stosw
    
     mov eax, dword [screen_y_center]
     sub eax, 93
     mov dword [cursor_line], eax
     mov eax, dword [screen_x_center]
     add eax, 120
     mov dword [cursor_column], eax
     mov dword [text_input_pointer], be_function_cell_number_input
     mov dword [text_input_length], 9
     call number_input
     mov eax, dword [number_input_return]
     mov dword [be_function_rule_number], eax
     jmp .function
    .if_func_sumif_key_d:
   .if_not_function_sumif_countif_keyboard
   
   cmp dword [be_function_list_pointer], 5
   jne .if_function_average_keyboard
    cmp byte [key_code], KEY_C
    jne .if_func_average_key_c
     mov dword [be_function_cell_number_input], 0
     
     mov eax, dword [screen_y_center]
     sub eax, 93
     mov dword [cursor_line], eax
     mov eax, dword [screen_x_center]
     add eax, 180
     mov dword [cursor_column], eax
     mov dword [text_input_pointer], be_function_cell_number_input
     mov dword [text_input_length], 1
     call number_input
     cmp byte [key_code], KEY_ESC
     je .function
     mov eax, dword [number_input_return]
     mov dword [be_function_num_after_comma], eax
     jmp .function
    .if_func_average_key_c:
   .if_function_average_keyboard:
   
   cmp byte [key_code], KEY_ENTER
   jne .function_if_key_enter
    cmp dword [be_function_list_pointer], 0 ;no function - delete everything from cell
    jne .function_if_no_function
     call be_selected_cell_pointer
     
     mov edi, esi
     add edi, 8
     mov eax, 0
     mov ecx, 200
     rep stosb
     jmp table_editor
    .function_if_no_function:
    
    mov edi, be_function_string
    mov eax, 0
    mov ecx, 100
    rep stosw
    
    cmp dword [be_function_list_pointer], 1 ;SUM function
    jne .function_if_sum_function
     ;test if cell range is valid
     mov ax, word [be_function_c1_line]
     cmp ax, word [be_function_c2_line]
     ja .invalid_cell_range
     mov ax, word [be_function_c1_column]
     cmp ax, word [be_function_c2_column]
     ja .invalid_cell_range

     mov word [be_function_string], 0xFFFF
     mov word [be_function_string+2], 1
     mov eax, dword [be_function_c1_line]
     mov word [be_function_string+4], ax
     mov eax, dword [be_function_c1_column]
     mov word [be_function_string+6], ax
     mov eax, dword [be_function_c2_line]
     mov word [be_function_string+8], ax
     mov eax, dword [be_function_c2_column]
     mov word [be_function_string+10], ax
    .function_if_sum_function:
    
    cmp dword [be_function_list_pointer], 2 ;SUMIF function
    jne .function_if_sumif_function
     ;test if cell range is valid
     mov ax, word [be_function_c1_line]
     cmp ax, word [be_function_c2_line]
     ja .invalid_cell_range
     mov ax, word [be_function_c1_column]
     cmp ax, word [be_function_c2_column]
     ja .invalid_cell_range

     mov word [be_function_string], 0xFFFF
     mov word [be_function_string+2], 2
     mov eax, dword [be_function_c1_line]
     mov word [be_function_string+4], ax
     mov eax, dword [be_function_c1_column]
     mov word [be_function_string+6], ax
     mov eax, dword [be_function_c2_line]
     mov word [be_function_string+8], ax
     mov eax, dword [be_function_c2_column]
     mov word [be_function_string+10], ax
     mov eax, dword [be_function_rule]
     mov word [be_function_string+12], ax
     mov eax, dword [be_function_rule_number]
     mov dword [be_function_string+14], eax
    .function_if_sumif_function:
    
    cmp dword [be_function_list_pointer], 3 ;COUNT function
    jne .function_if_count_function
     ;test if cell range is valid
     mov ax, word [be_function_c1_line]
     cmp ax, word [be_function_c2_line]
     ja .invalid_cell_range
     mov ax, word [be_function_c1_column]
     cmp ax, word [be_function_c2_column]
     ja .invalid_cell_range

     mov word [be_function_string], 0xFFFF
     mov word [be_function_string+2], 3
     mov eax, dword [be_function_c1_line]
     mov word [be_function_string+4], ax
     mov eax, dword [be_function_c1_column]
     mov word [be_function_string+6], ax
     mov eax, dword [be_function_c2_line]
     mov word [be_function_string+8], ax
     mov eax, dword [be_function_c2_column]
     mov word [be_function_string+10], ax
    .function_if_count_function:
    
    cmp dword [be_function_list_pointer], 4 ;COUNTIF function
    jne .function_if_countif_function
     ;test if cell range is valid
     mov ax, word [be_function_c1_line]
     cmp ax, word [be_function_c2_line]
     ja .invalid_cell_range
     mov ax, word [be_function_c1_column]
     cmp ax, word [be_function_c2_column]
     ja .invalid_cell_range

     mov word [be_function_string], 0xFFFF
     mov word [be_function_string+2], 4
     mov eax, dword [be_function_c1_line]
     mov word [be_function_string+4], ax
     mov eax, dword [be_function_c1_column]
     mov word [be_function_string+6], ax
     mov eax, dword [be_function_c2_line]
     mov word [be_function_string+8], ax
     mov eax, dword [be_function_c2_column]
     mov word [be_function_string+10], ax
     mov eax, dword [be_function_rule]
     mov word [be_function_string+12], ax
     mov eax, dword [be_function_rule_number]
     mov dword [be_function_string+14], eax
    .function_if_countif_function:
    
    cmp dword [be_function_list_pointer], 5 ;AVERAGE function
    jne .function_if_average_function
     ;test if cell range is valid
     mov ax, word [be_function_c1_line]
     cmp ax, word [be_function_c2_line]
     ja .invalid_cell_range
     mov ax, word [be_function_c1_column]
     cmp ax, word [be_function_c2_column]
     ja .invalid_cell_range

     mov word [be_function_string], 0xFFFF
     mov word [be_function_string+2], 5
     mov eax, dword [be_function_c1_line]
     mov word [be_function_string+4], ax
     mov eax, dword [be_function_c1_column]
     mov word [be_function_string+6], ax
     mov eax, dword [be_function_c2_line]
     mov word [be_function_string+8], ax
     mov eax, dword [be_function_c2_column]
     mov word [be_function_string+10], ax
     mov eax, dword [be_function_num_after_comma]
     mov word [be_function_string+12], ax
    .function_if_average_function:
    
    ;copy function and recalculate its value
    call be_selected_cell_pointer
    mov edi, esi
    add edi, 8
    push edi
    mov eax, 0
    mov ecx, 200 ;clear cell content
    rep stosb
    pop edi
    mov esi, be_function_string
    mov ecx, 100
    rep movsw ;copy function
    
    push dword [be_selected_cell_line]
    push dword [be_selected_cell_column]
    call be_recalculate_cell
    pop dword [be_selected_cell_column]
    pop dword [be_selected_cell_line]
    
    jmp table_editor
   .function_if_key_enter:
  jmp .function_halt
  
  .invalid_cell_range:
   mov dword [message_window_length], 250
   mov dword [message_window_heigth], COLUMNSZ*3+2
   call show_empty_message_window
   mov esi, be_message_invalid_cell_range
   mov eax, dword [screen_y_center]
   sub eax, 4
   mov dword [cursor_line], eax
   call print_to_message_window
   call redraw_screen
   WAIT 1000
  jmp .function
 
 .recalculate_functions:
  mov dword [message_window_length], 266
  mov dword [message_window_heigth], COLUMNSZ*3+2
  call show_empty_message_window
  mov esi, be_message_recalculating
  mov eax, dword [screen_y_center]
  sub eax, 4
  mov dword [cursor_line], eax
  call print_to_message_window
  call redraw_screen
 
  push dword [be_selected_cell_line]
  push dword [be_selected_cell_column]
  
  mov dword [be_selected_cell_line], 1
  mov dword [be_selected_cell_column], 0
  call be_selected_cell_pointer
  mov ecx, 999
  .recalculate_line:
  push ecx
   mov dword [be_selected_cell_column], 0
  
   mov ecx, 26
   .recalculate_column:
   push ecx
    cmp word [esi+8], 0xFFFF
    jne .if_cell_function
     push esi
     call be_recalculate_cell
     pop esi
    .if_cell_function
    add esi, 208
    inc dword [be_selected_cell_column]
   pop ecx
   loop .recalculate_column
   
   inc dword [be_selected_cell_line]
  pop ecx
  loop .recalculate_line
  
  pop dword [be_selected_cell_column]
  pop dword [be_selected_cell_line]
 jmp table_editor
 
 .draw_char:
  cmp byte [be_cursor_offset], 40
  je .be_halt
  call be_selected_cell_pointer
  cmp word [esi+8], 0xFFFF ;function
  je .be_halt
  
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
  
  SCREEN_X_SUB eax, 21+(13*18)
  TEST_CLICK_ZONE_WITH_JUMP click_cell_input, be_mouse_line, be_mouse_column, 21, 21+12, 27, eax, .select_text_cursor_by_mouse
 
  SCREEN_X_SUB eax, 20+(13*17)
  mov ebx, eax
  add ebx, 24
  TEST_CLICK_ZONE_WITH_JUMP click_zone_bg_color, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_background_color
  SCREEN_X_SUB eax, 19+(13*15)
  mov ebx, eax
  add ebx, 24
  TEST_CLICK_ZONE_WITH_JUMP click_zone_tx_color, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_text_color
  
  SCREEN_X_SUB eax, 17+(13*13)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_bold, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_bold
  
  SCREEN_X_SUB eax, 14+(13*12)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_line_up_aligment, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_line_up_aligment
  SCREEN_X_SUB eax, 13+(13*11)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_line_middle_aligment, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_line_middle_aligment
  SCREEN_X_SUB eax, 12+(13*10)
  mov ebx, eax
  add ebx, 12
  TEST_CLICK_ZONE_WITH_JUMP click_zone_line_down_aligment, be_mouse_line, be_mouse_column, 21, 21+12, eax, ebx, .change_line_down_aligment
  
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
 
 .select_text_cursor_by_mouse:
  mov eax, dword [be_mouse_column]
  sub eax, 25
  shr eax, 3 ;div 8
  mov dword [be_cursor_offset], eax
  
  push eax
  call be_selected_cell_pointer
  add esi, 8
  pop eax
  push esi
  shl eax, 1
  add esi, eax
  cmp word [esi], 0
  pop esi
  jne table_editor
  
  ;move cursor to end of text
  mov eax, 0
  .select_text_cursor_by_mouse_find_cursor:
   cmp word [esi], 0
   je .select_text_cursor_by_mouse_cursor_founded
   add esi, 2
   inc eax
  jmp .select_text_cursor_by_mouse_find_cursor
  .select_text_cursor_by_mouse_cursor_founded:
  mov dword [be_cursor_offset], eax
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
 
 .change_line_up_aligment:
  call be_selected_cell_pointer
  and byte [esi+1], 0xFC
 jmp table_editor
 
 .change_line_middle_aligment:
  call be_selected_cell_pointer
  and byte [esi+1], 0xFC
  or byte [esi+1], 0b01
 jmp table_editor
 
 .change_line_down_aligment:
  call be_selected_cell_pointer
  and byte [esi+1], 0xFC
  or byte [esi+1], 0b10
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

%include "source/programs/table_editor/draw_methods.asm"

%include "source/programs/table_editor/methods_of_functions.asm"
