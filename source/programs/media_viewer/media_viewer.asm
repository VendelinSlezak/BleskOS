;BleskOS

%define MV_IMAGE 1
%define MV_VIDEO 2
%define MV_SOUND 3

media_viewer_up_str db 'Media viewer', 0
media_viewer_down_str db '[F2] Open file', 0

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
  jmp media_viewer
  
  .convert_bmp_file:
  call convert_bmp_file
  mov eax, dword [file_memory]
  mov dword [media_viewer_file_pointer], eax
 jmp media_viewer_draw_image_to_center
  
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
