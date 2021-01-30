;BleskOS

%macro DRAW_PROGRAM 4
 mov dword [color], %4
 call draw_program

 mov dword [cursor_line], 6
 mov dword [cursor_column], 5
 mov esi, .%3_up_str
 call print

 mov eax, dword [screen_y]
 sub eax, 14
 mov dword [cursor_line], eax
 mov dword [cursor_column], 5
 mov esi, .%3_down_str
 call print

 call redraw_screen

 jmp .%3_over
 .%3_up_str db %1, 0
 .%3_down_str db %2, 0
 .%3_over:
%endmacro

draw_program:
 call clear_screen
 ret
