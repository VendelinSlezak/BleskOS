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

%define PLAIN 0
%define BOLD 1

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
screen_mem_pointer dd MEMORY_RAM_SCREEN

cursor_line dd 0
cursor_column dd 0
old_cursor_data times 110 dd 0
first_redraw_line dd 0
how_much_lines_redraw dd 0
line_length dd 0
column_heigth dd 0
square_length dd 0
square_heigth dd 0
color dd 0

type_of_text dd PLAIN
size_of_text dd 1
font_pointer dd bleskos_font
char_for_print dd 0
hex_print_value dd 0
hex_string times 11 db 0
var_print_value dd 0
var_string times 11 db 0
debug_line dd 0

cycle_var dd 0
actual_char_pixel dd 0
actual_char_line dd 0

x1 dd 0
y1 dd 0
x2 dd 0
y2 dd 0
line_direction dd 0
line_x_length dd 0
line_y_length dd 0
line_x_pointer dd 0
line_y_pointer dd 0
ellipse_x_radius dd 0
ellipse_y_radius dd 0
circle_radius dd 0
decision dd 0
de_val dd 0
dse_val dd 0

;;;;; END OF DEFINITIONS OF VARIABILES ;;;;;

%macro CALCULATE_CURSOR_POSITION 0
 mov eax, dword [cursor_line]
 mov ebx, dword [screen_x]
 mul ebx
 add eax, dword [cursor_column]
 mov ebx, 4
 mul ebx
 add eax, dword [screen_mem_pointer]

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
  mov dword [edx+(%2*4)], ebp ;draw pixel on screen
 .%1_over:
%endmacro

%macro DRAW_PIXEL_OF_BIG_CHAR 1
 push edi
 push ecx
 test byte [esi], %1
 jz .%1_nothing_to_draw
 
 ;draw full square
 .draw_lines_%1:
  mov eax, edi
  push ecx
  .draw_line_%1:
   mov dword [eax], ebp
   add eax, 4
  dec cl
  cmp cl, 0
  jne .draw_line_%1
  pop ecx
  
  add edi, edx ;move pointer to next monitor line
 dec ch
 cmp ch, 0
 jne .draw_lines_%1
 pop ecx
 pop edi
 add edi, ebx ;move to position of next pixel of char
 jmp .%1_over
 
 .%1_nothing_to_draw:
 mov dword [actual_char_pixel], %1
 ;;;;;call draw_antialiased_pixel_of_char
 pop ecx
 pop edi
 add edi, ebx ;move to position of next pixel of char
 
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
 LOG 'VESA graphic card driver', 0xA

 ;read linear frame buffer
 mov eax, dword [0x70028]
 mov dword [screen_lfb], eax
 LOG 'Linear frame buffer: '
 LOG_HEX eax
 
 LOG 0xA, 'Screen resolution: '

 ;read number of columns
 mov eax, 0
 mov ax, word [0x70012]
 mov dword [screen_x], 0
 mov word [screen_x], ax
 LOG_VAR eax

 ;read number of lines
 mov ax, word [0x70014]
 mov dword [screen_y], 0
 mov word [screen_y], ax
 LOG_VAR eax

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
 mov eax, 0
 mov al, byte [0x70019]
 LOG_VAR eax
 LOG 0xA
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
  mov ax, word [esi]
  mov word [edi], ax

  mov al, byte [esi+2]
  mov byte [edi+2], al
  
  add esi, 4
  add edi, 3
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
  mov ax, word [esi]
  mov word [edi], ax

  mov al, byte [esi+2]
  mov byte [edi+2], al
  
  add esi, 4
  add edi, 3
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
 ;line will be drawed from down to up
 mov eax, dword [y1]
 cmp eax, dword [y2]
 ja .if_reverse
  mov ebx, dword [y2]
  mov dword [y2], eax
  mov dword [y1], ebx
  
  mov eax, dword [x1]
  mov ebx, dword [x2]
  mov dword [x1], ebx
  mov dword [x2], eax
 .if_reverse:
 
 ;direction of line
 mov eax, dword [x1]
 cmp eax, dword [x2]
 ja .direction_left
 
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
 mov eax, dword [color]
 mov edi, eax
 
 mov eax, dword [line_x_length]
 mov ebx, dword [line_y_length]
 mov ecx, dword [line_x_length]
 mov edx, dword [line_y_length]
 
 ;draw line
 mov eax, dword [line_x_length]
 cmp dword [line_y_length], eax
 ja .draw_line_up
  cmp dword [line_direction], 1
  je .right_draw_pixel_of_line
  jmp .left_draw_pixel_of_line
 .draw_line_up:
  cmp dword [line_direction], 1
  je .right_up_draw_pixel_of_line
  jmp .left_up_draw_pixel_of_line
 
 .left_draw_pixel_of_line:
  mov ebp, dword [line_x_pointer]
  cmp ebp, dword [x2]
  ja .continue_draw_left
  jmp .finish_line
  
  .continue_draw_left:
  mov dword [esi], edi ;draw pixel
  
  cmp eax, ebx
  ja .move_cursor_left
  
  ;move cursor left up
  sub esi, dword [screen_pixels_per_line] ;up
  dec dword [line_y_pointer]
  add eax, ecx
  
  .move_cursor_left:
  sub esi, 4 ;right
  dec dword [line_x_pointer]
  add ebx, edx
 jmp .left_draw_pixel_of_line
 
 .left_up_draw_pixel_of_line:
  mov ebp, dword [line_y_pointer]
  cmp ebp, dword [y2]
  ja .continue_draw_left_up
  jmp .finish_line
  
  .continue_draw_left_up:
  mov dword [esi], edi ;draw pixel
  
  cmp ebx, eax
  ja .move_cursor_left_up
  
  ;move cursor left up
  sub esi, 4 ;left
  inc dword [line_x_pointer]
  add ebx, edx
  
  .move_cursor_left_up:
  sub esi, dword [screen_pixels_per_line] ;up
  dec dword [line_y_pointer]
  add eax, ecx
 jmp .left_up_draw_pixel_of_line
 
 .right_draw_pixel_of_line:
  mov ebp, dword [line_x_pointer]
  cmp ebp, dword [x2]
  jb .continue_draw_right
  jmp .finish_line
  
  .continue_draw_right:
  mov dword [esi], edi ;draw pixel
  
  cmp eax, ebx
  ja .move_cursor_right
  
  ;move cursor right up
  sub esi, dword [screen_pixels_per_line] ;up
  dec dword [line_y_pointer]
  add eax, ecx
  
  .move_cursor_right:
  add esi, 4 ;right
  inc dword [line_x_pointer]
  add ebx, edx
 jmp .right_draw_pixel_of_line
 
 .right_up_draw_pixel_of_line:
  mov ebp, dword [line_y_pointer]
  cmp ebp, dword [y2]
  ja .continue_draw_right_up
  jmp .finish_line
  
  .continue_draw_right_up:
  mov dword [esi], edi ;draw pixel
  
  cmp ebx, eax
  ja .move_cursor_right_up
  
  ;move cursor right up
  add esi, 4 ;right
  inc dword [line_x_pointer]
  add ebx, edx
  
  .move_cursor_right_up:
  sub esi, dword [screen_pixels_per_line] ;up
  dec dword [line_y_pointer]
  add eax, ecx
 jmp .right_up_draw_pixel_of_line
 
 .finish_line:
 mov dword [esi], edi ;draw last pixel
 ret

circle_draw_8_pixels:
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [x1]
 mul ebx
 mov ecx, eax
 
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [y1]
 mul ebx
 mov ebx, dword [x1]
 shl ebx, 2 ;mul 4
 mov edx, dword [y1]
 shl edx, 2 ;mul 4
 
 mov edi, esi
 sub edi, eax
 add edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 sub edi, eax
 sub edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 add edi, edx
 sub edi, ecx
 mov dword [edi], ebp
 
 mov edi, esi
 sub edi, edx
 sub edi, ecx
 mov dword [edi], ebp
 
 mov edi, esi
 add edi, eax
 add edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 add edi, eax
 sub edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 add edi, edx
 add edi, ecx
 mov dword [edi], ebp
 
 mov edi, esi
 sub edi, edx
 add edi, ecx
 mov dword [edi], ebp
 
 ret
 
circle_draw_4_lines:
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [y1]
 mul ebx
 mov ebx, eax
 mov edx, dword [x1]
 shl edx, 2 ;mul 4

 mov edi, esi
 sub edi, ebx
 sub edi, edx
 mov eax, ebp
 mov ecx, dword [x1]
 shl ecx, 1 ;mul 2
 rep stosd
 
 mov edi, esi
 add edi, ebx
 sub edi, edx
 mov eax, ebp
 mov ecx, dword [x1]
 shl ecx, 1 ;mul 2
 rep stosd
 
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [x1]
 mul ebx
 mov ebx, eax
 mov edx, dword [y1]
 shl edx, 2 ;mul 4

 mov edi, esi
 sub edi, ebx
 sub edi, edx
 mov eax, ebp
 mov ecx, dword [y1]
 shl ecx, 1 ;mul 2
 rep stosd
 
 mov edi, esi
 add edi, ebx
 sub edi, edx
 mov eax, ebp
 mov ecx, dword [y1]
 shl ecx, 1 ;mul 2
 rep stosd
 
 ret

draw_empty_circle:
 CALCULATE_CURSOR_POSITION
 mov esi, eax
 mov ebp, dword [color]
 
 mov eax, dword [circle_radius]
 mov dword [y1], eax
 mov dword [x1], 0
 mov eax, 1001
 sub eax, dword [circle_radius]
 mov dword [decision], eax
 mov dword [de_val], 3
 mov eax, 1000
 mov ebx, dword [circle_radius]
 shl ebx, 1 ;mul 2
 sub ebx, 5
 sub eax, ebx
 mov dword [dse_val], eax
 
 .draw_pixel:
  mov eax, dword [x1]
  cmp dword [y1], eax
  je .done
  jb .done
  
  call circle_draw_8_pixels
  
  cmp dword [decision], 1000
  jb .negative_decision
  
  cmp dword [dse_val], 1000
  ja .add_dse
   mov eax, 1000
   sub eax, dword [dse_val]
   sub dword [decision], eax
   jmp .other
  .add_dse:
   mov eax, dword [dse_val]
   add dword [decision], eax
  .other:
  add dword [de_val], 2
  add dword [dse_val], 4
  inc dword [x1]
  dec dword [y1]
 jmp .draw_pixel
  
  .negative_decision:
  mov eax, dword [de_val]
  add dword [decision], eax
  add dword [de_val], 2
  add dword [dse_val], 2
  inc dword [x1]
 jmp .draw_pixel
  
 .done:
 call circle_draw_8_pixels
 ret
 
draw_circle:
 CALCULATE_CURSOR_POSITION
 mov esi, eax
 mov ebp, dword [color]
 
 mov eax, dword [circle_radius]
 mov dword [y1], eax
 mov dword [x1], 0
 mov eax, 1001
 sub eax, dword [circle_radius]
 mov dword [decision], eax
 mov dword [de_val], 3
 mov eax, 1000
 mov ebx, dword [circle_radius]
 shl ebx, 1 ;mul 2
 sub ebx, 5
 sub eax, ebx
 mov dword [dse_val], eax
 
 .draw_pixel:
  mov eax, dword [x1]
  cmp dword [y1], eax
  je .done
  jb .done
  
  call circle_draw_4_lines
  
  cmp dword [decision], 1000
  jb .negative_decision
  
  cmp dword [dse_val], 1000
  ja .add_dse
   mov eax, 1000
   sub eax, dword [dse_val]
   sub dword [decision], eax
   jmp .other
  .add_dse:
   mov eax, dword [dse_val]
   add dword [decision], eax
  .other:
  add dword [de_val], 2
  add dword [dse_val], 4
  inc dword [x1]
  dec dword [y1]
 jmp .draw_pixel
  
  .negative_decision:
  mov eax, dword [de_val]
  add dword [decision], eax
  add dword [de_val], 2
  add dword [dse_val], 2
  inc dword [x1]
 jmp .draw_pixel
  
 .done:
 call circle_draw_4_lines
 ret
 
el_ta dd 0
el_tb dd 0
el_xc dd 0
el_yc dd 0
el_x dd 0
el_y dd 0
el_ee dd 0
el_sx dd 0
el_sy dd 0

ellipse_draw_4_pixels:
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [el_x]
 mul ebx
 mov ecx, eax
 
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [el_y]
 mul ebx
 mov ebx, dword [el_x]
 shl ebx, 2 ;mul 4
 mov edx, dword [el_y]
 shl edx, 2 ;mul 4
 
 mov edi, esi
 sub edi, eax
 add edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 sub edi, eax
 sub edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 add edi, eax
 add edi, ebx
 mov dword [edi], ebp
 
 mov edi, esi
 add edi, eax
 sub edi, ebx
 mov dword [edi], ebp

 ret
 
ellipse_draw_2_lines:
 mov eax, dword [screen_pixels_per_line]
 mov ebx, dword [el_y]
 mul ebx
 mov ebx, eax
 mov edx, dword [el_x]
 shl edx, 2 ;mul 4

 mov edi, esi
 sub edi, ebx
 sub edi, edx
 mov eax, ebp
 mov ecx, dword [el_x]
 shl ecx, 1 ;mul 2
 rep stosd
 
 mov edi, esi
 add edi, ebx
 sub edi, edx
 mov eax, ebp
 mov ecx, dword [el_x]
 shl ecx, 1 ;mul 2
 rep stosd
 
 ret
 
draw_empty_ellipse:
 cmp dword [ellipse_x_radius], 3
 jb .done
 cmp dword [ellipse_y_radius], 3
 jb .done
 
 mov eax, dword [ellipse_x_radius]
 cmp eax, dword [ellipse_y_radius]
 jne .if_draw_circle
  mov dword [circle_radius], eax
  jmp draw_empty_circle
 .if_draw_circle:
 
 CALCULATE_CURSOR_POSITION
 mov esi, eax
 mov ebp, dword [color]
 
 ;calculate values
 mov eax, dword [ellipse_x_radius]
 mov ebx, eax
 mul ebx
 shl eax, 1 ;mul 2
 mov dword [el_ta], eax
 mov eax, dword [ellipse_y_radius]
 mov ebx, eax
 mul ebx
 shl eax, 1 ;mul 2
 mov dword [el_tb], eax
 
 mov dword [el_x], 0
 
 mov eax, dword [ellipse_y_radius]
 mov dword [el_y], eax
 
 mov eax, dword [ellipse_y_radius]
 mov ebx, eax
 mul ebx
 mov dword [el_xc], eax
 
 mov eax, dword [ellipse_y_radius]
 shl eax, 1 ;mul 2
 mov ebx, 1
 sub ebx, eax
 mov eax, ebx
 mov ebx, dword [ellipse_x_radius]
 mul ebx
 mul ebx
 mov dword [el_yc], eax
 
 mov dword [el_ee], 0
 
 mov dword [el_sx], 0
 
 mov eax, dword [el_ta]
 mov ebx, dword [ellipse_y_radius]
 mul ebx
 mov dword [el_sy], eax
 
 .draw_first_half:
  mov eax, dword [el_sx]
  cmp eax, dword [el_sy]
  ja .calculate_for_second_half
  
  call ellipse_draw_4_pixels
  
  inc dword [el_x]
  
  mov eax, dword [el_tb]
  add dword [el_sx], eax
  
  mov eax, dword [el_xc]
  add dword [el_ee], eax
  
  mov eax, dword [el_tb]
  add dword [el_xc], eax
  
  mov eax, dword [el_ee]
  shl eax, 1 ;mul 2
  add eax, dword [el_yc]
  cmp eax, 0x80000000
  ja .draw_first_half
  
   dec dword [el_y]
   
   mov eax, dword [el_ta]
   sub dword [el_sy], eax
   
   mov eax, dword [el_yc]
   add dword [el_ee], eax
   
   mov eax, dword [el_ta]
   add dword [el_yc], eax
  jmp .draw_first_half
  
 .calculate_for_second_half:
 mov dword [el_y], 0
 
 mov eax, dword [ellipse_x_radius]
 mov dword [el_x], eax
 
 mov eax, dword [ellipse_x_radius]
 mov ebx, eax
 mul ebx
 mov dword [el_yc], eax
 
 mov eax, dword [ellipse_x_radius]
 shl eax, 1 ;mul 2
 mov ebx, 1
 sub ebx, eax
 mov eax, ebx
 mov ebx, dword [ellipse_y_radius]
 mul ebx
 mul ebx
 mov dword [el_xc], eax
 
 mov dword [el_ee], 0
 
 mov dword [el_sy], 0
 
 mov eax, dword [el_tb]
 mov ebx, dword [ellipse_x_radius]
 mul ebx
 mov dword [el_sx], eax
 
 .draw_second_half:
  mov eax, dword [el_sx]
  cmp eax, dword [el_sy]
  jb .done
  
  call ellipse_draw_4_pixels
  
  inc dword [el_y]
  
  mov eax, dword [el_ta]
  add dword [el_sy], eax
  
  mov eax, dword [el_yc]
  add dword [el_ee], eax
  
  mov eax, dword [el_ta]
  add dword [el_yc], eax
  
  mov eax, dword [el_ee]
  shl eax, 1 ;mul 2
  add eax, dword [el_xc]
  cmp eax, 0x80000000
  ja .draw_second_half
  
   dec dword [el_x]
   
   mov eax, dword [el_tb]
   sub dword [el_sx], eax
   
   mov eax, dword [el_xc]
   add dword [el_ee], eax
   
   mov eax, dword [el_tb]
   add dword [el_xc], eax
  jmp .draw_second_half
  
 .done:
 ret
 
draw_ellipse:
 cmp dword [ellipse_x_radius], 3
 jb .done
 cmp dword [ellipse_y_radius], 3
 jb .done
 
 mov eax, dword [ellipse_x_radius]
 cmp eax, dword [ellipse_y_radius]
 jne .if_draw_circle
  mov dword [circle_radius], eax
  jmp draw_circle
 .if_draw_circle:
 
 CALCULATE_CURSOR_POSITION
 mov esi, eax
 mov ebp, dword [color]
 
 ;calculate values
 mov eax, dword [ellipse_x_radius]
 mov ebx, eax
 mul ebx
 shl eax, 1 ;mul 2
 mov dword [el_ta], eax
 mov eax, dword [ellipse_y_radius]
 mov ebx, eax
 mul ebx
 shl eax, 1 ;mul 2
 mov dword [el_tb], eax
 
 mov dword [el_x], 0
 
 mov eax, dword [ellipse_y_radius]
 mov dword [el_y], eax
 
 mov eax, dword [ellipse_y_radius]
 mov ebx, eax
 mul ebx
 mov dword [el_xc], eax
 
 mov eax, dword [ellipse_y_radius]
 shl eax, 1 ;mul 2
 mov ebx, 1
 sub ebx, eax
 mov eax, ebx
 mov ebx, dword [ellipse_x_radius]
 mul ebx
 mul ebx
 mov dword [el_yc], eax
 
 mov dword [el_ee], 0
 
 mov dword [el_sx], 0
 
 mov eax, dword [el_ta]
 mov ebx, dword [ellipse_y_radius]
 mul ebx
 mov dword [el_sy], eax
 
 .draw_first_half:
  mov eax, dword [el_sx]
  cmp eax, dword [el_sy]
  ja .calculate_for_second_half
  
  call ellipse_draw_2_lines
  
  inc dword [el_x]
  
  mov eax, dword [el_tb]
  add dword [el_sx], eax
  
  mov eax, dword [el_xc]
  add dword [el_ee], eax
  
  mov eax, dword [el_tb]
  add dword [el_xc], eax
  
  mov eax, dword [el_ee]
  shl eax, 1 ;mul 2
  add eax, dword [el_yc]
  cmp eax, 0x80000000
  ja .draw_first_half
  
   dec dword [el_y]
   
   mov eax, dword [el_ta]
   sub dword [el_sy], eax
   
   mov eax, dword [el_yc]
   add dword [el_ee], eax
   
   mov eax, dword [el_ta]
   add dword [el_yc], eax
  jmp .draw_first_half
  
 .calculate_for_second_half:
 mov dword [el_y], 0
 
 mov eax, dword [ellipse_x_radius]
 mov dword [el_x], eax
 
 mov eax, dword [ellipse_x_radius]
 mov ebx, eax
 mul ebx
 mov dword [el_yc], eax
 
 mov eax, dword [ellipse_x_radius]
 shl eax, 1 ;mul 2
 mov ebx, 1
 sub ebx, eax
 mov eax, ebx
 mov ebx, dword [ellipse_y_radius]
 mul ebx
 mul ebx
 mov dword [el_xc], eax
 
 mov dword [el_ee], 0
 
 mov dword [el_sy], 0
 
 mov eax, dword [el_tb]
 mov ebx, dword [ellipse_x_radius]
 mul ebx
 mov dword [el_sx], eax
 
 .draw_second_half:
  mov eax, dword [el_sx]
  cmp eax, dword [el_sy]
  jb .done
  
  call ellipse_draw_2_lines
  
  inc dword [el_y]
  
  mov eax, dword [el_ta]
  add dword [el_sy], eax
  
  mov eax, dword [el_yc]
  add dword [el_ee], eax
  
  mov eax, dword [el_ta]
  add dword [el_yc], eax
  
  mov eax, dword [el_ee]
  shl eax, 1 ;mul 2
  add eax, dword [el_xc]
  cmp eax, 0x80000000
  ja .draw_second_half
  
   dec dword [el_x]
   
   mov eax, dword [el_tb]
   sub dword [el_sx], eax
   
   mov eax, dword [el_xc]
   add dword [el_ee], eax
   
   mov eax, dword [el_tb]
   add dword [el_xc], eax
  jmp .draw_second_half
  
 .done:
 ret

fill_area:
 mov edi, MEMORY_GRAPHIC_FILL
 mov eax, 0
 mov ecx, 0x40000
 rep stosb
 
 CALCULATE_CURSOR_POSITION
 mov esi, eax
 mov dword [MEMORY_GRAPHIC_FILL], eax ;pointer to point
 mov ax, word [cursor_line]
 mov word [MEMORY_GRAPHIC_FILL+4], ax
 mov ax, word [cursor_column]
 mov word [MEMORY_GRAPHIC_FILL+6], ax
 mov ebx, dword [esi] ;color for replace
 mov ebp, dword [color] ;our color
 cmp ebx, ebp
 je .done
 
 mov ecx, dword [screen_pixels_per_line]
 
 .fill_1:
 mov dword [MEMORY_GRAPHIC_FILL+0x20000], 0
 
 mov eax, MEMORY_GRAPHIC_FILL
 mov edi, MEMORY_GRAPHIC_FILL+0x20000
 cmp dword [eax], 0
 je .done
  
 .expand_points:
  cmp dword [eax], 0
  je .fill_2
  
  mov esi, dword [eax]
  mov dword [esi], ebp ;draw pixel on screen
  
  cmp word [eax+4], 0
  je .if_up_1
   sub esi, ecx
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_up_1_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    dec dx
    mov word [edi+4], dx
    mov dx, word [eax+6]
    mov word [edi+6], dx
    
    add edi, 8
   .if_up_1_add_pixel:
   add esi, ecx
  .if_up_1:
  
  mov edx, dword [screen_y]
  cmp word [eax+4], dx
  je .if_down_1
   add esi, ecx
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_down_1_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    inc dx
    mov word [edi+4], dx
    mov dx, word [eax+6]
    mov word [edi+6], dx
    
    add edi, 8
   .if_down_1_add_pixel:
   sub esi, ecx 
  .if_down_1:
  
  cmp word [eax+6], 0
  je .if_left_1
   sub esi, 4
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_left_1_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    mov word [edi+4], dx
    mov dx, word [eax+6]
    dec dx
    mov word [edi+6], dx
    
    add edi, 8
   .if_left_1_add_pixel:
   add esi, 4 
  .if_left_1:
  
  mov edx, dword [screen_x]
  cmp word [eax+6], dx
  je .if_right_1
   add esi, 4
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_right_1_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    mov word [edi+4], dx
    mov dx, word [eax+6]
    inc dx
    mov word [edi+6], dx
    
    add edi, 8
   .if_right_1_add_pixel:
   sub esi, 4 
  .if_right_1:
   
  add eax, 8
 jmp .expand_points
  
 .fill_2:
 mov dword [MEMORY_GRAPHIC_FILL], 0
 
 mov eax, MEMORY_GRAPHIC_FILL+0x20000
 mov edi, MEMORY_GRAPHIC_FILL
 cmp dword [eax], 0
 je .done
  
 .expand_points_2:
  cmp dword [eax], 0
  je .fill_1
  
  mov esi, dword [eax]
  mov dword [esi], ebp ;draw pixel on screen
  
  cmp word [eax+4], 0
  je .if_up_2
   sub esi, ecx
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_up_2_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    dec dx
    mov word [edi+4], dx
    mov dx, word [eax+6]
    mov word [edi+6], dx
    
    add edi, 8
   .if_up_2_add_pixel:
   add esi, ecx 
  .if_up_2:
  
  mov edx, dword [screen_y]
  cmp word [eax+4], dx
  je .if_down_2
   add esi, ecx
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_down_2_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    inc dx
    mov word [edi+4], dx
    mov dx, word [eax+6]
    mov word [edi+6], dx
    
    add edi, 8
   .if_down_2_add_pixel:
   sub esi, ecx 
  .if_down_2:
  
  cmp word [eax+6], 0
  je .if_left_2
   sub esi, 4
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_left_2_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    mov word [edi+4], dx
    mov dx, word [eax+6]
    dec dx
    mov word [edi+6], dx
    
    add edi, 8
   .if_left_2_add_pixel:
   add esi, 4 
  .if_left_2:
  
  mov edx, dword [screen_x]
  cmp word [eax+6], dx
  je .if_right_2
   add esi, 4
   cmp dword [esi], ebx ;there is pixel of filled color
   jne .if_right_2_add_pixel
    mov dword [edi], esi
    mov dword [esi], ebp ;fill pixel
    mov dx, word [eax+4]
    mov word [edi+4], dx
    mov dx, word [eax+6]
    inc dx
    mov word [edi+6], dx
    
    add edi, 8
   .if_right_2_add_pixel:
   sub esi, 4 
  .if_right_2:
   
  add eax, 8
 jmp .expand_points_2
 
 .done:
 ret
 
draw_cursor:
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [screen_pixels_per_line]
 
 mov ecx, dword [screen_x]
 sub ecx, dword [cursor_column]
 cmp ecx, 8
 jb .border

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

 ret
 
 .border: ;if is cursor on border, draw only part
 cmp ecx, 0
 je .done
 
 ;line 1
 mov dword [eax], BLACK

 ;line 2
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line3
 mov dword [eax+4], BLACK

 ;line 3
 .line3:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line4
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line4
 mov dword [eax+8], BLACK

 ;line 4
 .line4:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line5
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line5
 mov dword [eax+8], WHITE
 cmp ecx, 3
 je .line5
 mov dword [eax+12], BLACK

 ;line 5
 .line5:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line6
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line6
 mov dword [eax+8], WHITE
 cmp ecx, 3
 je .line6
 mov dword [eax+12], WHITE
 cmp ecx, 4
 je .line6
 mov dword [eax+16], BLACK

 ;line 6
 .line6:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line7
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line7
 mov dword [eax+8], WHITE
 cmp ecx, 3
 je .line7
 mov dword [eax+12], WHITE
 cmp ecx, 4
 je .line7
 mov dword [eax+16], WHITE
 cmp ecx, 5
 je .line7
 mov dword [eax+20], BLACK

 ;line 7
 .line7:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line8
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line8
 mov dword [eax+8], WHITE
 cmp ecx, 3
 je .line8
 mov dword [eax+12], WHITE
 cmp ecx, 4
 je .line8
 mov dword [eax+16], WHITE
 cmp ecx, 5
 je .line8
 mov dword [eax+20], WHITE
 cmp ecx, 6
 je .line8
 mov dword [eax+24], BLACK

 ;line 8
 .line8:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line9
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line9
 mov dword [eax+8], WHITE
 cmp ecx, 3
 je .line9
 mov dword [eax+12], WHITE
 cmp ecx, 4
 je .line9
 mov dword [eax+16], WHITE
 cmp ecx, 5
 je .line9
 mov dword [eax+20], BLACK

 ;line 9
 .line9:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line10
 mov dword [eax+4], WHITE
 cmp ecx, 2
 je .line10
 mov dword [eax+8], WHITE
 cmp ecx, 3
 je .line10
 mov dword [eax+12], BLACK
 cmp ecx, 4
 je .line10
 mov dword [eax+16], BLACK

 ;line 10
 .line10:
 add eax, ebx ;next line
 mov dword [eax], BLACK
 cmp ecx, 1
 je .line11
 mov dword [eax+4], BLACK
 cmp ecx, 2
 je .line11
 mov dword [eax+8], BLACK

 ;line 11
 .line11:
 add eax, ebx ;next line
 mov dword [eax], BLACK

 .done:
 ret

read_cursor_bg:
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [screen_pixels_per_line]
 mov esi, old_cursor_data

 mov ecx, 11
 .read_cursor_bg:
 push ecx
  mov ecx, 10
  .read_cursor_bg_line:
   mov edx, dword [eax]
   mov dword [esi], edx
   add esi, 4
   add eax, 4
  loop .read_cursor_bg_line
  
  sub eax, 40 ;cursor length
  add eax, ebx ;next line
 pop ecx
 loop .read_cursor_bg

 ret

write_cursor_bg:
 CALCULATE_CURSOR_POSITION
 mov ebx, dword [screen_pixels_per_line]
 mov esi, old_cursor_data

 mov ecx, 11
 .read_cursor_bg:
 push ecx
  mov ecx, 10
  .read_cursor_bg_line:
   mov edx, dword [esi]
   mov dword [eax], edx
   add esi, 4
   add eax, 4
  loop .read_cursor_bg_line
  
  sub eax, 40 ;cursor length
  add eax, ebx ;next line
 pop ecx
 loop .read_cursor_bg

 ret
 
print_char:
 mov eax, dword [screen_y]
 cmp dword [cursor_line], eax
 ja .done
 mov eax, dword [screen_x]
 mov ebx, dword [size_of_text]
 shl ebx, 3 ;mul 8
 sub eax, ebx
 cmp dword [cursor_column], eax
 ja .done

 cmp dword [type_of_text], BOLD
 je .bold
 
 cmp dword [size_of_text], 1
 je print_char_size1
 
 push esi
 call print_bigger_char
 pop esi
 mov eax, dword [size_of_text]
 mov ebx, 8
 mul ebx
 sub eax, 8 ;this will be added in print method
 add dword [cursor_column], eax 
 
 .done:
 ret
 
 .bold:
 cmp dword [size_of_text], 1
 ja .big_bold
 
 call print_char_size1
 inc dword [cursor_column]
 call print_char_size1
 
 ret
 
 .big_bold:
 push esi
 call print_bigger_char
 mov eax, dword [size_of_text]
 add dword [cursor_column], eax
 call print_bigger_char
 pop esi
 mov eax, dword [size_of_text]
 mov ebx, 8
 mul ebx
 sub eax, 8 ;this will be added in print method
 add dword [cursor_column], eax 
 
 ret

print_char_size1:
 CALCULATE_CURSOR_POSITION
 
 mov eax, dword [char_for_print]
 cmp eax, 512
 jb .if_unsupported_char
  mov eax, 30 ;square
 .if_unsupported_char:

 ;calculate char memory
 mov ebx, 8
 mul ebx
 add eax, dword [font_pointer]
 mov edi, eax

 ;pointer to char memory
 mov edx, dword [screen_pointer]

 ;char have eight lines
 mov ebp, dword [color]
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
 
print_bigger_char:
 CALCULATE_CURSOR_POSITION
 
 ;pointer to screen memory
 mov edi, eax
 
 mov eax, dword [char_for_print]
 cmp eax, 512
 jb .if_unsupported_char
  mov eax, 30 ;square
 .if_unsupported_char:

 ;calculate char memory
 mov ebx, 8
 mul ebx
 add eax, dword [font_pointer]
 mov esi, eax
 
 ;other values
 mov eax, dword [size_of_text]
 mov ebx, 4
 mul ebx
 mov ebx, eax ;how many bytes to next pixel of char
 mov edx, dword [screen_pixels_per_line]
 mov ch, byte [size_of_text]
 mov cl, ch ;value for cycles
 mov word [cycle_var], cx

 ;char have eight lines
 mov ebp, dword [color]
 mov dword [actual_char_line], 0
 mov ecx, 8
 .print_char:
 push ecx
  push edi
  
  mov cx, word [cycle_var]
  DRAW_PIXEL_OF_BIG_CHAR 0x80
  DRAW_PIXEL_OF_BIG_CHAR 0x40
  DRAW_PIXEL_OF_BIG_CHAR 0x20
  DRAW_PIXEL_OF_BIG_CHAR 0x10
  DRAW_PIXEL_OF_BIG_CHAR 0x08
  DRAW_PIXEL_OF_BIG_CHAR 0x04
  DRAW_PIXEL_OF_BIG_CHAR 0x02
  DRAW_PIXEL_OF_BIG_CHAR 0x01

  ;go to next line
  pop edi
  mov al, cl ;skip all lines of one pixel of char
  .next_line:
   add edi, edx ;move on next monitor line
  dec al
  cmp al, 0
  jne .next_line
  
  inc esi ;next line of char
 pop ecx
 dec ecx
 inc dword [actual_char_line]
 cmp ecx, 0
 jne .print_char

 ret
 
draw_antialiased_pixel_of_char:
 push ebx
 mov eax, 0 ;no full pixel
 
 ;left side
 cmp dword [actual_char_pixel], 0x80
 je .if_left
  mov ebx, dword [actual_char_pixel]
  shl ebx, 1
  test byte [esi], bl
  jz .no_pixel_on_left
   or eax, 0x01000000 ;save if is some pixel here
  .no_pixel_on_left:
 .if_left:
 
 ;right side
 cmp dword [actual_char_pixel], 0x01
 je .if_right
  mov ebx, dword [actual_char_pixel]
  shr ebx, 1
  test byte [esi], bl
  jz .no_pixel_on_right
   or eax, 0x00010000 ;save if is some pixel here
  .no_pixel_on_right:
 .if_right:
 
 ;up side
 cmp dword [actual_char_line], 0
 je .if_up
  dec esi
  mov bl, byte [actual_char_pixel]
  test byte [esi], bl
  jz .no_pixel_on_up
   or eax, 0x00000100 ;save if is some pixel here
  .no_pixel_on_up:
  inc esi
 .if_up:
 
 ;down side
 cmp dword [actual_char_line], 7
 je .if_down
  mov bl, byte [actual_char_pixel]
  test byte [esi+1], bl
  jz .no_pixel_on_down
   or eax, 0x00000001 ;save if is some pixel here
  .no_pixel_on_down:
 .if_down:
 
 pop ebx
 push ebx

 ;select what will be drawed for antialiasing
 cmp eax, 0x0
 je .done
 cmp eax, 0x01010101
 je .done
 cmp eax, 0x01000100
 je .triangle_left_dec
 cmp eax, 0x01000001
 je .triangle_left_inc
 cmp eax, 0x00010100
 je .triangle_right_dec
 cmp eax, 0x00010001
 je .triangle_right_inc
 
 cmp eax, 0x00010101
 je .triangle_right_dec
 cmp eax, 0x01000101
 je .triangle_left_dec
 pop ebx
 ret
 
 .triangle_left_inc:
  mov bl, 1 ;length of drawed line
  mov cl, bl
  .draw_triangle_left_inc:
   mov eax, edi
   .draw_line_left_inc:
    mov dword [eax], BLACK
    add eax, 4
   dec cl
   cmp cl, 0
   jne .draw_line_left_inc
  
   inc bl ;next line will be longer
   mov cl, bl
   add edi, edx ;move on next line on monitor
  dec ch
  cmp ch, 0
  jne .draw_triangle_left_inc
 jmp .done
 
 .triangle_left_dec:
  mov bl, ch ;length of drawed line
  mov cl, bl
  .draw_triangle_left_dec:
   mov eax, edi
   .draw_line_left_dec:
    mov dword [eax], BLACK
    add eax, 4
   dec cl
   cmp cl, 0
   jne .draw_line_left_dec
  
   dec bl ;next line will be shorter
   mov cl, bl
   add edi, edx ;move on next line on monitor
  dec ch
  cmp ch, 0
  jne .draw_triangle_left_dec
 jmp .done
 
 .triangle_right_inc:
  add edi, ebx
  sub edi, 4
  mov bl, 1 ;length of drawed line
  mov cl, bl
  .draw_triangle_right_inc:
   mov eax, edi
   .draw_line_right_inc:
    mov dword [eax], BLACK
    sub eax, 4
   dec cl
   cmp cl, 0
   jne .draw_line_right_inc
  
   inc bl ;next line will be longer
   mov cl, bl
   add edi, edx ;move on next line on monitor
  dec ch
  cmp ch, 0
  jne .draw_triangle_right_inc
 jmp .done
 
 .triangle_right_dec:
  add edi, ebx
  sub edi, 4
  mov bl, ch ;length of drawed line
  mov cl, bl
  .draw_triangle_right_dec:
   mov eax, edi
   .draw_line_right_dec:
    mov dword [eax], BLACK
    sub eax, 4
   dec cl
   cmp cl, 0
   jne .draw_line_right_dec
  
   dec bl ;next line will be shorter
   mov cl, bl
   add edi, edx ;move on next line on monitor
  dec ch
  cmp ch, 0
  jne .draw_triangle_right_dec
 
 .done:
 pop ebx
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
 push dword [cursor_line]
 push dword [cursor_column]
 
 IF_E dword [debug_line], 600, if1
  mov dword [debug_line], 0
 ENDIF if1

 mov eax, dword [debug_line]
 mov dword [cursor_line], eax
 mov dword [cursor_column], 0
 push dword [color]
 mov dword [color], BLACK
 call print
 pop dword [color]

 ;show on screen
 mov eax, dword [debug_line]
 REDRAW_LINES_SCREEN eax, 10

 add dword [debug_line], 10

 pop dword [cursor_column]
 pop dword [cursor_line]
 
 ret

phex:
 push dword [cursor_line]
 push dword [cursor_column]
 
 IF_E dword [debug_line], 600, if1
  mov dword [debug_line], 0
 ENDIF if1

 mov eax, dword [debug_line]
 mov dword [cursor_line], eax
 mov dword [cursor_column], 0
 push dword [color]
 mov dword [color], BLACK
 call print_hex
 pop dword [color]

 ;show on screen
 mov eax, dword [debug_line]
 REDRAW_LINES_SCREEN eax, 10

 add dword [debug_line], 10

 pop dword [cursor_column]
 pop dword [cursor_line]
 
 ret

pvar:
 push dword [cursor_line]
 push dword [cursor_column]
 
 IF_E dword [debug_line], 600, if1
  mov dword [debug_line], 0
 ENDIF if1

 mov eax, dword [debug_line]
 mov dword [cursor_line], eax
 mov dword [cursor_column], 0
 push dword [color]
 mov dword [color], BLACK
 call print_var
 pop dword [color]

 ;show on screen
 mov eax, dword [debug_line]
 REDRAW_LINES_SCREEN eax, 10

 add dword [debug_line], 10

 pop dword [cursor_column]
 pop dword [cursor_line]
 
 ret
