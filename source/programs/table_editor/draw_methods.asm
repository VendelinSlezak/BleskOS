;BleskOS

; file of Table editor

be_draw_table:
 DRAW_WINDOW table_editor_up_str, table_editor_down_str, 0x00FF00, WHITE
 mov dword [size_of_text], 1
 
 mov eax, dword [screen_x]
 DRAW_SQUARE 20, 0, eax, 15, 0xAAAAAA
 mov eax, dword [screen_y]
 sub eax, 40
 DRAW_SQUARE 35, 0, 27, eax, 0xFF8000
 mov eax, dword [screen_x]
 DRAW_SQUARE 35, 0, eax, 13, 0xFF8000
 
 ;draw table lines
 mov eax, dword [screen_x]
 DRAW_LINE 35, 0, eax, BLACK
 mov eax, dword [screen_x]
 DRAW_LINE 48, 0, eax, BLACK
 mov dword [cursor_line], 48
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 .draw_lines:
  push esi
  mov dword [cursor_column], 0
  mov dword [color], BLACK
  mov dword [line_length], 27
  call draw_line
  mov dword [cursor_column], 27
  mov dword [color], 0xBBBBBB
  SCREEN_X_SUB eax, 27
  mov dword [line_length], eax
  call draw_line
  pop esi
  
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [cursor_line], eax
  inc esi
  
  cmp esi, be_lines_length+1000
  je .table_lines_erase_place_after
  
  mov eax, dword [cursor_line]
  cmp eax, dword [screen_y]
  ja .table_lines_drawed
 jmp .draw_lines
 
 ;erase place after last line
 .table_lines_erase_place_after:
  inc dword [cursor_line]
  mov dword [cursor_column], 0
  mov dword [color], WHITE
  mov eax, dword [screen_x]
  mov dword [square_length], eax
  mov eax, dword [screen_y]
  cmp eax, dword [cursor_line]
  jb .table_lines_drawed
  je .table_lines_drawed
  sub eax, dword [cursor_line]
  call draw_square 
 .table_lines_drawed:
 
 ;draw table columns
 mov dword [cursor_line], 35
 mov dword [cursor_column], 0
 mov dword [color], BLACK
 mov eax, dword [screen_y]
 sub eax, 40
 mov dword [column_heigth], eax
 call draw_column
 add dword [cursor_column], 27
 call draw_column
 mov dword [color], 0xBBBBBB
 mov edx, 27 ;drawed columns
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .draw_columns:
  mov eax, 0
  mov al, byte [esi]
  shl eax, 3 ;mul 8
  add eax, 3 ;number of columns to next drawed column
  mov ebx, edx
  add ebx, eax
  cmp ebx, dword [screen_x]
  ja .columns_drawed
  
  add dword [cursor_column], eax
  add edx, eax
  
  push esi
  push edx
  push ecx
  mov dword [cursor_line], 35
  mov dword [column_heigth], 13
  mov dword [color], BLACK
  call draw_column
  mov dword [cursor_line], 49
  mov eax, dword [screen_y]
  sub eax, 44
  mov dword [column_heigth], eax
  mov dword [color], 0xBBBBBB
  call draw_column
  pop ecx
  pop edx
  pop esi
  
  inc esi
 loop .draw_columns
 
 ;all columns are on screen, so this will erase place after them
 mov eax, 0
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .count_columns:
  mov ebx, 0
  mov bl, byte [esi]
  shl ebx, 3 ;mul 8
  add ebx, 3
  add eax, ebx
  inc esi
 loop .count_columns
 add eax, 28
 mov dword [cursor_column], eax
 mov dword [cursor_line], 35
 mov ebx, dword [screen_x]
 sub ebx, eax
 mov dword [square_length], ebx
 SCREEN_Y_SUB eax, 40
 mov dword [square_heigth], eax
 mov dword [color], WHITE
 call draw_square
 
 .columns_drawed:
 
 ;print numbers of lines
 mov eax, dword [be_first_show_line]
 mov dword [var_print_value], eax
 mov dword [cursor_line], 48
 mov dword [color], BLACK
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 .draw_lines_numbers:
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  shr eax, 1 ;div 2
  sub eax, 2
  add dword [cursor_line], eax ;middle of line
 
  mov dword [cursor_column], 11
  cmp dword [var_print_value], 10
  jb .print_line_number
  mov dword [cursor_column], 6
  cmp dword [var_print_value], 100
  jb .print_line_number
  mov dword [cursor_column], 2
  .print_line_number:
  push eax
  push esi
  call print_var
  pop esi
  pop eax
  sub dword [cursor_line], eax
  
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [cursor_line], eax
  inc esi
  
  cmp esi, be_lines_length+1000
  je .table_lines_numbers_drawed
  
  mov eax, dword [cursor_line]
  cmp eax, dword [screen_y]
  ja .table_lines_numbers_drawed
  inc dword [var_print_value]
 jmp .draw_lines_numbers
 .table_lines_numbers_drawed:
 
 ;print chars of columns
 mov eax, dword [be_first_show_column]
 add eax, 'A'
 mov dword [char_for_print], eax
 mov dword [cursor_line], 39
 mov dword [cursor_column], 27
 mov dword [color], BLACK
 mov edx, 27 ;drawed columns
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .draw_columns_chars:
  mov eax, 0
  mov al, byte [esi]
  shl eax, 3 ;mul 8
  add eax, 3 ;number of columns to next drawed column
  mov edi, eax
  mov ebx, edx
  add ebx, eax
  
  add dword [cursor_column], eax
  add edx, eax
  
  push ebx
  push esi
  push edx
  push ecx
  push dword [cursor_column]
  shr edi, 1 ;div 2
  add edi, 4
  sub dword [cursor_column], edi
  call print_char
  pop dword [cursor_column]
  pop ecx
  pop edx
  pop esi
  pop ebx
  
  inc esi
  inc dword [char_for_print]
  
  cmp ebx, dword [screen_x]
  ja .columns_chars_drawed
 loop .draw_columns_chars
 .columns_chars_drawed:
 
 ;PRINT CELLS CONTENT
 ;
 ; byte 0: bit 0 - bold
 ;         bit 1 - italic
 ;         bit 2:3 - aligment 0=left 1=middle 2=right
 ;         bit 4 - right border
 ;         bit 5 - left border
 ;         bit 6 - down border
 ;         bit 7 - up border
 ; byte 1: bit 0:1 - aligment 0=up 1=middle 2=right
 ; byte 2-5: color of background
 ; byte 6-8: color of text
 ; byte 9-208: cell text in unicode
 ;
 ; if first char is 0xFFFF, it mean that this cell contain function
 ; next word contain number of function
 ; 1 = SUM
 
 mov ebp, dword [table_editor_file_pointer]
 mov eax, dword [be_first_show_line]
 dec eax
 mov ebx, 208*26 ;bytes per line
 mul ebx
 add ebp, eax
 mov eax, dword [be_first_show_column]
 mov ebx, 208
 mul ebx
 add ebp, eax ;pointer to first showed cell content
 mov dword [cursor_line], 20+15+14
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi ;pointer to length of cell
 mov edi, be_lines_length
 add edi, dword [be_first_show_line]
 dec edi ;pointer to heigth of cell
 .draw_cells_lines_cycle:
 push esi
  push ebp
  
  mov esi, be_columns_length
  add esi, dword [be_first_show_column] ;pointer to length of cell
  mov eax, 27
  .draw_cells_columns_cycle:
   mov dword [cursor_column], eax
   inc dword [cursor_column]
   
   mov ebx, 0
   mov bl, byte [esi]
   shl ebx, 3 ;mul 8
   add ebx, 3
   add ebx, eax
   
   ;here we have cursor_line and cursor_column on start of cell and ebp points to it's content
   pusha
    ;draw background
    mov eax, dword [ebp+2]
    and eax, 0x00FFFFFF
    cmp eax, WHITE
    je .skip_drawing_background
    mov dword [color], eax
    mov eax, 0
    mov al, byte [esi]
    shl eax, 3 ;mul 8
    add eax, 3
    mov dword [square_length], eax
    dec dword [square_length]
    mov eax, 0
    mov al, byte [edi]
    mov ebx, 13
    mul ebx
    dec eax
    mov dword [square_heigth], eax
    call draw_square
    .skip_drawing_background:
    
    ;draw border
    dec dword [cursor_line]
    dec dword [cursor_column]
    mov al, byte [ebp+0]
    test al, 0x80 ;up border
    jz .if_up_border
     mov eax, 0
     mov al, byte [esi]
     shl eax, 3 ;mul 8
     add eax, 3
     mov dword [line_length], eax
     mov dword [color], BLACK
     push edi
     call draw_line
     pop edi
    .if_up_border:
    
    mov al, byte [ebp+0]
    test al, 0x40 ;down border
    jz .if_down_border
     mov eax, 0
     mov al, byte [edi]
     mov ebx, 13
     mul ebx
     add dword [cursor_line], eax
     mov eax, 0
     mov al, byte [esi]
     shl eax, 3 ;mul 8
     add eax, 3
     mov dword [line_length], eax
     mov dword [color], BLACK
     push edi
     call draw_line
     pop edi
     
     mov eax, 0
     mov al, byte [edi]
     mov ebx, 13
     mul ebx
     sub dword [cursor_line], eax
    .if_down_border:
    
    mov al, byte [ebp+0]
    test al, 0x20 ;left border
    jz .if_left_border
     mov eax, 0
     mov al, byte [edi]
     mov ebx, 13
     mul ebx
     inc eax
     mov dword [column_heigth], eax
     mov dword [color], BLACK
     call draw_column
    .if_left_border:
    
    mov al, byte [ebp+0]
    test al, 0x10 ;right border
    jz .if_right_border
     push dword [cursor_column]
     mov eax, 0
     mov al, byte [esi]
     shl eax, 3 ;mul 8
     add eax, 3
     add dword [cursor_column], eax
     mov eax, 0
     mov al, byte [edi]
     mov ebx, 13
     mul ebx
     inc eax
     mov dword [column_heigth], eax
     mov dword [color], BLACK
     call draw_column
     pop dword [cursor_column]
    .if_right_border:
    inc dword [cursor_line]
    inc dword [cursor_column]
    
    ;set text setting
    mov eax, dword [ebp+5]
    and eax, 0x00FFFFFF
    mov dword [color], eax
    mov dword [type_of_text], PLAIN
    test byte [ebp], 0x1
    jz .if_bold_text
     mov dword [type_of_text], BOLD
    .if_bold_text:
    
    ;set text aligment
    push dword [cursor_line]
    push dword [cursor_column]
    
    add dword [cursor_line], 2
    
    mov al, byte [ebp+1]
    and al, 0x03
    cmp al, 0
    je .if_aligment_line_up
     cmp byte [edi], 1
     je .if_aligment_line_up
     
     cmp al, 0x1
     jne .if_aligment_line_middle
      mov eax, 0
      mov al, byte [edi]
      mov ebx, 13
      mul ebx
      shr eax, 1
      sub eax, 6
      add dword [cursor_line], eax
      jmp .if_aligment_line_up
     .if_aligment_line_middle:
     
     cmp al, 0x2
     jne .if_aligment_line_down
      mov eax, 0
      mov al, byte [edi]
      mov ebx, 13
      mul ebx
      sub eax, 13
      add dword [cursor_line], eax
      jmp .if_aligment_line_up
     .if_aligment_line_down:
    .if_aligment_line_up:
    
    mov al, byte [ebp+0]
    and al, 0x0C
    cmp al, 0x00
    jne .if_aligment_left
     inc dword [cursor_column]
     jmp .print_cell_string
    .if_aligment_left:
    
    cmp al, 0x04
    jne .if_aligment_middle
     mov edi, ebp
     add edi, 8
     cmp word [edi], 0xFFFF ;function
     jne .if_aligment_middle_function
      add edi, 100
     .if_aligment_middle_function:
     mov eax, 0 ;number of chars in string
     mov bl, byte [esi] ;number of showed chars in column
     .aligment_middle_count_chars:
      cmp word [edi], 0
      je .aligment_middle_calculate
      cmp al, bl
      je .aligment_middle_calculate
      inc eax
      add edi, 2
     jmp .aligment_middle_count_chars
     
     .aligment_middle_calculate:
     shl eax, 2 ;mul 4 ;this is half of length of string
     mov ebx, 0
     mov bl, byte [esi]
     shl ebx, 3 ;mul 8
     add ebx, 3 ;this is length of column
     shr ebx, 1 ;div 2 
     sub ebx, eax ;this is first draw column for aligmented string
     add dword [cursor_column], ebx
     jmp .print_cell_string
    .if_aligment_middle:
    
    cmp al, 0x08
    jne .if_aligment_right
     mov edi, ebp
     add edi, 8
     cmp word [edi], 0xFFFF ;function
     jne .if_aligment_right_function
      add edi, 100
     .if_aligment_right_function:
     mov eax, 0 ;number of chars in string
     mov bl, byte [esi] ;number of showed chars in column
     .aligment_right_count_chars:
      cmp word [edi], 0
      je .aligment_right_calculate
      cmp al, bl
      je .aligment_right_calculate
      inc eax
      add edi, 2
     jmp .aligment_right_count_chars
     
     .aligment_right_calculate:
     shl eax, 3 ;mul 8 
     inc eax ;this is length of string
     mov ebx, 0
     mov bl, byte [esi]
     shl ebx, 3 ;mul 8
     add ebx, 3 ;this is length of column
     sub ebx, eax ;this is first draw column for aligmented string
     add dword [cursor_column], ebx
     jmp .print_cell_string
    .if_aligment_right:
    
    ;print string  
    .print_cell_string:  
    mov eax, ebp
    add eax, 8 ;text offset
    cmp word [eax], 0xFFFF ;function
    jne .if_print_function
     add eax, 100 ;text offset in function
    .if_print_function:
    mov ecx, 0
    mov cl, byte [esi] ;number of showed chars
    test byte [ebp], 0x1
    jz .if_bold_text_2
     dec cl
    .if_bold_text_2:
    .print_char_from_cell:
     mov ebx, 0
     mov bx, word [eax]
     cmp bx, 0
     je .end_of_printing_cell
     mov dword [char_for_print], ebx
     push ecx
     push eax
     call print_char
     pop eax
     pop ecx
     add eax, 2
     add dword [cursor_column], COLUMNSZ
    loop .print_char_from_cell
    .end_of_printing_cell:
    
    pop dword [cursor_column]
    pop dword [cursor_line]
   popa
   
   cmp ebx, dword [screen_x]
   ja .draw_cells_next_line
   
   mov eax, ebx
   inc esi
   add ebp, 208
  jmp .draw_cells_columns_cycle
  
  .draw_cells_next_line:
  pop ebp
  add ebp, 208*26 ;bytes per line
  inc edi
 pop esi
 mov eax, 0
 mov al, byte [esi]
 mov ebx, 13
 mul ebx
 add dword [cursor_line], eax
 inc esi
 
 cmp esi, be_lines_length+1000
 je .cell_content_drawed
 
 mov eax, dword [cursor_line]
 cmp eax, dword [screen_y]
 jb .draw_cells_lines_cycle
 .cell_content_drawed:

 ;highlight selected cell
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 mov dword [cursor_line], 48
 mov ecx, dword [be_first_show_line]
 .find_selected_cell_line:
  cmp ecx, dword [be_selected_cell_line]
  je .selected_cell_line_founded
  
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [cursor_line], eax
  inc ecx
  inc esi
 jmp .find_selected_cell_line 
 .selected_cell_line_founded:
 mov eax, 0
 mov al, byte [esi]
 mov ebx, 13
 mul ebx
 mov dword [square_heigth], eax
 
 mov eax, 27 ;column
 mov ebx, dword [be_first_show_column] ;skiped columns
 mov esi, be_columns_length
 add esi, dword [be_first_show_column]
 mov ecx, 26
 sub ecx, dword [be_first_show_column]
 .find_selected_cell_column:
  cmp ebx, dword [be_selected_cell_column]
  je .selected_cell_column_founded
  
  mov ecx, 0
  mov cl, byte [esi]
  shl ecx, 3 ;mul 8
  add ecx, 3
  add eax, ecx
  inc ebx
  inc esi
 loop .find_selected_cell_column
 
 .selected_cell_column_founded:
 mov dword [cursor_column], eax
 mov esi, be_columns_length
 add esi, dword [be_selected_cell_column]
 mov eax, 0
 mov al, byte [esi]
 shl eax, 3 ;mul 8
 add eax, 3
 mov dword [square_length], eax
 mov dword [color], BLACK
 call draw_empty_square
 dec dword [cursor_line]
 dec dword [cursor_column]
 add dword [square_length], 2
 add dword [square_heigth], 2
 call draw_empty_square
 
 ;DRAW TOOLS
 call be_redraw_input
 
 call be_selected_cell_pointer
 push esi
 push esi
 push esi
 push esi
 
 ;full border
 mov al, byte [esi]
 and al, 0xF0
 cmp al, 0xF0
 jne .if_show_full_border
  SCREEN_X_SUB eax, 1+13
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_full_border:
 SCREEN_X_SUB eax, 1+13
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 1+13-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, BLACK
 
 ;down border
 pop esi
 test byte [esi], 0x40
 jz .if_show_down_border
  SCREEN_X_SUB eax, 2+(13*2)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_down_border:
 SCREEN_X_SUB eax, 2+(13*2)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 2+(13*2)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 2+(13*2)-2
 DRAW_LINE 23+8, eax, 9, BLACK
 
 ;right border
 pop esi
 test byte [esi], 0x10
 jz .if_show_right_border
  SCREEN_X_SUB eax, 3+(13*3)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_right_border:
 SCREEN_X_SUB eax, 3+(13*3)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 3+(13*3)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 3+(13*3)-10
 DRAW_COLUMN 23, eax, 9, BLACK
 
 ;up border
 pop esi
 test byte [esi], 0x80
 jz .if_show_up_border
  SCREEN_X_SUB eax, 4+(13*4)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_up_border:
 SCREEN_X_SUB eax, 4+(13*4)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 4+(13*4)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 4+(13*4)-2
 DRAW_LINE 23, eax, 9, BLACK
 
 ;left border
 pop esi
 test byte [esi], 0x20
 jz .if_show_left_border
  SCREEN_X_SUB eax, 5+(13*5)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_left_border:
 SCREEN_X_SUB eax, 5+(13*5)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 5+(13*5)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 SCREEN_X_SUB eax, 5+(13*5)-2
 DRAW_COLUMN 23, eax, 9, BLACK
 
 ;no border
 mov al, byte [esi]
 and al, 0xF0
 cmp al, 0x00
 jne .if_show_no_border
  SCREEN_X_SUB eax, 6+(13*6)
  DRAW_SQUARE 21, eax, 12, 12, 0x00FF00
 .if_show_no_border:
 SCREEN_X_SUB eax, 6+(13*6)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 6+(13*6)-2
 DRAW_EMPTY_SQUARE 23, eax, 8, 8, 0x888888
 
 ;show aligment
 mov al, byte [esi]
 and al, 0x0C
 cmp al, 0x08
 jne .if_show_right_aligment
  SCREEN_X_SUB eax, 9+(13*7)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_right_aligment:
 SCREEN_X_SUB eax, 9+(13*7)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 9+(13*7)-2
 PRINT_CHAR 'R', 24, eax
 
 mov al, byte [esi]
 and al, 0x0C
 cmp al, 0x04
 jne .if_show_middle_aligment
  SCREEN_X_SUB eax, 10+(13*8)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_middle_aligment:
 SCREEN_X_SUB eax, 10+(13*8)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 10+(13*8)-2
 PRINT_CHAR 'M', 24, eax
 
 mov al, byte [esi]
 and al, 0x0C
 cmp al, 0x00
 jne .if_show_left_aligment
  SCREEN_X_SUB eax, 11+(13*9)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_left_aligment:
 SCREEN_X_SUB eax, 11+(13*9)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 11+(13*9)-2
 PRINT_CHAR 'L', 24, eax
 
 mov al, byte [esi+1]
 and al, 0x03
 cmp al, 0x02
 jne .if_show_line_down_aligment
  SCREEN_X_SUB eax, 12+(13*10)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_line_down_aligment:
 SCREEN_X_SUB eax, 12+(13*10)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 12+(13*10)-2
 DRAW_SQUARE 30, eax, 9, 2, BLACK
 
 mov al, byte [esi+1]
 and al, 0x03
 cmp al, 0x01
 jne .if_show_line_middle_aligment
  SCREEN_X_SUB eax, 13+(13*11)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_line_middle_aligment:
 SCREEN_X_SUB eax, 13+(13*11)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 13+(13*11)-2
 DRAW_SQUARE 27, eax, 9, 2, BLACK
 
 mov al, byte [esi+1]
 and al, 0x03
 cmp al, 0x00
 jne .if_show_line_up_aligment
  SCREEN_X_SUB eax, 14+(13*12)
  DRAW_SQUARE 21, eax, 12, 12, 0xFF0000
 .if_show_line_up_aligment:
 SCREEN_X_SUB eax, 14+(13*12)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 14+(13*12)-2
 DRAW_SQUARE 23, eax, 9, 2, BLACK
 
 ;show type of text
 test byte [esi], 0x1
 jz .if_show_bold
  SCREEN_X_SUB eax, 17+(13*13)
  DRAW_SQUARE 21, eax, 12, 12, 0x0088FF
 .if_show_bold:
 SCREEN_X_SUB eax, 17+(13*13)
 DRAW_EMPTY_SQUARE 21, eax, 12, 12, BLACK
 SCREEN_X_SUB eax, 17+(13*13)-2
 PRINT_CHAR 'B', 24, eax
 
 ;color of text
 call be_selected_cell_pointer
 mov eax, dword [esi+5]
 and eax, 0x00FFFFFF
 mov dword [color], eax
 SCREEN_X_SUB eax, 19+(13*15)-2
 push eax
 push eax
 PRINT_CHAR 'A', 24, eax
 pop eax
 add eax, COLUMNSZ
 PRINT_CHAR 'B', 24, eax
 pop eax
 add eax, COLUMNSZ*2
 PRINT_CHAR 'C', 24, eax
 
 ;color of background
 call be_selected_cell_pointer
 mov ebx, dword [esi+2]
 and ebx, 0x00FFFFFF
 mov dword [color], ebx
 SCREEN_X_SUB eax, 20+(13*17)
 DRAW_SQUARE 21, eax, 24, 12, ebx
 SCREEN_X_SUB eax, 20+(13*17)
 DRAW_EMPTY_SQUARE 21, eax, 24, 12, BLACK
 SCREEN_X_SUB eax, 20+(13*17)-1
 DRAW_EMPTY_SQUARE 22, eax, 22, 10, BLACK
 
 mov dword [type_of_text], PLAIN
 DRAW_WINDOW_BORDERS table_editor_up_str, table_editor_down_str, 0x00FF00
 
 mov eax, dword [be_mouse_line]
 mov dword [cursor_line], eax
 mov eax, dword [be_mouse_column]
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 ret
 
be_redraw_input:
 SCREEN_X_SUB eax, 27+21+(13*18)
 DRAW_SQUARE 21, 27, eax, 13, WHITE
 SCREEN_X_SUB eax, 27+21+(13*18)
 DRAW_EMPTY_SQUARE 21, 27, eax, 12, BLACK
 call be_selected_cell_pointer
 add esi, 8 ;text offset
 
 cmp word [esi], 0xFFFF ;function
 jne .if_function
  mov dword [cursor_line], 23
  mov dword [cursor_column], 29
  mov dword [char_for_print], 31
  call print_char
  add dword [cursor_column], COLUMNSZ
  
  cmp word [esi+2], 1 ;SUM
  jne .if_sum_function
   push esi
   mov esi, be_sum_str
   call print
   pop esi
   
   mov eax, 0
   mov ax, word [esi+6]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+4]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ':'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+10]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+8]
   mov dword [var_print_value], eax
   call print_var
   
   mov dword [char_for_print], ')'
   call print_char
  .if_sum_function:
  
  cmp word [esi+2], 2 ;SUMIF
  jne .if_sumif_function
   push esi
   mov esi, be_sumif_str
   call print
   pop esi
   
   mov eax, 0
   mov ax, word [esi+6]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+4]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ':'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+10]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+8]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ';'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov dword [char_for_print], '"'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+12]
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [esi+14]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], '"'
   call print_char
   
   add dword [cursor_column], COLUMNSZ
   mov dword [char_for_print], ')'
   call print_char
  .if_sumif_function:
  
  cmp word [esi+2], 3 ;COUNT
  jne .if_count_function
   push esi
   mov esi, be_count_str
   call print
   pop esi
   
   mov eax, 0
   mov ax, word [esi+6]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+4]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ':'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+10]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+8]
   mov dword [var_print_value], eax
   call print_var
   
   mov dword [char_for_print], ')'
   call print_char
  .if_count_function:
  
  cmp word [esi+2], 4 ;COUNTIF
  jne .if_countif_function
   push esi
   mov esi, be_countif_str
   call print
   pop esi
   
   mov eax, 0
   mov ax, word [esi+6]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+4]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ':'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+10]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+8]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ';'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov dword [char_for_print], '"'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+12]
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, dword [esi+14]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], '"'
   call print_char
   
   add dword [cursor_column], COLUMNSZ
   mov dword [char_for_print], ')'
   call print_char
  .if_countif_function:
  
  cmp word [esi+2], 5 ;AVERAGE
  jne .if_average_function
   push esi
   mov esi, be_average_str
   call print
   pop esi
   
   mov eax, 0
   mov ax, word [esi+6]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+4]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ':'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+10]
   add eax, 'A'
   mov dword [char_for_print], eax
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+8]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ';'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], COLUMNSZ
   mov eax, 0
   mov ax, word [esi+20]
   mov dword [var_print_value], eax
   push esi
   call print_var
   pop esi
   
   mov dword [char_for_print], ')'
   call print_char
  .if_average_function:
  
  mov dword [be_cursor_offset], 0
  ret
 .if_function:
 
 mov dword [cursor_line], 23
 mov dword [cursor_column], 29
 mov ecx, 60 ;show max 60 chars
 .draw_char:
  mov eax, 0
  mov ax, word [esi]
  cmp ax, 0
  je .draw_cursor
  mov dword [char_for_print], eax
  push esi
  push ecx
  call print_char
  pop ecx
  pop esi
  add dword [cursor_column], COLUMNSZ
  add esi, 2
 loop .draw_char
 
 .draw_cursor:
 mov eax, dword [be_cursor_offset]
 shl eax, 3 ;mul 8
 add eax, 29
 cmp dword [be_cursor_offset], 0
 jne .if_zero
  dec eax
 .if_zero:
 mov dword [cursor_column], eax
 mov dword [cursor_line], 22
 mov dword [column_heigth], 11
 mov dword [color], 0x444444
 call draw_column
 ret
 
be_redraw_cell:
 mov esi, be_lines_length
 add esi, dword [be_first_show_line]
 dec esi
 mov dword [first_redraw_line], 48
 mov ecx, dword [be_first_show_line]
 .skip_line:
  cmp ecx, dword [be_selected_cell_line]
  je .redraw_cell
 
  mov eax, 0
  mov al, byte [esi]
  mov ebx, 13
  mul ebx
  add dword [first_redraw_line], eax
  inc esi
  inc ecx
 jmp .skip_line
 
 .redraw_cell:
 mov eax, 0
 mov al, byte [esi]
 mov ebx, 13
 mul ebx
 mov dword [how_much_lines_redraw], eax
 call redraw_lines_screen
 
 ret
