;BleskOS

encrypting_memory dd 0
encrypting_size dd 0
encrypting_password times 20 db 0
encrypting_password_length dd 0

encrypt_memory:
 mov esi, dword [encrypting_memory]
 
 mov eax, dword [encrypting_size]
 mov ebx, 2
 mov edx, 0
 div ebx ;in one cycle we are encrypting word
 mov ecx, eax
 .encrypt_word:
 push ecx
  mov edi, encrypting_password
  mov ecx, dword [encrypting_password_length]
  .encrypt_with_one_char:
   mov al, byte [esi]
   mov bl, byte [esi+1]
   mov dl, byte [edi]
   and al, dl
   and bl, dl
   not dl
   and byte [esi], dl
   and byte [esi+1], dl
   or byte [esi], bl
   or byte [esi+1], al
   not word [esi]
   
   inc edi
  loop .encrypt_with_one_char
  
  add esi, 2
 pop ecx
 loop .encrypt_word
 
 ret
 
decrypt_memory:
 mov esi, dword [encrypting_memory]
 
 mov eax, dword [encrypting_size]
 mov ebx, 2
 mov edx, 0
 div ebx ;in one cycle we are encrypting word
 mov ecx, eax
 .encrypt_word:
 push ecx
  mov edi, encrypting_password
  add edi, dword [encrypting_password_length]
  dec edi
  mov ecx, dword [encrypting_password_length]
  .encrypt_with_one_char:
   mov al, byte [esi]
   mov bl, byte [esi+1]
   mov dl, byte [edi]
   and al, dl
   and bl, dl
   not dl
   and byte [esi], dl
   and byte [esi+1], dl
   or byte [esi], bl
   or byte [esi+1], al
   not word [esi]
   
   dec edi
  loop .encrypt_with_one_char
  
  add esi, 2
 pop ecx
 loop .encrypt_word
 
 ret
