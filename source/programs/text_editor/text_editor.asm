;BleskOS

text_editor_up_str db 'Text editor', 0
text_editor_down_str db '[F1] Save file [F2] Open file', 0
te_line_str db 'Line:', 0

te_max_column dd 0
te_max_line dd 0
te_column dd 0
te_line dd 0
te_draw_column dd 0
te_draw_line dd 0

text_editor_mem dd 0
text_editor_end_mem dd 0
text_editor_first_line_mem dd 0
te_pointer dd 0
te_pointer_end dd 0
te_cursor_offset dd 0
te_length_of_text dd 0

text_editor_files_mem times 4*10 dd 0
 
text_editor: 
 call te_draw_text
 call redraw_screen
 
 .te_halt:
  call wait_for_usb_keyboard
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F1
  je .save_file
  
  cmp byte [key_code], KEY_F2
  je .open_file
  
  cmp byte [key_code], KEY_BACKSPACE
  je .key_backspace
  
  cmp byte [key_code], KEY_DELETE
  je .key_delete
  
  cmp byte [key_code], KEY_LEFT
  je .key_left
  
  cmp byte [key_code], KEY_RIGHT
  je .key_right
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp word [key_unicode], 0
  jne .draw_char
 jmp .te_halt
 
 .save_file:
  mov eax, dword [text_editor_mem]
  mov dword [file_memory], eax
  mov dword [file_size], 100 ;in KB
  mov dword [file_type], 'TXT'
  call file_dialog_save
 jmp text_editor
 
 .open_file:
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je text_editor
  cmp dword [file_type], 'BTXT'
  je .bleskos_txt
  
  call text_editor_convert_to_unicode
  jmp text_editor
  
  .bleskos_txt:
  mov eax, dword [file_memory]
  mov esi, eax
  mov dword [text_editor_mem], eax
  mov dword [text_editor_first_line_mem], eax
  mov dword [te_pointer], eax
  mov dword [te_pointer_end], eax
  mov dword [te_length_of_text], 0
  mov dword [te_cursor_offset], 0
  mov dword [te_draw_line], 0
  mov dword [te_draw_column], 0
  
  add eax, 0x100000
  mov dword [text_editor_end_mem], eax
  
  mov ecx, 0x100000
  .find_end_of_file:
   cmp word [esi], 0
   je text_editor
   add esi, 2
   inc dword [te_length_of_text]
   add dword [te_pointer_end], 2
  loop .find_end_of_file

 jmp text_editor
  
 .key_backspace:
  cmp dword [te_cursor_offset], 0
  je .te_halt
  
  call text_editor_found_cursor_position
  mov eax, dword [te_draw_line]
  add eax, dword [te_draw_column]
  cmp eax, 0
  jne .backspace
  
  call te_move_first_line_up
  
  .backspace:
  sub dword [te_pointer_end], 2
  sub dword [te_pointer], 2
  dec dword [te_cursor_offset]
  dec dword [te_column]
  dec dword [te_length_of_text]
  
  mov edi, dword [te_pointer]
  mov esi, edi
  add esi, 2
  mov ecx, dword [te_length_of_text]
  mov edx, dword [te_cursor_offset]
  sub ecx, edx
  inc ecx
  cld
  rep movsw ;move all text
 jmp .te_redraw_screen
 
 .key_delete:
  mov eax, dword [te_pointer_end]
  cmp dword [te_pointer], eax
  je .te_halt
  
  sub dword [te_pointer_end], 2
  dec dword [te_length_of_text]
 
  mov edi, dword [te_pointer]
  mov esi, edi
  add esi, 2
  mov ecx, dword [te_length_of_text]
  mov edx, dword [te_cursor_offset]
  sub ecx, edx
  inc ecx
  cld
  rep movsw ;move all text
 jmp .te_redraw_screen
 
 .key_left:
  cmp dword [te_cursor_offset], 0
  je .te_halt
  
  call text_editor_found_cursor_position
  mov eax, dword [te_draw_line]
  add eax, dword [te_draw_column]
  cmp eax, 0
  jne .move_cursor_left

  call te_move_first_line_up

  .move_cursor_left:
  sub dword [te_pointer], 2
  dec dword [te_cursor_offset]
  dec dword [te_column]
  dec dword [te_draw_column]
  
  mov eax, dword [te_pointer]
  cmp word [eax], 0xA
  jne .if_line_left
   dec dword [te_line]
   dec dword [te_draw_line] 
  .if_line_left:
 jmp .te_redraw_screen
 
 .key_right:
  mov eax, dword [te_pointer]
  cmp eax, dword [text_editor_end_mem]
  je .te_halt
  
  mov eax, dword [te_pointer]
  cmp word [eax], 0
  je .te_halt
  
  call text_editor_found_cursor_position
  mov eax, dword [te_max_line]
  cmp eax, dword [te_line]
  jne .move_cursor_right
  mov eax, dword [te_pointer]
  cmp word [eax], 0xA
  jne .move_cursor_right

  call te_move_first_line_down

  .move_cursor_right:
  add dword [te_pointer], 2
  inc dword [te_cursor_offset]
  inc dword [te_column]
  inc dword [te_draw_column]
  
  mov eax, dword [te_pointer]
  sub eax, 2
  cmp word [eax], 0xA
  jne .if_line_right
   inc dword [te_line] 
   inc dword [te_draw_column]
  .if_line_right:
 jmp .te_redraw_screen
 
 .key_up:
  cmp dword [te_cursor_offset], 0
  je .te_halt
  
  mov esi, dword [te_pointer]
  
  call text_editor_found_cursor_position
  cmp dword [te_draw_line], 0
  jne .move_cursor_up
  
  call te_move_first_line_up
  
  .move_cursor_up:
   sub dword [te_pointer], 2
   sub esi, 2
   dec dword [te_cursor_offset]
   cmp dword [te_cursor_offset], 0
   je .te_redraw_screen
   cmp word [esi], 0xA ;new line char
   je .te_redraw_screen
  jmp .move_cursor_up
 jmp .te_redraw_screen

 .key_down:
  mov eax, dword [text_editor_end_mem]
  cmp dword [te_pointer], eax
  je .te_halt
  
  mov esi, dword [te_pointer]
  cmp word [esi], 0
  je .te_halt  
  
  call text_editor_found_cursor_position
  mov eax, dword [te_max_line]
  dec eax
  cmp dword [te_draw_line], eax
  jne .move_cursor_down
  
  call te_move_first_line_down
  
  .move_cursor_down:
   add dword [te_pointer], 2
   add esi, 2
   inc dword [te_cursor_offset]
   cmp word [esi], 0
   je .te_redraw_screen
   cmp word [esi], 0xA ;new line char
   je .te_redraw_screen
  jmp .move_cursor_down
 jmp .te_redraw_screen
 
 .draw_char:
  call text_editor_found_cursor_position
  mov eax, dword [te_max_column]
  dec eax
  cmp dword [te_column], eax
  je .te_halt ;if we are on last column, do not write more
  
  mov eax, dword [te_pointer]
  
  cmp eax, dword [text_editor_end_mem]
  je .te_redraw_screen

  ;test if we are not on the end of text
  cmp word [eax], 0
  jne .if_end_of_text
   mov word [eax+2], 0 ;if yes, insert zero after new char
   jmp .insert_char
  .if_end_of_text:

  ;move all chars
  mov edi, dword [te_pointer_end]
  mov esi, dword [te_pointer_end]
  sub esi, 2
  mov ecx, dword [te_length_of_text]
  mov edx, dword [te_cursor_offset]
  sub ecx, edx
  std
  rep movsw ;move all text
  cld
  
  .insert_char:
  mov eax, dword [te_pointer]
  mov bx, word [key_unicode]
  mov word [eax], bx
  add dword [te_pointer], 2
  add dword [te_pointer_end], 2
  inc dword [te_cursor_offset]
  inc dword [te_length_of_text]
  
  ;TODO: if we are on corner, move screen
  
  cmp word [key_unicode], 0xA
  jne .if_enter
   inc dword [te_line]
   mov dword [te_column], 0
   
   call text_editor_found_cursor_position
   mov eax, dword [te_max_line]
   cmp dword [te_draw_line], eax
   jne .te_redraw_screen ;if we are not on the last line, only redraw screen
   call te_move_first_line_down ;if we are on last line, move screen
   jmp .te_redraw_screen
  .if_enter:
  
  inc dword [te_column]
 jmp .te_redraw_screen
 
 .te_redraw_screen:
  call te_draw_text
  call redraw_screen
 jmp .te_halt

te_draw_text:
 push dword [te_draw_line]
 push dword [te_draw_column]
 DRAW_WINDOW text_editor_up_str, text_editor_down_str, 0xFF0000, WHITE
 
 ;print number of line
 mov dword [cursor_line], 5
 mov dword [color], BLACK
 mov esi, te_line_str
 SCREEN_X_SUB eax, COLUMNSZ*8
 mov dword [cursor_column], eax
 cmp dword [te_line], 10
 jl .print_line_num
 SCREEN_X_SUB eax, COLUMNSZ*9
 mov dword [cursor_column], eax
 cmp dword [te_line], 100
 jl .print_line_num
 SCREEN_X_SUB eax, COLUMNSZ*10
 mov dword [cursor_column], eax
 cmp dword [te_line], 1000
 jl .print_line_num
 .print_line_num:
 call print
 mov eax, dword [te_line]
 mov dword [var_print_value], eax
 add dword [cursor_column], COLUMNSZ
 call print_var
 
 ;set variabiles
 mov esi, dword [text_editor_first_line_mem]
 mov dword [te_draw_column], 0
 mov dword [te_draw_line], 0
 mov dword [cursor_line], 21
 mov dword [cursor_column], 0
 mov dword [color], BLACK
 mov ebp, 0 ;if is drawing on the end of column
 
 .draw_char:
  cmp word [esi], 0
  je .test_cursor
  
  ;test if we are not on end of screen
  mov eax, dword [te_max_line]
  cmp dword [te_draw_line], eax
  jne .print_char
  mov eax, dword [te_max_column]
  cmp dword [te_draw_column], eax
  je .done
  
  .print_char:
  ;draw cursor if we are on his position
  cmp esi, dword [te_pointer]
  jne .if_draw_cursor
   dec dword [cursor_line]
   cmp dword [te_draw_column], 0
   je .if_not_first_column
    ;dec dword [cursor_column]
   .if_not_first_column:
   
   mov dword [column_heigth], LINESZ
   mov dword [color], 0x4E4E4E ;gray
   push esi
   call draw_column
   pop esi
   
   inc dword [cursor_line]
   cmp dword [te_draw_column], 0
   je .if_not_first_column_2
    ;inc dword [cursor_column]
   .if_not_first_column_2:
   
   mov dword [color], BLACK
  .if_draw_cursor:
  
  ;get number of char
  mov ax, word [esi]
  
  cmp ax, 0xA ;enter
  je .enter
  
  ;draw char on screen
  cmp ebp, 1
  je .not_draw_char
  mov dword [char_for_print], 0
  mov word [char_for_print], ax
  call print_char
  .not_draw_char:
  
  ;move draw position
  inc dword [te_draw_column]
  mov eax, dword [te_draw_column]
  cmp eax, dword [te_max_column]
  jl .next_column
  
  mov ebp, 1 ;this line has more chars than monitor columns, so disable drawing chars
  jmp .next_char
  
  ;move draw position to next line
  .next_line:
  mov ebp, 0 ;on next line enable drawing chars
  mov dword [cursor_column], 0
  inc dword [te_draw_line]
  mov dword [te_draw_column], 0
  ;test if we are on end of screen
  mov eax, dword [te_max_line]
  cmp dword [te_draw_line], eax
  jg .done
  add dword [cursor_line], LINESZ ;move on next line
  jmp .next_char
 
  ;move draw position to next column
  .next_column:
  add dword [cursor_column], COLUMNSZ
  jmp .next_char
  
  .enter:
  mov eax, dword [te_max_line]
  cmp dword [te_draw_line], eax
  je .done ;we are on end of screen
  jmp .next_line ;otherwise we can move on next line
  
 .next_char:
 add esi, 2
 jmp .draw_char
 
 .test_cursor:
 cmp esi, dword [te_pointer]
 jne .if_draw_cursor_2
  dec dword [cursor_line]
  mov dword [column_heigth], LINESZ
  mov dword [color], 0x4E4E4E ;gray
  call draw_column
 .if_draw_cursor_2:
 
 .done:
 pop dword [te_draw_column]
 pop dword [te_draw_line]
 ret

te_move_first_line_up:
 mov eax, 0
 mov edi, dword [text_editor_first_line_mem]
  
 .move:
  cmp edi, dword [text_editor_mem]
  je .done
  sub edi, 2
  cmp word [edi], 0xA
  jne .move
  
  inc eax
  cmp eax, 2
  jl .move
  add edi, 2
 
 .done:
 mov dword [text_editor_first_line_mem], edi
 inc dword [te_line]
 ret

te_move_first_line_down:
 mov edi, dword [text_editor_first_line_mem]
  
 .move:
  cmp edi, dword [te_pointer_end]
  je .done
  add edi, 2
  cmp word [edi], 0xA
  jne .move
  add edi, 2
 
 .done:
 mov dword [text_editor_first_line_mem], edi
 dec dword [te_line]
 ret
 
text_editor_found_cursor_position:
 mov ebx, 0
 mov ecx, 0
 mov eax, dword [text_editor_first_line_mem]
 .move:
  cmp eax, dword [te_pointer]
  je .done
  cmp word [eax], 0
  je .done
  cmp word [eax], 0xA
  jne .if_enter
   inc ebx
   mov ecx, 0
  .if_enter:
  add eax, 2
  inc ecx
 jmp .move
 
 .done:
 mov dword [te_draw_line], ebx
 mov dword [te_draw_column], ecx
 ret

text_editor_convert_to_unicode:
 push dword [allocated_memory_pointer]
 push dword [allocated_size]
 mov edi, dword [allocated_memory_pointer]
 push edi

 mov dword [allocated_size], 1
 call allocate_memory
 cmp dword [allocated_memory_pointer], 0
 je .not_enough_memory
 mov edi, dword [allocated_memory_pointer]
 mov eax, 0
 mov ecx, 0x100000
 rep stosb ;clear memory
 
 ;set variabiles
 mov eax, dword [allocated_memory_pointer]
 mov dword [text_editor_mem], eax
 mov dword [text_editor_first_line_mem], eax
 mov dword [te_pointer], eax
 mov dword [te_pointer_end], eax
 mov dword [te_cursor_offset], 0
 mov dword [te_length_of_text], 0
 mov dword [te_draw_line], 0
 mov dword [te_draw_column], 0
 add eax, 0x100000
 mov dword [text_editor_end_mem], eax
 
 pop edi
 mov esi, dword [allocated_memory_pointer]
 .convert_char:
  cmp word [edi], 0
  je .done
  
  mov eax, 0
  mov al, byte [edi]
  test al, 0x80
  jz .ascii_char
  
  and al, 0xE0
  cmp al, 0xC0
  je .utf_2_byte
  
  jmp .unicode
  
  .ascii_char:
   mov word [esi], ax
   add esi, 2
   inc edi
   add dword [te_pointer_end], 2
   inc dword [te_length_of_text]
  jmp .convert_char
  
  .utf_2_byte:
   mov al, byte [edi]
   mov bx, 0
   mov bl, byte [edi+1]
   and al, 0x1F
   shl ax, 6
   and bl, 0x3F
   or ax, bx
   mov word [esi], ax
   add esi, 2
   add edi, 2
   add dword [te_pointer_end], 2
   inc dword [te_length_of_text]
  jmp .convert_char
  
  .unicode:
   mov ax, word [edi]
   mov word [esi], ax
   add esi, 2
   add edi, 2
   add dword [te_pointer_end], 2
   inc dword [te_length_of_text]
  jmp .convert_char
 
 .done:
 ;release memory with uncoded file
 pop dword [allocated_size]
 pop dword [allocated_memory_pointer]
 call release_memory
 ret
 
 .not_enough_memory:
 pop eax
 pop eax
 pop eax
 ret
