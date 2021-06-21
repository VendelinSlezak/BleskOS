;BleskOS

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
