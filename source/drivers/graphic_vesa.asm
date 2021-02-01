;BleskOS

;IMPORTANT: we still working on make this code more readable

;BleskOS use double graphic memory model. It means that in ram memory is area use as video memory. In this memory are write all
;graphic. After drawing is done, part of this ram memory is copied into video memory. So user not see any redrawing because to
;video memory is show only done thing. Also, reading/writing to ram memory is much faster as reading/writing video memory

%define WHITE 0xFFFF
%define BLACK 0x0000
%define COLOR(red, green, blue) ( ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3) )

%define LINESZ 10
%define COLUMNSZ 8
%define LINE(x) x*LINESZ+1
%define COLUMN(x) x*COLUMNSZ
%macro SCREEN_X_SUB 2
 mov %1, dword [screen_x]
 sub %1, %2
%endmacro
%macro SCREEN_Y_SUB 2
 mov %1, dword [screen_y]
 sub %1, %2
%endmacro

screen_lfb dd 0
screen_x dd 0
screen_y dd 0
screen_bites_per_pixel dd 0
screen_bpp dd 0
screen_all_pixels dd 0
screen_pixels_per_line dd 0
screen_pointer dd 0

cursor_line dd 0
cursor_column dd 0
first_redraw_line dd 0
how_much_lines_redraw dd 0
line_lenght dd 0
line_height dd 0
square_lenght dd 0
square_height dd 0
color dd 0

char_for_print dd 0
hex_print_value dd 0
hex_string times 11 db 0
var_print_value dd 0
var_string times 11 db 0
debug_line dd 0

;;;;; END OF DEFINITIONS OF VARIABILES ;;;;;

%macro CALCULATE_CURSOR_POSITION 0
 mov eax, dword [cursor_line]
 mov ebx, dword [screen_x]
 mul ebx
 add eax, dword [cursor_column]

 mov ebx, dword [screen_bpp]
 mul ebx
 add eax, MEMORY_RAM_SCREEN

 mov dword [screen_pointer], eax
%endmacro

%macro MOVE_CURSOR_NEXT_LINE 0
 push eax
 mov eax, dword [screen_pixels_per_line]
 add dword [screen_pointer], eax
 pop eax
%endmacro

%macro REDRAW_LINES_SCREEN 2
 mov dword [first_redraw_line], %1
 mov dword [how_much_lines_redraw], %2
 call redraw_lines_screen
%endmacro

%macro CLEAR_SCREEN 1
 mov dword [color], %1
 call clear_screen
%endmacro

%macro DRAW_LINE 4
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [line_lenght], %3
 mov dword [color], %4
 call draw_line
%endmacro

%macro DRAW_COLUMN 4
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [line_height], %3
 mov dword [color], %4
 call draw_column
%endmacro

%macro DRAW_SQUARE 5
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [square_lenght], %3
 mov dword [square_height], %4
 mov dword [color], %5
 call draw_square
%endmacro

%macro DRAW_PIXEL_OF_CHAR 1
 mov al, byte [edi]
 and al, %1
 cmp al, %1
 jne .%1_over
  mov word [edx], BLACK ;draw pixel on screen
 .%1_over:
 add edx, 2
%endmacro

%macro CONVERT_HEX_TO_CHAR 2
 MOV_0xF_SHIFT eax, dword [hex_print_value], %1
 add al, '0' ;convert to char
 mov byte [hex_string+%2], al
 IF_H al, '9', if%2
   add al, 7 ;convert to char
   mov byte [hex_string+%2], al
 ENDIF if%2
%endmacro

%macro SEPARATE_DIGIT_PRINT_VAR 1
 mov ebx, 10
 mov edx, 0 ;need for divine
 div ebx
 add dl, '0' ;convert to char
 mov byte [var_string+%1], dl
%endmacro

%macro IF_DIGIT_NON_ZERO_PRINT_VAR 1
 cmp byte [var_string+%1], '0'
 jne .print_var
 inc esi
%endmacro

%macro PRINT_CHAR 3
 mov dword [cursor_line], %2
 mov dword [cursor_column], %3
 mov dword [char_for_print], %1
 call print_char
%endmacro

%macro PRINT 4
 mov dword [cursor_line], %3
 mov dword [cursor_column], %4
 mov esi, .%2
 call print
 jmp .%2_over
 .%2 db %1, 0
 .%2_over:
%endmacro

%macro PRINT_HEX 3
 mov dword [cursor_line], %2
 mov dword [cursor_column], %3
 mov dword [hex_print_value], %1
 call print_hex
%endmacro

%macro PRINT_VAR 3
 mov dword [cursor_line], %2
 mov dword [cursor_column], %3
 mov dword [var_print_value], %1
 call print_var
%endmacro

%macro PSTR 2
 mov esi, .%2
 call pstr
 jmp .%2_over
 .%2 db %1, 0
 .%2_over:
%endmacro

%macro PHEX 1
 mov dword [hex_print_value], %1
 call phex
%endmacro

%macro PVAR 1
 mov dword [var_print_value], %1
 call pvar
%endmacro

;;;;; END OF DEFINITIONS OF MACROS ;;;;;

init_graphic:
 ;read linear frame buffer
 mov eax, dword [0x70028]
 mov dword [screen_lfb], eax

 ;read number of columns
 mov ax, word [0x70012]
 mov dword [screen_x], 0
 mov word [screen_x], ax

 ;read number of lines
 mov ax, word [0x70014]
 mov dword [screen_y], 0
 mov word [screen_y], ax

 ;calculate bytes per pixel
 mov al, byte [0x70019]
 mov dword [screen_bites_per_pixel], 0
 mov byte [screen_bites_per_pixel], al
 mov dword [screen_bpp], 0

 IF_E al, 16, if_bpp16
  mov dword [screen_bpp], 2
 ENDIF if_bpp16

 ;calculate pixels per line
 mov dword [screen_pixels_per_line], 0
 mov eax, dword [screen_x]
 mov ebx, dword [screen_bpp]
 mul ebx
 mov dword [screen_pixels_per_line], eax

 ;calculate all screen pixels
 mov eax, 0
 mov ax, word [screen_x]
 mov ebx, 0
 mov bx, word [screen_y]
 mul ebx
 mov dword [screen_all_pixels], eax

 ret

redraw_screen:
 mov eax, MEMORY_RAM_SCREEN
 mov ebx, dword [screen_lfb]
 mov ecx, dword [screen_all_pixels]

 .redraw_screen_loop:
  mov edx, dword [eax]
  mov dword [ebx], edx
  add eax, 4
  add ebx, 4
 loop .redraw_screen_loop

 ret

redraw_lines_screen:
 ;calculate memory of start line
 mov eax, dword [first_redraw_line]
 mov ebx, dword [screen_pixels_per_line]
 mul ebx

 ;set pointer registers
 mov esi, eax
 mov edi, eax
 add esi, MEMORY_RAM_SCREEN
 add edi, dword [screen_lfb]

 ;calculate lenght of pixels
 mov eax, dword [how_much_lines_redraw]
 mov ebx, dword [screen_pixels_per_line]
 mul ebx

 FOR eax, cycle_redraw_lines_screen
  mov dx, word [esi]
  mov word [edi], dx
  add esi, 2
  add edi, 2
 ENDFOR cycle_redraw_lines_screen

 ret

clear_screen:
 mov eax, MEMORY_RAM_SCREEN
 mov bx, word [color]
 mov ecx, dword [screen_all_pixels]

 .clear_screen_loop:
  mov word [eax], bx
  add eax, 2
 loop .clear_screen_loop

 ret

draw_line:
 CALCULATE_CURSOR_POSITION
 mov eax, dword [screen_pointer]
 mov ebx, dword [color]

 FOR dword [line_lenght], draw_line_cycle
  mov word [eax], bx
  add eax, 2
 ENDFOR draw_line_cycle

 ret

draw_column:
 CALCULATE_CURSOR_POSITION
 mov eax, dword [screen_pointer]
 mov ebx, dword [color]

 FOR dword [line_height], draw_column_cycle
  mov word [eax], bx
  add eax, dword [screen_pixels_per_line]
 ENDFOR draw_column_cycle

 ret

draw_square:
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [color]

 FOR dword [square_height], cycle1
  mov eax, dword [screen_pointer]
  FOR dword [square_lenght], cycle2
   mov word [eax], bx
   add eax, 2
  ENDFOR cycle2

  MOVE_CURSOR_NEXT_LINE
 ENDFOR cycle1

 ret

draw_cursor:
 CALCULATE_CURSOR_POSITION

 ;line 1
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK

 ;line 2
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], BLACK

 ;line 3
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK

 ;line 4
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK

 ;line 5
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK

 ;line 6
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK

 ;line 7
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK

 ;line 8
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK

 ;line 9
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], WHITE
 add ebx, 2
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], BLACK

 ;line 10
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], BLACK
 add ebx, 2
 mov word [ebx], BLACK

 ;line 11
 MOVE_CURSOR_NEXT_LINE
 mov ebx, dword [screen_pointer]
 mov word [ebx], BLACK

 ret

print_char:
 CALCULATE_CURSOR_POSITION

 ;calculate char memory
 mov eax, dword [char_for_print]
 mov ebx, 8
 mul ebx
 add eax, vesa_font
 mov edi, eax

 ;pointer to char memory
 mov edx, dword [screen_pointer]

 ;char have eight lines
 FOR 8, print_char_cycle
  DRAW_PIXEL_OF_CHAR 0x80
  DRAW_PIXEL_OF_CHAR 0x40
  DRAW_PIXEL_OF_CHAR 0x20
  DRAW_PIXEL_OF_CHAR 0x10
  DRAW_PIXEL_OF_CHAR 0x08
  DRAW_PIXEL_OF_CHAR 0x04
  DRAW_PIXEL_OF_CHAR 0x02
  DRAW_PIXEL_OF_CHAR 0x01

  ;go to next line
  add edx, dword [screen_pixels_per_line] ;move one line down
  sub edx, 16 ;every line of char have 8 pixels and 16 bpp
  ;next byte of char
  inc edi
 ENDFOR print_char_cycle

 ret

print:
 .print_char:
  mov al, byte [esi]
  cmp al, 0 ;end of string
  je .end

  ;print char
  mov dword [char_for_print], 0
  mov byte [char_for_print], al
  call print_char

  ;move to next char position
  add dword [cursor_column], 8
  inc esi
 jmp .print_char

 .end:
 ret

print_hex:
 mov byte [hex_string], '0'
 mov byte [hex_string+1], 'x'

 CONVERT_HEX_TO_CHAR 28, 2
 CONVERT_HEX_TO_CHAR 24, 3
 CONVERT_HEX_TO_CHAR 20, 4
 CONVERT_HEX_TO_CHAR 16, 5
 CONVERT_HEX_TO_CHAR 12, 6
 CONVERT_HEX_TO_CHAR 8, 7
 CONVERT_HEX_TO_CHAR 4, 8
 CONVERT_HEX_TO_CHAR 0, 9
 mov byte [hex_string+10], 0

 mov esi, hex_string
 call print

 ret

print_var:
 mov eax, [var_print_value]
 SEPARATE_DIGIT_PRINT_VAR 9
 SEPARATE_DIGIT_PRINT_VAR 8
 SEPARATE_DIGIT_PRINT_VAR 7
 SEPARATE_DIGIT_PRINT_VAR 6
 SEPARATE_DIGIT_PRINT_VAR 5
 SEPARATE_DIGIT_PRINT_VAR 4
 SEPARATE_DIGIT_PRINT_VAR 3
 SEPARATE_DIGIT_PRINT_VAR 2
 SEPARATE_DIGIT_PRINT_VAR 1
 SEPARATE_DIGIT_PRINT_VAR 0
 mov byte [var_string+10], 0 ;end of string

 ;find start of non-zero string
 mov esi, var_string

 IF_DIGIT_NON_ZERO_PRINT_VAR 0
 IF_DIGIT_NON_ZERO_PRINT_VAR 1
 IF_DIGIT_NON_ZERO_PRINT_VAR 2
 IF_DIGIT_NON_ZERO_PRINT_VAR 3
 IF_DIGIT_NON_ZERO_PRINT_VAR 4
 IF_DIGIT_NON_ZERO_PRINT_VAR 5
 IF_DIGIT_NON_ZERO_PRINT_VAR 6
 IF_DIGIT_NON_ZERO_PRINT_VAR 7
 IF_DIGIT_NON_ZERO_PRINT_VAR 8

 .print_var:
 call print
 .nothing:

 ret

pstr:
 IF_E dword [debug_line], 500, if1
  mov dword [debug_line], 0
 ENDIF if1

 mov eax, dword [debug_line]
 mov dword [cursor_line], eax
 mov dword [cursor_column], 0
 call print

 ;show on screen
 mov eax, dword [debug_line]
 REDRAW_LINES_SCREEN eax, 10

 add dword [debug_line], 10

 ret

phex:
 IF_E dword [debug_line], 500, if1
  mov dword [debug_line], 0
 ENDIF if1

 mov eax, dword [debug_line]
 mov dword [cursor_line], eax
 mov dword [cursor_column], 0
 call print_hex

 ;show on screen
 mov eax, dword [debug_line]
 REDRAW_LINES_SCREEN eax, 10

 add dword [debug_line], 10

 ret

pvar:
 IF_E dword [debug_line], 500, if1
  mov dword [debug_line], 0
 ENDIF if1

 mov eax, dword [debug_line]
 mov dword [cursor_line], eax
 mov dword [cursor_column], 0
 call print_var

 ;show on screen
 mov eax, dword [debug_line]
 REDRAW_LINES_SCREEN eax, 10

 add dword [debug_line], 10

 ret
