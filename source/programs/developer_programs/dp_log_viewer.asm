;BleskOS

log_viewer_pointer dd MEMORY_LOG

log_viewer:
 CLEAR_SCREEN 0x00F580
 mov dword [color], BLACK
 mov dword [cursor_line], 10
 mov dword [cursor_column], 10

 SCREEN_Y_SUB eax, 20
 mov ebx, 10
 mov edx, 0
 div ebx
 mov ecx, eax ;number of lines
 mov esi, dword [log_viewer_pointer]
 .print_log_char:
  mov al, byte [esi]
  cmp al, 0 ;end of string
  je .redraw
  cmp al, 0xA ;end of line
  jne .if_end_of_line
   add dword [cursor_line], 10
   mov dword [cursor_column], 10
   inc esi
   dec ecx
   cmp ecx, 0
   je .redraw
   jmp .print_log_char
  .if_end_of_line:

  mov dword [char_for_print], 0
  mov byte [char_for_print], al
  push ecx
  push esi
  call print_char
  pop esi
  pop ecx
  add dword [cursor_column], 8 ;position of next char
  inc esi
 jmp .print_log_char

 .redraw:
 call redraw_screen
 
 .halt:
  call wait_for_keyboard
  
  cmp byte [key_code], KEY_ESC
  je developer_zone
  
  cmp byte [key_code], KEY_UP
  jne .if_key_up
   cmp dword [log_viewer_pointer], MEMORY_LOG
   je .halt
   
   mov esi, dword [log_viewer_pointer]
   dec esi
   cmp byte [esi], 0xA
   jne .move_up_line
    dec esi
   .move_up_line:
    cmp esi, MEMORY_LOG
    je .move_up_done
    dec esi
    cmp byte [esi], 0xA
    jne .if_move_up_enter
     inc esi
     jmp .move_up_done
    .if_move_up_enter:
   jmp .move_up_line
    
   .move_up_done:
   mov dword [log_viewer_pointer], esi
   jmp log_viewer
  .if_key_up:
  
  cmp byte [key_code], KEY_DOWN
  jne .if_key_down
   cmp dword [log_viewer_pointer], MEMORY_LOG+0xFFFF0
   ja .halt

   mov esi, dword [log_viewer_pointer]
   .move_down_line:
    cmp byte [esi], 0
    je .move_down_done
    cmp esi, MEMORY_LOG+0xFFFF0
    je .move_down_done
    inc esi
    cmp byte [esi], 0xA
    jne .if_move_down_enter
     inc esi
     jmp .move_down_done
    .if_move_down_enter:
   jmp .move_down_line
    
   .move_down_done:
   mov dword [log_viewer_pointer], esi
   jmp log_viewer
  .if_key_down:
 jmp .halt
