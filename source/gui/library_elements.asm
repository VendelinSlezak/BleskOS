;BleskOS

;;;;; Window ;;;;;

window_up_string_ptr dd 0
window_down_string_ptr dd 0
window_border_color dd 0

%macro DRAW_WINDOW 4
 mov dword [size_of_text], 1
 mov dword [type_of_text], PLAIN
 mov dword [window_up_string_ptr], %1
 mov dword [window_down_string_ptr], %2
 mov dword [window_border_color], %3
 mov dword [color], %4
 call draw_window
%endmacro

%macro DRAW_WINDOW_BORDERS 3
 mov dword [size_of_text], 1
 mov dword [type_of_text], PLAIN
 mov dword [window_up_string_ptr], %1
 mov dword [window_down_string_ptr], %2
 mov dword [window_border_color], %3
 call draw_window_borders
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
 
draw_window_borders:
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
 
;;;;; Window for choosing color ;;;;;

color_window_color dd 0x000000
color_window_mc_line dd 0
color_window_mc_column dd 0

draw_color_window:
 mov dword [message_window_length], 0xFF*2+60
 mov dword [message_window_heigth], 0xFF+60
 call show_empty_message_window
 
 mov eax, dword [screen_x_center]
 sub eax, 0xFF
 mov ebx, dword [screen_y_center]
 add ebx, 128+11
 mov dword [color], BLACK
 PRINT '[esc] Back [enter] Choose color', text_string, ebx, eax
 
 mov eax, dword [screen_x_center]
 sub eax, 0xFF
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128
 mov dword [cursor_line], eax
 
 CALCULATE_CURSOR_POSITION
 
 ;draw square with colors
 mov ebx, dword [color_window_color]
 and ebx, 0xFF0000
 mov ecx, 256
 .draw_lines:
 push ecx
 
  mov ecx, 256
  .draw_columns:
   mov dword [eax], ebx
   inc ebx
   add eax, 4
  loop .draw_columns

 pop ecx
 add eax, dword [screen_pixels_per_line]
 sub eax, 256*4
 dec ecx
 cmp ecx, 0
 jne .draw_lines
 
 ;draw scrollbars
 mov dword [line_length], 20
 mov eax, dword [color_window_color]
 and eax, 0x00FFFF
 mov dword [color], eax

 mov eax, dword [screen_x_center]
 add eax, 20
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128
 mov dword [cursor_line], eax
 mov ecx, 256
 .draw_red_scrollbar:
  push ecx
  call draw_line
  pop ecx
  inc dword [cursor_line]
  add dword [color], 0x010000
 loop .draw_red_scrollbar
 
 mov dword [square_length], 20
 mov dword [square_heigth], 0xFF+1
 mov dword [color], 0xBBBBBB
 
 mov eax, dword [screen_x_center]
 sub eax, 0xFF+20
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128
 mov dword [cursor_line], eax
 call draw_square
 
 mov dword [square_length], 0xFF+1
 mov dword [square_heigth], 20
 mov eax, dword [screen_x_center]
 sub eax, 0xFF
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128+20
 mov dword [cursor_line], eax
 call draw_square
 
 ;draw color position on scrollbars
 mov dword [line_length], 20
 mov dword [color], BLACK
 
 mov eax, dword [screen_x_center]
 add eax, 20
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128
 mov ebx, dword [color_window_color]
 shr ebx, 16
 add eax, ebx
 mov dword [cursor_line], eax
 call draw_line
 
 mov eax, dword [screen_x_center]
 sub eax, 0xFF+20
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128
 mov ebx, dword [color_window_color]
 shr ebx, 8
 and ebx, 0xFF
 add eax, ebx
 mov dword [cursor_line], eax
 call draw_line
 
 mov dword [column_heigth], 20
 mov eax, dword [screen_x_center]
 sub eax, 0xFF
 mov ebx, dword [color_window_color]
 and ebx, 0xFF
 add eax, ebx
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128+20
 mov dword [cursor_line], eax
 call draw_column
 
 ;draw actual color
 mov eax, dword [color_window_color]
 mov dword [color], eax
 mov dword [hex_print_value], eax
 
 mov dword [square_length], 50
 mov dword [square_heigth], 25
 mov eax, dword [screen_x_center]
 add eax, 60
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 sub eax, 128
 mov dword [cursor_line], eax
 call draw_square
 
 mov dword [color], BLACK
 add dword [cursor_column], 60
 add dword [cursor_line], 9
 call print_hex
 
 ret
 
color_window:
 call draw_color_window
 
 mov eax, dword [screen_x_center]
 mov dword [color_window_mc_column], eax
 mov dword [cursor_column], eax
 mov eax, dword [screen_y_center]
 mov dword [color_window_mc_line], eax
 mov dword [cursor_line], eax
 call read_cursor_bg
 call draw_cursor
 call redraw_screen
 
 mov eax, dword [screen_y_center]
 sub eax, 128+30
 mov dword [mcursor_up_side], eax
 add eax, 256+60
 mov dword [mcursor_down_side], eax
 mov eax, dword [screen_x_center]
 sub eax, 256+30
 mov dword [mcursor_left_side], eax
 add eax, 512+60
 mov dword [mcursor_right_side], eax
 
 .halt:
  call wait_for_usb_mouse
  
  cmp dword [key_code], KEY_ESC
  je .exit
  cmp dword [key_code], KEY_ENTER
  je .choose_color
  
  cmp byte [usb_mouse_data], 0
  je .halt
  
  ;move mouse cursor
  mov eax, dword [color_window_mc_line]
  mov dword [cursor_line], eax
  mov eax, dword [color_window_mc_column]
  mov dword [cursor_column], eax
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [color_window_mc_line], eax
  mov eax, dword [cursor_column]
  mov dword [color_window_mc_column], eax
  
  ;test mouse_click
  cmp dword [usb_mouse_dnd], 0x1
  je .click
  cmp dword [usb_mouse_dnd], 0x2
  je .click
  jmp .halt
  
  .click:  
  mov eax, dword [screen_y_center]
  sub eax, 128
  mov ebx, dword [screen_y_center]
  add ebx, 128
  mov ecx, dword [screen_x_center]
  sub ecx, 0xFF
  mov edx, dword [screen_x_center]
  TEST_CLICK_ZONE_WITH_JUMP click_on_color_square, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_square
  
  mov eax, dword [screen_y_center]
  sub eax, 127
  mov ebx, dword [screen_y_center]
  add ebx, 128
  mov ecx, dword [screen_x_center]
  add ecx, 20
  mov edx, dword [screen_x_center]
  add edx, 40
  TEST_CLICK_ZONE_WITH_JUMP click_on_red, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_red
  sub eax, 20
  sub ebx, 0xFF
  TEST_CLICK_ZONE_WITH_JUMP click_on_red_0, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_red_0
  add eax, 0xFF+20
  add ebx, 0xFF+20
  TEST_CLICK_ZONE_WITH_JUMP click_on_red_256, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_red_256
  
  mov eax, dword [screen_y_center]
  sub eax, 127
  mov ebx, dword [screen_y_center]
  add ebx, 128
  mov ecx, dword [screen_x_center]
  sub ecx, 0xFF+20
  mov edx, dword [screen_x_center]
  sub edx, 0xFF
  TEST_CLICK_ZONE_WITH_JUMP click_on_green, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_green
  sub eax, 10
  sub ebx, 0xFF
  sub edx, 10
  TEST_CLICK_ZONE_WITH_JUMP click_on_green_0, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_green_0
  add eax, 0xFF+10
  add ebx, 0xFF+20
  add edx, 10
  TEST_CLICK_ZONE_WITH_JUMP click_on_green_256, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_green_256
  
  mov eax, dword [screen_y_center]
  sub eax, 128+20
  mov ebx, dword [screen_y_center]
  sub ebx, 128
  mov ecx, dword [screen_x_center]
  sub ecx, 0xFF
  mov edx, dword [screen_x_center]
  TEST_CLICK_ZONE_WITH_JUMP click_on_blue, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_blue
  sub ecx, 10
  sub edx, 0xFF
  sub ebx, 10
  TEST_CLICK_ZONE_WITH_JUMP click_on_blue_0, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_blue_0
  add ecx, 0xFF+10
  add edx, 0xFF+20
  add ebx, 10
  TEST_CLICK_ZONE_WITH_JUMP click_on_blue_256, color_window_mc_line, color_window_mc_column, eax, ebx, ecx, edx, .click_color_blue_256
 jmp .halt
 
 .click_color_square:
  mov eax, dword [color_window_mc_column]
  sub eax, dword [screen_y_center]
  add eax, 127
  and eax, 0xFF
  and dword [color_window_color], 0xFFFF00
  or dword [color_window_color], eax
  
  mov eax, dword [color_window_mc_line]
  sub eax, dword [screen_x_center]
  add eax, 255
  and eax, 0xFF
  shl eax, 8
  and dword [color_window_color], 0xFF00FF
  or dword [color_window_color], eax
 jmp .redraw
 
 .click_color_red:
  mov eax, dword [color_window_mc_line]
  sub eax, dword [screen_x_center]
  add eax, 255
  and eax, 0xFF
  shl eax, 16
  and dword [color_window_color], 0x00FFFF
  or dword [color_window_color], eax
 jmp .redraw
 .click_color_red_0:
  and dword [color_window_color], 0x00FFFF
 jmp .redraw
 .click_color_red_256:
  or dword [color_window_color], 0xFF0000
 jmp .redraw
 
 .click_color_green:
  mov eax, dword [color_window_mc_line]
  sub eax, dword [screen_x_center]
  add eax, 255
  and eax, 0xFF
  shl eax, 8
  and dword [color_window_color], 0xFF00FF
  or dword [color_window_color], eax
 jmp .redraw
 .click_color_green_0:
  and dword [color_window_color], 0xFF00FF
 jmp .redraw
 .click_color_green_256:
  or dword [color_window_color], 0x00FF00
 jmp .redraw
 
 .click_color_blue:
  mov eax, dword [color_window_mc_column]
  sub eax, dword [screen_x_center]
  add eax, 255
  and eax, 0xFF
  and dword [color_window_color], 0xFFFF00
  or dword [color_window_color], eax
 jmp .redraw
 .click_color_blue_0:
  and dword [color_window_color], 0xFFFF00
 jmp .redraw
 .click_color_blue_256:
  or dword [color_window_color], 0x0000FF
 jmp .redraw
 
 .redraw:
  call draw_color_window
  mov eax, dword [color_window_mc_column]
  mov dword [cursor_column], eax
  mov eax, dword [color_window_mc_line]
  mov dword [cursor_line], eax
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .halt
 
 .exit:
  mov dword [color_window_color], 0xFFFFFFFF
 ret
 
 .choose_color:
 ret
 
;;;;; Message window ;;;;;

message_window_key0 dd 0
message_window_key1 dd 0
message_window_key2 dd 0
message_window_key3 dd 0
message_window_key4 dd 0
message_window_key5 dd 0

show_message_window:
 push edi
 push esi
 
 mov eax, 0
 .length_of_string:
  cmp byte [esi], 0
  je .length_of_2_string
  inc eax
  inc esi
 jmp .length_of_string
 
 .length_of_2_string:
 mov ebx, 0
 .length_of_string_2:
  cmp byte [edi], 0
  je .compare_strings_length
  inc ebx
  inc edi
 jmp .length_of_string_2
 
 .compare_strings_length:
 cmp eax, ebx
 ja .draw_window
 mov eax, ebx 
 .draw_window:
 add eax, 2
 push eax ;number of columns of message
 mov ebx, COLUMNSZ
 mul ebx
 shr eax, 1 ;div 2
 mov ebx, dword [screen_x_center]
 sub ebx, eax
 mov dword [cursor_column], ebx
 
 mov eax, dword [screen_y_center]
 sub eax, LINESZ*5/2
 mov dword [cursor_line], eax
 
 mov dword [square_heigth], LINESZ*5
 
 pop eax
 mov ebx, COLUMNSZ
 mul ebx
 mov dword [square_length], eax
 
 ;draw background
 mov dword [color], 0xDD8000 ;orange
 call draw_square
 mov dword [color], BLACK
 call draw_empty_square
 
 ;print messages
 add dword [cursor_column], COLUMNSZ
 add dword [cursor_line], LINESZ
 pop esi
 call print
 add dword [cursor_line], LINESZ*2
 pop eax
 mov esi, eax
 mov ebx, 0
 .length_of_second_string:
  cmp byte [eax], 0
  je .print_second_string
  inc ebx ;length of string
  inc eax
 jmp .length_of_second_string
 .print_second_string:
 mov eax, ebx
 mov ebx, COLUMNSZ
 mul ebx
 shr eax, 1 ;div 2
 mov ebx, dword [screen_x_center]
 sub ebx, eax
 mov dword [cursor_column], ebx
 call print
 
 call redraw_screen
 
 .halt:
  call wait_for_keyboard
  
  cmp byte [key_code], KEY_ESC
  je .return
  
  cmp byte [key_code], KEY_ENTER
  je .return
  
  mov eax, dword [message_window_key0]
  cmp byte [key_code], al
  je .return
  mov eax, dword [message_window_key1]
  cmp byte [key_code], al
  je .return
  mov eax, dword [message_window_key2]
  cmp byte [key_code], al
  je .return
  mov eax, dword [message_window_key3]
  cmp byte [key_code], al
  je .return
  mov eax, dword [message_window_key4]
  cmp byte [key_code], al
  je .return
  mov eax, dword [message_window_key5]
  cmp byte [key_code], al
  je .return
 jmp .halt
 
 .return:
 ret
 
message_window_length dd 0
message_window_heigth dd 0
message_window_line dd 0
message_window_column dd 0

show_empty_message_window:
 mov eax, dword [screen_x_center]
 mov ecx, dword [message_window_length]
 shr ecx, 1 ;div 2
 sub eax, ecx
 mov dword [cursor_column], eax
 mov dword [message_window_column], eax
 mov eax, dword [screen_y_center]
 mov ecx, dword [message_window_heigth]
 shr ecx, 1 ;div 2
 sub eax, ecx
 mov dword [cursor_line], eax
 mov dword [message_window_line], eax
 
 mov eax, dword [message_window_length]
 mov dword [square_length], eax
 mov eax, dword [message_window_heigth]
 mov dword [square_heigth], eax
 
 mov dword [color], 0xDD8000
 call draw_square
 mov dword [color], BLACK
 call draw_empty_square
 
 ret
 
print_to_message_window:
 mov edi, esi
 mov eax, 0
 .length_of_message:
  cmp byte [edi], 0
  je .print
  inc eax
  inc edi
 jmp .length_of_message
 
 .print:
 shl eax, 2 ;mul 4
 mov ebx, dword [screen_x_center]
 sub ebx, eax
 mov dword [cursor_column], ebx
 mov dword [color], BLACK
 call print
 
 ret

;;;;; Text input ;;;;;

%define TI_NO_CURSOR 0xFFFFFFFF

text_input_pointer dd 0
text_input_length dd 0
text_input_cursor dd 0

%define TEXT_INPUT_ALL 0
%define TEXT_INPUT_NUMBERS 1
text_input_type dd 0

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
  
  cmp byte [key_code], KEY_ESC
  je .done

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

  cmp dword [text_input_type], TEXT_INPUT_NUMBERS
  je .test_if_number_input
  
  cmp word [key_unicode], 0
  jne .char
 jmp .text_input_halt
  .test_if_number_input:
  cmp word [key_unicode], '0'
  jb .text_input_halt
  cmp word [key_unicode], '9'
  ja .text_input_halt
 jmp .char
  
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
 shl eax, 1 ;mul 2
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
 shl eax, 1 ;mul 2
 add eax, dword [text_input_pointer]
 mov edi, eax
 add eax, 2
 mov esi, eax
 mov ecx, dword [text_input_length]
 sub ecx, dword [text_input_cursor]
 rep movsw
 
 mov eax, dword [text_input_length]
 shl eax, 1 ;mul 2
 sub eax, 2
 add eax, dword [text_input_pointer]
 mov word [eax], 0

 call draw_text_input
 call redraw_screen
 jmp .text_input_halt

 .char:
 mov eax, dword [text_input_length]
 cmp dword [text_input_cursor], eax
 je .text_input_halt
 
 ;move other chars
 mov edi, dword [text_input_pointer]
 add edi, dword [text_input_length]
 add edi, dword [text_input_length]
 mov esi, edi
 sub esi, 2
 mov ecx, dword [text_input_length]
 sub ecx, dword [text_input_cursor]
 std
 rep movsw
 cld
 
 mov eax, dword [text_input_pointer]
 mov ebx, dword [text_input_cursor]
 mov cx, word [key_unicode]
 mov word [eax+(ebx*2)], cx
 inc dword [text_input_cursor]
 call draw_text_input
 call redraw_screen
 jmp .text_input_halt
 
number_input_return dd 0 
 
number_input:
 mov dword [text_input_type], TEXT_INPUT_NUMBERS
 call text_input
 mov dword [text_input_type], TEXT_INPUT_ALL
 
 mov dword [number_input_return], 0
 mov ecx, 0
 mov esi, dword [text_input_pointer]
 .calculate_number:
  cmp word [esi], 0
  je .done
  mov eax, ecx
  mov ebx, 10
  mul ebx
  mov bx, word [esi]
  sub ebx, '0'
  add eax, ebx
  mov ecx, eax
  add esi, 2
 jmp .calculate_number
  
 .done:
 mov dword [number_input_return], ecx
 ret

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
 dec ebx
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
 mov eax, dword [screen_x]
 cmp dword [cursor_column], eax
 jb .if_outside_of_x_screen
  mov dword [cursor_column], ecx
 .if_outside_of_x_screen:
  
 ;MOVE CURSOR VERTICAL
 .test_vertical_move:
 mov edx, dword [mcursor_down_side]
 dec edx
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
 mov eax, dword [screen_y]
 cmp dword [cursor_line], eax
 jb .if_outside_of_y_screen
  mov dword [cursor_line], edx
 .if_outside_of_y_screen:
 jmp .draw_cursor
  
 .move_up:
 mov eax, 0xFF
 sub al, byte [usb_mouse_data+2]
 mov ebx, dword [cursor_line]
 sub ebx, eax
 cmp ebx, dword [screen_y]
 ja .above_screen
 cmp ebx, dword [mcursor_up_side]
 jb .too_up
 sub dword [cursor_line], eax
 jmp .draw_cursor
 
 .above_screen:
 mov dword [cursor_line], 20 
 jmp .draw_cursor
 
 .too_up:
 mov eax, dword [mcursor_up_side]
 mov dword [cursor_line], eax
 
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
