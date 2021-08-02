;BleskOS

dp_hda_selected_output dd 1
dp_hda_selected_input dd 1

dp_hda:
 ;orange background
 CLEAR_SCREEN DEVELOPER_ZONE_COLOR

 ;text
 PRINT 'Line Out:', line_out_str, LINE(1), COLUMN(1)
 mov eax, dword [hda_line_out_node]
 PRINT_VAR eax, LINE(1), COLUMN(11)

 PRINT 'Line In:', line_in_str, LINE(3), COLUMN(1)
 mov eax, dword [hda_line_in_node]
 PRINT_VAR eax, LINE(3), COLUMN(10)

 PRINT 'Speaker:', speaker_str, LINE(5), COLUMN(1)
 mov eax, dword [hda_speaker_node]
 PRINT_VAR eax, LINE(5), COLUMN(10)

 PRINT 'Microphone:', mic_str, LINE(7), COLUMN(1)
 mov eax, dword [hda_mic_node]
 PRINT_VAR eax, LINE(7), COLUMN(13)

 PRINT '[a] codec 0 [b] codec 1', commands_str, LINE(9), COLUMN(1)

 call redraw_screen

 .hda_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone

  cmp byte [key_code], KEY_A
  je .key_a

  cmp byte [key_code], KEY_B
  je .key_b
 jmp .hda_halt

 .key_a:
  mov dword [verb_codec], 0
  call codec_find_widgets
 jmp dp_hda

 .key_b:
  mov dword [verb_codec], 1
  call codec_find_widgets
 jmp dp_hda
