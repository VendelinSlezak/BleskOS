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
 mov dword [square_lenght], 800
 mov dword [square_height], 20
 mov eax, dword [window_border_color]
 mov dword [color], eax
 call draw_square

 mov dword [cursor_line], 580
 call draw_square

 mov dword [cursor_line], 5
 mov dword [cursor_column], 15
 mov dword [color], BLACK
 mov esi, dword [window_up_string_ptr]
 call print

 mov dword [cursor_line], 585
 mov esi, dword [window_down_string_ptr]
 call print

 ret

;;;;; List ;;;;;

list_chars_lenght dd 0
list_items_lenght dd 0
list_selected_item dd 0
list_background dd 0

%macro DRAW_LIST 6
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [list_chars_lenght], %3
 mov dword [list_items_lenght], %4
 mov eax, dword [%5]
 mov dword [list_selected_item], eax
 mov dword [list_background], %6
 call draw_list
%endmacro

draw_list:
 mov eax, dword [list_background]
 mov dword [color], eax

 mov eax, dword [list_chars_lenght]
 mov ebx, 8
 mul ebx
 add eax, 4
 mov dword [square_lenght], eax

 mov eax, dword [list_items_lenght]
 mov ebx, 10
 mul ebx
 add eax, 4
 mov dword [square_height], eax

 call draw_square

 mov dword [color], BLACK
 call draw_empty_square
 inc dword [cursor_line]
 inc dword [cursor_column]
 sub dword [square_lenght], 2
 sub dword [square_height], 2
 call draw_empty_square

 inc dword [cursor_line]
 inc dword [cursor_column]
 mov dword [color], 0xFF83
 mov eax, dword [list_selected_item]
 mov ebx, 10
 mul ebx
 add dword [cursor_line], eax
 dec dword [square_lenght]
 mov dword [square_height], 10
 call draw_square

 ret
