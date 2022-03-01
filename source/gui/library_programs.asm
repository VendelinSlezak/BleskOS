;BleskOS

init_memory_of_programs:
 ;FILE DIALOG
 mov eax, dword [screen_y]
 sub eax, LINESZ*6
 mov ebx, LINESZ
 mov edx, 0
 div ebx
 mov dword [fd_files_on_screen], eax
 
 mov dword [fd_first_file], 0
 mov dword [fd_selected_file], 0
 mov dword [fd_highlighted_file], 0
 mov dword [fd_loaded_folder], 0
 
 ;TEXT EDITOR
 mov dword [allocated_size], 1 ;one MB
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov edi, eax
 
 mov dword [text_editor_file_pointer], eax
 mov dword [text_editor_mem], eax
 mov dword [text_editor_first_line_mem], eax
 mov dword [te_pointer], eax
 mov dword [te_pointer_end], eax
 mov dword [te_cursor_offset], 0
 mov dword [te_length_of_text], 0
 add eax, 0x100000
 mov dword [text_editor_end_mem], eax
 mov dword [te_draw_line], 0
 mov dword [te_draw_column], 0
 
 ;clear memory of text editor
 mov eax, 0
 mov ecx, 0x100000
 rep stosb
 
 ;calculate screen size
 mov eax, dword [screen_x]
 mov ebx, COLUMNSZ
 mov edx, 0
 div ebx
 mov dword [te_max_column], eax
 mov eax, dword [screen_y]
 sub eax, 40
 mov ebx, LINESZ
 mov edx, 0
 div ebx
 mov dword [te_max_line], eax
 
 ;GRAPHIC EDITOR
 mov dword [allocated_size], 2
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov dword [ge_image_pointer], eax
 mov edi, eax
 mov eax, WHITE
 mov ecx, 640*480
 rep stosd ;clear image
 
 ;DOCUMENT EDITOR
 mov dword [allocated_size], 1
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov dword [document_editor_file_pointer], eax
 mov dword [de_pointer], eax
 mov dword [de_pointer_end_of_text], eax
 add eax, 0x100000
 mov dword [document_editor_file_end], eax
 ;clear memory of document_editor
 sub eax, 0x100000
 mov edi, eax
 mov eax, 0
 mov ecx, 0x100000
 rep stosb
 
 mov eax, dword [screen_y]
 sub eax, 40
 mov dword [de_last_show_line], eax
 
 mov eax, dword [screen_x]
 sub eax, 600
 shr eax, 1 ;div 2
 mov dword [de_left_border_start], eax

 ret
