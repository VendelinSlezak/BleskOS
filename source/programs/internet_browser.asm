;BleskOS

internet_browser_up_str db 'Internet browser', 0
internet_browser_down_str db '[F2] Open file', 0
internet_browser_file_pointer dd 0
internet_browser_file_size dd 0

internet_browser:
 DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
 cmp dword [internet_browser_file_pointer], 0
 jne .draw_html
 PRINT 'This is now only HTML viewer', only_html_viewer_str, 20+LINESZ*1, COLUMNSZ*1
 call redraw_screen
 
 .halt:
  call wait_for_keyboard
  
  cmp byte [key_code], KEY_ESC
  je main_window

  cmp byte [key_code], KEY_F2
  je .open
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [key_code], KEY_HOME
  je .key_home
 jmp .halt
 
 .key_up:
  cmp dword [html_start_of_screen], 0
  je .halt
  sub dword [html_start_of_screen], 20
  sub dword [html_end_of_screen], 20
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call redraw_screen
 jmp .halt
 
 .key_down:
  cmp dword [html_start_of_screen], 10000
  je .halt
  add dword [html_start_of_screen], 20
  add dword [html_end_of_screen], 20
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call redraw_screen
 jmp .halt
 
 .key_home:
  mov dword [html_start_of_screen], 0
  SCREEN_Y_SUB eax, 40
  mov dword [html_end_of_screen], eax
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call redraw_screen
 jmp .halt
 
 .open:
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je internet_browser
  
  cmp dword [file_type], 'TXT'
  je .open_html
  cmp dword [file_type], 'txt'
  je .open_html
  cmp dword [file_type], 'HTM'
  je .open_html
  cmp dword [file_type], 'htm'
  je .open_html
  
  call release_memory
 jmp .halt

 .open_html:
  ;release memory of previous file
  cmp dword [internet_browser_file_pointer], 0
  je .if_opened_file
   push dword [allocated_memory_pointer]
   push dword [allocated_size]
   mov eax, dword [internet_browser_file_pointer]
   mov dword [allocated_memory_pointer], eax
   mov eax, dword [internet_browser_file_size]
   mov dword [allocated_size], eax
   call release_memory
   pop dword [allocated_size]
   pop dword [allocated_memory_pointer]
  .if_opened_file:
  
  call internet_browser_convert_to_unicode
  
  mov eax, dword [file_memory]
  mov dword [html_memory], eax
  mov dword [html_start_of_screen], 0
  SCREEN_Y_SUB eax, 40
  mov dword [html_end_of_screen], eax
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  mov dword [html_debug], 0
  call draw_html_code
  call redraw_screen
 jmp .halt
 
 .draw_html:
  mov eax, dword [internet_browser_file_pointer]
  mov dword [html_memory], eax
  
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  PRINT 'Rendering loaded HTML...', rendering_html_str, 20+LINESZ*1, COLUMNSZ*1
  call redraw_screen
  
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call redraw_screen
 jmp .halt
 
internet_browser_convert_to_unicode:
 push dword [allocated_memory_pointer]
 push dword [allocated_size]
 mov esi, dword [allocated_memory_pointer]
 push esi
 
 mov dword [allocated_size], 1
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov dword [internet_browser_file_pointer], eax
 mov dword [file_memory], eax
 mov eax, dword [allocated_size]
 mov dword [internet_browser_file_size], eax
 
 mov edi, dword [allocated_memory_pointer]
 mov eax, 0
 mov ecx, 0x100000
 rep stosb
 
 pop esi
 mov edi, dword [allocated_memory_pointer]
 .convert_char:
  cmp word [esi], 0
  je .done
  
  mov eax, 0
  mov al, byte [esi]
  test al, 0x80
  jz .ascii_char
  
  and al, 0xE0
  cmp al, 0xC0
  je .utf_2_byte
  
  jmp .unicode
  
  .ascii_char:
   mov word [edi], ax
   add edi, 2
   inc esi
  jmp .convert_char
  
  .utf_2_byte:
   mov al, byte [esi]
   mov bx, 0
   mov bl, byte [esi+1]
   and al, 0x1F
   shl ax, 6
   and bl, 0x3F
   or ax, bx
   mov word [edi], ax
   add esi, 2
   add edi, 2
  jmp .convert_char
  
  .unicode:
   mov ax, word [esi]
   mov word [edi], ax
   add esi, 2
   add edi, 2
  jmp .convert_char
  
  .done:  
  pop dword [allocated_size]
  pop dword [allocated_memory_pointer]
  call release_memory
  ret
