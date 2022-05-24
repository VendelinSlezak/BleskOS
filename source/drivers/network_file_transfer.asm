;BleskOS

network_ft_url_of_file times 256 dw 0
network_ft_number_of_reloading dd 0

%define NFT_NO_FILE 0
%define NFT_SEARCHING_FOR_DNS 1
%define NFT_ESTABILISHING_CONNECTION 2
%define NFT_TRANSFERRING_FILE 3
%define NFT_ERROR 0xFF
network_ft_state dd 0
network_ft_timer dd 0

network_file_transfer_pointer dd 0
network_file_transfer_length dd 0
%define NFT_TRANSFERRING_FILE 1
%define NFT_FILE_TRANSFERRED 2
%define NFT_FILE_UNKNOWN_STATE 3
network_file_transfer_state dd 0

download_file_from_network:
 cmp word [network_ft_url_of_file], 0
 je .error
 cmp word [network_ft_url_of_file], '/'
 je .error
 
 mov dword [network_ft_number_of_reloading], 0
 mov dword [network_file_transfer_state], NFT_TRANSFERRING_FILE
 mov dword [network_file_transfer_length], 0
 LOG 'Network: request for file', 0xA
 
 .transfer_file:  
 mov edi, url_for_dns
 mov ecx, 256
 mov eax, 0
 rep stosb
 mov edi, tcp_path_to_file
 mov ecx, 256
 mov eax, 0
 rep stosb
  
 ;WORKAROUND FOR HTTPS PAGES - these pages are accessed through page gate.aspero.pro
 cmp dword [network_ft_url_of_file], 'h' | ('t' << 16)
 jne .if_transfer_https_dns
 cmp dword [network_ft_url_of_file+4], 't' | ('p' << 16)
 jne .if_transfer_https_dns
 cmp dword [network_ft_url_of_file+8], 's' | (':' << 16)
 jne .if_transfer_https_dns
 cmp dword [network_ft_url_of_file+12], '/' | ('/' << 16)
 jne .if_transfer_https_dns
  mov dword [url_for_dns], 'gate'
  mov dword [url_for_dns+4], '.asp'
  mov dword [url_for_dns+8], 'ero.'
  mov word [url_for_dns+12], 'pr'
  mov byte [url_for_dns+14], 'o'
  jmp .send_dns
 .if_transfer_https_dns:
 
 ;send DNS
 mov ecx, 100
 mov esi, network_ft_url_of_file
 mov edi, url_for_dns
 .create_dns:
  cmp word [esi], 0
  je .send_dns
  cmp word [esi], '/'
  je .send_dns
  mov al, byte [esi]
  mov byte [edi], al
  add esi, 2
  inc edi
 loop .create_dns
 .send_dns:
 mov dword [type_of_received_packet], 0
 mov dword [dns_report], 0
 mov dword [network_ft_state], NFT_SEARCHING_FOR_DNS
 mov dword [network_ft_timer], 1000
 call get_ip_address_of_url
 
 ret
 
 .error:
  LOG 'Network: request for invalid URL address', 0xA
 ret
 
network_file_transfer_dns_report:
 cmp dword [dns_report], 1
 je .dns_response
 cmp dword [dns_report], 2
 je .dns_response_no_ip
 ret

 .dns_response:
 mov dword [tcp_communication_type], 0 ;NO_TCP
 mov dword [network_ft_state], NFT_ESTABILISHING_CONNECTION
 mov dword [network_ft_timer], 1000
 call create_tcp_connection
 ret
 
 .dns_response_no_ip:
 mov dword [network_ft_state], NFT_ERROR
 LOG 'Network: DNS report: this file do not exist', 0xA
 ret
 
network_file_transfer_connection_estabilished:
 mov dword [allocated_size], 1
 call allocate_memory
 mov eax, dword [allocated_memory_pointer]
 mov dword [network_file_transfer_pointer], eax
 mov dword [http_file_pointer], eax
 mov edi, eax
 mov eax, 0
 mov ecx, 0x100000
 rep stosb
  
 ;WORKAROUND FOR HTTPS PAGES - these pages are accessed through page gate.aspero.pro
 cmp dword [network_ft_url_of_file], 'h' | ('t' << 16)
 jne .if_transfer_https_tcp
 cmp dword [network_ft_url_of_file+4], 't' | ('p' << 16)
 jne .if_transfer_https_tcp
 cmp dword [network_ft_url_of_file+8], 's' | (':' << 16)
 jne .if_transfer_https_tcp
 cmp dword [network_ft_url_of_file+12], '/' | ('/' << 16)
 jne .if_transfer_https_tcp
  mov dword [tcp_path_to_file], '/?si'
  mov dword [tcp_path_to_file+4], 'te= '

  mov ecx, 100
  mov esi, network_ft_url_of_file
  mov edi, tcp_path_to_file+7
  .https_create_tcp:
   cmp word [esi], 0
   je .transfer_tcp
   mov al, byte [esi]
   mov byte [edi], al
   add esi, 2
   inc edi
  loop .https_create_tcp
  jmp .transfer_tcp
 .if_transfer_https_tcp:
  
 ;parse path from URL
 mov byte [tcp_path_to_file], '/'
 mov ecx, 100
 mov esi, network_ft_url_of_file
 mov edi, tcp_path_to_file
 .skip_to_tcp:
  cmp word [esi], 0
  je .transfer_tcp
  cmp word [esi], '/'
  je .create_tcp
  add esi, 2
 jmp .skip_to_tcp
 .create_tcp:
  cmp word [esi], 0
  je .transfer_tcp
  mov al, byte [esi]
  mov byte [edi], al
  add esi, 2
  inc edi
 loop .create_tcp
 
 ;start transferring file
 .transfer_tcp:
 mov dword [network_ft_state], NFT_TRANSFERRING_FILE
 call tcp_transfer_file
 mov dword [network_ft_timer], 3000
 
 ret
  
network_file_transfer_packet:
 cmp dword [tcp_communication_type], 6 ;TCP_FINALIZED
 je network_file_transfer_explore_file

 mov eax, dword [tcp_file_transferred_length]
 mov dword [network_file_transfer_length], eax
 mov dword [network_ft_timer], 3000 ;reset timer
 
 ret

network_file_transfer_explore_file:
 mov dword [network_ft_state], NFT_NO_FILE
 mov dword [network_ft_timer], 0
 
 cmp dword [http_reported_state], 0
 je .file_was_successfully_transfered
 cmp dword [http_reported_state], 1 ;HTTP_MOVED_PERMANENTLY
 je .load_new_location
 
 .unknown_state:
  mov eax, dword [network_file_transfer_pointer]
  mov dword [allocated_memory_pointer], eax
  mov dword [allocated_size], 1
  call release_memory
   
  mov dword [network_file_transfer_state], NFT_FILE_UNKNOWN_STATE
  LOG 'Network: file was not transferred', 0xA
 ret
 
 .file_was_successfully_transfered:
  mov dword [network_file_transfer_state], NFT_FILE_TRANSFERRED
  mov eax, dword [tcp_file_transferred_length]
  mov dword [network_file_transfer_length], eax
  LOG 'Network: file successfully transferred with length '
  shr eax, 10 ;convert to KB
  LOG_VAR eax
  LOG 'KB', 0xA
 ret

 .load_new_location:
  LOG 'Network: url was redirected', 0xA
  mov eax, dword [network_file_transfer_pointer]
  mov dword [allocated_memory_pointer], eax
  mov dword [allocated_size], 1
  call release_memory
   
  cmp dword [network_ft_number_of_reloading], 5
  ja .too_many_reloading
  cmp byte [http_moved_permanently_url], '/'
  je .new_location_add
  cmp dword [http_moved_permanently_url+1], 'ttp:'
  je .new_location_http
  cmp dword [http_moved_permanently_url+1], 'ttps'
  je .new_location_https
  LOG 'Network: unknown HTTP response '
  mov esi, http_moved_permanently_url
  call log_print
  LOG 0xA
  jmp .unknown_state
  
  .new_location_add:
   mov edi, network_ft_url_of_file+510
   .new_location_add_move_to_end_of_url:
    cmp edi, network_ft_url_of_file
    je .unknown_state ;error
    cmp word [edi], '/'
    je .new_location_add_copy_url
    cmp word [edi], 0
    jne .new_location_add_to_url_with_end_0
    sub edi, 2
   jmp .new_location_add_move_to_end_of_url
    
   .new_location_add_to_url_with_end_0:
   add edi, 2
   mov word [edi], '/'
   .new_location_add_copy_url:
   mov esi, http_moved_permanently_url
   mov ecx, 100
   .new_location_add_copy_char:
    mov ax, 0
    mov al, byte [esi]
    mov word [edi], ax
    inc esi
    add edi, 2
   loop .new_location_add_copy_char
  jmp .reload_url
   
  .new_location_http:
   mov edi, network_ft_url_of_file
   mov eax, 0
   mov ecx, 100
   rep stosw
   mov esi, http_moved_permanently_url+7
   mov edi, network_ft_url_of_file
   mov ecx, 100
   .new_location_http_copy_char:
    mov ax, 0
    mov al, byte [esi]
    mov word [edi], ax
    inc esi
    add edi, 2
   loop .new_location_http_copy_char
  jmp .reload_url
   
  .new_location_https:
   mov edi, network_ft_url_of_file
   mov eax, 0
   mov ecx, 100
   rep stosw
   mov esi, http_moved_permanently_url
   mov edi, network_ft_url_of_file
   mov ecx, 100
   .new_location_https_copy_char:
    mov ax, 0
    mov al, byte [esi]
    mov word [edi], ax
    inc esi
    add edi, 2
   loop .new_location_https_copy_char
  jmp .reload_url
   
 .reload_url:
  inc dword [network_ft_number_of_reloading]
 jmp download_file_from_network.transfer_file ;reload url
   
 .too_many_reloading:
  LOG 'Network: URL was redirected too many times', 0xA
 jmp .unknown_state
