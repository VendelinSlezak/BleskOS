;BleskOS

init_memory_of_programs:
 ;TEXT EDITOR
 mov dword [allocated_size], 1 ;one MB
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov edi, eax
 
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

 ret
