;BleskOS

document_editor_up_str db 'Document editor', 0
document_editor_down_str db '[F1] Save file [F2] Open file [F3] New file [page up/down] Move page', 0
de_message_new_file_up db 'Are you sure you want to erase actual document?', 0
de_message_new_file_down db '[enter] Yes [esc] Cancel', 0
de_message_change_color_str_up db 'Choose color:', 0
de_message_change_color_str_down db '[a] black [r] red [g] green [b] blue [p] light blue [o] orange [esc] back', 0

document_editor_file_pointer dd 0
de_pointer dd 0
de_pointer_end_of_text dd 0
document_editor_file_end dd 0
document_editor_file_size dd 1

de_cursor_line dd 0
de_printing_base_line dd 0
de_first_show_line dd 0
de_last_show_line dd 0
de_draw_line dd 0
de_draw_column dd 0
de_left_border_column dd 0
de_right_border_column dd 600

de_left_border_start dd 0

de_cursor_char_font dd 0
de_cursor_char_type dd 0
de_cursor_char_size dd 0
de_cursor_char_aligment dd 0
de_cursor_char_color dd 0

%define DE_LEFT 0
%define DE_MIDDLE 1
%define DE_RIGHT 2
de_aligment dd DE_LEFT
de_actual_page dd 0

de_mouse_line dd 20
de_mouse_column dd 0
de_mouse_cursor_document_pointer dd 0
de_left_column_of_print dd 0
de_draw_on_end_of_line dd 0

%define DE_COMMAND 127

document_editor:
 call de_draw_document
 mov eax, dword [de_mouse_line]
 mov dword [cursor_line], eax
 mov eax, dword [de_mouse_column]
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 call redraw_screen
 
 mov dword [mcursor_up_side], 20
 mov dword [mcursor_left_side], 0
 SCREEN_Y_SUB eax, 20
 mov dword [mcursor_down_side], eax
 mov eax, dword [screen_x]
 mov dword [mcursor_right_side], eax

 .halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F1
  je .save_file
  
  cmp byte [key_code], KEY_F2
  je .open_file
  
  cmp byte [key_code], KEY_F3
  je .new_file
  
  cmp byte [key_code], KEY_F5
  je .change_bold
  
  cmp byte [key_code], KEY_F6
  je .change_size_sub
  
  cmp byte [key_code], KEY_F7
  je .change_size_add
  
  cmp byte [key_code], KEY_F8
  je .change_aligment
  
  cmp byte [key_code], KEY_F9
  je .change_font
  
  cmp byte [key_code], KEY_F10
  je .change_color
  
  cmp byte [key_code], KEY_F11
  je .clear_commands
  
  cmp byte [key_code], KEY_DELETE
  je .key_delete
  
  cmp byte [key_code], KEY_BACKSPACE
  je .key_backspace
  
  cmp byte [key_code], KEY_LEFT
  je .key_left
  
  cmp byte [key_code], KEY_RIGHT
  je .key_right
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [key_code], KEY_PAGE_UP
  je .key_page_up
  
  cmp byte [key_code], KEY_PAGE_DOWN
  je .key_page_down
  
  cmp byte [key_code], KEY_HOME
  je .key_home
  
  cmp byte [key_code], KEY_END
  je .key_end
  
  cmp byte [usb_mouse_data], 0
  jne .mouse_event
  
  .test_if_add_char:
  cmp word [key_unicode], 0
  jne .add_char
 jmp .halt
 
 .save_file:
  mov eax, dword [document_editor_file_pointer]
  mov dword [allocated_memory_pointer], eax
  call document_editor_convert_to_utf8
  
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
  mov dword [file_type], 'td'
  call file_dialog_save
 jmp document_editor
 
 .open_file:
  mov dword [fd_file_type_1], 'td'
  mov dword [fd_file_type_2], 'TD'
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je document_editor
  
  ;release memory of previous file
  cmp dword [document_editor_file_pointer], 0
  je .if_loaded_file
   push dword [allocated_memory_pointer]
   push dword [allocated_size]
   mov eax, dword [document_editor_file_pointer]
   mov dword [allocated_memory_pointer], eax
   mov dword [allocated_size], 1
   call release_memory
   pop dword [allocated_size]
   pop dword [allocated_memory_pointer]
  .if_loaded_file:
  
  call document_editor_convert_to_unicode
  
  ;calculate length of text
  mov eax, dword [document_editor_file_pointer]
  mov ebx, 0
  .calculate_length_of_document:
   cmp word [eax], 0
   je .end_of_calculating_length
   cmp eax, dword [document_editor_file_end]
   je .end_of_calculating_length
   add eax, 2
  jmp .calculate_length_of_document
  .end_of_calculating_length:
  mov dword [de_pointer_end_of_text], eax
 jmp document_editor
 
 .new_file:
  mov esi, de_message_new_file_up
  mov edi, de_message_new_file_down
  call show_message_window
  cmp byte [key_code], KEY_ESC
  je .cancel
  
  ;release actual memory
  mov eax, dword [document_editor_file_pointer]
  mov dword [allocated_memory_pointer], eax
  mov dword [allocated_size], 1
  call release_memory

  ;create new file
  mov dword [allocated_size], 1
  call allocate_memory
  mov eax, dword [allocated_memory_pointer]
  mov dword [document_editor_file_pointer], eax
  mov dword [de_pointer], eax
  mov dword [de_pointer_end_of_text], eax
  add eax, 0x100000-2
  mov dword [document_editor_file_end], eax
  
  ;clear memory of document_editor
  sub eax, 0x100000
  mov edi, eax
  mov eax, 0
  mov ecx, 0x100000
  rep stosb
  
  .cancel:
  mov byte [key_code], 0
 jmp document_editor
 
 ;CHANGES ON DOCUMENT FORMATTING
 .change_bold:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_bold_group
  cmp word [edi], DE_COMMAND
  je .change_bold_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_bold_add
  .change_bold_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_bold_group_add
  jmp .change_bold_skip_commands
  
  .change_bold_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  cmp dword [de_cursor_char_type], PLAIN
  jne .change_bold_if_plain
   mov word [edi+2], 'b'
  .change_bold_if_plain:
  cmp dword [de_cursor_char_type], BOLD
  jne .change_bold_if_bold
   mov word [edi+2], 'p'
  .change_bold_if_bold:
  jmp .key_right ;skip all commands
  
  .change_bold_group_add:
  add edi, 4
  .change_bold_group:
   cmp word [edi], DE_COMMAND
   jne .change_bold_add ;in this group is not any bold/plain command
   cmp word [edi+2], 'b'
   je .change_bold_to_plain
   cmp word [edi+2], 'p'
   je .change_plain_to_bold
   add edi, 4
  jmp .change_bold_group
  
  .change_bold_to_plain:
   mov word [edi+2], 'p'
   jmp document_editor
  .change_plain_to_bold:
   mov word [edi+2], 'b'
   jmp document_editor
   
 .change_size_add:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_size_add_group
  cmp word [edi], DE_COMMAND
  je .change_size_add_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_size_add_add
  .change_size_add_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_size_add_group_add
  jmp .change_size_add_skip_commands
  
  .change_size_add_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov eax, dword [de_cursor_char_size]
  shr eax, 3 ;div 8
  cmp eax, 9
  je .halt
  inc eax
  add eax, '0'
  mov word [edi+2], ax
  jmp .key_right ;skip all commands
  
  .change_size_add_group_add:
  add edi, 4
  .change_size_add_group:
   cmp word [edi], DE_COMMAND
   jne .change_size_add_add ;in this group is not any size command
   cmp word [edi+2], '1'
   jb .change_size_add_next_command
   cmp word [edi+2], '9'
   ja .change_size_add_next_command
   
   cmp word [edi+2], '9'
   je .halt
   inc word [edi+2]
   jmp document_editor
   
   .change_size_add_next_command:
   add edi, 4
  jmp .change_size_add_group
  
 .change_size_sub:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_size_sub_group
  cmp word [edi], DE_COMMAND
  je .change_size_sub_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_size_sub_add
  .change_size_sub_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_size_sub_group_add
  jmp .change_size_sub_skip_commands
  
  .change_size_sub_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov eax, dword [de_cursor_char_size]
  shr eax, 3 ;div 8
  cmp eax, 1
  je .halt
  dec eax
  add eax, '0'
  mov word [edi+2], ax
  jmp .key_right ;skip all commands
  
  .change_size_sub_group_add:
  add edi, 4
  .change_size_sub_group:
   cmp word [edi], DE_COMMAND
   jne .change_size_sub_add ;in this group is not any size command
   cmp word [edi+2], '1'
   jb .change_size_sub_next_command
   cmp word [edi+2], '9'
   ja .change_size_sub_next_command
   
   cmp word [edi+2], '1'
   je .halt
   dec word [edi+2]
   jmp document_editor
   
   .change_size_sub_next_command:
   add edi, 4
  jmp .change_size_sub_group
  
 .change_aligment:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_aligment_group
  cmp word [edi], DE_COMMAND
  je .change_aligment_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_aligment_add
  .change_aligment_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_aligment_group_add
  jmp .change_aligment_skip_commands
  
  .change_aligment_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  cmp dword [de_cursor_char_aligment], DE_LEFT
  jne .change_aligment_if_left
   mov word [edi+2], 'm'
  .change_aligment_if_left:
  cmp dword [de_cursor_char_aligment], DE_MIDDLE
  jne .change_aligment_if_middle
   mov word [edi+2], 'r'
  .change_aligment_if_middle:
  cmp dword [de_cursor_char_aligment], DE_RIGHT
  jne .change_aligment_if_right
   mov word [edi+2], 'l'
  .change_aligment_if_right:
  jmp .key_right ;skip all commands
  
  .change_aligment_group_add:
  add edi, 4
  .change_aligment_group:
   cmp word [edi], DE_COMMAND
   jne .change_aligment_add ;in this group is not any bold/plain command
   cmp word [edi+2], 'l'
   je .change_aligment_to_middle
   cmp word [edi+2], 'm'
   je .change_aligment_to_right
   cmp word [edi+2], 'r'
   je .change_aligment_to_left
   add edi, 4
  jmp .change_aligment_group
  
  .change_aligment_to_middle:
   mov word [edi+2], 'm'
   jmp document_editor
  .change_aligment_to_right:
   mov word [edi+2], 'r'
   jmp document_editor
  .change_aligment_to_left:
   mov word [edi+2], 'l'
   jmp document_editor
   
 .change_aligment_left:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_aligment_left_group
  cmp word [edi], DE_COMMAND
  je .change_aligment_left_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_aligment_left_add
  .change_aligment_left_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_aligment_left_group_add
  jmp .change_aligment_left_skip_commands
  
  .change_aligment_left_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov word [edi+2], 'l'
  jmp .key_right ;skip all commands
  
  .change_aligment_left_group_add:
  add edi, 4
  .change_aligment_left_group:
   cmp word [edi], DE_COMMAND
   jne .change_aligment_left_add ;in this group is not any aligment command
   cmp word [edi+2], 'l'
   je document_editor
   cmp word [edi+2], 'm'
   je .change_aligment_left_to_left
   cmp word [edi+2], 'r'
   je .change_aligment_left_to_left
   add edi, 4
  jmp .change_aligment_left_group
  
  .change_aligment_left_to_left:
   mov word [edi+2], 'l'
   jmp document_editor
   
 .change_aligment_middle:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_aligment_middle_group
  cmp word [edi], DE_COMMAND
  je .change_aligment_middle_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_aligment_middle_add
  .change_aligment_middle_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_aligment_middle_group_add
  jmp .change_aligment_middle_skip_commands
  
  .change_aligment_middle_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov word [edi+2], 'm'
  jmp .key_right ;skip all commands
  
  .change_aligment_middle_group_add:
  add edi, 4
  .change_aligment_middle_group:
   cmp word [edi], DE_COMMAND
   jne .change_aligment_middle_add ;in this group is not any aligment command
   cmp word [edi+2], 'l'
   je .change_aligment_middle_to_middle
   cmp word [edi+2], 'm'
   je document_editor
   cmp word [edi+2], 'r'
   je .change_aligment_middle_to_middle
   add edi, 4
  jmp .change_aligment_middle_group
  
  .change_aligment_middle_to_middle:
   mov word [edi+2], 'm'
   jmp document_editor
   
 .change_aligment_right:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_aligment_right_group
  cmp word [edi], DE_COMMAND
  je .change_aligment_right_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_aligment_right_add
  .change_aligment_right_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_aligment_right_group_add
  jmp .change_aligment_right_skip_commands
  
  .change_aligment_right_add:
  mov eax, 4
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov word [edi+2], 'r'
  jmp .key_right ;skip all commands
  
  .change_aligment_right_group_add:
  add edi, 4
  .change_aligment_right_group:
   cmp word [edi], DE_COMMAND
   jne .change_aligment_right_add ;in this group is not any aligment command
   cmp word [edi+2], 'l'
   je .change_aligment_right_to_right
   cmp word [edi+2], 'm'
   je .change_aligment_right_to_right
   cmp word [edi+2], 'r'
   je document_editor
   add edi, 4
  jmp .change_aligment_right_group
  
  .change_aligment_right_to_right:
   mov word [edi+2], 'r'
   jmp document_editor
   
 .change_font:
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_font_group
  cmp word [edi], DE_COMMAND
  je .change_font_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_font_add
  .change_font_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_font_group_add
  jmp .change_font_skip_commands
  
  .change_font_add:
  mov eax, 8
  call document_editor_shift_text
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov word [edi+2], 'f'
  mov word [edi+4], DE_COMMAND
  mov word [edi+6], 'B'
  cmp dword [de_cursor_char_font], bleskos_font
  jne .change_font_if_bleskos_font
   mov word [edi+6], 'V' ;VGA font
  .change_font_if_bleskos_font:
  cmp dword [de_cursor_char_font], vga_font
  jne .change_font_if_vga_font
   mov word [edi+6], 'B' ;BleskOS font
  .change_font_if_vga_font:
  jmp .key_right ;skip all commands
  
  .change_font_group_add:
  add edi, 4
  .change_font_group:
   cmp word [edi], DE_COMMAND
   jne .change_font_add ;in this group is not any bold/plain command
   cmp word [edi+2], 'f'
   je .change_font_execute
   add edi, 4
  jmp .change_font_group
  
  .change_font_execute:
  cmp word [edi+6], 'B'
  jne .change_font_execute_if_bleskos_font
   mov word [edi+6], 'V' ;VGA font
  .change_font_execute_if_bleskos_font:
  cmp word [edi+6], 'V'
  jne .change_font_execute_if_vga_font
   mov word [edi+6], 'B' ;BleskOS font
  .change_font_execute_if_vga_font:
  jmp document_editor
  
 .change_color:
  mov dword [message_window_key0], KEY_A
  mov dword [message_window_key1], KEY_R
  mov dword [message_window_key2], KEY_G
  mov dword [message_window_key3], KEY_B
  mov dword [message_window_key4], KEY_P
  mov dword [message_window_key5], KEY_O
  mov esi, de_message_change_color_str_up
  mov edi, de_message_change_color_str_down
  call show_message_window
  mov dword [message_window_key0], 0
  mov dword [message_window_key1], 0
  mov dword [message_window_key2], 0
  mov dword [message_window_key3], 0
  mov dword [message_window_key4], 0
  mov dword [message_window_key5], 0
  cmp byte [key_code], KEY_ESC
  je document_editor
  cmp byte [key_code], KEY_A
  jne .change_color_if_black
   mov bx, 0x0000
  .change_color_if_black:
  cmp byte [key_code], KEY_R
  jne .change_color_if_red
   mov bx, 0xF800
  .change_color_if_red:
  cmp byte [key_code], KEY_G
  jne .change_color_if_green
   mov bx, 0x07E0
  .change_color_if_green:
  cmp byte [key_code], KEY_B
  jne .change_color_if_blue
   mov bx, 0x001F
  .change_color_if_blue:
  cmp byte [key_code], KEY_P
  jne .change_color_if_light_blue
   mov bx, 0x07FF
  .change_color_if_light_blue:
  cmp byte [key_code], KEY_O
  jne .change_color_if_orange
   mov bx, 0xFAE0
  .change_color_if_orange:
  
  mov edi, dword [de_pointer]
  cmp edi, dword [document_editor_file_pointer]
  je .change_color_group
  cmp word [edi], DE_COMMAND
  je .change_color_group
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .change_color_add
  .change_color_skip_commands:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .change_color_group_add
  jmp .change_color_skip_commands
  
  .change_color_add:
  mov eax, 8
  push ebx
  call document_editor_shift_text
  pop ebx
  mov edi, dword [de_pointer]
  mov word [edi], DE_COMMAND
  mov word [edi+2], 'c'
  mov word [edi+4], DE_COMMAND
  mov word [edi+6], bx
  jmp .key_right ;skip all commands
  
  .change_color_group_add:
  add edi, 4
  .change_color_group:
   cmp word [edi], DE_COMMAND
   jne .change_color_add ;in this group is not any color command
   cmp word [edi+2], 'c'
   je .change_color_execute
   add edi, 4
  jmp .change_color_group
  
  .change_color_execute:
  mov word [edi+6], bx
  jmp document_editor
  
 .clear_commands:
  mov eax, 0
  mov edi, dword [de_pointer]
  cmp word [edi], DE_COMMAND
  je .clear_commands_execute
  sub edi, 4
  cmp word [edi], DE_COMMAND
  jne .halt
  .clear_commands_find_first:
   sub edi, 4
   cmp word [edi], DE_COMMAND
   jne .clear_commands_first_founded
  jmp .clear_commands_find_first
  
  .clear_commands_first_founded:
  add edi, 4
  mov dword [de_pointer], edi
  .clear_commands_execute:
   add eax, 4
   add edi, 4
   cmp word [edi], DE_COMMAND
   je .clear_commands_execute
  call document_editor_delete_text
  jmp document_editor
 
 ;CHANGES IN DOCUMENT CONTENT
 .key_delete:
  mov ebx, dword [de_pointer]
  .key_delete_skip_commands:
  cmp word [ebx], DE_COMMAND
  jne .key_delete_char
  add ebx, 4
  jmp .key_delete_skip_commands
  .key_delete_char:
  mov dword [de_pointer], ebx
  mov eax, 2
  call document_editor_delete_text
 jmp document_editor
 
 .key_backspace:
  mov ebx, dword [de_pointer]
  mov ecx, dword [document_editor_file_pointer]
  sub ecx, 4
  .key_backspace_skip_commands:
  sub ebx, 4
  cmp ebx, ecx
  je .halt
  cmp word [ebx], DE_COMMAND
  jne .key_backspace_delete_char
  jmp .key_backspace_skip_commands
  .key_backspace_delete_char:
  add ebx, 2
  mov dword [de_pointer], ebx
  mov eax, 2
  call document_editor_delete_text
 jmp document_editor
 
 .key_left:
  mov eax, dword [document_editor_file_pointer]
  mov ebx, dword [de_pointer]
  mov ecx, ebx
  cmp dword [de_pointer], eax
  je .halt
  sub ebx, 4
  cmp word [ebx], DE_COMMAND
  jne .key_left_move_2
  .key_left_move_through_commands:
   sub ebx, 4
   cmp word [ebx], DE_COMMAND
   je .key_left_move_through_commands
   add ebx, 4
   jmp .key_left_move
  .key_left_move_2:
  add ebx, 2
  .key_left_move:
  mov dword [de_pointer], ebx
 jmp document_editor
 
 .key_right:
  mov eax, dword [de_pointer]
  cmp word [eax], 0
  je .halt
  cmp word [eax], DE_COMMAND
  jne .key_right_move_2
  .key_right_move_through_commands:
   add eax, 4
   cmp word [eax], DE_COMMAND
   je .key_right_move_through_commands
   jmp .key_right_move
  .key_right_move_2:
  add eax, 2   
  .key_right_move:
  mov dword [de_pointer], eax
 jmp document_editor
 
 .key_up:
  mov eax, dword [de_pointer]
  cmp eax, dword [document_editor_file_pointer]
  je .halt
  .key_up_move:
  cmp eax, dword [document_editor_file_pointer]
  je .move_key_up
  sub eax, 4
  cmp word [eax], DE_COMMAND
  je .key_up_command
  add eax, 2
  cmp word [eax], 0xA
  je .move_key_up
  jmp .key_up_move
  
  .key_up_command:
  cmp word [eax+2], 'l'
  je .move_key_up_skip_commands
  cmp word [eax+2], 'm'
  je .move_key_up_skip_commands
  cmp word [eax+2], 'r'
  je .move_key_up_skip_commands
  jmp .key_up_move
  
  .move_key_up_skip_commands:
   sub eax, 4
   cmp word [eax], DE_COMMAND
   jne .move_key_up_after_commands
  jmp .move_key_up_skip_commands
  
  .move_key_up_after_commands:
  add eax, 4
  .move_key_up:
  mov dword [de_pointer], eax
 jmp document_editor
 
 .key_down:
  mov eax, dword [de_pointer]
  .key_down_move:
  cmp word [eax], 0
  je .move_key_down
  cmp word [eax], DE_COMMAND
  je .key_down_command
  add eax, 2
  cmp word [eax], 0xA
  je .move_key_down
  jmp .key_down_move
  
  .key_down_command:
  cmp word [eax+2], 'l'
  je .move_key_down
  cmp word [eax+2], 'm'
  je .move_key_down
  cmp word [eax+2], 'r'
  je .move_key_down
  add eax, 4
  jmp .key_down_move
  
  .move_key_down:
  mov dword [de_pointer], eax
 jmp document_editor
 
 .key_page_up:
  cmp dword [de_first_show_line], 0
  je .halt
  sub dword [de_first_show_line], 40
  sub dword [de_last_show_line], 40
 jmp document_editor
 
 .key_page_down:
  cmp dword [de_first_show_line], 880*39
  je .halt
  add dword [de_first_show_line], 40
  add dword [de_last_show_line], 40
 jmp document_editor
 
 .key_home:
  mov dword [de_first_show_line], 0
 jmp document_editor
 
 .key_end:
  mov dword [de_first_show_line], 880*39
 jmp document_editor
 
 .mouse_event:
  mov eax, dword [de_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [de_mouse_column]
  mov dword [cursor_column], eax
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [de_mouse_line], eax
  mov eax, dword [cursor_column]
  mov dword [de_mouse_column], eax
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
 jmp .test_if_add_char
 
 .mouse_click:
  TEST_CLICK_ZONE_WITH_JUMP click_zone_font, de_mouse_line, de_mouse_column, 18+LINESZ, 18+LINESZ+14, COLUMNSZ-2, COLUMNSZ*15-2, .change_font
  TEST_CLICK_ZONE_WITH_JUMP click_zone_bold, de_mouse_line, de_mouse_column, 20+LINESZ*3, 20+LINESZ*3+16, COLUMNSZ-2, COLUMNSZ+14, .change_bold
  TEST_CLICK_ZONE_WITH_JUMP click_zone_size_sub, de_mouse_line, de_mouse_column, 20+LINESZ*3, 20+LINESZ*3+16, COLUMNSZ*15-26, COLUMNSZ*15-14, .change_size_sub
  TEST_CLICK_ZONE_WITH_JUMP click_zone_size_add, de_mouse_line, de_mouse_column, 20+LINESZ*3, 20+LINESZ*3+16, COLUMNSZ*15-14, COLUMNSZ*15-2, .change_size_add
  TEST_CLICK_ZONE_WITH_JUMP click_zone_left, de_mouse_line, de_mouse_column, 20+LINESZ*5, 20+LINESZ*5+16, COLUMNSZ-2, COLUMNSZ+14, .change_aligment_left
  TEST_CLICK_ZONE_WITH_JUMP click_zone_middle, de_mouse_line, de_mouse_column, 20+LINESZ*5, 20+LINESZ*5+16, COLUMNSZ*4-2, COLUMNSZ*4+14, .change_aligment_middle
  TEST_CLICK_ZONE_WITH_JUMP click_zone_right, de_mouse_line, de_mouse_column, 20+LINESZ*5, 20+LINESZ*5+16, COLUMNSZ*7-2, COLUMNSZ*7+14, .change_aligment_right
  TEST_CLICK_ZONE_WITH_JUMP click_zone_color, de_mouse_line, de_mouse_column, 20+LINESZ*7, 20+LINESZ*7+16, COLUMNSZ-2, COLUMNSZ*15-2, .change_color
  
  SCREEN_Y_SUB eax, 20
  mov ebx, dword [de_left_border_start]
  mov ecx, ebx
  add ecx, dword [de_right_border_column]
  sub ebx, 40
  add ecx, 40
  TEST_CLICK_ZONE_WITH_JUMP click_zone_document, de_mouse_line, de_mouse_column, 20, eax, ebx, ecx, .mouse_click_document
 jmp .test_if_add_char
 
 .mouse_click_document:
  mov eax, dword [de_pointer]
  mov dword [de_mouse_cursor_document_pointer], eax
  call de_draw_document
  mov eax, dword [de_mouse_cursor_document_pointer]
  mov dword [de_pointer], eax
  mov dword [de_mouse_cursor_document_pointer], 0
 jmp document_editor
 
 .add_char:
  mov eax, 2
  call document_editor_shift_text
  mov eax, 0
  mov ax, word [key_unicode]
  mov esi, dword [de_pointer]
  mov word [esi], ax
  add dword [de_pointer], 2
 jmp document_editor
 
de_draw_document:
 DRAW_WINDOW document_editor_up_str, document_editor_down_str, 0x4444FF, 0xAAAAAA
 
 ;draw document background
 mov dword [square_length], 680
 mov eax, dword [screen_y]
 sub eax, 40
 mov dword [square_heigth], eax
 mov dword [cursor_line], 20
 mov eax, dword [screen_x]
 sub eax, 680
 shr eax, 1 ;div 2
 mov dword [cursor_column], eax
 mov dword [color], WHITE
 push dword [cursor_column]
 push dword [square_heigth]
 push dword [cursor_column]
 push dword [square_heigth]
 call draw_square
 pop ebx
 pop eax
 DRAW_COLUMN 20, eax, ebx, BLACK
 pop ebx
 pop eax
 add eax, 680
 DRAW_COLUMN 20, eax, ebx, BLACK
 
 ;draw lines between pages
 mov eax, dword [de_first_show_line]
 mov ebx, 880
 mov edx, 0
 div ebx
 mov eax, 880
 sub eax, edx
 
 mov ebx, eax
 add ebx, 880
 
 mov ecx, ebx
 add ecx, 880
 
 add eax, 20
 add ebx, 20
 add ecx, 20
 
 cmp eax, dword [screen_y]
 ja .skip_page_lines
 mov dword [line_length], 680
 mov dword [color], BLACK
 mov dword [cursor_line], eax
 mov edx, dword [screen_x]
 sub edx, 680
 shr edx, 1 ;div 2
 mov dword [cursor_column], edx
 pusha
 call draw_line
 popa
 cmp ebx, dword [screen_y]
 ja .skip_page_lines
 mov dword [line_length], 680
 mov dword [color], BLACK
 mov dword [cursor_line], ebx
 mov dword [cursor_column], edx
 pusha
 call draw_line
 popa
 cmp ecx, dword [screen_y]
 ja .skip_page_lines
 mov dword [line_length], 680
 mov dword [color], BLACK
 mov dword [cursor_line], ecx
 mov dword [cursor_column], edx
 call draw_line
 .skip_page_lines:
 
 mov dword [de_draw_line], 40
 mov dword [de_printing_base_line], 48
 mov dword [de_draw_column], 0
 mov eax, dword [de_left_border_start]
 mov dword [de_left_column_of_print], eax
  
 mov eax, dword [document_editor_file_pointer]
 mov esi, eax
 .draw_char:
  cmp word [esi], 0
  je .done
  cmp esi, dword [document_editor_file_end]
  je .done
  
  ;find if this is on end of line
  mov dword [de_draw_on_end_of_line], 0
  mov edi, esi
  .find_if_this_is_end_of_line:
   cmp word [edi], DE_COMMAND
   jne .done_find_if_this_is_end_of_line
   cmp word [edi+2], 'l'
   je .find_if_this_is_end_of_line_yes
   cmp word [edi+2], 'm'
   je .find_if_this_is_end_of_line_yes
   cmp word [edi+2], 'r'
   je .find_if_this_is_end_of_line_yes
   add edi, 4
  jmp .find_if_this_is_end_of_line
  .find_if_this_is_end_of_line_yes:
   mov dword [de_draw_on_end_of_line], 1
  .done_find_if_this_is_end_of_line:
  
  cmp word [esi], DE_COMMAND
  jne .if_command
  .execute_command:
   ;draw cursor if is on this command
   cmp esi, dword [de_pointer]
   jne .if_cursor_on_command
    mov eax, dword [de_printing_base_line]
    mov ebx, dword [size_of_text]
    shl ebx, 3 ;mul 8
    sub eax, ebx
    sub eax, dword [de_first_show_line]
    add eax, 20
    mov dword [cursor_line], eax
    mov eax, dword [de_draw_column]
    add eax, dword [de_left_border_start]
    mov dword [cursor_column], eax
  
    mov eax, dword [size_of_text]
    shl eax, 3 ;mul 8
    mov dword [column_heigth], eax
    push dword [color]
    mov dword [color], BLACK
    call draw_column
    pop dword [color]
    
    ;save variabiles of char
    mov eax, dword [font_pointer]
    mov dword [de_cursor_char_font], eax
    mov eax, dword [type_of_text]
    mov dword [de_cursor_char_type], eax
    mov eax, dword [size_of_text]
    shl eax, 3 ;mul 8
    mov dword [de_cursor_char_size], eax
    mov eax, dword [color]
    mov dword [de_cursor_char_color], eax
    mov eax, dword [de_aligment]
    mov dword [de_cursor_char_aligment], eax
    
    ;actual page number
    mov eax, dword [de_draw_line]
    mov ebx, 880
    mov edx, 0
    div ebx
    inc eax
    mov dword [de_actual_page], eax
   .if_cursor_on_command:
 
   cmp word [esi+2], 'p'
   jne .if_command_plain
    mov dword [type_of_text], PLAIN
    add esi, 4
    jmp .if_command
   .if_command_plain:
   
   cmp word [esi+2], 'b'
   jne .if_command_bold
    mov dword [type_of_text], BOLD
    add esi, 4
    jmp .if_command
   .if_command_bold:
   
   cmp word [esi+2], 'l'
   jne .if_command_aligment_left
    mov dword [de_aligment], DE_LEFT
    add esi, 4
    call document_editor_move_to_next_line
    jmp .if_command
   .if_command_aligment_left:
   
   cmp word [esi+2], 'm'
   jne .if_command_aligment_middle
    mov dword [de_aligment], DE_MIDDLE
    add esi, 4
    call document_editor_move_to_next_line
    jmp .if_command
   .if_command_aligment_middle:
   
   cmp word [esi+2], 'r'
   jne .if_command_aligment_right
    mov dword [de_aligment], DE_RIGHT
    add esi, 4
    call document_editor_move_to_next_line
    jmp .if_command
   .if_command_aligment_right:
   
   cmp word [esi+2], '1'
   jb .if_change_size
   cmp word [esi+2], '9'
   ja .if_change_size
    mov eax, 0
    mov ax, word [esi+2]
    sub eax, '0'
    mov dword [size_of_text], eax
    add esi, 4
    jmp .if_command
   .if_change_size:
   
   cmp word [esi+2], 'c'
   jne .if_command_change_font_color
    mov eax, 0
    mov ax, word [esi+6]
    mov ebx, eax
    mov ecx, eax
    shr eax, 11
    shl eax, 19
    shr ebx, 5
    and ebx, 0x3F
    shl ebx, 10
    and ecx, 0x1F
    shl ecx, 3
    or eax, ebx
    or eax, ecx
    mov dword [color], eax   
    add esi, 8
    jmp .if_command
   .if_command_change_font_color:
   
   cmp word [esi+2], 'f'
   jne .if_command_change_font
    mov dword [font_pointer], bleskos_font
    cmp word [esi+6], 'V' ;VGA font
    jne .if_vga_font
     mov dword [font_pointer], vga_font
    .if_vga_font:
    add esi, 8
    jmp .if_command
   .if_command_change_font:
   
   cmp word [esi+2], 31
   jne .if_command_comma
    add esi, 2 ;print this comma to middle of line
    jmp .if_command
   .if_command_comma:
   
   cmp word [esi+2], 'L'
   jne .if_command_change_left_border
    mov eax, 0
    mov ax, word [esi+6]
    and eax, 0x1FF
    mov dword [de_left_border_column], eax
    add esi, 8
    call document_editor_move_to_next_line
    jmp .if_command
   .if_command_change_left_border:
   
   add esi, 4
  .if_command:
  cmp word [esi], DE_COMMAND ;next command
  je .execute_command
  
  ;if this is not on screen, do not draw it
  mov eax, dword [de_draw_line]
  cmp eax, dword [de_first_show_line]
  jb .do_not_draw
  cmp eax, dword [de_last_show_line]
  ja .done
  
  cmp word [esi], 0xA
  jne .if_enter_char
   mov dword [de_draw_on_end_of_line], 1
  .if_enter_char:
  mov eax, dword [size_of_text]
  shl eax, 3 ;mul 8
  add eax, dword [de_draw_column]
  cmp eax, dword [de_right_border_column]
  jb .if_on_end_of_line2
   mov dword [de_draw_on_end_of_line], 2
  .if_on_end_of_line2:
  
  ;test if mouse cursor is not here
  cmp dword [de_mouse_cursor_document_pointer], 0
  je .if_mouse_cursor_here
  mov eax, dword [de_mouse_line]
  mov ebx, dword [cursor_line]
  cmp eax, ebx
  jb .if_mouse_cursor_here ;if cursor below first line of char
  mov ecx, dword [size_of_text]
  shl ecx, 3 ;mul 8
  add ebx, ecx
  cmp eax, ebx
  ja .if_mouse_cursor_here ;if cursor above last line of char
  mov eax, dword [de_mouse_column]
  mov ebx, dword [cursor_column]
  cmp eax, ebx
  jb .if_mouse_cursor_here_start_of_line ;if cursor above first column of char
  add ebx, ecx
  cmp eax, ebx
  ja .if_mouse_cursor_here_end_of_line ;if cursor below last line of char
   ;mouse cursor is on this char
   shr ecx, 1 ;div 2
   sub ebx, ecx ;half of char
   cmp eax, ebx
   jb .if_mouse_cursor_move_cursor_back ;cursor in first half of char - move cursor pointer to this char
   jmp .if_mouse_cursor_move_cursor_here ;cursor in second half of char - move cursor pointer on next char
  .if_mouse_cursor_here_start_of_line: ;click below first drawed char - move on first drawed char
   mov eax, dword [de_left_column_of_print]
   cmp ebx, dword [de_left_column_of_print]
   jne .if_mouse_cursor_here ;this char is not on start of line
   ;move cursor to start of this line
   .if_mouse_cursor_move_cursor_back:
   mov dword [de_mouse_cursor_document_pointer], esi
   sub dword [de_mouse_cursor_document_pointer], 2
   jmp .if_mouse_cursor_here
  .if_mouse_cursor_here_end_of_line: ;click above last drawed char - move on last drawed char
   cmp dword [de_draw_on_end_of_line], 0
   je .if_mouse_cursor_here
   mov edi, esi
   .move_cursor_back_commands:
    sub edi, 4
    cmp word [edi], DE_COMMAND
    jne .move_cursor_end_of_line
   jmp .move_cursor_back_commands
   .move_cursor_end_of_line:
    add edi, 4
    mov dword [de_mouse_cursor_document_pointer], edi
   cmp dword [de_draw_on_end_of_line], 1 ;to next line is enter/aligment
   je .if_mouse_cursor_here
   cmp edi, dword [document_editor_file_pointer]
   je .if_mouse_cursor_here ;we are on start of document
    sub edi, 2
    mov dword [de_mouse_cursor_document_pointer], edi
   jmp .if_mouse_cursor_here
  .if_mouse_cursor_move_cursor_here:
   mov dword [de_mouse_cursor_document_pointer], esi
  .if_mouse_cursor_here:
  
  ;if on end of line, move to next line
  mov eax, dword [size_of_text]
  shl eax, 3 ;mul 8
  add eax, dword [de_draw_column]
  cmp eax, dword [de_right_border_column]
  jb .if_on_end_of_line
   call document_editor_move_to_next_line
  .if_on_end_of_line:
  
  ;draw char on screen
  mov eax, dword [de_printing_base_line]
  mov ebx, dword [size_of_text]
  shl ebx, 3 ;mul 8
  sub eax, ebx
  sub eax, dword [de_first_show_line]
  add eax, 20
  mov dword [cursor_line], eax
  mov eax, dword [de_draw_column]
  add eax, dword [de_left_border_start]
  mov dword [cursor_column], eax
  
  cmp word [esi], 0xA
  je .if_enter_do_not_print
  mov eax, 0
  mov ax, word [esi]
  mov dword [char_for_print], eax
  push dword [cursor_column]
  push dword [cursor_line]
  push esi
  call print_char
  pop esi
  pop dword [cursor_line]
  pop dword [cursor_column]
  .if_enter_do_not_print:
  
  ;draw cursor
  cmp esi, dword [de_pointer]
  jne .if_draw_cursor
   mov eax, dword [cursor_line]
   mov dword [de_cursor_line], eax
   
   mov eax, dword [size_of_text]
   shl eax, 3 ;mul 8
   mov dword [column_heigth], eax
   push dword [color]
   mov dword [color], BLACK
   call draw_column
   pop dword [color]
   
   ;save variabiles of char
   mov eax, dword [font_pointer]
   mov dword [de_cursor_char_font], eax
   mov eax, dword [type_of_text]
   mov dword [de_cursor_char_type], eax
   mov eax, dword [size_of_text]
   shl eax, 3 ;mul 8
   mov dword [de_cursor_char_size], eax
   mov eax, dword [color]
   mov dword [de_cursor_char_color], eax
   mov eax, dword [de_aligment]
   mov dword [de_cursor_char_aligment], eax
   
   ;actual page number
   mov eax, dword [de_draw_line]
   mov ebx, 880
   mov edx, 0
   div ebx
   inc eax
   mov dword [de_actual_page], eax
  .if_draw_cursor:
  .do_not_draw:
  
  ;change variabiles
  cmp word [esi], 0xA
  jne .if_enter
   add esi, 2
   call document_editor_move_to_next_line
   sub esi, 2
   jmp .next_char
  .if_enter:
  mov eax, dword [size_of_text]
  shl eax, 3 ;mul 8
  add dword [de_draw_column], eax
 
 .next_char:
 add esi, 2
 jmp .draw_char
 
 .done:
 cmp dword [de_mouse_cursor_document_pointer], 0
  je .if_mouse_cursor_on_end_of_document
  mov eax, dword [de_mouse_line]
  mov ebx, dword [cursor_line]
  cmp eax, ebx
  jb .if_mouse_cursor_on_end_of_document
  mov eax, dword [de_mouse_column]
  mov ebx, dword [cursor_column]
  cmp eax, ebx
  jb .if_mouse_cursor_on_end_of_document
  
  mov dword [de_mouse_cursor_document_pointer], esi
 .if_mouse_cursor_on_end_of_document:
 
 cmp esi, dword [de_pointer]
 jne .if_cursor_end_of_text
  mov eax, dword [de_printing_base_line]
  mov ebx, dword [size_of_text]
  shl ebx, 3 ;mul 8
  sub eax, ebx
  sub eax, dword [de_first_show_line]
  add eax, 20
  mov dword [cursor_line], eax
  mov eax, dword [de_draw_column]
  add eax, dword [de_left_border_start]
  mov dword [cursor_column], eax
  
  mov eax, dword [size_of_text]
  shl eax, 3 ;mul 8
  mov dword [column_heigth], eax
  push dword [color]
  mov dword [color], BLACK
  call draw_column
  pop dword [color]
  
  ;save variabiles of char
  mov eax, dword [font_pointer]
  mov dword [de_cursor_char_font], eax
  mov eax, dword [type_of_text]
  mov dword [de_cursor_char_type], eax
  mov eax, dword [size_of_text]
  shl eax, 3 ;mul 8
  mov dword [de_cursor_char_size], eax
  mov eax, dword [color]
  mov dword [de_cursor_char_color], eax
  mov eax, dword [de_aligment]
  mov dword [de_cursor_char_aligment], eax
  
  ;actual page number
  mov eax, dword [de_draw_line]
  mov ebx, 880
  mov edx, 0
  div ebx
  inc eax
  mov dword [de_actual_page], eax
 .if_cursor_end_of_text:
 mov dword [type_of_text], PLAIN
 mov dword [size_of_text], 1
 mov dword [font_pointer], bleskos_font
 mov dword [de_left_border_column], 0
 mov dword [de_right_border_column], 600
 DRAW_WINDOW_BORDERS document_editor_up_str, document_editor_down_str, 0x4444FF
 
 ;print actual page number
 cmp dword [de_actual_page], 10
 jb .actual_page_below_10
 
 .actual_page_below_100:
 SCREEN_X_SUB eax, COLUMNSZ*8
 PRINT 'page', de_page_str10, 5, eax
 mov eax, dword [de_actual_page]
 SCREEN_X_SUB ebx, COLUMNSZ*3
 PRINT_VAR eax, 5, ebx
 jmp document_editor_draw_tools
 
 .actual_page_below_10:
 SCREEN_X_SUB eax, COLUMNSZ*7
 PRINT 'page', de_page_str20, 5, eax
 mov eax, dword [de_actual_page]
 SCREEN_X_SUB ebx, COLUMNSZ*2
 PRINT_VAR eax, 5, ebx
 
document_editor_draw_tools:
 ;background
 mov dword [square_heigth], 20+LINESZ*15
 mov dword [square_length], COLUMNSZ*16
 mov dword [cursor_line], 20
 mov dword [cursor_column], 0
 mov dword [color], 0x804200
 call draw_square
 
 ;print font
 DRAW_SQUARE 18+LINESZ, COLUMNSZ-2, COLUMNSZ*14, 14, WHITE
 DRAW_EMPTY_SQUARE 18+LINESZ, COLUMNSZ-2, COLUMNSZ*14, 14, BLACK
 cmp dword [de_cursor_char_font], bleskos_font
 jne .de_if_bleskos_font
  PRINT 'BleskOS font', de_bleskos_font_str, 20+LINESZ+2, COLUMNSZ
 .de_if_bleskos_font:
 cmp dword [de_cursor_char_font], vga_font
 jne .de_if_vga_font
  PRINT 'VGA font', de_vga_font_str, 20+LINESZ+2, COLUMNSZ
 .de_if_vga_font:
 
 ;print type of char
 cmp dword [de_cursor_char_type], BOLD
 jne .de_if_char_bold
  DRAW_SQUARE 20+LINESZ*3, COLUMNSZ-2, 16, 16, 0xFF0000
 .de_if_char_bold:
 DRAW_EMPTY_SQUARE 20+LINESZ*3, COLUMNSZ-2, 16, 16, BLACK
 mov dword [color], BLACK
 PRINT_CHAR 'B', 20+LINESZ*3+5, COLUMNSZ+2
 
 ;print size of char
 DRAW_SQUARE 20+LINESZ*3, COLUMNSZ*15-26, 24, 16, WHITE
 DRAW_EMPTY_SQUARE 20+LINESZ*3, COLUMNSZ*15-26, 24, 16, BLACK
 mov eax, dword [de_cursor_char_size]
 PRINT_VAR eax, 20+LINESZ*3+5, COLUMNSZ*15-24
 
 ;print aligment of char
 cmp dword [de_cursor_char_aligment], DE_LEFT
 jne .de_if_aligment_left
  DRAW_SQUARE 20+LINESZ*5, COLUMNSZ-2, 16, 16, 0xFF0000
 .de_if_aligment_left:
 cmp dword [de_cursor_char_aligment], DE_MIDDLE
 jne .de_if_aligment_middle
  DRAW_SQUARE 20+LINESZ*5, COLUMNSZ*4-2, 16, 16, 0xFF0000
 .de_if_aligment_middle:
 cmp dword [de_cursor_char_aligment], DE_RIGHT
 jne .de_if_aligment_right
  DRAW_SQUARE 20+LINESZ*5, COLUMNSZ*7-2, 16, 16, 0xFF0000
 .de_if_aligment_right:
 DRAW_EMPTY_SQUARE 20+LINESZ*5, COLUMNSZ-2, 16, 16, BLACK
 mov dword [color], BLACK
 PRINT_CHAR 'L', 20+LINESZ*5+5, COLUMNSZ+2
 DRAW_EMPTY_SQUARE 20+LINESZ*5, COLUMNSZ*4-2, 16, 16, BLACK
 mov dword [color], BLACK
 PRINT_CHAR 'M', 20+LINESZ*5+5, COLUMNSZ*4+2
 DRAW_EMPTY_SQUARE 20+LINESZ*5, COLUMNSZ*7-2, 16, 16, BLACK
 mov dword [color], BLACK
 PRINT_CHAR 'R', 20+LINESZ*5+5, COLUMNSZ*7+2
 
 ;print color of char
 mov eax, dword [de_cursor_char_color]
 mov dword [color], eax
 PRINT 'TEXT COLOR', de_text_color_str, 20+LINESZ*7+5, COLUMNSZ-2
 mov dword [color], BLACK
 
 ;print how to change text
 PRINT '[F5] Bold/Plain', de_tools_bold_plain_str, 20+LINESZ*9+5, COLUMNSZ-2
 PRINT '[F6/F7] Size', de_tools_size_str, 20+LINESZ*10+5, COLUMNSZ-2
 PRINT '[F8] Aligment', de_tools_aligment_str, 20+LINESZ*11+5, COLUMNSZ-2
 PRINT '[F9] Font', de_tools_font_str, 20+LINESZ*12+5, COLUMNSZ-2
 PRINT '[F10] Color', de_tools_color_str, 20+LINESZ*13+5, COLUMNSZ-2
 PRINT '[F11] Clear', de_tools_clear_str, 20+LINESZ*14+5, COLUMNSZ-2

 ret
 
document_editor_move_to_next_line:
 push dword [size_of_text]
 
 ;move on next line
 mov edi, esi
 mov eax, dword [de_left_border_column] ;pointer to actual column
 mov ebx, dword [de_right_border_column] ;end of line
 mov ecx, 0 ;maximal size of chars on this line
 .calculate_next_line:
  cmp word [edi], 0
  je .change_line
  cmp word [edi], 0xA
  je .change_line  
  cmp word [edi], DE_COMMAND
  jne .if_command
  cmp word [edi+2], 'l'
  je .change_line
  cmp word [edi+2], 'm'
  je .change_line
  cmp word [edi+2], 'r'
  je .change_line
  cmp word [edi+2], '1'
  jb .if_command
  cmp word [edi+2], '9'
  ja .if_command
  mov edx, 0
  mov dx, word [edi+2]
  sub edx, '0'
  mov dword [size_of_text], edx
  cmp ecx, edx
  ja .if_command
  mov ecx, edx
  add edi, 4
  jmp .calculate_next_line
  .if_command:
  
  mov edx, dword [size_of_text]
  shl edx, 3 ;mul 8 - size of char in pixels
  add eax, edx
  cmp eax, ebx
  ja .change_line ;end of this line
  
  add edi, 2
  jmp .calculate_next_line  
 .change_line:
 cmp ecx, 0
 jne .if_no_change_size_command
  mov ecx, dword [size_of_text]
 .if_no_change_size_command:
 shl ecx, 3 ;mul 8
 add ecx, 2
 add dword [de_draw_line], ecx
 add dword [de_printing_base_line], ecx
 
 ;if we are on end of page, move to next page
 mov eax, dword [de_draw_line]
 mov ebx, 880
 mov edx, 0
 div ebx
 cmp edx, 840
 jb .aligment
 mov eax, 880
 sub eax, edx ;end of this page
 add eax, 40 ;start of next page
 add dword [de_draw_line], eax
 add dword [de_printing_base_line], eax
 
 ;move on new column
 .aligment:
 cmp dword [de_aligment], DE_LEFT
 je .left
 
 ;calculate chars on this line
 mov edi, esi
 mov eax, dword [de_left_border_column] ;pointer to actual column
 mov ebx, dword [de_right_border_column] ;end of line
 mov ecx, 0 ;length of line in pixels
 .calculate_aligment:
  cmp word [edi], 0
  je .aligment_middle_right
  cmp word [edi], 0xA
  je .aligment_middle_right
  cmp word [edi], DE_COMMAND
  je .command
  
  mov edx, dword [size_of_text]
  shl edx, 3 ;mul 8 - size of char in pixels
  add eax, edx
  add ecx, edx
  cmp eax, ebx
  ja .left ;this line is full of chars, no aligment needed
  
  add edi, 2
  jmp .calculate_aligment
  
  .command:
  cmp word [edi+2], 'l'
  je .aligment_middle_right
  cmp word [edi+2], 'm'
  je .aligment_middle_right
  cmp word [edi+2], 'r'
  je .aligment_middle_right
  cmp word [edi+2], '1'
  jb .if_change_size
  cmp word [edi+2], '9'
  ja .if_change_size
   mov eax, 0
   mov ax, word [edi+2]
   sub eax, '0'
   mov dword [size_of_text], eax
  .if_change_size:
  add edi, 4
  jmp .calculate_aligment
 .aligment_middle_right:
 cmp dword [de_aligment], DE_MIDDLE
 je .middle
 cmp dword [de_aligment], DE_RIGHT
 je .right
 
 .left:
 mov eax, dword [de_left_border_column]
 mov dword [de_draw_column], eax 
 jmp .done
 
 .middle:
 mov eax, dword [de_right_border_column]
 sub eax, dword [de_left_border_column]
 shr eax, 1 ;div 2
 shr ecx, 1 ;div 2
 sub eax, ecx
 mov dword [de_draw_column], eax
 jmp .done
 
 .right:
 mov eax, dword [de_right_border_column]
 sub eax, ecx
 dec eax
 mov dword [de_draw_column], eax
 
 .done:
 pop dword [size_of_text]
 mov eax, dword [de_draw_column]
 add eax, dword [de_left_border_start]
 mov dword [de_left_column_of_print], eax
 ret

document_editor_shift_text:
 mov esi, dword [de_pointer_end_of_text]
 sub esi, 2
 mov edi, esi
 add edi, eax
 mov ecx, edi
 sub ecx, dword [de_pointer]
 add dword [de_pointer_end_of_text], eax
 std
 rep movsb
 cld
 
 ret
 
document_editor_delete_text:
 mov edi, dword [de_pointer]
 mov esi, edi
 add esi, eax
 mov ecx, dword [de_pointer_end_of_text]
 sub ecx, dword [de_pointer]
 cmp ecx, 0
 je .done
 sub dword [de_pointer_end_of_text], eax
 cld
 rep movsb
 
 .done:
 ret

document_editor_convert_to_unicode:
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
 mov dword [document_editor_file_pointer], eax
 mov dword [de_pointer], eax
 add eax, 0x100000
 mov dword [document_editor_file_end], eax
 
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

document_editor_convert_to_utf8:
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
