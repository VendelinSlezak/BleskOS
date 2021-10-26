;BleskOS

;;;;; Window ;;;;;

window_up_string_ptr dd 0
window_down_string_ptr dd 0
window_border_color dd 0

%macro DRAW_WINDOW 4
 mov dword [window_up_string_ptr], %1
 mov dword [window_down_string_ptr], %2
 mov dword [window_border_color], %3
 mov dword [color], %4
 call draw_window
%endmacro

draw_window:
 call clear_screen

 mov dword [cursor_line], 0
 mov dword [cursor_column], 0
 mov eax, dword [screen_x]
 mov dword [square_length], eax
 mov dword [square_heigth], 20
 mov eax, dword [window_border_color]
 mov dword [color], eax
 call draw_square

 SCREEN_Y_SUB eax, 20
 mov dword [cursor_line], eax
 call draw_square

 mov dword [cursor_line], 5
 mov dword [cursor_column], 8
 mov dword [color], BLACK
 mov esi, dword [window_up_string_ptr]
 call print

 SCREEN_Y_SUB eax, 15
 mov dword [cursor_line], eax
 mov dword [cursor_column], 8
 mov esi, dword [window_down_string_ptr]
 call print

 ret

;;;;; Text input ;;;;;

%define TI_NO_CURSOR 0xFFFFFFFF

text_input_pointer dd 0
text_input_length dd 0
text_input_cursor dd 0

draw_text_input:
 push dword [cursor_line]
 push dword [cursor_column]

 mov eax, dword [text_input_length]
 mov ebx, COLUMNSZ
 mul ebx
 add eax, 2
 mov dword [square_length], eax
 mov dword [square_heigth], LINESZ+1
 mov dword [color], WHITE
 call draw_square
 mov dword [color], BLACK
 call draw_empty_square

 add dword [cursor_line], 2
 inc dword [cursor_column]
 mov esi, dword [text_input_pointer]
 mov ecx, dword [text_input_length]
 .draw_char:
 push ecx
  mov dword [char_for_print], 0
  mov ax, word [esi]
  mov word [char_for_print], ax
  push esi
  call print_char
  pop esi
  add dword [cursor_column], COLUMNSZ
  add esi, 2
 pop ecx
 loop .draw_char
 pop dword [cursor_column]
 pop dword [cursor_line]

 cmp dword [text_input_cursor], TI_NO_CURSOR
 je .done

 push dword [cursor_line]
 push dword [cursor_column]
 mov eax, dword [text_input_cursor]
 mov ebx, COLUMNSZ
 mul ebx
 inc eax
 inc dword [cursor_line]
 add dword [cursor_column], eax
 mov dword [column_heigth], COLUMNSZ+2
 mov dword [color], 0x4E4E4E
 call draw_column
 pop dword [cursor_column]
 pop dword [cursor_line]

 .done:
 ret

text_input:
 mov dword [text_input_cursor], 0
 call draw_text_input
 call redraw_screen

 .text_input_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ENTER
  je .done

  cmp byte [key_code], KEY_LEFT
  je .key_left

  cmp byte [key_code], KEY_RIGHT
  je .key_right

  cmp byte [key_code], KEY_DELETE
  je .key_delete

  cmp byte [key_code], KEY_BACKSPACE
  je .key_backspace

  cmp word [key_unicode], 0
  jne .char
 jmp .text_input_halt

 .done:
 ret

 .key_left:
 cmp dword [text_input_cursor], 0
 je .text_input_halt

 dec dword [text_input_cursor]
 call draw_text_input
 call redraw_screen
 jmp .text_input_halt

 .key_right:
 mov eax, dword [text_input_length]
 cmp dword [text_input_cursor], eax
 je .text_input_halt

 inc dword [text_input_cursor]
 call draw_text_input
 call redraw_screen
 jmp .text_input_halt

 .key_delete:
 mov eax, dword [text_input_length]
 cmp dword [text_input_cursor], eax
 je .text_input_halt

 mov eax, dword [text_input_cursor]
 mov ebx, 2
 mul ebx
 add eax, dword [text_input_pointer]
 mov edi, eax
 add eax, 2
 mov esi, eax
 mov ecx, dword [text_input_length]
 sub ecx, dword [text_input_cursor]
 rep movsw

 call draw_text_input
 call redraw_screen
 jmp .text_input_halt

 .key_backspace:
 cmp dword [text_input_cursor], 0
 je .text_input_halt

 dec dword [text_input_cursor]

 mov eax, dword [text_input_cursor]
 mov ebx, 2
 mul ebx
 add eax, dword [text_input_pointer]
 mov edi, eax
 add eax, 2
 mov esi, eax
 mov ecx, dword [text_input_length]
 sub ecx, dword [text_input_cursor]
 rep movsw

 call draw_text_input
 call redraw_screen
 jmp .text_input_halt

 .char:
 mov eax, dword [text_input_length]
 cmp dword [text_input_cursor], eax
 je .text_input_halt
 mov eax, dword [text_input_pointer]
 mov ebx, dword [text_input_cursor]
 mov cx, word [key_unicode]
 mov word [eax+(ebx*2)], cx
 inc dword [text_input_cursor]
 call draw_text_input
 call redraw_screen
 jmp .text_input_halt

;;; Mouse move ;;;

mcursor_up_side dd 0
mcursor_down_side dd 0
mcursor_left_side dd 0
mcursor_right_side dd 0

move_mouse_cursor:
 call write_cursor_bg ;erase cursor from screen
 push dword [cursor_line]
  
 ;MOVE CURSOR HORIZONTAL
 mov ebx, dword [mcursor_right_side]
 mov ecx, dword [mcursor_left_side]
 cmp byte [usb_mouse_data+1], 0x7F
 ja .move_left
  
 ;move right
 mov eax, 0
 mov al, byte [usb_mouse_data+1]
 add dword [cursor_column], eax
  
 cmp dword [cursor_column], ebx
 jb .if_too_right
  mov dword [cursor_column], ebx
 .if_too_right:
 jmp .test_vertical_move
  
 .move_left:
 mov eax, 0xFF
 sub al, byte [usb_mouse_data+1]
 sub dword [cursor_column], eax
  
 cmp dword [cursor_column], ecx
 ja .if_too_left
  mov dword [cursor_column], ecx
 .if_too_left:
  
 ;MOVE CURSOR VERTICAL
 .test_vertical_move:
 mov edx, dword [mcursor_down_side]
 cmp byte [usb_mouse_data+2], 0x7F
 ja .move_up
  
 ;move down
 mov eax, 0
 mov al, byte [usb_mouse_data+2]
 add dword [cursor_line], eax
  
 cmp dword [cursor_line], edx
 jb .if_too_down
  mov dword [cursor_line], edx
 .if_too_down:
 jmp .draw_cursor
  
 .move_up:
 mov eax, 0xFF
 sub al, byte [usb_mouse_data+2]
 mov ebx, dword [cursor_line]
 sub ebx, eax
 cmp ebx, dword [screen_y]
 ja .too_up
 cmp ebx, dword [mcursor_up_side]
 jb .too_up
 sub dword [cursor_line], eax
 jmp .draw_cursor
 
 .too_up:
 mov dword [cursor_line], 20 
 
 .draw_cursor: 
 mov eax, dword [cursor_line]
 mov dword [first_redraw_line], eax
 call read_cursor_bg
 call draw_cursor
 
 mov dword [how_much_lines_redraw], 11
 call redraw_lines_screen
  
 pop dword [first_redraw_line]
 call redraw_lines_screen ;erase old cursor from screen
 
 ret
