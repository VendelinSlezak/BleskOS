;BleskOS

; file of Table editor

;
; FUNCTIONS METHODS
;

be_cell_number_value dd 0
be_cell_function_value dd 0
be_number_to_str times 20 dw 0

be_return_number_of_cell:
 call be_selected_cell_pointer
 add esi, 8
 
 mov eax, 0
 mov ebx, 10
 mov ecx, 9
 .one_char:
  mov edx, 0
  mov dx, word [esi]
  cmp dx, 0
  je .done
  cmp dx, '0'
  jb .not_number
  cmp dx, '9'
  ja .not_number
  mul ebx
  mov edx, 0
  mov dx, word [esi]
  sub edx, '0'
  add eax, edx
  add esi, 2
 loop .one_char
 
 .done:
 mov dword [be_cell_number_value], eax
 ret
 
 .not_number:
 mov dword [be_cell_number_value], 0
 ret
 
be_recalculate_cell:
 mov dword [be_cell_function_value], 0
 
 call be_selected_cell_pointer
 cmp word [esi+8], 0xFFFF
 jne .done ;not function cell
 
 cmp word [esi+10], 1
 jne .if_function_sum
  push esi
  call be_create_list_of_cell_area
  mov edx, 0
  mov esi, dword [be_function_cell_list_pointer]
  .function_sum_calculate_cell:
   cmp byte [esi], 0
   je .function_sum_done
   cmp byte [esi], 1
   jne .function_sum_next_cell
   add edx, dword [esi+1]
   .function_sum_next_cell:
   add esi, 5
  jmp .function_sum_calculate_cell
  
  .function_sum_done:
  pop esi
  add esi, 108
  mov dword [converted_value], edx
  call convert_number_to_string
  
  ret
 .if_function_sum:
 
 cmp word [esi+10], 2
 jne .if_function_sumif
  mov ebx, 0
  mov bx, word [esi+20]
  mov ecx, dword [esi+22]
  push esi
  push ebx
  push ecx
  call be_create_list_of_cell_area
  pop ecx
  pop ebx
  mov edx, 0
  mov esi, dword [be_function_cell_list_pointer]
  .function_sumif_calculate_cell:
   cmp byte [esi], 0
   je .function_sumif_done
   cmp byte [esi], 1
   jne .function_sumif_next_cell

   cmp ebx, '='
   je .function_sumif_equal
   cmp ebx, '<'
   je .function_sumif_below
   cmp ebx, '>'
   je .function_sumif_above
   jmp .function_sumif_done
   
   .function_sumif_equal:
   cmp dword [esi+1], ecx
   je .function_sumif_add
   jmp .function_sumif_next_cell
   
   .function_sumif_below:
   cmp dword [esi+1], ecx
   jb .function_sumif_add
   jmp .function_sumif_next_cell
   
   .function_sumif_above:
   cmp dword [esi+1], ecx
   ja .function_sumif_add
   jmp .function_sumif_next_cell
   
   .function_sumif_add:
   add edx, dword [esi+1]
   
   .function_sumif_next_cell:
   add esi, 5
  jmp .function_sumif_calculate_cell
  
  .function_sumif_done:
  pop esi
  add esi, 108
  mov dword [converted_value], edx
  call convert_number_to_string
  
  ret
 .if_function_sumif:
 
 cmp word [esi+10], 3
 jne .if_function_count
  push esi
  call be_create_list_of_cell_area
  mov edx, 0
  mov esi, dword [be_function_cell_list_pointer]
  .function_count_calculate_cell:
   cmp byte [esi], 0
   je .function_count_done
   cmp byte [esi], 1
   jne .function_count_next_cell
   inc edx
   .function_count_next_cell:
   add esi, 5
  jmp .function_count_calculate_cell
  
  .function_count_done:
  pop esi
  add esi, 108
  mov dword [converted_value], edx
  call convert_number_to_string
  
  ret
 .if_function_count:
 
 cmp word [esi+10], 4
 jne .if_function_countif
  mov ebx, 0
  mov bx, word [esi+20]
  mov ecx, dword [esi+22]
  push esi
  push ebx
  push ecx
  call be_create_list_of_cell_area
  pop ecx
  pop ebx
  mov edx, 0
  mov esi, dword [be_function_cell_list_pointer]
  .function_countif_calculate_cell:
   cmp byte [esi], 0
   je .function_countif_done
   cmp byte [esi], 1
   jne .function_countif_next_cell

   cmp ebx, '='
   je .function_countif_equal
   cmp ebx, '<'
   je .function_countif_below
   cmp ebx, '>'
   je .function_countif_above
   jmp .function_countif_done
   
   .function_countif_equal:
   cmp dword [esi+1], ecx
   je .function_countif_add
   jmp .function_countif_next_cell
   
   .function_countif_below:
   cmp dword [esi+1], ecx
   jb .function_countif_add
   jmp .function_countif_next_cell
   
   .function_countif_above:
   cmp dword [esi+1], ecx
   ja .function_countif_add
   jmp .function_countif_next_cell
   
   .function_countif_add:
   inc edx
   
   .function_countif_next_cell:
   add esi, 5
  jmp .function_countif_calculate_cell
  
  .function_countif_done:
  pop esi
  add esi, 108
  mov dword [converted_value], edx
  call convert_number_to_string
  
  ret
 .if_function_countif:
 
 cmp word [esi+10], 5
 jne .if_function_average
  push esi
  call be_create_list_of_cell_area
  mov ecx, 0
  mov edx, 0
  mov esi, dword [be_function_cell_list_pointer]
  .function_average_calculate_cell:
   cmp byte [esi], 0
   je .function_average_done
   cmp byte [esi], 1
   jne .function_average_next_cell
   inc ecx
   add edx, dword [esi+1]
   .function_average_next_cell:
   add esi, 5
  jmp .function_average_calculate_cell
  
  .function_average_done:
  pop esi
  mov eax, 0
  mov ax, word [esi+20]
  mov dword [numbers_after_comma], eax
  mov dword [first_number], edx
  mov dword [second_number], ecx
  push eax
  push esi
  call divide_with_comma
  pop esi
  add esi, 108
  
  mov eax, dword [divide_number_before_comma]
  mov dword [converted_value], eax
  call convert_number_to_string
  pop eax
  cmp eax, 0
  je .done
  sub edi, 2
  mov word [edi], ','
  add edi, 2
  mov esi, edi
  mov eax, dword [divide_number_after_comma]
  mov dword [converted_value], eax
  call convert_number_to_string
  
  ret
 .if_function_average:
 
 .done:
 ret
 
be_create_list_of_cell_area:
 push dword [be_selected_cell_line]
 push dword [be_selected_cell_column]
 mov edi, dword [be_function_cell_list_pointer]
 
 mov eax, 0
 mov ax, word [esi+12]
 mov dword [be_selected_cell_line], eax
 mov ax, word [esi+14]
 mov dword [be_selected_cell_column], eax
 
 mov ecx, 0
 mov cx, word [esi+16]
 sub cx, word [esi+12]
 inc ecx
 and ecx, 0xFFF
 .line:
 push ecx
  push dword [be_selected_cell_column]
  
  mov ecx, 0
  mov cx, word [esi+18]
  sub cx, word [esi+14]
  inc ecx
  and ecx, 0xFFF
  .column:
  push ecx
  push esi
   call be_selected_cell_pointer
   
   mov ax, word [esi+8]
   cmp ax, 0
   je .skip_cell
   cmp ax, '0'
   jb .string_cell
   cmp ax, '9'
   ja .string_cell
   
   ;number cell
   add esi, 8
   call convert_string_to_number
   mov byte [edi], 1
   mov eax, dword [converted_value]
   mov dword [edi+1], eax
   add edi, 5
   jmp .skip_cell
   
   .string_cell:
   mov byte [edi], 2
   mov dword [edi+1], esi
   add edi, 5
   
   .skip_cell:
   inc dword [be_selected_cell_column]
  pop esi
  pop ecx
  loop .column
  
  inc dword [be_selected_cell_line]
  pop dword [be_selected_cell_column]
 pop ecx
 loop .line
 
 mov dword [edi], 0
 
 pop dword [be_selected_cell_column]
 pop dword [be_selected_cell_line]
 
 ret

be_function_input_cell:
 mov eax, dword [screen_y_center]
 sub eax, 93
 mov dword [cursor_line], eax
 mov eax, dword [screen_x_center]
 add eax, 160
 mov dword [cursor_column], eax
 mov dword [text_input_pointer], be_function_cell_input_string
 mov dword [text_input_length], 4
 call text_input
 cmp byte [key_code], KEY_ESC
 je .done
 
 mov esi, be_function_cell_input_string 
 mov ecx, 4
 .change_spaces_to_zero:
  cmp word [esi], ' '
  jne .next_char
  mov word [esi], 0
 .next_char:
 add esi, 2
 loop .change_spaces_to_zero
 
 cmp word [be_function_cell_input_string], 'A'
 jb .not_cell
 cmp word [be_function_cell_input_string], 'Z'
 ja .not_cell
 cmp word [be_function_cell_input_string+2], '0'
 jb .not_cell
 cmp word [be_function_cell_input_string+2], '9'
 ja .not_cell
 cmp word [be_function_cell_input_string+4], 0
 je .convert_to_line_and_column
 cmp word [be_function_cell_input_string+4], '0'
 jb .not_cell
 cmp word [be_function_cell_input_string+4], '9'
 ja .not_cell
 cmp word [be_function_cell_input_string+6], 0
 je .convert_to_line_and_column
 cmp word [be_function_cell_input_string+6], '0'
 jb .not_cell
 cmp word [be_function_cell_input_string+6], '9'
 ja .not_cell
 
 .convert_to_line_and_column:
 mov eax, 0
 mov ax, word [be_function_cell_input_string]
 sub eax, 'A'
 mov dword [be_function_cell_input_column], eax
 
 mov ax, word [be_function_cell_input_string+2]
 sub eax, '0'
 mov dword [be_function_cell_input_line], eax
 cmp word [be_function_cell_input_string+4], 0
 je .test_line
 mov ebx, 10
 mul ebx
 mov ecx, 0
 mov cx, word [be_function_cell_input_string+4]
 sub ecx, '0'
 add eax, ecx
 mov dword [be_function_cell_input_line], eax
 cmp word [be_function_cell_input_string+6], 0
 je .test_line
 mov ebx, 10
 mul ebx
 mov cx, word [be_function_cell_input_string+6]
 sub ecx, '0'
 add eax, ecx
 mov dword [be_function_cell_input_line], eax
 
 .test_line:
 cmp dword [be_function_cell_input_line], 0
 jne .done
 inc dword [be_function_cell_input_line]
 .done:
 ret

 .not_cell:
  mov edi, be_function_cell_input_string
  mov eax, 0
  mov ecx, 5
  rep stosw
 jmp be_function_input_cell
