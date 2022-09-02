;BleskOS

converted_value dd 0
converted_value_string times 10 dw 0
first_number dd 0
second_number dd 0
divide_number_before_comma dd 0
divide_number_after_comma dd 0
divide_number_after_comma_string times 10 dw 0
numbers_after_comma dd 0

convert_string_to_number:
 mov dword [converted_value], 0
 
 cmp word [esi], '0'
 je .done
 
 mov eax, 0
 mov ecx, 0
 mov ebx, 10
 .convert_char:
  cmp word [esi], '0'
  jb .converted
  cmp word [esi], '9'
  ja .converted
  mul ebx
  mov cx, word [esi]
  sub cx, '0'
  add eax, ecx
  add esi, 2
 jmp .convert_char
 
 .converted:
 mov dword [converted_value], eax
 
 .done:
 ret

convert_number_to_string: 
 mov edi, converted_value_string
 mov eax, 0
 mov ecx, 10
 rep stosw
 
 mov edi, converted_value_string+16
 mov eax, dword [converted_value]
 mov ecx, 0
 mov ebx, 10
 .convert_char:
  mov edx, 0
  div ebx
  add edx, '0'
  mov word [edi], dx
  inc ecx
  cmp ecx, 9
  je .copy
  cmp eax, 0
  je .copy
  sub edi, 2
 jmp .convert_char
 
 .copy:
 inc ecx
 mov edi, esi
 mov esi, converted_value_string
 push ecx
 mov ecx, 9
 .skip_zeroes:
  cmp word [esi], 0
  jne .copy_string
  add esi, 2
 loop .skip_zeroes
 .copy_string:
 pop ecx
 rep movsw
 
 .done:
 ret

divide_with_comma:
 mov dword [divide_number_before_comma], 0
 mov dword [divide_number_after_comma], 0
 
 cmp dword [second_number], 0
 je .done
 
 mov edi, divide_number_after_comma_string
 mov eax, 0
 mov ecx, 10
 rep stosw
 
 mov eax, dword [first_number]
 mov ebx, dword [second_number]
 mov edx, 0
 div ebx
 mov dword [divide_number_before_comma], eax
 
 mov esi, divide_number_after_comma_string
 mov ecx, dword [numbers_after_comma]
 cmp ecx, 0
 je .return
 .divide_one_number_after_comma:
  cmp edx, 0
  je .done
  mov eax, edx
  mov ebx, 10
  mul ebx
  mov ebx, dword [second_number]
  mov edx, 0
  div ebx
  add ax, '0'
  mov word [esi], ax
  add esi, 2
 loop .divide_one_number_after_comma
 
 .done:
 mov esi, divide_number_after_comma_string
 call convert_string_to_number
 mov eax, dword [converted_value]
 mov dword [divide_number_after_comma], eax
 .return
 ret
