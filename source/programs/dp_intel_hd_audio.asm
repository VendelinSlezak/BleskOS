;BleskOS

dp_hda_selected_output dd 1
dp_hda_selected_input dd 1

dp_hda:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 cmp dword [hda_base], 0
 je .no_hda

 ;text
 mov dword [cursor_line], LINE(1)
 mov dword [color], BLACK
 mov esi, hda_nodes_list
 mov ecx, 32
 .print_values:
 push ecx
  mov dword [cursor_column], COLUMN(1)
  mov eax, 0
  mov al, byte [esi]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(12)
  mov eax, 0
  mov al, byte [esi+1]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(23)
  mov eax, 0
  mov al, byte [esi]
  mov dword [verb_node], eax
  mov dword [verb_verb], 0xF02
  mov dword [verb_command], 0x0
  call hda_send_verb
  mov eax, dword [hda_response]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(50)
  mov eax, 0
  mov al, byte [esi+64]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(62)
  mov eax, 0
  mov al, byte [esi+65]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(73)
  mov eax, 0
  mov al, byte [esi+64]
  mov dword [verb_node], eax
  mov dword [verb_verb], 0xF02
  mov dword [verb_command], 0x0
  call hda_send_verb
  mov eax, dword [hda_response]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

 add dword [cursor_line], LINESZ
 add esi, 2
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .print_values

 call redraw_screen

 .hda_halt:
  call wait_for_usb_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .hda_halt

 .no_hda:
 PRINT 'This sound card is not HDA audio compatibile', no_hda_str, LINE(1), COLUMN(1)
 call redraw_screen
 .no_hda_halt:
  call wait_for_usb_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .no_hda_halt
