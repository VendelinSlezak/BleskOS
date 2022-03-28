;BleskOS

%define LEFT 0
%define CENTER 1
%define RIGHT 2

%define NO_LIST 0
%define UNORDERED_LIST 0xFFFF
%define ORDERED_LIST 1

html_memory dd 0
html_do_not_draw dd 0
html_debug dd 0

html_start_of_screen dd 0
html_end_of_screen dd 0
html_draw_line_start dd 0
html_draw_line_end dd 0
html_draw_column_start dd 0
html_draw_column_end dd 0
html_one_char_lines dd 0
html_one_char_columns dd 0
html_aligment dd 0

html_url times 257 db 0
html_cursor_on_url dd 0

html_list_type dd NO_LIST
html_list_stack times 50 dw 0
html_list_stack_pointer dd html_list_stack

html_table_rendered dd 0
html_table_line_start dd 0
html_table_line_end dd 0
html_table_first_column dd 0
html_table_column_left dd 0
html_table_column_right dd 0

html_tag_head dw 'h', 'e', 'a', 'd', '>', 0
html_end_tag_head dw '/', 'h', 'e', 'a', 'd', '>', 0
html_tag_script dw 's', 'c', 'r', 'i', 'p', 't', '>', 0
html_tag_script_param dw 's', 'c', 'r', 'i', 'p', 't', ' ', 0
html_end_tag_script dw '/', 's', 'c', 'r', 'i', 'p', 't', '>', 0
html_tag_style dw 's', 't', 'y', 'l', 'e', '>', 0
html_tag_style_param dw 's', 't', 'y', 'l', 'e', ' ', 0
html_end_tag_style dw '/', 's', 't', 'y', 'l', 'e', '>', 0

html_tag_p dw 'p', '>', 0
html_end_tag_p dw '/', 'p', '>', 0
html_tag_br dw 'b', 'r', '>', 0
html_tag_br2 dw 'b', 'r', '/', '>', 0
html_tag_br3 dw 'b', 'r', ' ', '/', '>', 0
html_tag_b dw 'b', '>', 0
html_end_tag_b dw '/', 'b', '>', 0
html_tag_hr dw 'h', 'r', '>', 0
html_tag_hr_param dw 'h', 'r', ' ', 0
html_tag_a_param dw 'a', ' ', 0
html_end_tag_a dw '/', 'a', '>', 0

html_tag_h1 dw 'h', '1', '>', 0
html_tag_h1_param dw 'h', '1', ' ', 0
html_end_tag_h1 dw '/', 'h', '1', '>', 0
html_tag_h2 dw 'h', '2', '>', 0
html_tag_h2_param dw 'h', '2', ' ', 0
html_end_tag_h2 dw '/', 'h', '2', '>', 0
html_tag_h3 dw 'h', '3', '>', 0
html_tag_h3_param dw 'h', '3', ' ', 0
html_end_tag_h3 dw '/', 'h', '3', '>', 0
html_tag_h4 dw 'h', '4', '>', 0
html_tag_h4_param dw 'h', '4', ' ', 0
html_end_tag_h4 dw '/', 'h', '4', '>', 0
html_tag_h5 dw 'h', '5', '>', 0
html_tag_h5_param dw 'h', '5', ' ', 0
html_end_tag_h5 dw '/', 'h', '5', '>', 0
html_tag_h6 dw 'h', '6', '>', 0
html_tag_h6_param dw 'h', '6', ' ', 0
html_end_tag_h6 dw '/', 'h', '6', '>', 0

html_tag_ul dw 'u', 'l', '>', 0
html_end_tag_ul dw '/', 'u', 'l', '>', 0
html_tag_ol dw 'o', 'l', '>', 0
html_end_tag_ol dw '/', 'o', 'l', '>', 0
html_tag_li dw 'l', 'i', '>', 0
html_end_tag_li dw '/', 'l', 'i', '>', 0

html_compare_strings:
 push esi
 
 .compare_byte:
  cmp word [edi], 0
  je .equal
  mov ax, word [esi]
  cmp ax, word [edi]
  jne .not_equal
  add esi, 2
  add edi, 2
 jmp .compare_byte
  
 .equal:
 pop esi
 mov eax, 1
 ret
 
 .not_equal:
 pop esi
 mov eax, 0
 ret
 
html_set_cursor_column:
 mov eax, dword [html_draw_column_start]
 mov dword [cursor_column], eax
 ret
 
transform_html_code:
 mov eax, 0
 mov edx, 0
 mov esi, dword [html_memory]
 .transform_char:
  cmp word [esi], 0
  je .done
  cmp word [esi], '&'
  je .unicode_character
  cmp word [esi], '<'
  je .start_of_tag
  cmp word [esi], '>'
  je .end_of_tag
  cmp word [esi], ' '
  je .eat_spaces
  cmp word [esi], 0x9
  je .eat_spaces
  cmp word [esi], 0xA
  je .eat_spaces
  
  cmp eax, 1 ;convert all letters to small in tag
  jne .if_in_tag
   mov bx, word [esi]
   cmp bx, 65
   jb .if_in_tag
   cmp bx, 90
   ja .if_in_tag
   add bx, 32 ;convert from big to small letter
   mov word [esi], bx
  .if_in_tag:
  
  mov edx, 0
  
  .next_char:
  add esi, 2
 jmp .transform_char
 
 .done:
 ret
 
 .unicode_character:
  mov word [esi], 1
  mov word [esi+2], 1
  mov word [esi+4], 1
  mov word [esi+6], 1
  mov word [esi+8], 1
  mov word [esi+10], 1
  add esi, 12
 jmp .transform_char
 
 .start_of_tag:
  mov eax, 1
 jmp .next_char
 
 .end_of_tag:
  mov eax, 0
 jmp .next_char
 
 .eat_spaces:
  cmp edx, 0
  jne .if_first_space
   mov edx, 1
   jmp .next_char
  .if_first_space:
  
  ;next space
  mov word [esi], 1
  jmp .next_char

draw_html_code:
 mov dword [html_draw_line_start], 0
 mov dword [html_draw_line_end], LINESZ
 mov dword [html_draw_column_start], 0
 mov eax, dword [screen_x]
 mov dword [html_draw_column_end], eax
 mov dword [html_one_char_lines], LINESZ
 mov dword [html_one_char_columns], COLUMNSZ
 mov dword [cursor_column], 0
 mov dword [html_do_not_draw], 0
 mov dword [html_list_stack_pointer], html_list_stack
 mov dword [html_list_type], 0
 mov dword [html_cursor_on_url], 0
 
 mov esi, dword [html_memory]
 .execute_byte:
  cmp word [esi], 0
  je .done
   
  cmp word [esi], 32
  jb .next_char
  cmp word [esi], 512
  ja .next_char
  
  cmp word [esi], '<'
  jne .if_tag
  .execute_tag:
   add esi, 2
   
   ;COMMENT
   cmp dword [esi], '!' | ('-' << 16)
   jne .if_comment
    .skip_comment:
     cmp word [esi], 0
     je .done
     cmp dword [esi], '-' | ('>' << 16)
     je .end_of_comment
     add esi, 2
    jmp .skip_comment
    
    .end_of_comment:
    add esi, 4
    jmp .execute_byte
   .if_comment:
   
   ;<HEAD>
   mov edi, html_tag_head
   call html_compare_strings
   cmp eax, 1
   je .tag_head
   mov edi, html_end_tag_head
   call html_compare_strings
   cmp eax, 1
   je .end_tag_head
   
   ;<SCRIPT>
   mov edi, html_tag_script
   call html_compare_strings
   cmp eax, 1
   je .tag_script
   mov edi, html_tag_script_param
   call html_compare_strings
   cmp eax, 1
   je .tag_script
   mov edi, html_end_tag_script
   call html_compare_strings
   cmp eax, 1
   je .end_tag_script
   
   ;<STYLE>
   mov edi, html_tag_style
   call html_compare_strings
   cmp eax, 1
   je .tag_style
   mov edi, html_tag_style_param
   call html_compare_strings
   cmp eax, 1
   je .tag_style
   mov edi, html_end_tag_style
   call html_compare_strings
   cmp eax, 1
   je .end_tag_style
   
   cmp dword [html_do_not_draw], 1
   je .skip_rest_of_tag
   
   cmp word [esi], '/'
   je .ending_tag
   
   ;<P>
   mov edi, html_tag_p
   call html_compare_strings
   cmp eax, 1
   je .tag_p
   
   ;<A>
   mov edi, html_tag_a_param
   call html_compare_strings
   cmp eax, 1
   je .tag_a
   
   ;<B>
   mov edi, html_tag_b
   call html_compare_strings
   cmp eax, 1
   je .tag_b
   mov edi, html_end_tag_b
   call html_compare_strings
   cmp eax, 1
   je .end_tag_b
   
   ;<BR> <BR/> <BR />
   mov edi, html_tag_br
   call html_compare_strings
   cmp eax, 1
   je .tag_br
   mov edi, html_tag_br2
   call html_compare_strings
   cmp eax, 1
   je .tag_br
   mov edi, html_tag_br3
   call html_compare_strings
   cmp eax, 1
   je .tag_br
   
   ;<HR>
   mov edi, html_tag_hr
   call html_compare_strings
   cmp eax, 1
   je .tag_hr
   mov edi, html_tag_hr_param
   call html_compare_strings
   cmp eax, 1
   je .tag_hr
   
   ;<H1> - <H6>
   mov edi, html_tag_h1
   call html_compare_strings
   cmp eax, 1
   je .tag_h1
   mov edi, html_tag_h1_param
   call html_compare_strings
   cmp eax, 1
   je .tag_h1
   mov edi, html_end_tag_h1
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   mov edi, html_tag_h2
   call html_compare_strings
   cmp eax, 1
   je .tag_h2
   mov edi, html_tag_h2_param
   call html_compare_strings
   cmp eax, 1
   je .tag_h2
   mov edi, html_end_tag_h2
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   mov edi, html_tag_h3
   call html_compare_strings
   cmp eax, 1
   je .tag_h3
   mov edi, html_tag_h3_param
   call html_compare_strings
   cmp eax, 1
   je .tag_h3
   mov edi, html_end_tag_h3
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   mov edi, html_tag_h4
   call html_compare_strings
   cmp eax, 1
   je .tag_h4
   mov edi, html_tag_h4_param
   call html_compare_strings
   cmp eax, 1
   je .tag_h4
   mov edi, html_end_tag_h4
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   mov edi, html_tag_h5
   call html_compare_strings
   cmp eax, 1
   je .tag_h5
   mov edi, html_tag_h5_param
   call html_compare_strings
   cmp eax, 1
   je .tag_h5
   mov edi, html_end_tag_h5
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   mov edi, html_tag_h6
   call html_compare_strings
   cmp eax, 1
   je .tag_h6
   mov edi, html_tag_h6_param
   call html_compare_strings
   cmp eax, 1
   je .tag_h6
   mov edi, html_end_tag_h6
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   ;<UL>
   mov edi, html_tag_ul
   call html_compare_strings
   cmp eax, 1
   je .tag_ul
   mov edi, html_end_tag_ul
   call html_compare_strings
   cmp eax, 1
   je .end_tag_ul
   
   ;<OL>
   mov edi, html_tag_ol
   call html_compare_strings
   cmp eax, 1
   je .tag_ol
   mov edi, html_end_tag_ol
   call html_compare_strings
   cmp eax, 1
   je .end_tag_ol
   
   ;<LI>
   mov edi, html_tag_li
   call html_compare_strings
   cmp eax, 1
   je .tag_li
   mov edi, html_end_tag_li
   call html_compare_strings
   cmp eax, 1
   je .end_tag_li
   
   jmp .skip_rest_of_tag
   
   ;ENDING TAG
   .ending_tag:
   
   ;</P>
   mov edi, html_end_tag_p
   call html_compare_strings
   cmp eax, 1
   je .end_tag_p
   
   ;</A>
   mov edi, html_end_tag_a
   call html_compare_strings
   cmp eax, 1
   je .end_tag_a
   
   ;</B>
   mov edi, html_end_tag_b
   call html_compare_strings
   cmp eax, 1
   je .end_tag_b
   
   ;</H1> - </H6>
   mov edi, html_end_tag_h1
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6

   mov edi, html_end_tag_h2
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   mov edi, html_end_tag_h3
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6

   mov edi, html_end_tag_h4
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6

   mov edi, html_end_tag_h5
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6

   mov edi, html_end_tag_h6
   call html_compare_strings
   cmp eax, 1
   je .end_tag_h1_h6
   
   ;</UL>
   mov edi, html_end_tag_ul
   call html_compare_strings
   cmp eax, 1
   je .end_tag_ul
   
   ;</OL>
   mov edi, html_end_tag_ol
   call html_compare_strings
   cmp eax, 1
   je .end_tag_ol
   
   ;</LI>
   mov edi, html_end_tag_li
   call html_compare_strings
   cmp eax, 1
   je .end_tag_li
   
   .skip_rest_of_tag:
   add esi, 2
   cmp word [esi], 0
   je .done
   cmp word [esi], '>'
   jne .skip_rest_of_tag
   add esi, 2
  .if_tag:
  
  cmp word [esi], '<'
  je .execute_tag
  cmp word [esi], 32
  jb .next_char
  cmp word [esi], 512
  ja .next_char  
  cmp dword [html_do_not_draw], 1
  je .next_char
  
  mov eax, dword [html_draw_line_start]
  cmp eax, dword [html_end_of_screen]
  ja .next_char
  cmp eax, dword [html_start_of_screen]
  jb .test_end_of_line
  
  ;print char on screen
  mov eax, dword [html_draw_line_start]
  sub eax, dword [html_start_of_screen]
  add eax, 20
  mov dword [cursor_line], eax
  
  mov eax, dword [html_one_char_columns]
  add eax, dword [cursor_column]
  mov ebx, dword [html_draw_column_end]
  cmp eax, ebx
  ja .char_to_next_line
  
  ;print char to same line
  mov eax, 0
  mov ax, word [esi]
  mov dword [char_for_print], eax
  push esi
  call print_char
  pop esi
  
  jmp .test_cursor
  
  ;print char to next line
  .char_to_next_line:
  call html_set_cursor_column
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  sub eax, dword [html_start_of_screen]
  add eax, 20
  mov dword [cursor_line], eax
  mov eax, dword [html_one_char_lines]
  add dword [html_draw_line_end], eax
  
  mov eax, 0
  mov ax, word [esi]
  mov dword [char_for_print], eax
  push esi
  call print_char
  pop esi
  
  ;test if cursor is not here
  .test_cursor:
  cmp dword [html_cursor_on_url], 1
  je .if_mouse_cursor_here
  mov eax, dword [ib_mouse_line]
  mov ebx, dword [cursor_line]
  dec ebx
  cmp eax, ebx
  jb .if_mouse_cursor_here ;if cursor below first line of char
  add ebx, dword [html_one_char_lines] ;size of line
  cmp eax, ebx
  ja .if_mouse_cursor_here ;if cursor above last line of char
  mov eax, dword [ib_mouse_column]
  mov ebx, dword [cursor_column]
  cmp eax, ebx
  jb .if_mouse_cursor_here ;if cursor above first column of char
  add ebx, dword [html_one_char_columns] ;size of column
  cmp eax, ebx
  ja .if_mouse_cursor_here
   mov dword [html_cursor_on_url], 1
  .if_mouse_cursor_here:
  
  add dword [cursor_column], 8
  jmp .next_char
  
  ;move position for undrawed chars
  .test_end_of_line:
  mov eax, dword [html_one_char_columns]
  add eax, dword [cursor_column]
  mov ebx, dword [html_draw_column_end]
  cmp eax, ebx
  ja .end_of_line
  mov eax, dword [html_one_char_columns]
  add dword [cursor_column], eax
  jmp .next_char
  .end_of_line:
  call html_set_cursor_column
  mov eax, dword [html_one_char_columns]
  add dword [cursor_column], eax
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add dword [html_draw_line_end], eax
  jmp .next_char
  
  .next_char:
  add esi, 2
 jmp .execute_byte
  
 .done:
 DRAW_WINDOW_BORDERS internet_browser_up_str, internet_browser_down_str, 0x17A4AD ;light blue
 mov dword [type_of_text], PLAIN
 mov dword [size_of_text], 1
 ret
 
 .tag_head:
  mov dword [html_do_not_draw], 1
 jmp .skip_rest_of_tag
 
 .end_tag_head:
  mov dword [html_do_not_draw], 0
 jmp .skip_rest_of_tag
 
 .tag_script:
  mov dword [html_do_not_draw], 1
 jmp .skip_rest_of_tag
 
 .end_tag_script:
  mov dword [html_do_not_draw], 0
 jmp .skip_rest_of_tag
 
 .tag_style:
  mov dword [html_do_not_draw], 1
 jmp .skip_rest_of_tag
 
 .end_tag_style:
  mov dword [html_do_not_draw], 0
 jmp .skip_rest_of_tag
 
 .tag_b:
  mov dword [type_of_text], BOLD
 jmp .skip_rest_of_tag
 
 .end_tag_b:
  mov dword [type_of_text], PLAIN
 jmp .skip_rest_of_tag
 
 .tag_p:
  call html_set_cursor_column
  mov eax, dword [html_draw_line_end]
  add eax, 4
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add eax, 4
  add dword [html_draw_line_end], eax
 jmp .skip_rest_of_tag
 
 .end_tag_p:
  call html_set_cursor_column
  mov eax, dword [html_draw_line_end]
  add eax, 4
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add eax, 4
  add dword [html_draw_line_end], eax
 jmp .skip_rest_of_tag
 
 .tag_a:
  mov dword [color], 0x0000FF
  cmp dword [html_cursor_on_url], 1
  je .skip_rest_of_tag
  
  mov edi, html_url
  mov eax, 0
  mov ecx, 257
  rep stosb
  
  mov edi, esi
  
  ;find url
  .tag_a_find_param_href:
   cmp word [edi], 'h'
   jne .tag_a_next_char
   cmp word [edi+2], 'r'
   jne .tag_a_next_char
   cmp word [edi+4], 'e'
   jne .tag_a_next_char
   cmp word [edi+6], 'f'
   jne .tag_a_next_char
   cmp word [edi+8], '='
   jne .tag_a_next_char
   cmp word [edi+10], '"'
   jne .tag_a_next_char
   
   add edi, 12
   push esi
   mov esi, edi
   mov ecx, 0
   .tag_a_length_of_url:
    cmp word [edi], ' '
    je .tag_a_copy_url
    cmp word [edi], '"'
    je .tag_a_copy_url
    cmp word [edi], '>'
    je .tag_a_copy_url
    cmp word [edi], 0
    je .tag_a_copy_url
    
    inc ecx
    add edi, 2
   jmp .tag_a_length_of_url
   
   .tag_a_copy_url:
   cmp ecx, 0
   je .tag_a_end
   and ecx, 0xFF
   mov edi, html_url
   .tag_a_copy_url_char:
    mov al, byte [esi]
    mov byte [edi], al
    add esi, 2
    inc edi
   loop .tag_a_copy_url_char
   
   .tag_a_end:   
   pop esi
   jmp .skip_rest_of_tag
   
  .tag_a_next_char:
  add edi, 2
  cmp word [edi], 0
  je .skip_rest_of_tag
  cmp word [edi], '>'
  je .skip_rest_of_tag
  jmp .tag_a_find_param_href
 
 .end_tag_a:
  mov dword [color], BLACK
 jmp .skip_rest_of_tag
 
 .tag_br:
  call html_set_cursor_column
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add dword [html_draw_line_end], eax
 jmp .skip_rest_of_tag
 
 .tag_hr:
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 21
  cmp eax, dword [html_start_of_screen]
  jb .skip_rest_of_tag
  cmp eax, dword [html_end_of_screen]
  ja .skip_rest_of_tag
  sub eax, dword [html_start_of_screen]
  add eax, 12
  mov dword [cursor_line], eax
  call html_set_cursor_column
  add dword [cursor_column], 4
  
  mov eax, dword [html_draw_column_end]
  sub eax, dword [html_draw_column_start]
  sub eax, 8
  mov dword [line_length], eax
  mov dword [color], BLACK
  
  push esi
  call draw_line
  pop esi
 jmp .skip_rest_of_tag
 
 .tag_h1:
  mov eax, dword [html_draw_line_end]
  add eax, 8
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 4*LINESZ+8
 
  mov dword [size_of_text], 4
  mov dword [html_one_char_lines], 4*LINESZ
  mov dword [html_one_char_columns], 4*COLUMNSZ
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_h2:
  mov eax, dword [html_draw_line_end]
  add eax, 6
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 3*LINESZ+6
 
  mov dword [size_of_text], 3
  mov dword [html_one_char_lines], 3*LINESZ
  mov dword [html_one_char_columns], 3*COLUMNSZ
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_h3:
  mov eax, dword [html_draw_line_end]
  add eax, 4
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 2*LINESZ+4
 
  mov dword [type_of_text], BOLD
  mov dword [size_of_text], 2
  mov dword [html_one_char_lines], 2*LINESZ
  mov dword [html_one_char_columns], 2*COLUMNSZ
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_h4:
  mov eax, dword [html_draw_line_end]
  add eax, 4
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 2*LINESZ+4
 
  mov dword [size_of_text], 2
  mov dword [html_one_char_lines], 2*LINESZ
  mov dword [html_one_char_columns], 2*COLUMNSZ
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_h5:
  mov eax, dword [html_draw_line_end]
  add eax, 2
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 1*LINESZ+2
 
  mov dword [type_of_text], BOLD
  mov dword [size_of_text], 1
  mov dword [html_one_char_lines], 1*LINESZ
  mov dword [html_one_char_columns], 1*COLUMNSZ
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_h6:
  mov eax, dword [html_draw_line_end]
  add eax, 2
  mov dword [html_draw_line_start], eax
  add dword [html_draw_line_end], 1*LINESZ+2
 
  mov dword [size_of_text], 1
  mov dword [html_one_char_lines], 1*LINESZ
  mov dword [html_one_char_columns], 1*COLUMNSZ
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .end_tag_h1_h6:
  call html_set_cursor_column
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  add eax, LINESZ
  mov dword [html_draw_line_end], eax
  
  mov dword [type_of_text], PLAIN
  mov dword [size_of_text], 1
  
  mov dword [html_one_char_lines], LINESZ
  mov dword [html_one_char_columns], COLUMNSZ
 jmp .skip_rest_of_tag
 
 .tag_ul:
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  add eax, LINESZ
  mov dword [html_draw_line_end], eax
  
  add dword [html_list_stack_pointer], 2
  mov eax, dword [html_list_stack_pointer]
  mov word [eax], UNORDERED_LIST
  mov dword [html_list_type], UNORDERED_LIST
  add dword [html_draw_column_start], 8
  
  mov dword [type_of_text], PLAIN
  mov dword [size_of_text], 1
  
  mov dword [html_one_char_lines], LINESZ
  mov dword [html_one_char_columns], COLUMNSZ
  
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .end_tag_ul:
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add dword [html_draw_line_end], eax
  
  cmp dword [html_list_stack_pointer], html_list_stack
  je .skip_rest_of_tag
  sub dword [html_list_stack_pointer], 2
  mov eax, dword [html_list_stack_pointer]
  mov ebx, 0
  mov bx, word [eax]
  mov dword [html_list_type], ebx
  sub dword [html_draw_column_start], 8
  
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_ol:
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  add eax, LINESZ
  mov dword [html_draw_line_end], eax
  
  add dword [html_list_stack_pointer], 2
  mov eax, dword [html_list_stack_pointer]
  mov word [eax], ORDERED_LIST
  mov dword [html_list_type], ORDERED_LIST
  add dword [html_draw_column_start], 8
  
  mov dword [type_of_text], PLAIN
  mov dword [size_of_text], 1
  
  mov dword [html_one_char_lines], LINESZ
  mov dword [html_one_char_columns], COLUMNSZ
  
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .end_tag_ol:
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add dword [html_draw_line_end], eax
  
  cmp dword [html_list_stack_pointer], html_list_stack
  je .skip_rest_of_tag
  sub dword [html_list_stack_pointer], 2
  mov eax, dword [html_list_stack_pointer]
  mov ebx, 0
  mov bx, word [eax]
  mov dword [html_list_type], ebx
  sub dword [html_draw_column_start], 8
  
  call html_set_cursor_column
 jmp .skip_rest_of_tag
 
 .tag_li: 
  mov eax, dword [html_draw_line_start]
  cmp eax, dword [html_end_of_screen]
  ja .tag_li_not_in_screen
  cmp eax, dword [html_start_of_screen]
  jb .tag_li_not_in_screen
   
  mov eax, dword [html_draw_line_start]
  sub eax, dword [html_start_of_screen]
  add eax, 20
  mov dword [cursor_line], eax
  
  cmp dword [html_list_type], UNORDERED_LIST
  jne .if_unordered_list
   mov dword [char_for_print], 31 ;comma in middle of line
   push esi
   call print_char
   pop esi
  
   add dword [cursor_column], 8
   cmp dword [size_of_text], 1
   jne .if_tag_li_text_1
    add dword [cursor_column], 8
   .if_tag_li_text_1:
   
   jmp .skip_rest_of_tag
  .if_unordered_list:
  
  cmp dword [html_list_type], NO_LIST
  je .if_ordered_list
   mov eax, dword [html_list_stack_pointer]
   mov ebx, 0
   mov bx, word [eax]
   inc word [eax]
   inc dword [html_list_type]
   mov dword [var_print_value], ebx
   push esi
   call print_var
   pop esi
   mov dword [char_for_print], '.'
   push esi
   call print_char
   pop esi
   
   add dword [cursor_column], 8
   cmp dword [size_of_text], 1
   jne .if_tag_li_text_2
    add dword [cursor_column], 8
   .if_tag_li_text_2:
   
   jmp .skip_rest_of_tag
  .if_ordered_list:
 jmp .skip_rest_of_tag
 .tag_li_not_in_screen:
  cmp dword [html_list_type], UNORDERED_LIST
  jne .if_not_screen_unordered_list
   mov eax, dword [html_one_char_columns]
   add eax, 8
   add dword [cursor_column], eax
  .if_not_screen_unordered_list:
  
  cmp dword [html_list_type], NO_LIST
  je .skip_rest_of_tag
   ;get number of chars
   mov eax, dword [html_list_stack_pointer]
   mov ebx, 0
   mov bx, word [eax]
   mov eax, ebx
   mov ebx, 10
   mov ecx, 0
   .li_not_screen_item_chars:
    mov edx, 0
    div ebx
    inc ecx
    cmp eax, 0
    je .li_move_column
   jmp .li_not_screen_item_chars   
   .li_move_column:
   mov eax, dword [html_one_char_columns]
   mov ebx, ecx
   mul ebx
   add eax, 8
   add dword [cursor_column], eax
 jmp .skip_rest_of_tag
 
 .end_tag_li:
  mov eax, dword [html_draw_line_end]
  mov dword [html_draw_line_start], eax
  mov eax, dword [html_one_char_lines]
  add dword [html_draw_line_end], eax
  call html_set_cursor_column
 jmp .skip_rest_of_tag
