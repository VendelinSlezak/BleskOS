;BleskOS

%macro LOG 1+
 section string_data
%%string:
 db %1, 0
 section .text
 
 pusha
 mov esi, %%string
 call log_print
 popa
%endmacro

%macro LOG_HEX 1
 pusha
 %if %1!=eax
 mov eax, %1
 %endif
 mov dword [log_hex_print_value], eax
 call log_print_hex
 popa
%endmacro

%macro LOG_VAR 1
 pusha
 %if %1!=eax
 mov eax, %1
 %endif
 mov dword [log_var_print_value], eax
 call log_print_var
 popa
%endmacro

log_pointer dd MEMORY_LOG
log_hex_print_value dd 0
log_var_print_value dd 0
log_hex_string times 12 db 0
log_var_string times 12 db 0

log_print:
 mov edi, dword [log_pointer]
 .insert_char:
  mov al, byte [esi]
  cmp al, 0
  je .done
  
  mov byte [edi], al
  
  inc esi
  inc edi
  
  cmp edi, MEMORY_LOG+0xFFFFF
  jne .if_end_of_log
   mov edi, MEMORY_LOG
  .if_end_of_log:
 jmp .insert_char
 
 .done:
 mov dword [log_pointer], edi
 ret
 
%macro LOG_CONVERT_HEX_TO_CHAR 2
 MOV_0xF_SHIFT eax, dword [log_hex_print_value], %1
 add al, '0' ;convert to char
 mov byte [log_hex_string+%2], al
 IF_H al, '9', if%2
   add al, 7 ;convert to char
   mov byte [log_hex_string+%2], al
 ENDIF if%2
%endmacro

log_print_hex:
 mov byte [log_hex_string], '0'
 mov byte [log_hex_string+1], 'x'

 LOG_CONVERT_HEX_TO_CHAR 28, 2
 LOG_CONVERT_HEX_TO_CHAR 24, 3
 LOG_CONVERT_HEX_TO_CHAR 20, 4
 LOG_CONVERT_HEX_TO_CHAR 16, 5
 LOG_CONVERT_HEX_TO_CHAR 12, 6
 LOG_CONVERT_HEX_TO_CHAR 8, 7
 LOG_CONVERT_HEX_TO_CHAR 4, 8
 LOG_CONVERT_HEX_TO_CHAR 0, 9
 mov byte [log_hex_string+10], ' '
 mov byte [log_hex_string+11], 0

 mov esi, log_hex_string
 call log_print
 
 ret
 
%macro LOG_SEPARATE_DIGIT_PRINT_VAR 1
 mov ebx, 10
 mov edx, 0 ;need for divide
 div ebx
 add dl, '0' ;convert to char
 mov byte [log_var_string+%1], dl
%endmacro

%macro LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 1
 cmp byte [log_var_string+%1], '0'
 jne .print_var
 inc esi
%endmacro

log_print_var:
 mov eax, [log_var_print_value]
 LOG_SEPARATE_DIGIT_PRINT_VAR 9
 LOG_SEPARATE_DIGIT_PRINT_VAR 8
 LOG_SEPARATE_DIGIT_PRINT_VAR 7
 LOG_SEPARATE_DIGIT_PRINT_VAR 6
 LOG_SEPARATE_DIGIT_PRINT_VAR 5
 LOG_SEPARATE_DIGIT_PRINT_VAR 4
 LOG_SEPARATE_DIGIT_PRINT_VAR 3
 LOG_SEPARATE_DIGIT_PRINT_VAR 2
 LOG_SEPARATE_DIGIT_PRINT_VAR 1
 LOG_SEPARATE_DIGIT_PRINT_VAR 0
 mov byte [log_var_string+10], ' '
 mov byte [log_var_string+11], 0 ;end of string

 ;find start of non-zero string
 mov esi, log_var_string

 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 0
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 1
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 2
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 3
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 4
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 5
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 6
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 7
 LOG_IF_DIGIT_NON_ZERO_PRINT_VAR 8

 .print_var:
 call log_print
 
 ret
