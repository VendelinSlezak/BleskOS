;BleskOS

dp_hda_selected_output dd 1
dp_hda_selected_input dd 1

dp_hda:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 ;text
 PRINT 'Intel HD Audio', dz_hda_up_str, LINE(1), COLUMN(1)
 PRINT 'Audio Output widgets', dz_hda_aowidgets_str, LINE(3), COLUMN(1)
 PRINT 'Number of widgets:', dz_hda_ao_numof_str, LINE(5), COLUMN(1)
 mov eax, dword [hda_audio_output_numof]
 PRINT_VAR eax, LINE(5), COLUMN(21)
 PRINT 'Selected widget:', dz_hda_ao_selected_str, LINE(7), COLUMN(1)
 mov eax, dword [dp_hda_selected_output]
 PRINT_VAR eax, LINE(7), COLUMN(19)
 PRINT '[arrow up/down] +1/-1 [space] Test sound', dz_hda_ao_keys_str, LINE(9), COLUMN(1)

 call redraw_screen

 .hda_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone

  cmp byte [key_code], KEY_UP
  je .key_up

  cmp byte [key_code], KEY_DOWN
  je .key_down
 jmp .hda_halt

 .key_up:
  mov eax, dword [hda_audio_output_numof]
  cmp eax, dword [dp_hda_selected_output]
  je .hda_halt

  inc dword [dp_hda_selected_output]
  mov eax, dword [dp_hda_selected_output]
  dec eax
  mov ebx, 4
  mul ebx

  mov ecx, hda_audio_output_list
  add ecx, eax
  mov ebx, dword [ecx]
  mov dword [verb_node], ebx
  call hda_set_output_node

  DRAW_SQUARE LINE(7), COLUMN(19), COLUMNSZ*2, LINESZ, DEVELOPER_ZONE_COLOR
  mov eax, dword [dp_hda_selected_output]
  PRINT_VAR eax, LINE(7), COLUMN(19)
  REDRAW_LINES_SCREEN LINE(7), LINESZ
 jmp .hda_halt

 .key_down:
  cmp dword [dp_hda_selected_output], 1
  je .hda_halt

  dec dword [dp_hda_selected_output]
  mov eax, dword [dp_hda_selected_output]
  dec eax
  mov ebx, 4
  mul ebx

  mov ecx, hda_audio_output_list
  add ecx, eax
  mov ebx, dword [ecx]
  mov dword [verb_node], ebx
  call hda_set_output_node

  DRAW_SQUARE LINE(7), COLUMN(19), COLUMNSZ*2, LINESZ, DEVELOPER_ZONE_COLOR
  mov eax, dword [dp_hda_selected_output]
  PRINT_VAR eax, LINE(7), COLUMN(19)
  REDRAW_LINES_SCREEN LINE(7), LINESZ
 jmp .hda_halt
