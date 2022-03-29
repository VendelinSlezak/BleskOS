;BleskOS

%macro IB_MESSAGE 2
 mov eax, dword [screen_x]
 DRAW_SQUARE 20+LINESZ*1, 0, eax, LINESZ, WHITE
 mov dword [color], BLACK
 PRINT %1, %2, 20+LINESZ*1, COLUMNSZ*1
 REDRAW_LINES_SCREEN 20+LINESZ*1, LINESZ
%endmacro

internet_browser_up_str db 'Internet browser', 0
internet_browser_down_str db '[F1] Clear URL [F2] Back [F3] Open file [F4] Close file [up/down] Move page', 0
internet_browser_file_pointer dd 0
internet_browser_file_size dd 0
internet_browser_url times 202 dw 0
internet_browser_previous_url_1 times 202 dw 0
internet_browser_previous_url_2 times 202 dw 0
internet_browser_previous_url_3 times 202 dw 0
internet_browser_previous_url_4 times 202 dw 0
ib_url_history_state dd 0
ib_url_cursor dd 0
ib_file_opened dd 0

ib_mouse_line dd 20
ib_mouse_column dd 0
ib_mouse_halt dd 0

internet_browser:
 mov dword [ib_mouse_halt], 1
 DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
 
 cmp dword [internet_connection_status], 1
 je .connected_to_internet
  cmp dword [internet_browser_file_pointer], 0
  jne .draw_html
  call redraw_screen
  IB_MESSAGE 'You are not connected to internet', no_internet_connection_str
  .no_connection_halt:
   call wait_for_keyboard
   
   cmp byte [key_code], KEY_ESC
   je main_window
   
   cmp byte [key_code], KEY_F2
   je .open
  jmp .no_connection_halt
 .connected_to_internet:
 
 mov dword [mcursor_up_side], 20
 mov dword [mcursor_left_side], 0
 SCREEN_Y_SUB eax, 20
 mov dword [mcursor_down_side], eax
 mov eax, dword [screen_x]
 mov dword [mcursor_right_side], eax
 
 call internet_browser_redraw_url
 cmp dword [internet_browser_file_pointer], 0
 jne .draw_html
 call redraw_screen
 
 .halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F3
  je .open
  
  cmp byte [key_code], KEY_F4
  je .close
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [usb_mouse_data], 0
  jne .mouse_event
  
 cmp dword [ib_file_opened], 1
 je .halt
  
  cmp byte [key_code], KEY_F1
  je .clear_url
  
  cmp byte [key_code], KEY_F2
  je .back
  
  cmp byte [key_code], KEY_HOME
  je .key_home
  
  cmp byte [key_code], KEY_ENTER
  je .key_enter
  
  cmp byte [key_code], KEY_DELETE
  je .key_backspace
  
  cmp byte [key_code], KEY_BACKSPACE
  je .key_backspace
  
  cmp byte [key_code], KEY_RIGHT
  je .key_right
  cmp byte [key_code], KEY_LEFT
  je .key_left
  cmp word [key_unicode], 0
  jne .add_char
 jmp .halt
 
 .clear_url:
  mov edi, internet_browser_url
  mov ecx, 100
  mov eax, 0
  rep stosw
  
  mov dword [ib_url_cursor], 0
 jmp .redraw_url
 
 .key_up:
  cmp dword [html_start_of_screen], 0
  je .halt
  sub dword [html_start_of_screen], 20
  sub dword [html_end_of_screen], 20
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call internet_browser_redraw_url
  
  mov eax, dword [ib_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ib_mouse_column]
  mov dword [cursor_column], eax
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .redraw_url
 
 .key_down:
  cmp dword [html_start_of_screen], 10000
  je .halt
  add dword [html_start_of_screen], 20
  add dword [html_end_of_screen], 20
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call internet_browser_redraw_url
  
  mov eax, dword [ib_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ib_mouse_column]
  mov dword [cursor_column], eax
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .redraw_url
 
 .key_right:
  mov eax, dword [ib_url_cursor]
  shl eax, 1
  add eax, internet_browser_url
  cmp word [eax], 0
  je .halt
  
  inc dword [ib_url_cursor]
 jmp .redraw_url
 
 .key_left:
  cmp dword [ib_url_cursor], 0
  je .halt
  
  dec dword [ib_url_cursor]
 jmp .redraw_url
 
 .key_backspace:
  cmp dword [ib_url_cursor], 0
  je .halt
  
  dec dword [ib_url_cursor]
 jmp .key_delete
 
 .key_delete:
  mov eax, dword [ib_url_cursor]
  shl eax, 1 ;mul 2
  add eax, internet_browser_url
  mov edi, eax
  mov esi, eax
  add esi, 2
  mov ecx, 100
  sub ecx, dword [ib_url_cursor]
  rep movsb
 jmp .redraw_url
 
 .add_char:
  mov eax, dword [ib_url_cursor]
  shl eax, 1 ;mul 2
  add eax, internet_browser_url
  mov bx, word [key_unicode]
  cmp bx, ' '+1
  jb .halt
  cmp bx, 'z'
  ja .halt
  
  mov edi, internet_browser_url+200
  mov esi, internet_browser_url+198
  mov ecx, 100
  sub ecx, dword [ib_url_cursor]
  std
  rep movsw
  cld
  
  mov word [eax], bx
  inc dword [ib_url_cursor]
 
 .redraw_url:
  call internet_browser_redraw_url
 jmp .halt
 
 .key_home:
  mov dword [html_start_of_screen], 0
  SCREEN_Y_SUB eax, 40
  mov dword [html_end_of_screen], eax
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call internet_browser_redraw_url
  call redraw_screen
 jmp .halt
 
 .back:
  cmp word [internet_browser_previous_url_2], 0
  je .halt

  mov esi, internet_browser_previous_url_2
  mov edi, internet_browser_url
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_2
  mov edi, internet_browser_previous_url_1
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_3
  mov edi, internet_browser_previous_url_2
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_4
  mov edi, internet_browser_previous_url_3
  mov ecx, 100
  rep movsw
  
  mov edi, internet_browser_previous_url_4
  mov ecx, 100
  mov eax, 0
  rep stosw
  
  call internet_browser_move_url_cursor_to_end
  call internet_browser_redraw_url
 jmp .transfer_page
 
 .move:
 
 .key_enter:
  cmp word [internet_browser_url], 0
  je .halt
  cmp word [internet_browser_url], '/'
  je .halt
  
  mov esi, internet_browser_previous_url_3
  mov edi, internet_browser_previous_url_4
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_2
  mov edi, internet_browser_previous_url_3
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_1
  mov edi, internet_browser_previous_url_2
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_url
  mov edi, internet_browser_previous_url_1
  mov ecx, 100
  rep movsw

 .transfer_page:
  cmp word [internet_browser_url], 0
  je .halt
  cmp word [internet_browser_url], '/'
  je .halt
 
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call internet_browser_redraw_url
  IB_MESSAGE 'Searching for IP address of website...', searching_for_ip_address_st
  call redraw_screen
  
  mov edi, url_for_dns
  mov ecx, 256
  mov eax, 0
  rep stosb
  mov edi, tcp_path_to_file
  mov ecx, 256
  mov eax, 0
  rep stosb
  
  mov ecx, 100
  mov esi, internet_browser_url
  mov edi, url_for_dns
  .create_dns:
   cmp word [esi], 0
   je .send_dns
   cmp word [esi], '/'
   je .send_dns
   mov al, byte [esi]
   mov byte [edi], al
   add esi, 2
   inc edi
  loop .create_dns
  .send_dns:
  mov dword [type_of_received_packet], 0
  mov dword [dns_report], 0
  call get_ip_address_of_url
  
  mov dword [ticks], 0
  .wait_for_dns_response:
   cmp dword [dns_report], 1
   je .dns_response
   cmp dword [dns_report], 2
   je .dns_response_no_ip
   hlt
  cmp dword [ticks], 1000
  jb .wait_for_dns_response
  .dns_response_no_ip:
  IB_MESSAGE 'This website do not exist', this_website_do_not_exist_str
  mov dword [ib_mouse_halt], 1
  jmp .halt
  
  .dns_response:
  IB_MESSAGE 'Estabilishing connection with website...', estabilishing_connection_with_website_str
  mov dword [tcp_communication_type], NO_TCP
  call create_tcp_connection
  mov dword [ticks], 0
  .wait_for_tcp_handshake:
   cmp dword [tcp_communication_type], TCP_HANDSHAKE_RECEIVED
   je .transfer_main_html_file
   hlt
  cmp dword [ticks], 1000
  jb .wait_for_tcp_handshake
  IB_MESSAGE 'Server is not responding', server_is_not_responding
  mov dword [ib_mouse_halt], 1
  jmp .halt
  
  .transfer_main_html_file:
  IB_MESSAGE 'Transferring HTML file...', transferring_html_file_str
  mov dword [allocated_size], 1
  call allocate_memory
  mov eax, dword [allocated_memory_pointer]
  mov dword [http_file_pointer], eax
  
  mov edi, eax
  mov eax, 0
  mov ecx, 0x100000
  rep stosb
  
  mov byte [tcp_path_to_file], '/'
  mov ecx, 100
  mov esi, internet_browser_url
  mov edi, tcp_path_to_file
  .skip_to_tcp:
   cmp word [esi], 0
   je .transfer_tcp
   cmp word [esi], '/'
   je .create_tcp
   add esi, 2
  jmp .skip_to_tcp
  .create_tcp:
   cmp word [esi], 0
   je .transfer_tcp
   mov al, byte [esi]
   mov byte [edi], al
   add esi, 2
   inc edi
  loop .create_tcp
  .transfer_tcp:
  
  mov eax, 0
  call tcp_transfer_file
  mov dword [ticks], 0
  .wait_for_tcp_file:
   cmp dword [tcp_communication_type], TCP_FINALIZED
   je .show_html   
   hlt
   cmp eax, dword [tcp_file_transferred_length]
   je .if_transferred_length_change
    mov eax, dword [tcp_file_transferred_length]
    push eax
    DRAW_SQUARE 20+LINESZ, COLUMNSZ*28, COLUMNSZ*4, LINESZ, WHITE
    pop eax
    mov dword [color], BLACK
    mov dword [var_print_value], eax
    push eax
    call print_var
    REDRAW_LINES_SCREEN 20+LINESZ, LINESZ
    pop eax
    mov dword [ticks], 0 ;server is not responding if nothing arrived during 3 seconds
   .if_transferred_length_change:
  cmp dword [ticks], 3000
  jb .wait_for_tcp_file
  IB_MESSAGE 'File was not transferred', file_was_not_transferred
  call release_memory
  mov dword [ib_mouse_halt], 1
  jmp .halt
  
  .show_html:
  mov dword [ib_file_opened], 0
  mov eax, dword [allocated_memory_pointer]
  mov dword [file_memory], eax
  jmp .open_html
 jmp .halt
 
 .close:
  mov dword [ib_file_opened], 0
  mov eax, dword [internet_browser_file_pointer]
  mov dword [allocated_memory_pointer], eax
  mov eax, dword [internet_browser_file_size]
  mov dword [allocated_size], eax
  call release_memory
  
  mov dword [internet_browser_file_pointer], 0
 jmp internet_browser
 
 .open:
  mov dword [fd_file_type_1], 'htm'
  mov dword [fd_file_type_2], 'HTM'
  mov dword [fd_file_type_3], 'txt'
  mov dword [fd_file_type_4], 'TXT'
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je internet_browser
  
  mov dword [ib_file_opened], 1
  cmp dword [file_type], 'TXT'
  je .open_html
  cmp dword [file_type], 'txt'
  je .open_html
  cmp dword [file_type], 'HTM'
  je .open_html
  cmp dword [file_type], 'htm'
  je .open_html
  mov dword [ib_file_opened], 0
  
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
  call transform_html_code
  call draw_html_code
  call internet_browser_redraw_url
  
  mov eax, dword [ib_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ib_mouse_column]
  mov dword [cursor_column], eax
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
  
  mov dword [ib_mouse_halt], 0
 jmp .redraw_url
 
 .draw_html:
  mov eax, dword [internet_browser_file_pointer]
  mov dword [html_memory], eax
  
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  PRINT 'Rendering loaded HTML...', rendering_html_str, 20+LINESZ*1, COLUMNSZ*1
  call internet_browser_redraw_url
  call redraw_screen
  
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call draw_html_code
  call internet_browser_redraw_url
  
  cmp dword [internet_connection_status], 1
  jne .draw_html_not_connected_to_internet
  
  mov eax, dword [ib_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ib_mouse_column]
  mov dword [cursor_column], eax
  call read_cursor_bg
  call draw_cursor
 
  call redraw_screen
  
  mov dword [ib_mouse_halt], 0
 jmp .halt
 
 .draw_html_not_connected_to_internet:
  call redraw_screen
  mov dword [ib_mouse_halt], 0
 jmp .no_connection_halt
 
 .mouse_event:
  cmp dword [internet_browser_file_pointer], 0
  je .halt
  cmp dword [ib_mouse_halt], 1
  je .halt
  
  mov eax, dword [ib_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ib_mouse_column]
  mov dword [cursor_column], eax
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [ib_mouse_line], eax
  mov eax, dword [cursor_column]
  mov dword [ib_mouse_column], eax
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  cmp byte [usb_mouse_data+3], 0xFF
  je .key_up
  cmp byte [usb_mouse_data+3], 0x1
  je .key_down
 jmp .halt
 
 .mouse_click:
  cmp dword [internet_connection_status], 1
  jne .no_connection_halt
  cmp dword [ib_file_opened], 1
  je .halt
 
  call draw_html_code
  cmp dword [html_cursor_on_url], 0
  je .halt
  
  mov esi, html_url
  mov edi, internet_browser_url
  cmp dword [esi], 'java'
  je .halt
  cmp byte [esi], '/'
  je .mouse_click_copy_same_page_url
  cmp word [esi], '..'
  je .mouse_click_copy_same_page_url_double_comma
  cmp byte [esi], '.'
  je .mouse_click_copy_same_page_url_comma
  cmp dword [esi], 'http'
  je .mouse_click_http_test
  
  mov esi, html_url+200
  mov edi, html_url+201
  mov ecx, 201
  std
  rep movsb
  cld
  mov byte [html_url], '/'
  mov esi, html_url
  mov edi, internet_browser_url
  jmp .mouse_click_copy_same_page_url
  
  .mouse_click_http_test:
   mov eax, 0
   mov ecx, 100
   rep stosw
   mov edi, internet_browser_url
     
  cmp dword [esi+4], 's://'
  je .mouse_click_https
  add esi, 7
  jmp .mouse_click_copy_new_url
  .mouse_click_https:
  add esi, 8
  .mouse_click_copy_new_url:
   cmp byte [esi], 0
   je .mouse_click_done
   mov ax, 0
   mov al, byte [esi]
   mov word [edi], ax
   inc esi
   add edi, 2
  jmp .mouse_click_copy_new_url
  
  .mouse_click_copy_same_page_url_double_comma:
  inc esi
  .mouse_click_copy_same_page_url_comma:
  inc esi
  .mouse_click_copy_same_page_url:
  
  .mouse_click_skip_www_part:
   cmp word [edi], 0
   je .mouse_click_clear_url
   cmp word [edi], '/'
   je .mouse_click_clear_url
   add edi, 2
  jmp .mouse_click_skip_www_part
  
  .mouse_click_clear_url:
  cmp edi, internet_browser_url
  je .halt ;no url
  push edi
  mov eax, 0
  mov ecx, 100
  rep stosw
  pop edi
  
  .mouse_click_copy_url:
   cmp byte [esi], 0
   je .mouse_click_done
   mov ax, 0
   mov al, byte [esi]
   mov word [edi], ax
   inc esi
   add edi, 2
  jmp .mouse_click_copy_url
  
  .mouse_click_done:
  mov word [edi], 0
  
  DRAW_WINDOW internet_browser_up_str, internet_browser_down_str, 0x17A4AD, WHITE ;light blue
  call internet_browser_redraw_url
  call redraw_screen
  
  cmp word [internet_browser_url], 0
  je .halt
  cmp word [internet_browser_url], '/'
  je .halt
  cmp word [internet_browser_url], '.'
  je .halt
  
  mov esi, internet_browser_previous_url_3
  mov edi, internet_browser_previous_url_4
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_2
  mov edi, internet_browser_previous_url_3
  mov ecx, 100
  rep movsw
  
  mov esi, internet_browser_previous_url_1
  mov edi, internet_browser_previous_url_2
  mov ecx, 100
  rep movsw

  mov esi, internet_browser_url
  mov edi, internet_browser_previous_url_1
  mov ecx, 100
  rep movsw

  call internet_browser_move_url_cursor_to_end
 jmp .transfer_page
 
  .mouse_click_print:
  call internet_browser_redraw_url
  call redraw_screen
 jmp .halt
 
internet_browser_redraw_url:
 cmp dword [ib_file_opened], 1
 je .done
 
 mov dword [cursor_line], 5
 SCREEN_X_SUB eax, COLUMNSZ*101
 mov dword [cursor_column], eax
 mov dword [text_input_pointer], internet_browser_url
 mov dword [text_input_length], 100
 mov eax, dword [ib_url_cursor]
 mov dword [text_input_cursor], eax
 call draw_text_input
 REDRAW_LINES_SCREEN 5, 10
 
 .done:
 ret
 
internet_browser_move_url_cursor_to_end:
 mov eax, internet_browser_url
 
 .move_cursor:
  cmp word [eax], 0
  je .done
  add eax, 2
 jmp .move_cursor
 
 .done:
 sub eax, internet_browser_url
 shr eax, 1 ;div 2
 mov dword [ib_url_cursor], eax
 ret
 
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
