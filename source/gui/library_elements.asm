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
