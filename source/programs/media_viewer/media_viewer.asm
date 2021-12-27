;BleskOS

%define MV_IMAGE 1
%define MV_VIDEO 2
%define MV_SOUND 3

%define MV_STOP 0
%define MV_RESUME 1
%define MV_PLAY 2
%define MV_PLAY32 3

media_viewer_up_str db 'Media viewer', 0
media_viewer_down_str db '[F2] Open file', 0
media_viewer_down_close_str db '[F3] Close file', 0

media_viewer_file_type dd 0
media_viewer_file_mem dd 0
media_viewer_img_width dd 0
media_viewer_img_heigth dd 0
media_viewer_img_first_line dd 0
media_viewer_img_first_column dd 0

media_viewer_img_monitor_line dd 0
media_viewer_img_monitor_column dd 0
media_viewer_img_max_lines dd 0
media_viewer_img_max_columns dd 0

media_viewer_sound_blocks dd 0
media_viewer_sound_samples dd 0
media_viewer_sound_length dd 0
media_viewer_sound_length_sec dd 0
media_viewer_sound_length_min dd 0
media_viewer_sound_length_milisec dd 0
media_viewer_sound_played_sec dd 0
media_viewer_sound_played_min dd 0
media_viewer_sound_timer dd 0
media_viewer_sound_state dd 0

media_viewer_file_pointer dd 0

media_viewer:
 DRAW_WINDOW media_viewer_up_str, media_viewer_down_str, 0x00FFEF, WHITE
 cmp dword [media_viewer_file_pointer], 0
 jne .draw_loaded_image
 
 .redraw_screen:
 call redraw_screen

 .media_viewer_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F2
  je .open_file
 jmp .media_viewer_halt
 
 .draw_loaded_image:
  mov eax, dword [media_viewer_file_pointer]
  mov dword [file_memory], eax
  jmp media_viewer_draw_image_to_center

 .open_file:
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je media_viewer
  
  cmp dword [file_type], 'bmp'
  je .convert_bmp_file
  cmp dword [file_type], 'BMP'
  je .convert_bmp_file
  cmp dword [file_type], 'wav'
  je .play_wav_file
  cmp dword [file_type], 'WAV'
  je .play_wav_file
  jmp media_viewer
  
  .convert_bmp_file:
  call convert_bmp_file
  mov eax, dword [file_memory]
  mov dword [media_viewer_file_pointer], eax
  jmp media_viewer_draw_image_to_center
  
  .play_wav_file:
  mov esi, dword [allocated_memory_pointer]
  jmp media_viewer_play_file
  
media_viewer_draw_image_to_center:
 mov eax, dword [file_memory]
 cmp word [eax], 0
 je media_viewer ;invalid header
 cmp word [eax+2], 0
 je media_viewer ;invalid header
  
 add eax, 8 ;skip image info
 mov dword [media_viewer_file_mem], eax
 sub eax, 8
 mov bx, word [eax]
 mov word [media_viewer_img_width], bx
 mov bx, word [eax+2]
 mov word [media_viewer_img_heigth], bx
 mov dword [media_viewer_img_first_line], 0
 mov dword [media_viewer_img_first_column], 0
  
 mov eax, dword [screen_x]
 cmp eax, dword [media_viewer_img_width]
 jb .image_width_above_monitor
  mov eax, dword [media_viewer_img_width]
  mov ebx, 2
  mov edx, 0
  div ebx
  mov ecx, dword [screen_x_center]
  sub ecx, eax
  mov dword [media_viewer_img_monitor_column], ecx
  
  mov eax, dword [media_viewer_img_width]
  mov dword [media_viewer_img_max_columns], eax
  jmp .cmp_image_heigth
 .image_width_above_monitor:
  mov dword [media_viewer_img_monitor_column], 0
  mov eax, dword [screen_x]
  mov dword [media_viewer_img_max_columns], eax
  
 .cmp_image_heigth:  
 mov eax, dword [screen_y]
 sub eax, 40
 cmp eax, dword [media_viewer_img_heigth]
 jb .image_heigth_above_monitor
  mov eax, dword [media_viewer_img_heigth]
  mov ebx, 2
  mov edx, 0
  div ebx
  mov ecx, dword [screen_y_center]
  sub ecx, eax
  mov dword [media_viewer_img_monitor_line], ecx
  
  mov eax, dword [media_viewer_img_heigth]
  mov dword [media_viewer_img_max_lines], eax
  jmp media_viewer_draw_image
 .image_heigth_above_monitor:
  mov dword [media_viewer_img_monitor_line], 20
  mov eax, dword [screen_y]
  sub eax, 40
  mov dword [media_viewer_img_max_lines], eax
 jmp media_viewer_draw_image

media_viewer_draw_image:
 mov eax, dword [media_viewer_file_mem]
 mov dword [li_source_memory], eax
 mov eax, dword [media_viewer_img_width]
 mov dword [li_source_width], eax
 mov eax, dword [media_viewer_img_heigth]
 mov dword [li_source_heigth], eax
 mov eax, dword [media_viewer_img_first_line]
 mov dword [li_source_line], eax
 mov eax, dword [media_viewer_img_first_column]
 mov dword [li_source_column], eax
 mov eax, dword [media_viewer_img_max_lines]
 mov dword [li_image_heigth], eax
 mov eax, dword [media_viewer_img_max_columns]
 mov dword [li_image_width], eax
 
 mov dword [li_destination_memory], LI_MONITOR
 mov eax, dword [media_viewer_img_monitor_line]
 mov dword [li_destination_line], eax
 mov eax, dword [media_viewer_img_monitor_column]
 mov dword [li_destination_column], eax
 
 DRAW_WINDOW media_viewer_up_str, media_viewer_down_str, 0x00FFEF, WHITE
 call transfer_image
 
 jmp media_viewer.redraw_screen

media_viewer_play_file:
 cmp dword [ac97_nam_base], 0
 jne .ac97
 
 ;Intel HD Audio
 mov eax, esi
 add eax, 40
 mov dword [hda_data_pointer], eax
 mov eax, dword [esi+4]
 mov dword [hda_data_length], eax
 mov word [hda_data_format], 0x0011
 call hda_play_sound
 
 DRAW_WINDOW media_viewer_up_str, media_viewer_down_str, 0x00FFEF, WHITE
 call redraw_screen
 
 WAIT 2000
 
 HDA_OUTPUT_STREAM_POS
 PHEX eax
 
 jmp media_viewer.redraw_screen
 
 .ac97:
 mov eax, dword [esi+4]
 cmp word [esi+20], 1
 jne .can_not_play
 cmp word [esi+22], 2
 jne .can_not_play
 
 ;calculate number of ac97 blocks
 mov eax, dword [esi+4]
 mov ebx, 0xFFFF*2
 mov edx, 0
 div ebx
 inc eax
 mov dword [media_viewer_sound_blocks], eax
 
 ;save number of all samples
 mov eax, dword [esi+40]
 mov dword [media_viewer_sound_samples], eax
 mov dword [media_viewer_sound_length], eax
 
 ;calculate number of seconds and minutes
 mov ebx, dword [esi+28] ;samples per second
 mov edx, 0
 div ebx
 mov dword [media_viewer_sound_length_sec], eax
 mov ebx, 500
 mul ebx
 mov dword [media_viewer_sound_length_milisec], eax
 mov eax, dword [media_viewer_sound_length_sec]
 mov ebx, 60
 mov edx, 0
 div ebx
 mov dword [media_viewer_sound_length_min], eax
 mov dword [media_viewer_sound_length_sec], edx
 
 ;set sample rate
 mov eax, dword [esi+24]
 mov dword [ac97_sample_rate], eax
 call ac97_set_sample_rate
 
 ;set start of sound data
 mov dword [media_viewer_sound_state], MV_STOP
 mov eax, esi
 add eax, 44
 mov dword [ac97_sound_data], eax
 cmp dword [media_viewer_sound_samples], 0xFFFF*2
 ja .many_buffers
 ;one buffer
 mov ax, word [media_viewer_sound_samples]
 mov word [ac97_sound_length], ax
 call ac97_fill_buffer
 mov dword [media_viewer_sound_samples], 0
 jmp .ac97_redraw
 
 .many_buffers:
 mov word [ac97_sound_length], 0xFFFF
 call ac97_fill_buffer
 sub dword [media_viewer_sound_samples], 0xFFFF*2
 add dword [ac97_sound_data], 0xFFFF*2
 
 mov dword [enable_ticks3], 0
 mov dword [ticks3], 0
 
 .ac97_redraw:
 DRAW_WINDOW media_viewer_up_str, media_viewer_down_close_str, 0x00FFEF, WHITE
 
 PRINT 'AUDIO FILE', audio_file_str, 20+LINESZ*1, COLUMNSZ*1
 
 SCREEN_Y_SUB eax, 20+LINESZ*2
 PRINT '[space] play [enter] stop [up/down] volume 1 step [right/left] volume 10 steps', playing_keys_str, eax, COLUMNSZ*1
 
 SCREEN_Y_SUB eax, 20+LINESZ*2
 SCREEN_X_SUB ebx, COLUMNSZ*12
 PRINT '00:00/', time_str, eax, ebx
 
 SCREEN_Y_SUB eax, 20+LINESZ*2
 mov dword [cursor_line], eax
 SCREEN_X_SUB eax, COLUMNSZ*6
 mov dword [cursor_column], eax
 
 mov dword [char_for_print], '0'
 cmp dword [media_viewer_sound_length_min], 9
 ja .if_min_above_10
  call print_char
  add dword [cursor_column], COLUMNSZ
 .if_min_above_10:
 mov eax, dword [media_viewer_sound_length_min]
 mov dword [var_print_value], eax
 call print_var
 
 mov dword [char_for_print], ':'
 call print_char
 add dword [cursor_column], COLUMNSZ
 
 mov dword [char_for_print], '0'
 cmp dword [media_viewer_sound_length_sec], 9
 ja .if_sec_above_10
  call print_char
  add dword [cursor_column], COLUMNSZ
 .if_sec_above_10:
 mov eax, dword [media_viewer_sound_length_sec]
 mov dword [var_print_value], eax
 call print_var
 
 SCREEN_Y_SUB ebx, 20+LINESZ*6
 PRINT 'Volume:', volume_str, ebx, COLUMNSZ*1
 SCREEN_Y_SUB ebx, 20+LINESZ*6
 mov eax, dword [ac97_volume]
 PRINT_VAR eax, ebx, COLUMNSZ*9
 
 SCREEN_Y_SUB eax, 20+LINESZ*4
 SCREEN_X_SUB ebx, COLUMNSZ*2
 DRAW_EMPTY_SQUARE eax, COLUMNSZ*1, ebx, LINESZ, BLACK
 
 call redraw_screen
 
 .halt:
  call wait_for_keyboard
  
  ;draw square of played time
  cmp dword [media_viewer_sound_state], MV_PLAY
  jne .if_playing
   mov eax, dword [ticks3]
   SCREEN_X_SUB ebx, COLUMNSZ*2+2
   mul ebx
   mov ebx, dword [media_viewer_sound_length_milisec]
   mov edx, 0
   div ebx
   cmp eax, 0
   je .skip_drawing_square
   SCREEN_Y_SUB ebx, 20+LINESZ*4-1
   DRAW_SQUARE ebx, COLUMNSZ*1+1, eax, LINESZ-1, 0x0900FF ;deep blue
   SCREEN_Y_SUB ebx, 20+LINESZ*4
   REDRAW_LINES_SCREEN ebx, LINESZ
   .skip_drawing_square:
   
   ;draw seconds
   SCREEN_Y_SUB eax, 20+LINESZ*2
   SCREEN_X_SUB ebx, COLUMNSZ*12
   DRAW_SQUARE eax, ebx, COLUMNSZ*5, LINESZ, WHITE
   
   mov eax, dword [ticks3]
   mov ebx, 500
   mov edx, 0
   div ebx
   mov ebx, 60
   mov edx, 0
   div ebx
   mov dword [media_viewer_sound_played_sec], edx
   mov dword [media_viewer_sound_played_min], eax
   
   SCREEN_Y_SUB eax, 20+LINESZ*2
   mov dword [cursor_line], eax
   SCREEN_X_SUB eax, COLUMNSZ*12
   mov dword [cursor_column], eax
 
   mov dword [char_for_print], '0'
   cmp dword [media_viewer_sound_played_min], 9
   ja .if_played_min_above_10
    call print_char
    add dword [cursor_column], COLUMNSZ
   .if_played_min_above_10:
   mov eax, dword [media_viewer_sound_played_min]
   mov dword [var_print_value], eax
   call print_var
 
   mov dword [char_for_print], ':'
   call print_char
   add dword [cursor_column], COLUMNSZ
 
   mov dword [char_for_print], '0'
   cmp dword [media_viewer_sound_played_sec], 9
   ja .if_played_sec_above_10
    call print_char
    add dword [cursor_column], COLUMNSZ
   .if_played_sec_above_10:
   mov eax, dword [media_viewer_sound_played_sec]
   mov dword [var_print_value], eax
   call print_var
   
   SCREEN_Y_SUB eax, 20+LINESZ*2
   mov dword [cursor_line], eax
   REDRAW_LINES_SCREEN eax, LINESZ
   
   mov eax, dword [ticks3]
   cmp eax, dword [media_viewer_sound_length_milisec]
   jb .if_stop_playing
    call ac97_stop_sound
    call ac97_clear_buffer
    mov dword [media_viewer_sound_state], MV_STOP
    
    ;draw full square
    SCREEN_Y_SUB eax, 20+LINESZ*4
    SCREEN_X_SUB ebx, COLUMNSZ*1
    DRAW_SQUARE eax, ebx, COLUMNSZ, LINESZ, WHITE
    
    SCREEN_Y_SUB eax, 20+LINESZ*4
    SCREEN_X_SUB ebx, COLUMNSZ*2
    DRAW_EMPTY_SQUARE eax, COLUMNSZ*1, ebx, LINESZ, BLACK
 
    SCREEN_Y_SUB ebx, 20+LINESZ*4-1
    SCREEN_X_SUB ecx, COLUMNSZ*2+1
    DRAW_SQUARE ebx, COLUMNSZ*1+1, ecx, LINESZ-1, 0x0900FF ;deep blue
    SCREEN_Y_SUB ebx, 20+LINESZ*4
    REDRAW_LINES_SCREEN ebx, LINESZ
   .if_stop_playing:
  .if_playing:
  
  cmp byte [key_code], KEY_F3
  je .close_file
  
  cmp byte [key_code], KEY_UP
  je .volume_up
  
  cmp byte [key_code], KEY_RIGHT
  je .volume_up_10
  
  cmp byte [key_code], KEY_DOWN
  je .volume_down
  
  cmp byte [key_code], KEY_LEFT
  je .volume_down_10
  
  cmp byte [key_code], KEY_SPACE
  je .play
  
  cmp byte [key_code], KEY_ENTER
  je .stop
  
  cmp dword [media_viewer_sound_samples], 0
  je .if_all_is_played
  call ac97_sound_position
  mov ebx, dword [ac97_last_entry]
  cmp al, bl
  je .if_fill_next_buffer
   cmp dword [media_viewer_sound_samples], 0xFFFF*2
   ja .not_last_buffer
   ;last buffer
   mov ax, word [media_viewer_sound_samples]
   mov word [ac97_sound_length], ax
   call ac97_fill_buffer
   mov dword [media_viewer_sound_samples], 0
   jmp .halt
 
   .not_last_buffer:
   mov word [ac97_sound_length], 0xFFFF
   call ac97_fill_buffer
   sub dword [media_viewer_sound_samples], 0xFFFF*2
   add dword [ac97_sound_data], 0xFFFF*2
  .if_fill_next_buffer:
  .if_all_is_played:
 jmp .halt
 
 .close_file:
  call ac97_stop_sound
  call ac97_clear_buffer
  call release_memory
 jmp media_viewer
 
 .volume_up:
  cmp dword [ac97_volume], 100
  je .halt
  inc dword [ac97_volume]
  call ac97_set_volume
 jmp .volume_redraw
 
 .volume_up_10:
  cmp dword [ac97_volume], 100
  je .halt
  cmp dword [ac97_volume], 90
  ja .volume_100
  add dword [ac97_volume], 10
  call ac97_set_volume
 jmp .volume_redraw
  .volume_100:
  mov dword [ac97_volume], 100
  call ac97_set_volume
 jmp .volume_redraw
 
 .volume_down:
  cmp dword [ac97_volume], 0
  je .halt
  dec dword [ac97_volume]
  call ac97_set_volume
 jmp .volume_redraw
 
 .volume_down_10:
  cmp dword [ac97_volume], 0
  je .halt
  cmp dword [ac97_volume], 10
  jb .volume_0
  sub dword [ac97_volume], 10
  call ac97_set_volume
 jmp .volume_redraw
  .volume_0:
  mov dword [ac97_volume], 0
  call ac97_set_volume
 jmp .volume_redraw
 
 .volume_redraw:
  SCREEN_Y_SUB ebx, 20+LINESZ*6
  DRAW_SQUARE ebx, COLUMNSZ*9, COLUMNSZ*3, LINESZ, WHITE
  SCREEN_Y_SUB ebx, 20+LINESZ*6
  mov eax, dword [ac97_volume]
  PRINT_VAR eax, ebx, COLUMNSZ*9
  SCREEN_Y_SUB ebx, 20+LINESZ*6
  REDRAW_LINES_SCREEN ebx, LINESZ
 jmp .halt
 
 .stop:
  call ac97_stop_sound
  mov dword [media_viewer_sound_state], MV_STOP
  mov dword [enable_ticks3], 0
 jmp .halt
 
 .play:
  cmp dword [media_viewer_sound_state], MV_PLAY
  je .halt
  call ac97_play_sound
  mov dword [media_viewer_sound_state], MV_PLAY
  mov dword [enable_ticks3], 1
 jmp .halt
 
 .can_not_play:
  DRAW_WINDOW media_viewer_up_str, media_viewer_down_str, 0x00FFEF, WHITE
  PRINT 'This file can not be play on this hardware', hardware_can_not_play_file_str, 20+LINESZ*1, COLUMNSZ*1
  call redraw_screen
  jmp media_viewer.media_viewer_halt
