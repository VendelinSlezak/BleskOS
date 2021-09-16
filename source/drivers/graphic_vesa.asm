;BleskOS

%define WHITE 0x00FFFFFF
%define BLACK 0x00000000

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
screen_x_center dd 0
screen_y dd 0
screen_y_center dd 0
screen_bites_per_pixel dd 0
screen_bpp dd 0
screen_all_pixels dd 0
screen_pixels_per_line dd 0
screen_pointer dd 0

cursor_line dd 0
cursor_column dd 0
old_cursor_line dd 0
old_cursor_column dd 0
old_cursor_data times 100 dw 0
first_redraw_line dd 0
how_much_lines_redraw dd 0
line_length dd 0
column_heigth dd 0
square_length dd 0
square_heigth dd 0
color dd 0

char_for_print dd 0
hex_print_value dd 0
hex_string times 11 db 0
var_print_value dd 0
var_string times 11 db 0
debug_line dd 0

x1 dd 0
y1 dd 0
x2 dd 0
y2 dd 0
line_direction dd 0
line_x_length dd 0
line_y_length dd 0
line_x_pointer dd 0
line_y_pointer dd 0

;;;;; END OF DEFINITIONS OF VARIABILES ;;;;;

%macro CALCULATE_CURSOR_POSITION 0
 mov eax, dword [cursor_line]
 mov ebx, dword [screen_x]
 mul ebx
 add eax, dword [cursor_column]
 mov ebx, 4
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
 mov dword [line_length], %3
 mov dword [color], %4
 call draw_line
%endmacro

%macro DRAW_COLUMN 4
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [column_heigth], %3
 mov dword [color], %4
 call draw_column
%endmacro

%macro DRAW_SQUARE 5
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [square_length], %3
 mov dword [square_heigth], %4
 mov dword [color], %5
 call draw_square
%endmacro

%macro DRAW_EMPTY_SQUARE 5
 mov dword [cursor_line], %1
 mov dword [cursor_column], %2
 mov dword [square_length], %3
 mov dword [square_heigth], %4
 mov dword [color], %5
 call draw_empty_square
%endmacro

%macro DRAW_PIXEL_OF_CHAR 2
 mov al, bl
 test al, %1
 jz .%1_over
  mov dword [edx+(%2*4)], BLACK ;draw pixel on screen
 .%1_over:
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
 mov edx, 0 ;need for divide
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
 pusha
 mov esi, .%2
 call pstr
 jmp .%2_over
 .%2 db %1, 0
 .%2_over:
 popa
%endmacro

%macro PHEX 1
 pusha
 mov dword [hex_print_value], %1
 call phex
 popa
%endmacro

%macro PVAR 1
 pusha
 mov dword [var_print_value], %1
 call pvar
 popa
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

 ;calculate center
 mov eax, dword [screen_x]
 mov ebx, 2
 mov edx, 0
 div ebx
 mov dword [screen_x_center], eax
 mov eax, dword [screen_y]
 mov ebx, 2
 mov edx, 0
 div ebx
 mov dword [screen_y_center], eax

 ;calculate bytes per pixel
 mov al, byte [0x70019]
 mov dword [screen_bites_per_pixel], 0
 mov byte [screen_bites_per_pixel], al
 mov eax, dword [screen_bites_per_pixel]
 mov ebx, 8
 mov edx, 0
 div ebx
 mov dword [screen_bpp], eax

 ;calculate pixels per line
 mov eax, dword [screen_x]
 mov ebx, 4
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
 cmp dword [screen_bites_per_pixel], 24
 je .24_bpp
 cmp dword [screen_bites_per_pixel], 32
 je .32_bpp
 ret

 .24_bpp:
 mov esi, MEMORY_RAM_SCREEN
 mov edi, dword [screen_lfb]
 mov ecx, dword [screen_all_pixels]
 .redraw_24_bpp:
  mov al, byte [esi]
  mov byte [edi], al
  inc esi
  inc edi

  mov al, byte [esi]
  mov byte [edi], al
  inc esi
  inc edi

  mov al, byte [esi]
  mov byte [edi], al
  inc esi
  inc esi
  inc edi
 loop .redraw_24_bpp
 ret

 .32_bpp:
 mov esi, MEMORY_RAM_SCREEN
 mov edi, dword [screen_lfb]
 mov ecx, dword [screen_all_pixels]
 rep movsd
 ret

redraw_lines_screen:
 ;calculate memory of start line
 mov eax, dword [first_redraw_line]
 mov ebx, dword [screen_pixels_per_line]
 mul ebx
 mov esi, eax
 add esi, MEMORY_RAM_SCREEN

 ;set pointer registers
 mov eax, dword [screen_x]
 mov ebx, dword [screen_bpp]
 mul ebx
 mov ebx, eax
 mov eax, dword [first_redraw_line]
 mul ebx
 mov edi, eax
 add edi, dword [screen_lfb]

 ;calculate length of pixels
 mov eax, dword [how_much_lines_redraw]
 mov ebx, dword [screen_pixels_per_line]
 mul ebx
 mov ecx, eax

 cmp dword [screen_bites_per_pixel], 24
 je .24_bpp
 cmp dword [screen_bites_per_pixel], 32
 je .32_bpp
 ret

 .24_bpp:
  mov al, byte [esi]
  mov byte [edi], al
  inc esi
  inc edi

  mov al, byte [esi]
  mov byte [edi], al
  inc esi
  inc edi

  mov al, byte [esi]
  mov byte [edi], al
  inc esi
  inc esi
  inc edi
 loop .24_bpp
 ret

 .32_bpp:
 rep movsd
 ret
 

clear_screen:
 mov edi, MEMORY_RAM_SCREEN
 mov eax, dword [color]
 mov ecx, dword [screen_all_pixels]
 rep stosd

 ret

draw_line:
 CALCULATE_CURSOR_POSITION
 mov edi, dword [screen_pointer]
 mov eax, dword [color]
 mov ecx, dword [line_length]
 rep stosd

 ret

draw_column:
 CALCULATE_CURSOR_POSITION
 mov eax, dword [screen_pointer]
 mov ebx, dword [color]

 FOR dword [column_heigth], draw_column_cycle
  mov dword [eax], ebx
  add eax, dword [screen_pixels_per_line]
 ENDFOR draw_column_cycle

 ret

draw_square:
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [color]

 FOR dword [square_heigth], cycle1
  mov eax, dword [screen_pointer]
  FOR dword [square_length], cycle2
   mov dword [eax], ebx
   add eax, 4
  ENDFOR cycle2

  MOVE_CURSOR_NEXT_LINE
 ENDFOR cycle1

 ret

draw_empty_square:
 mov eax, dword [square_length]
 mov dword [line_length], eax
 call draw_line

 mov eax, dword [square_heigth]
 mov dword [column_heigth], eax
 call draw_column

 mov eax, dword [square_length]
 add dword [cursor_column], eax
 mov eax, dword [square_heigth]
 mov dword [column_heigth], eax
 call draw_column

 mov eax, dword [square_length]
 sub dword [cursor_column], eax
 mov eax, dword [square_heigth]
 add dword [cursor_line], eax
 mov eax, dword [square_length]
 mov dword [line_length], eax
 inc dword [line_length]
 call draw_line

 mov eax, dword [square_heigth]
 sub dword [cursor_line], eax

 ret
 
draw_line_all:
 ;if straigth line
 mov eax, dword [y1]
 cmp eax, dword [y2]
 je .draw_line
 
 ;line will be drawed from down to up
 cmp eax, dword [y2]
 jg .if_reverse
  mov ebx, dword [y2]
  mov dword [y2], eax
  mov dword [y1], ebx
  
  mov eax, dword [x1]
  mov ebx, dword [x2]
  mov dword [x1], ebx
  mov dword [x2], eax
 .if_reverse
 
 ;if straigth column
 mov eax, dword [x1]
 cmp eax, dword [x2]
 je .draw_column
 
 ;direction of line
 cmp eax, dword [x2]
 jg .direction_left
 
 mov dword [line_direction], 1 ;right
 mov eax, dword [x2]
 sub eax, dword [x1]
 mov dword [line_x_length], eax
 jmp .y_length
 
 .direction_left:
 mov dword [line_direction], 0 ;left
 mov eax, dword [x1]
 sub eax, dword [x2]
 mov dword [line_x_length], eax
 
 ;calculate y length
 .y_length:
 mov eax, dword [y1]
 sub eax, dword [y2]
 mov dword [line_y_length], eax
 
 ;set values
 mov eax, dword [x1]
 mov dword [line_x_pointer], eax
 mov dword [cursor_column], eax
 mov eax, dword [y1]
 mov dword [line_y_pointer], eax
 mov dword [cursor_line], eax
 CALCULATE_CURSOR_POSITION
 mov esi, eax ;pointer
 
 mov eax, dword [line_x_length]
 mov ebx, dword [line_y_length]
 mov ecx, dword [line_x_length]
 mov edx, dword [line_y_length]
 
 ;draw line
 cmp dword [line_direction], 1
 je .right_draw_pixel_of_line
 
 .left_draw_pixel_of_line:
  mov ebp, dword [line_x_pointer]
  cmp ebp, dword [x2]
  jg .continue_draw_left
  jmp .done
  
  .continue_draw_left:
  mov dword [esi], BLACK ;draw pixel
  
  cmp eax, ebx
  jg .move_cursor_left
  
  ;move cursor right up
  sub esi, dword [screen_pixels_per_line] ;up
  dec dword [line_y_pointer]
  add eax, ecx
  
  .move_cursor_left:
  sub esi, 4 ;right
  dec dword [line_x_pointer]
  add ebx, edx
 jmp .left_draw_pixel_of_line
 
 .right_draw_pixel_of_line:
  mov ebp, dword [line_x_pointer]
  cmp ebp, dword [x2]
  jl .continue_draw_right
  jmp .done
  
  .continue_draw_right:
  mov dword [esi], BLACK ;draw pixel
  
  cmp eax, ebx
  jg .move_cursor_right
  
  ;move cursor right up
  sub esi, dword [screen_pixels_per_line] ;up
  dec dword [line_y_pointer]
  add eax, ecx
  
  .move_cursor_right:
  add esi, 4 ;right
  inc dword [line_x_pointer]
  add ebx, edx
 jmp .right_draw_pixel_of_line
 
 .done:
 ret
 
 .draw_line:
 
 .draw_column:

draw_cursor:
 CALCULATE_CURSOR_POSITION
 push dword [cursor_column]
 push dword [cursor_line]
 push eax ;screen pointer
 push eax ;screen pointer

 ;erase cursor from screen
 mov eax, dword [old_cursor_line]
 mov dword [cursor_line], eax
 mov eax, dword [old_cursor_column]
 mov dword [cursor_column], eax
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [screen_pixels_per_line]
 mov esi, old_cursor_data

 FOR 11, erase_cursor
  FOR 10, erase_cursor_line
   mov edx, dword [esi]
   mov dword [eax], edx
   add esi, 4
   add eax, 4
  ENDFOR erase_cursor_line
  sub eax, 40 ;cursor length
  add eax, ebx ;next line
 ENDFOR erase_cursor

 ;read cursor
 pop eax ;screen pointer
 mov esi, old_cursor_data

 FOR 11, read_cursor_bg
  FOR 10, read_cursor_bg_line
   mov edx, dword [eax]
   mov dword [esi], edx
   add esi, 4
   add eax, 4
  ENDFOR read_cursor_bg_line
  sub eax, 40 ;cursor length
  add eax, ebx ;next line
 ENDFOR read_cursor_bg

 ;write cursor to screen
 pop eax ;screen pointer

 ;line 1
 mov dword [eax], BLACK

 ;line 2
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], BLACK

 ;line 3
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], BLACK

 ;line 4
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], WHITE
 mov dword [eax+12], BLACK

 ;line 5
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], WHITE
 mov dword [eax+12], WHITE
 mov dword [eax+16], BLACK

 ;line 6
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], WHITE
 mov dword [eax+12], WHITE
 mov dword [eax+16], WHITE
 mov dword [eax+20], BLACK

 ;line 7
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], WHITE
 mov dword [eax+12], WHITE
 mov dword [eax+16], WHITE
 mov dword [eax+20], WHITE
 mov dword [eax+24], BLACK

 ;line 8
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], WHITE
 mov dword [eax+12], WHITE
 mov dword [eax+16], WHITE
 mov dword [eax+20], BLACK

 ;line 9
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], WHITE
 mov dword [eax+8], WHITE
 mov dword [eax+12], BLACK
 mov dword [eax+16], BLACK

 ;line 10
 add eax, ebx ;next line
 mov dword [eax], BLACK
 mov dword [eax+4], BLACK
 mov dword [eax+8], BLACK

 ;line 11
 add eax, ebx ;next line
 mov dword [eax], BLACK

 ;update values
 pop eax
 mov dword [old_cursor_line], eax
 pop eax
 mov dword [old_cursor_column], eax

 ret

read_cursor_bg:
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [screen_pixels_per_line]
 mov esi, old_cursor_data

 FOR 11, read_cursor_bg
  FOR 10, read_cursor_bg_line
   mov edx, dword [eax]
   mov dword [esi], edx
   add esi, 4
   add eax, 4
  ENDFOR read_cursor_bg_line
  sub eax, 40 ;cursor length
  add eax, ebx ;next line
 ENDFOR read_cursor_bg

 ret

print_char:
 CALCULATE_CURSOR_POSITION

 ;calculate char memory
 mov eax, dword [char_for_print]
 mov ebx, 8
 mul ebx
 add eax, bleskos_font
 mov edi, eax

 ;pointer to char memory
 mov edx, dword [screen_pointer]

 ;char have eight lines
 FOR 8, print_char_cycle
  mov bl, byte [edi] ;load line of char
  DRAW_PIXEL_OF_CHAR 0x80, 0
  DRAW_PIXEL_OF_CHAR 0x40, 1
  DRAW_PIXEL_OF_CHAR 0x20, 2
  DRAW_PIXEL_OF_CHAR 0x10, 3
  DRAW_PIXEL_OF_CHAR 0x08, 4
  DRAW_PIXEL_OF_CHAR 0x04, 5
  DRAW_PIXEL_OF_CHAR 0x02, 6
  DRAW_PIXEL_OF_CHAR 0x01, 7

  ;go to next line
  add edx, dword [screen_pixels_per_line] ;move one line down
  inc edi ;next line of char
 ENDFOR print_char_cycle

 ret

print:
 .print_char:
  mov al, byte [esi]
  cmp al, 0 ;end of string
  je .end

  ;type of UTF-8 char
  mov bl, al
  and bl, 0x80
  cmp bl, 0x0
  je .ascii_char
  mov bl, al
  and bl, 0xE0
  cmp bl, 0xC0
  je .utf_8_two_bytes
  jmp .print_char ;skip this char

  ;print ascii char
  .ascii_char:
  mov dword [char_for_print], 0
  mov byte [char_for_print], al
  call print_char
  add dword [cursor_column], 8 ;position of next char
  inc esi
  jmp .print_char

  ;print utf-8 two bytes long char
  .utf_8_two_bytes:
  mov dword [char_for_print], 0
  and ax, 0x1F
  shl ax, 6
  mov bl, byte [esi+1]
  and bl, 0x3F
  or al, bl
  mov word [char_for_print], ax
  call print_char
  add dword [cursor_column], 8 ;position of next char
  add esi, 2
  jmp .print_char

 .end:
 ret

print_ascii:
 .print_char:
  mov al, byte [esi]
  cmp al, 0 ;end of string
  je .end

  mov dword [char_for_print], 0
  mov byte [char_for_print], al
  call print_char
  add dword [cursor_column], 8 ;position of next char
  inc esi
 jmp .print_char

 .end:
 ret

print_unicode:
 .print_char:
  mov ax, word [esi]
  cmp ax, 0 ;end of string
  je .end

  mov dword [char_for_print], 0
  mov word [char_for_print], ax
  call print_char
  add dword [cursor_column], 8 ;position of next char
  add esi, 2
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
 IF_E dword [debug_line], 600, if1
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
 IF_E dword [debug_line], 600, if1
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
 IF_E dword [debug_line], 600, if1
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
