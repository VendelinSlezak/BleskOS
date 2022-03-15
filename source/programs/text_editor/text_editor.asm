;BleskOS

text_editor_up_str db 'Text editor', 0
text_editor_down_str db '[F1] Save file [F2] Open file [F3] New file', 0
te_line_str db 'Line:', 0
te_message_new_file_up db 'Are you sure you want to erase actual text?', 0
te_message_new_file_down db '[enter] Yes [esc] Cancel', 0

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

te_mouse_line dd 20
te_mouse_column dd 0
te_mouse_cursor_text_pointer dd 0
te_mouse_cursor_first_pointer dd 0
te_mouse_cursor_second_pointer dd 0

text_editor_file_pointer dd 0
 
text_editor: 
 call te_draw_text
 call redraw_screen
 
 mov dword [mcursor_up_side], 20
 mov dword [mcursor_left_side], 0
 SCREEN_Y_SUB eax, 20
 mov dword [mcursor_down_side], eax
 mov eax, dword [screen_x]
 mov dword [mcursor_right_side], eax
 
 .te_halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F1
  je .save_file
  
  cmp byte [key_code], KEY_F2
  je .open_file
  
  cmp byte [key_code], KEY_F3
  je .new_file
  
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
  
  cmp byte [keyboard_ctrl], 1
  je .key_ctrl
  
  cmp byte [usb_mouse_data], 0
  jne .mouse_event
  
  .test_if_add_char:
  cmp word [key_unicode], 0
  jne .draw_char
 jmp .te_halt
 
 .save_file:
  mov eax, dword [text_editor_mem]
  mov dword [allocated_memory_pointer], eax
  call text_editor_convert_to_utf8
  
  mov eax, dword [allocated_memory_pointer]
  mov dword [file_memory], eax
  mov ebx, 0
  mov ecx, 0x100000
  .count_file_size:
   cmp byte [eax], 0
   je .end_of_file
   inc ebx
   inc eax
  loop .count_file_size
  .end_of_file:
  mov eax, ebx
  mov ebx, 1024
  mov edx, 0
  div ebx ;convert from bytes to KB
  inc eax
  mov dword [file_size], eax ;in KB
  mov dword [file_type], 'txt'
  call file_dialog_save
 jmp text_editor
 
 .open_file:
  mov dword [fd_file_type_1], 'txt'
  mov dword [fd_file_type_2], 'TXT'
  mov dword [fd_file_type_3], 'htm'
  mov dword [fd_file_type_4], 'HTM'
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je text_editor
  
  ;release memory of previous file
  cmp dword [text_editor_file_pointer], 0
  je .if_loaded_file
   push dword [allocated_memory_pointer]
   push dword [allocated_size]
   mov eax, dword [text_editor_file_pointer]
   mov dword [allocated_memory_pointer], eax
   mov dword [allocated_size], 1
   call release_memory
   pop dword [allocated_size]
   pop dword [allocated_memory_pointer]
  .if_loaded_file:
  
  call text_editor_convert_to_unicode
 jmp text_editor
 
 .new_file:
  mov esi, te_message_new_file_up
  mov edi, te_message_new_file_down
  call show_message_window
  cmp byte [key_code], KEY_ESC
  je .cancel
  
  ;release actual memory
  mov eax, dword [text_editor_file_pointer]
  mov dword [allocated_memory_pointer], eax
  mov dword [allocated_size], 1
  call release_memory

  ;create new file
  mov dword [allocated_size], 1
  call allocate_memory
  mov edi, dword [allocated_memory_pointer]
  mov ecx, 0x100000
  mov eax, 0
  rep stosb ;clean memory

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
  
  .cancel:
  mov byte [key_code], 0
 jmp .te_redraw_screen
  
 .key_backspace:
  mov eax, dword [te_mouse_cursor_first_pointer]
  cmp eax, dword [te_mouse_cursor_second_pointer]
  jne .delete_selected_text
  
  cmp dword [te_cursor_offset], 0
  je .te_halt
  
  call text_editor_found_cursor_position
  mov eax, dword [te_draw_line]
  add eax, dword [te_draw_column]
  cmp eax, 0
  jne .backspace
  
  call te_move_first_line_up
  
  .backspace:
  mov dword [te_mouse_cursor_first_pointer], 0
  mov dword [te_mouse_cursor_second_pointer], 0
  
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
  mov eax, dword [te_mouse_cursor_first_pointer]
  cmp eax, dword [te_mouse_cursor_second_pointer]
  jne .delete_selected_text
  
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
 .delete_selected_text:
  cmp eax, dword [te_mouse_cursor_second_pointer]
  jb .delete_if_second_below
   mov ebx, dword [te_mouse_cursor_second_pointer]
   mov dword [te_mouse_cursor_first_pointer], ebx
   mov dword [te_mouse_cursor_second_pointer], eax
  .delete_if_second_below:
  
  mov eax, dword [te_mouse_cursor_first_pointer]
  mov dword [te_pointer], eax
  sub eax, dword [text_editor_mem]
  shr eax, 1 ;div 2
  mov dword [te_cursor_offset], eax
  
  mov eax, dword [te_mouse_cursor_second_pointer]
  sub eax, dword [te_mouse_cursor_first_pointer] ;length of selected text
  sub dword [te_pointer_end], eax
  shr eax, 1 ;div 2
  sub dword [te_length_of_text], eax
  shl eax, 1 ;mul 2
  push eax
  
  mov edi, dword [te_mouse_cursor_first_pointer]
  mov esi, dword [te_mouse_cursor_second_pointer]
  mov ecx, dword [te_pointer_end]
  add ecx, eax
  sub ecx, dword [te_mouse_cursor_second_pointer]
  add ecx, 2
  cld
  rep movsb ;move all text
  
  pop eax
  mov edi, dword [te_pointer_end]
  mov ecx, eax
  mov eax, 0
  rep stosb ;clear previous text
  
  mov dword [te_mouse_cursor_first_pointer], 0
  mov dword [te_mouse_cursor_second_pointer], 0
 jmp .te_redraw_screen
 
 .key_left:
  mov dword [te_mouse_cursor_first_pointer], 0
  mov dword [te_mouse_cursor_second_pointer], 0
  
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
  mov dword [te_mouse_cursor_first_pointer], 0
  mov dword [te_mouse_cursor_second_pointer], 0
  
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
  mov dword [te_mouse_cursor_first_pointer], 0
  mov dword [te_mouse_cursor_second_pointer], 0
  
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
  mov dword [te_mouse_cursor_first_pointer], 0
  mov dword [te_mouse_cursor_second_pointer], 0
  
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
 
 .key_ctrl:
  cmp byte [key_code], KEY_C
  jne .if_ctrl_copy
   mov eax, dword [te_mouse_cursor_first_pointer]
   cmp eax, dword [te_mouse_cursor_second_pointer]
   je .te_halt ;nothing is selected
   jb .key_ctrl_copy_if_change_pointer
    mov ebx, dword [te_mouse_cursor_second_pointer]
    mov dword [te_mouse_cursor_first_pointer], ebx
    mov dword [te_mouse_cursor_second_pointer], eax
   .key_ctrl_copy_if_change_pointer:
   
   mov eax, dword [te_mouse_cursor_first_pointer]
   mov dword [copy_source], eax
   mov ebx, dword [te_mouse_cursor_second_pointer]
   sub ebx, eax
   mov dword [copy_length], ebx
   call copy_to_memory
   jmp .te_halt
  .if_ctrl_copy:
  
  cmp byte [key_code], KEY_V
  jne .if_ctrl_insert
   ;erase what is selected
   mov eax, dword [te_mouse_cursor_first_pointer]
   cmp eax, dword [te_mouse_cursor_second_pointer]
   je .ctrl_insert_if_selected_text
    cmp eax, dword [te_mouse_cursor_second_pointer]
    jb .ctrl_insert_if_second_below
     mov ebx, dword [te_mouse_cursor_second_pointer]
     mov dword [te_mouse_cursor_first_pointer], ebx
     mov dword [te_mouse_cursor_second_pointer], eax
    .ctrl_insert_if_second_below:
  
    mov eax, dword [te_mouse_cursor_first_pointer]
    mov dword [te_pointer], eax
    sub eax, dword [text_editor_mem]
    shr eax, 1 ;div 2
    mov dword [te_cursor_offset], eax
  
    mov eax, dword [te_mouse_cursor_second_pointer]
    sub eax, dword [te_mouse_cursor_first_pointer] ;length of selected text
    sub dword [te_pointer_end], eax
    shr eax, 1 ;div 2
    sub dword [te_length_of_text], eax
    shl eax, 1 ;mul 2
    push eax
  
    mov edi, dword [te_mouse_cursor_first_pointer]
    mov esi, dword [te_mouse_cursor_second_pointer]
    mov ecx, dword [te_pointer_end]
    add ecx, eax
    sub ecx, dword [te_mouse_cursor_second_pointer]
    add ecx, 2
    cld
    rep movsb ;move all text
  
    pop eax
    mov edi, dword [te_pointer_end]
    mov ecx, eax
    mov eax, 0
    rep stosb ;clear previous text
  
    mov dword [te_mouse_cursor_first_pointer], 0
    mov dword [te_mouse_cursor_second_pointer], 0
   .ctrl_insert_if_selected_text:
  
   cmp dword [copy_length], 0
   je .te_halt
   
   mov eax, dword [te_pointer]
   cmp dword [eax], 0
   je .ctrl_insert
   mov edi, dword [te_pointer_end]
   mov esi, dword [te_pointer_end]
   sub esi, dword [copy_length]
   mov ecx, dword [te_pointer_end]
   sub ecx, dword [te_pointer]
   std
   rep movsb ;move all text
   cld
  
   .ctrl_insert:
   mov eax, dword [te_pointer]
   mov dword [copy_destination], eax
   call copy_from_memory
   
   mov eax, dword [copy_length]
   add dword [te_pointer], eax
   add dword [te_pointer_end], eax
   shr eax, 1 ;div 2
   add dword [te_cursor_offset], eax
   add dword [te_length_of_text], eax
   
   jmp .te_redraw_screen
  .if_ctrl_insert:
 jmp .te_halt
 
 .mouse_event:
  mov eax, dword [te_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [te_mouse_column]
  mov dword [cursor_column], eax
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [te_mouse_line], eax
  mov eax, dword [cursor_column]
  mov dword [te_mouse_column], eax
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  cmp dword [usb_mouse_dnd], 0x2
  je .mouse_drag_and_drop
 jmp .test_if_add_char
 
 .mouse_click:
  mov eax, dword [te_pointer]
  mov dword [te_mouse_cursor_text_pointer], eax
  call te_draw_text
  mov eax, dword [te_mouse_cursor_text_pointer]
  mov dword [te_pointer], eax
  mov dword [te_mouse_cursor_first_pointer], eax
  mov dword [te_mouse_cursor_second_pointer], eax
  mov dword [te_mouse_cursor_text_pointer], 0
  sub eax, dword [text_editor_mem]
  shr eax, 1 ;div 2
  mov dword [te_cursor_offset], eax
 jmp .te_redraw_screen
 
 .mouse_drag_and_drop:
  mov eax, dword [te_pointer]
  mov dword [te_mouse_cursor_text_pointer], eax
  call te_draw_text
  mov eax, dword [te_mouse_cursor_text_pointer]
  mov dword [te_mouse_cursor_second_pointer], eax
  mov dword [te_mouse_cursor_text_pointer], 0
 jmp .te_redraw_screen
 
 .draw_char:
  ;test if something is not selected
  mov eax, dword [te_mouse_cursor_first_pointer]
  cmp eax, dword [te_mouse_cursor_second_pointer]
  je .draw_char_if_selected_text
   cmp eax, dword [te_mouse_cursor_second_pointer]
   jb .draw_char_if_second_below
    mov ebx, dword [te_mouse_cursor_second_pointer]
    mov dword [te_mouse_cursor_first_pointer], ebx
    mov dword [te_mouse_cursor_second_pointer], eax
   .draw_char_if_second_below:
  
   mov eax, dword [te_mouse_cursor_first_pointer]
   mov dword [te_pointer], eax
   sub eax, dword [text_editor_mem]
   shr eax, 1 ;div 2
   mov dword [te_cursor_offset], eax
  
   mov eax, dword [te_mouse_cursor_second_pointer]
   sub eax, dword [te_mouse_cursor_first_pointer] ;length of selected text
   sub dword [te_pointer_end], eax
   shr eax, 1 ;div 2
   sub dword [te_length_of_text], eax
   shl eax, 1 ;mul 2
   push eax
  
   mov edi, dword [te_mouse_cursor_first_pointer]
   mov esi, dword [te_mouse_cursor_second_pointer]
   mov ecx, dword [te_pointer_end]
   add ecx, eax
   sub ecx, dword [te_mouse_cursor_second_pointer]
   add ecx, 2
   cld
   rep movsb ;move all text
  
   pop eax
   mov edi, dword [te_pointer_end]
   mov ecx, eax
   mov eax, 0
   rep stosb ;clear previous text
  
   mov dword [te_mouse_cursor_first_pointer], 0xFFFFFFFF
   mov dword [te_mouse_cursor_second_pointer], 0xFFFFFFFF
  .draw_char_if_selected_text:
 
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
  
  ;redraw line where was char inserted
  call te_draw_text
  call text_editor_found_cursor_position
  mov eax, dword [te_draw_line]
  mov ebx, LINESZ
  mul ebx
  REDRAW_LINES_SCREEN eax, LINESZ
  
  cmp dword [te_mouse_cursor_first_pointer], 0xFFFFFFFF
  jne .te_halt
  call redraw_screen
 jmp .te_halt
 
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
   
   mov dword [column_heigth], LINESZ
   mov dword [color], 0x4E4E4E ;gray
   push esi
   call draw_column
   pop esi
   
   inc dword [cursor_line]   
   mov dword [color], BLACK
  .if_draw_cursor:
  
  ;test mouse cursor position
  cmp dword [te_mouse_cursor_text_pointer], 0
  je .if_mouse_cursor_here
  mov eax, dword [te_mouse_line]
  mov ebx, dword [cursor_line]
  dec ebx
  cmp eax, ebx
  jb .if_mouse_cursor_here ;if cursor below first line of char
  add ebx, 10 ;size of line
  cmp eax, ebx
  ja .if_mouse_cursor_here ;if cursor above last line of char
  mov eax, dword [te_mouse_column]
  mov ebx, dword [cursor_column]
  cmp eax, ebx
  jb .if_mouse_cursor_here ;if cursor above first column of char
  cmp word [esi], 0xA
  je .if_mouse_cursor_move_cursor_here ;if end of line, move cursor here
   ;mouse cursor is on this char
   add ebx, 4 ;half of char
   cmp eax, ebx
   jb .if_mouse_cursor_move_cursor_here ;cursor in first half of char - move cursor pointer to this char
   jmp .if_mouse_cursor_move_cursor_next ;cursor in second half of char - move cursor pointer on next char
  .if_mouse_cursor_move_cursor_here:
   mov dword [te_mouse_cursor_text_pointer], esi
   jmp .if_mouse_cursor_here
  .if_mouse_cursor_move_cursor_next:
   mov eax, esi
   add eax, 2
   mov dword [te_mouse_cursor_text_pointer], eax
  .if_mouse_cursor_here:
  
  ;draw background of char
  mov eax, dword [te_mouse_cursor_first_pointer]
  mov ebx, dword [te_mouse_cursor_second_pointer]
  cmp eax, ebx
  je .if_draw_background
  ja .second_is_above
   cmp esi, eax
   jb .if_draw_background
   cmp ebx, esi
   je .if_draw_background
   jb .if_draw_background
   
   push dword [cursor_line]
   push dword [cursor_column]
  
   dec dword [cursor_line]
   mov dword [square_length], 8
   mov dword [square_heigth], 10
   mov dword [color], 0x00A0FF
   cmp esi, dword [te_pointer]
   jne .if_cursor_background_1
    inc dword [cursor_column]
    mov dword [square_length], 7
   .if_cursor_background_1:
   call draw_square
   mov dword [color], BLACK
   
   pop dword [cursor_column]
   pop dword [cursor_line]
   jmp .if_draw_background
  .second_is_above:
   cmp esi, ebx
   jb .if_draw_background
   cmp eax, esi
   je .if_draw_background
   jb .if_draw_background
   
   push dword [cursor_line]
   push dword [cursor_column]
  
   dec dword [cursor_line]
   mov dword [square_length], 8
   mov dword [square_heigth], 10
   mov dword [color], 0x00A0FF
   cmp esi, dword [te_pointer]
   jne .if_cursor_background_2
    inc dword [cursor_column]
    mov dword [square_length], 7
   .if_cursor_background_2:
   call draw_square
   mov dword [color], BLACK
   
   pop dword [cursor_column]
   pop dword [cursor_line]
  .if_draw_background:
  
  ;test end of line
  cmp word [esi], 0xA ;enter
  je .enter
  
  ;draw char on screen
  cmp ebp, 1
  je .not_draw_char
  mov dword [char_for_print], 0
  mov ax, word [esi]
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
 
 ;test mouse cursor position
 cmp dword [te_mouse_cursor_text_pointer], 0
 je .if_end_mouse_cursor_here
 mov eax, dword [te_mouse_line]
 inc eax
 mov ebx, dword [cursor_line]
 cmp eax, ebx
 jb .if_end_mouse_cursor_here ;if cursor below first line of char
 mov eax, dword [te_mouse_column]
 mov ebx, dword [cursor_column]
 cmp eax, ebx
 jb .if_end_mouse_cursor_here ;if cursor above first column of char
  mov eax, dword [te_pointer_end]
  mov dword [te_mouse_cursor_text_pointer], eax
 .if_end_mouse_cursor_here:
 
 .done:
 pop dword [te_draw_column]
 pop dword [te_draw_line]
 DRAW_WINDOW_BORDERS text_editor_up_str, text_editor_down_str, 0xFF0000
 
 mov eax, dword [te_mouse_line]
 mov dword [cursor_line], eax
 mov eax, dword [te_mouse_column]
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 
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
 mov dword [text_editor_file_pointer], eax
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

text_editor_convert_to_utf8:
 push dword [allocated_memory_pointer]

 mov dword [allocated_size], 1
 call allocate_memory
 cmp dword [allocated_memory_pointer], 0
 je .not_enough_memory
 mov edi, dword [allocated_memory_pointer]
 mov eax, 0
 mov ecx, 0x100000
 rep stosb ;clear memory
 pop esi
 push dword [allocated_memory_pointer]
 mov edi, dword [allocated_memory_pointer]
 
 mov ecx, 0x100000
 .convert_char:
  cmp word [esi], 0
  je .done
  
  mov eax, 0
  mov ax, word [esi]
  test ax, 0x80
  jz .convert_to_ascii
  
  mov ax, word [esi]
  cmp ax, 0x7FF
  jb .convert_to_2_bytes
  jmp .next_char
  
  .convert_to_ascii:
  mov byte [edi], al
  inc edi
  jmp .next_char
  
  .convert_to_2_bytes:
  shr ax, 6
  and al, 0x1F
  or al, 0xC0
  mov byte [edi], al
  
  mov ax, word [esi]
  and al, 0x3F
  or al, 0x80
  mov byte [edi+1], al
  jmp .next_char
  
  .next_char:
  add esi, 2
 loop .convert_char
 
 .done:
 pop dword [allocated_memory_pointer]
 mov dword [allocated_size], 1
 call release_memory
 
 ret
 
 .not_enough_memory:
 pop eax
 ret
