;BleskOS

graphic_editor_up_str db 'Graphic editor', 0
graphic_editor_down_str db '[F1] Save file [F2] Open file [F3] New file [F4/5] Step +/- [q] Mouse on image [arrows] Move image', 0
ge_new_file_up_str db 'Please select size of new image:', 0
ge_new_file_down_str db '[a] 100x100 [b] 640x480 [c] 800x600 [d] 1024x768', 0

ge_selected_color dd BLACK
ge_drawing_state dd 0
%define GE_FREE_DRAW 0
%define GE_LINE_DRAW 1
%define GE_SQUARE 2
%define GE_CIRCLE 3
%define GE_FILL 4
ge_draw_object dd GE_FREE_DRAW
%define GE_EMPTY 0
%define GE_FULL 1
ge_tool_option dd 0

ge_draw_click1_line dd 0
ge_draw_click1_column dd 0
ge_draw_click2_line dd 0
ge_draw_click2_column dd 0
ge_mouse_img_line dd 21
ge_mouse_img_column dd 1
ge_mouse_line dd 0
ge_mouse_column dd 0

ge_image_width dd 640
ge_image_heigth dd 480
ge_image_show_width dd 640
ge_image_show_heigth dd 480
ge_image_first_show_line dd 0
ge_image_first_show_column dd 0
ge_image_bytes_per_line dd 640*4
ge_image_pointer dd 0
ge_image_step_back_pointer dd 0

ge_image_step dd 0

graphic_editor:
 DRAW_WINDOW graphic_editor_up_str, graphic_editor_down_str, 0xFFAE29, 0xBBBBBB
 call graphic_editor_draw_panel
 call graphic_editor_draw_image

 ;move mouse cursor to image
 mov eax, dword [ge_mouse_img_line]
 mov dword [cursor_line], eax
 mov eax, dword [ge_mouse_img_column]
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 mov dword [mcursor_left_side], 0
 mov eax, dword [ge_image_show_width]
 mov dword [mcursor_right_side], eax
 mov dword [mcursor_up_side], 20
 mov eax, dword [ge_image_show_heigth]
 add eax, 20
 mov dword [mcursor_down_side], eax
 call redraw_screen
 
 mov dword [usb_mouse_dnd], 0
 .graphic_editor_halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_F1
  je .save_file
  
  cmp byte [key_code], KEY_F2
  je .open_file
  
  cmp byte [key_code], KEY_F3
  je .new_file
  
  cmp byte [key_code], KEY_F4
  je .step_back
  
  cmp byte [key_code], KEY_F5
  je .step_forward
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [key_code], KEY_RIGHT
  je .key_right
  
  cmp byte [key_code], KEY_LEFT
  je .key_left
  
  cmp byte [key_code], KEY_W
  je ge_select_color
  
  cmp byte [key_code], KEY_A
  je .tool_free_draw
  
  cmp byte [key_code], KEY_S
  je .tool_line
  
  cmp byte [key_code], KEY_D
  je .tool_square
  
  cmp byte [key_code], KEY_F
  je .tool_circle
  
  cmp byte [key_code], KEY_G
  je .tool_fill
  
  cmp byte [key_code], KEY_1
  je .empty_tool
  
  cmp byte [key_code], KEY_2
  je .full_tool
  
  cmp dword [usb_mouse_data], 0
  jne .move_mouse
  
  .test_drag_and_drop:
  cmp dword [usb_mouse_dnd], 0x0
  je .mouse_end_of_click
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  cmp dword [usb_mouse_dnd], 0x2
  je .mouse_drag_and_drop
 jmp .graphic_editor_halt
 
 .save_file:
  mov eax, dword [ge_image_pointer]
  mov dword [file_memory], eax
  mov eax, dword [ge_image_width]
  mov dword [lc_image_width], eax
  mov eax, dword [ge_image_heigth]
  mov dword [lc_image_heigth], eax
  call create_bmp_file
  
  mov eax, dword [allocated_memory_pointer]
  mov dword [file_memory], eax
  mov eax, ecx ;copy length of file in BMP
  mov ebx, 1024
  mov edx, 0
  div ebx ;convert from bytes to KB
  inc eax
  mov dword [file_size], eax ;in KB
  mov dword [file_type], 'bmp'
  call file_dialog_save
  
  call release_memory ;delete created BMP file from memory
 jmp graphic_editor
 
 .open_file:
  mov dword [fd_file_type_1], 'bmp'
  mov dword [fd_file_type_2], 'BMP'
  call file_dialog_open
  cmp dword [fd_return], FD_NO_FILE
  je graphic_editor
  
  cmp dword [file_type], 'bmp'
  je .convert_bmp_file
  cmp dword [file_type], 'BMP'
  je .convert_bmp_file
  
  call release_memory ;this is not supported type of file
  jmp graphic_editor
  
  .convert_bmp_file:
  call convert_bmp_file
  
  ;set variabiles
  .set_new_image_variabiles:
  mov eax, dword [file_memory]
  add eax, 8
  mov dword [ge_image_pointer], eax
  
  mov eax, dword [lc_image_width]
  mov dword [ge_image_width], eax
  mov dword [ge_image_show_width], eax
  cmp eax, dword [ge_panel_column]
  jb .if_width_over_screen
   mov ebx, dword [ge_panel_column]
   mov dword [ge_image_show_width], ebx
  .if_width_over_screen:
  mov ebx, 4
  mul ebx
  mov dword [ge_image_bytes_per_line], eax
  
  mov eax, dword [lc_image_heigth]
  mov dword [ge_image_heigth], eax
  mov dword [ge_image_show_heigth], eax
  mov ebx, dword [screen_y]
  sub ebx, 40
  cmp eax, ebx
  jb .if_heigth_over_screen
   mov dword [ge_image_show_heigth], ebx
  .if_heigth_over_screen:
  
  mov dword [ge_image_first_show_line], 0
  mov dword [ge_image_first_show_column], 0
 jmp graphic_editor
 
 .new_file:
  mov esi, ge_new_file_up_str
  mov edi, ge_new_file_down_str
  mov dword [message_window_key0], KEY_A
  mov dword [message_window_key1], KEY_B
  mov dword [message_window_key2], KEY_C
  mov dword [message_window_key3], KEY_D
  call show_message_window
  mov dword [message_window_key0], 0
  mov dword [message_window_key1], 0
  mov dword [message_window_key2], 0
  mov dword [message_window_key3], 0
  cmp byte [key_code], KEY_ESC
  je graphic_editor
  
  mov eax, dword [ge_image_pointer]
  mov dword [allocated_memory_pointer], eax
  mov eax, dword [ge_image_width]
  mov ebx, dword [ge_image_heigth]
  mul ebx
  shl eax, 2 ;mul 4
  mov ebx, 0x100000
  mov edx, 0
  div ebx
  inc eax
  mov dword [allocated_size], eax
  call release_memory
  mov eax, dword [ge_image_step_back_pointer]
  mov dword [allocated_memory_pointer], eax
  call release_memory
  
  mov dword [ge_mouse_img_line], 20
  mov dword [ge_mouse_img_column], 0
  
  cmp byte [key_code], KEY_A
  jne .if_resoultion_a
   mov dword [lc_image_width], 100
   mov dword [lc_image_heigth], 100
   
   mov dword [allocated_size], 1
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [file_memory], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 100*100+2
   rep stosd
   
   mov dword [allocated_size], 1
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [ge_image_step_back_pointer], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 100*100+2
   rep stosd
   jmp .set_new_image_variabiles
  .if_resoultion_a:
  
  cmp byte [key_code], KEY_B
  jne .if_resoultion_b
   mov dword [lc_image_width], 640
   mov dword [lc_image_heigth], 480
   
   mov dword [allocated_size], 2
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [file_memory], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 640*480+2
   rep stosd
   
   mov dword [allocated_size], 2
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [ge_image_step_back_pointer], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 640*480+2
   rep stosd
   jmp .set_new_image_variabiles
  .if_resoultion_b:
  
  cmp byte [key_code], KEY_C
  jne .if_resoultion_c
   mov dword [lc_image_width], 800
   mov dword [lc_image_heigth], 600
   
   mov dword [allocated_size], 2
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [file_memory], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 800*600+2
   rep stosd
   
   mov dword [allocated_size], 2
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [ge_image_step_back_pointer], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 800*600+2
   rep stosd
   jmp .set_new_image_variabiles
  .if_resoultion_c:
  
  cmp byte [key_code], KEY_D
  jne .if_resoultion_d
   mov dword [lc_image_width], 1024
   mov dword [lc_image_heigth], 768
   
   mov dword [allocated_size], 3
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [file_memory], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 1024*768+2
   rep stosd
   
   mov dword [allocated_size], 3
   call allocate_memory
   mov eax, dword [allocated_memory_pointer]
   mov dword [ge_image_step_back_pointer], eax
   mov edi, eax
   mov eax, WHITE
   mov ecx, 1024*768+2
   rep stosd
   jmp .set_new_image_variabiles
  .if_resoultion_d:
 jmp graphic_editor
 
 .step_back:
  mov eax, dword [ge_image_pointer]
  mov ebx, dword [ge_image_step_back_pointer]
  mov dword [ge_image_pointer], ebx
  mov dword [ge_image_step_back_pointer], eax
 jmp graphic_editor
 
 .step_forward:
  mov eax, dword [ge_image_pointer]
  mov ebx, dword [ge_image_step_back_pointer]
  mov dword [ge_image_pointer], ebx
  mov dword [ge_image_step_back_pointer], eax
 jmp graphic_editor
  
 .key_up:
  mov eax, dword [ge_image_heigth]
  cmp dword [ge_image_show_heigth], eax
  je .graphic_editor_halt
  cmp dword [ge_image_first_show_line], 20
  jb .up_zero
  sub dword [ge_image_first_show_line], 20
  jmp .redraw_image
  .up_zero:
  mov dword [ge_image_first_show_line], 0
  jmp .redraw_image
  
 .key_down:
  mov eax, dword [ge_image_heigth]
  cmp dword [ge_image_show_heigth], eax
  je .graphic_editor_halt
  mov eax, dword [ge_image_heigth]
  sub eax, dword [screen_y]
  add eax, 40
  mov ebx, eax
  sub ebx, 20
  cmp dword [ge_image_first_show_line], ebx
  ja .down_max
  add dword [ge_image_first_show_line], 20
  jmp .redraw_image
  .down_max:
  mov dword [ge_image_first_show_line], eax
  jmp .redraw_image
  
 .key_right:
  mov eax, dword [ge_image_width]
  cmp dword [ge_image_show_width], eax
  je .graphic_editor_halt
  mov eax, dword [ge_image_width]
  sub eax, dword [ge_panel_column]
  mov ebx, eax
  sub ebx, 20
  cmp dword [ge_image_first_show_column], ebx
  ja .right_max
  add dword [ge_image_first_show_column], 20
  jmp .redraw_image
  .right_max:
  mov dword [ge_image_first_show_column], eax
  jmp .redraw_image
  
 .key_left:
  mov eax, dword [ge_image_width]
  cmp dword [ge_image_show_width], eax
  je .graphic_editor_halt
  cmp dword [ge_image_first_show_column], 20
  jb .left_zero
  sub dword [ge_image_first_show_column], 20
  jmp .redraw_image
  .left_zero:
  mov dword [ge_image_first_show_column], 0
  jmp .redraw_image
  
  .redraw_image:
  call write_cursor_bg
  call graphic_editor_draw_image
  call read_cursor_bg
  call redraw_screen
 jmp .graphic_editor_halt
 
 .tool_free_draw:
  mov dword [ge_draw_object], GE_FREE_DRAW
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .tool_line:
  mov dword [ge_draw_object], GE_LINE_DRAW
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .tool_square:
  mov dword [ge_draw_object], GE_SQUARE
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .tool_circle:
  mov dword [ge_draw_object], GE_CIRCLE
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .tool_fill:
  mov dword [ge_draw_object], GE_FILL
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .empty_tool:
  mov dword [ge_tool_option], GE_EMPTY
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .full_tool:
  mov dword [ge_tool_option], GE_FULL
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .mouse_end_of_click:
  cmp dword [ge_drawing_state], 0
  je .graphic_editor_halt
  
  cmp dword [ge_draw_object], GE_FREE_DRAW
  jne .end_of_click_if_free_draw
   mov dword [ge_drawing_state], 0
   jmp .graphic_editor_halt
  .end_of_click_if_free_draw:
  
  cmp dword [ge_draw_object], GE_FILL
  je .graphic_editor_halt
  
  ;save previous state
  mov esi, dword [ge_image_pointer]
  mov edi, dword [ge_image_step_back_pointer]
  mov eax, dword [ge_image_width]
  mov ebx, dword [ge_image_heigth]
  mul ebx
  mov ecx, eax
  rep movsd
  
  push dword [screen_mem_pointer]
  push dword [screen_x]
  push dword [screen_y]
  push dword [screen_pixels_per_line]
  mov eax, dword [ge_image_pointer]
  mov dword [screen_mem_pointer], eax
  mov eax, dword [ge_image_width]
  mov dword [screen_x], eax
  mov eax, dword [ge_image_heigth]
  mov dword [screen_y], eax
  mov eax, dword [ge_image_bytes_per_line]
  mov dword [screen_pixels_per_line], eax
  
  mov eax, dword [ge_selected_color]
  mov dword [color], eax
  
  call ge_draw_object_to_memory
  
  pop dword [screen_pixels_per_line]
  pop dword [screen_y]
  pop dword [screen_x]
  pop dword [screen_mem_pointer]
  
  mov dword [ge_drawing_state], 0

  DRAW_WINDOW graphic_editor_up_str, graphic_editor_down_str, 0xFFAE29, 0xBBBBBB
  call graphic_editor_draw_panel
  call graphic_editor_draw_image
  
  mov eax, dword [ge_draw_click2_line]
  mov dword [cursor_line], eax
  mov eax, dword [ge_draw_click2_column]
  mov dword [cursor_column], eax
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .graphic_editor_halt
 
 .mouse_click:
  mov dword [ge_drawing_state], 1
  
  cmp dword [ge_draw_object], GE_FREE_DRAW
  je .mouse_click_save_state
  cmp dword [ge_draw_object], GE_FILL
  jne .mouse_click_if_free_draw
  .mouse_click_save_state:
   ;save previous state
   mov esi, dword [ge_image_pointer]
   mov edi, dword [ge_image_step_back_pointer]
   mov eax, dword [ge_image_width]
   mov ebx, dword [ge_image_heigth]
   mul ebx
   mov ecx, eax
   rep movsd
  .mouse_click_if_free_draw:
  
  mov eax, dword [cursor_line]
  mov dword [ge_draw_click1_line], eax
  mov dword [ge_draw_click2_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_draw_click1_column], eax
  mov dword [ge_draw_click2_column], eax
  
  cmp dword [ge_draw_object], GE_FILL
  jne .if_mouse_click_fill
   push dword [screen_mem_pointer]
   push dword [screen_x]
   push dword [screen_y]
   push dword [screen_pixels_per_line]
   mov eax, dword [ge_image_pointer]
   mov dword [screen_mem_pointer], eax
   mov eax, dword [ge_image_width]
   mov dword [screen_x], eax
   mov eax, dword [ge_image_heigth]
   mov dword [screen_y], eax
   mov eax, dword [ge_image_bytes_per_line]
   mov dword [screen_pixels_per_line], eax
  
   mov eax, dword [ge_selected_color]
   mov dword [color], eax
   
   call ge_draw_object_to_memory
   
   pop dword [screen_pixels_per_line]
   pop dword [screen_y]
   pop dword [screen_x]
   pop dword [screen_mem_pointer]
   
   jmp graphic_editor
  .if_mouse_click_fill
 jmp .graphic_editor_halt
 
 .mouse_drag_and_drop:
  cmp dword [ge_draw_object], GE_FILL
  je .graphic_editor_halt
 
  mov eax, dword [cursor_line]
  mov dword [ge_draw_click2_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_draw_click2_column], eax
  
  push dword [cursor_line]
  push dword [cursor_column]
  
  cmp dword [ge_draw_object], GE_FREE_DRAW
  jne .drag_and_drop_if_free_draw 
   push dword [screen_mem_pointer]
   push dword [screen_x]
   push dword [screen_y]
   push dword [screen_pixels_per_line]
   mov eax, dword [ge_image_pointer]
   mov dword [screen_mem_pointer], eax
   mov eax, dword [ge_image_width]
   mov dword [screen_x], eax
   mov eax, dword [ge_image_heigth]
   mov dword [screen_y], eax
   mov eax, dword [ge_image_bytes_per_line]
   mov dword [screen_pixels_per_line], eax
  
   mov eax, dword [ge_selected_color]
   mov dword [color], eax
  
   call ge_draw_object_to_memory
  
   pop dword [screen_pixels_per_line]
   pop dword [screen_y]
   pop dword [screen_x]
   pop dword [screen_mem_pointer]
   
   mov eax, dword [ge_draw_click2_line]
   mov dword [ge_draw_click1_line], eax
   mov eax, dword [ge_draw_click2_column]
   mov dword [ge_draw_click1_column], eax
   
   DRAW_WINDOW graphic_editor_up_str, graphic_editor_down_str, 0xFFAE29, 0xBBBBBB
   call graphic_editor_draw_panel
   call graphic_editor_draw_image
   
   jmp .drag_and_drop_redraw_screen
  .drag_and_drop_if_free_draw:
  
  DRAW_WINDOW graphic_editor_up_str, graphic_editor_down_str, 0xFFAE29, 0xBBBBBB
  call graphic_editor_draw_panel
  call graphic_editor_draw_image
  mov eax, dword [ge_selected_color]
  mov dword [color], eax
  
  call ge_draw_object_on_screen 
  
  .drag_and_drop_redraw_screen:
  pop dword [cursor_column]
  pop dword [cursor_line]
  
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .graphic_editor_halt
 
 .move_mouse:
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [ge_mouse_img_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_mouse_img_column], eax
 jmp .test_drag_and_drop
 
ge_draw_object_to_memory:
 push dword [ge_draw_click2_line]
 push dword [ge_draw_click2_column]
 
 mov eax, dword [ge_image_first_show_line]
 add dword [ge_draw_click1_line], eax
 add dword [ge_draw_click2_line], eax
 mov eax, dword [ge_image_first_show_column]
 add dword [ge_draw_click1_column], eax
 add dword [ge_draw_click2_column], eax

 cmp dword [ge_draw_object], GE_FREE_DRAW
 je .draw_line
 cmp dword [ge_draw_object], GE_LINE_DRAW
 jne .if_draw_free
 .draw_line:
  mov eax, dword [ge_draw_click1_line]
  sub eax, 20
  mov dword [y1], eax
  mov eax, dword [ge_draw_click1_column]
  mov dword [x1], eax
  mov eax, dword [ge_draw_click2_line]
  sub eax, 20
  mov dword [y2], eax
  mov eax, dword [ge_draw_click2_column]
  mov dword [x2], eax
  call draw_line_all
  jmp .done
 .if_draw_free:
 
 cmp dword [ge_draw_object], GE_SQUARE
 jne .if_draw_square
  mov eax, dword [ge_draw_click1_line]
  cmp dword [ge_draw_click2_line], eax
  ja .if_draw_square_reverse_line
   mov eax, dword [ge_draw_click2_line]
  .if_draw_square_reverse_line:
  sub eax, 20
  mov dword [cursor_line], eax
  
  mov eax, dword [ge_draw_click1_column]
  cmp dword [ge_draw_click2_column], eax
  ja .if_draw_square_reverse_column
   mov eax, dword [ge_draw_click2_column]
  .if_draw_square_reverse_column:
  mov dword [cursor_column], eax
  
  mov eax, dword [ge_draw_click2_line]
  sub eax, dword [ge_draw_click1_line]
  cmp dword [ge_draw_click2_line], eax
  ja .if_draw_square_reverse_heigth
  je .if_draw_square_reverse_heigth
   mov eax, dword [ge_draw_click1_line]
   sub eax, dword [ge_draw_click2_line]
  .if_draw_square_reverse_heigth:
  cmp eax, 0
  je .if_draw_square
  inc eax
  mov dword [square_heigth], eax
  
  mov eax, dword [ge_draw_click2_column]
  sub eax, dword [ge_draw_click1_column]
  cmp dword [ge_draw_click2_column], eax
  ja .if_draw_square_reverse_length
  je .if_draw_square_reverse_length
   mov eax, dword [ge_draw_click1_column]
   sub eax, dword [ge_draw_click2_column]
  .if_draw_square_reverse_length:
  cmp eax, 0
  je .if_draw_square
  inc eax
  mov dword [square_length], eax
  
  mov eax, dword [ge_tool_option]
  cmp eax, GE_EMPTY
  je .empty_square
  
  call draw_square ;full square
  jmp .done
  
  .empty_square:
  call draw_empty_square
  jmp .done
 .if_draw_square:
 
 cmp dword [ge_draw_object], GE_CIRCLE
 jne .if_draw_circle
  mov eax, dword [ge_draw_click1_line]
  mov ebx, dword [ge_draw_click2_line]
  mov ecx, ebx
  sub ecx, eax
  shr ecx, 1 ;div 2
  mov dword [ellipse_y_radius], ecx
  mov dword [cursor_line], eax
  add dword [cursor_line], ecx
  cmp eax, ebx
  jb .if_circle_line
   mov ecx, eax
   sub ecx, ebx
   shr ecx, 1 ;div 2
   mov dword [ellipse_y_radius], ecx
   add ebx, ecx
   mov dword [cursor_line], ebx
  .if_circle_line:
  
  mov eax, dword [ge_draw_click1_column]
  mov ebx, dword [ge_draw_click2_column]
  mov ecx, ebx
  sub ecx, eax
  shr ecx, 1 ;div 2
  mov dword [ellipse_x_radius], ecx
  mov dword [cursor_column], eax
  add dword [cursor_column], ecx
  cmp eax, ebx
  jb .if_circle_column
   mov ecx, eax
   sub ecx, ebx
   shr ecx, 1 ;div 2
   mov dword [ellipse_x_radius], ecx
   add ebx, ecx
   mov dword [cursor_column], ebx
  .if_circle_column:
  
  sub dword [cursor_line], 20
  
  mov eax, dword [ge_tool_option]
  cmp eax, GE_EMPTY
  je .empty_ellipse
  
  call draw_ellipse ;full ellipse
  jmp .done
  
  .empty_ellipse:
  call draw_empty_ellipse
  jmp .done
 .if_draw_circle:
 
 cmp dword [ge_draw_object], GE_FILL
 jne .if_fill
  mov eax, dword [ge_draw_click1_line]
  sub eax, 20
  mov dword [cursor_line], eax
  mov eax, dword [ge_draw_click1_column]
  mov dword [cursor_column], eax
  call fill_area
  jmp .done
 .if_fill:
 
 .done:
 pop dword [ge_draw_click2_column]
 pop dword [ge_draw_click2_line] 
 ret
 
ge_draw_object_on_screen:
 push dword [ge_draw_click2_line]
 push dword [ge_draw_click2_column]
 
 cmp dword [ge_draw_object], GE_FREE_DRAW
 je .draw_line
 cmp dword [ge_draw_object], GE_LINE_DRAW
 jne .if_draw_free
 .draw_line:
  mov eax, dword [ge_draw_click1_line]
  mov dword [y1], eax
  mov eax, dword [ge_draw_click1_column]
  mov dword [x1], eax
  mov eax, dword [ge_draw_click2_line]
  mov dword [y2], eax
  mov eax, dword [ge_draw_click2_column]
  mov dword [x2], eax
  call draw_line_all
  jmp .done
 .if_draw_free:
 
 cmp dword [ge_draw_object], GE_SQUARE
 jne .if_draw_square
  mov eax, dword [ge_draw_click1_line]
  cmp dword [ge_draw_click2_line], eax
  ja .if_draw_square_reverse_line
   mov eax, dword [ge_draw_click2_line]
  .if_draw_square_reverse_line:
  mov dword [cursor_line], eax
  
  mov eax, dword [ge_draw_click1_column]
  cmp dword [ge_draw_click2_column], eax
  ja .if_draw_square_reverse_column
   mov eax, dword [ge_draw_click2_column]
  .if_draw_square_reverse_column:
  mov dword [cursor_column], eax
  
  mov eax, dword [ge_draw_click2_line]
  sub eax, dword [ge_draw_click1_line]
  cmp dword [ge_draw_click2_line], eax
  ja .if_draw_square_reverse_heigth
  je .if_draw_square_reverse_heigth
   mov eax, dword [ge_draw_click1_line]
   sub eax, dword [ge_draw_click2_line]
  .if_draw_square_reverse_heigth:
  cmp eax, 0
  je .if_draw_square
  inc eax
  mov dword [square_heigth], eax
  
  mov eax, dword [ge_draw_click2_column]
  sub eax, dword [ge_draw_click1_column]
  cmp dword [ge_draw_click2_column], eax
  ja .if_draw_square_reverse_length
  je .if_draw_square_reverse_length
   mov eax, dword [ge_draw_click1_column]
   sub eax, dword [ge_draw_click2_column]
  .if_draw_square_reverse_length:
  cmp eax, 0
  je .if_draw_square
  inc eax
  mov dword [square_length], eax
  
  mov eax, dword [ge_tool_option]
  cmp eax, GE_EMPTY
  je .empty_square
  
  call draw_square ;full square
  jmp .done
  
  .empty_square:
  call draw_empty_square
  jmp .done
 .if_draw_square:
 
 cmp dword [ge_draw_object], GE_CIRCLE
 jne .if_draw_circle
  mov eax, dword [ge_draw_click1_line]
  mov ebx, dword [ge_draw_click2_line]
  mov ecx, ebx
  sub ecx, eax
  shr ecx, 1 ;div 2
  mov dword [ellipse_y_radius], ecx
  mov dword [cursor_line], eax
  add dword [cursor_line], ecx
  cmp eax, ebx
  jb .if_circle_line
   mov ecx, eax
   sub ecx, ebx
   shr ecx, 1 ;div 2
   mov dword [ellipse_y_radius], ecx
   add ebx, ecx
   mov dword [cursor_line], ebx
  .if_circle_line:
  
  mov eax, dword [ge_draw_click1_column]
  mov ebx, dword [ge_draw_click2_column]
  mov ecx, ebx
  sub ecx, eax
  shr ecx, 1 ;div 2
  mov dword [ellipse_x_radius], ecx
  mov dword [cursor_column], eax
  add dword [cursor_column], ecx
  cmp eax, ebx
  jb .if_circle_column
   mov ecx, eax
   sub ecx, ebx
   shr ecx, 1 ;div 2
   mov dword [ellipse_x_radius], ecx
   add ebx, ecx
   mov dword [cursor_column], ebx
  .if_circle_column:
  
  mov eax, dword [ge_tool_option]
  cmp eax, GE_EMPTY
  je .empty_ellipse
  
  call draw_ellipse ;full ellipse
  jmp .done
  
  .empty_ellipse:
  call draw_empty_ellipse
  jmp .done
 .if_draw_circle:
 
 .done:
 pop dword [ge_draw_click2_column]
 pop dword [ge_draw_click2_line]
 ret

ge_select_color:
 call graphic_editor_draw_image
 mov dword [ge_mouse_line], 55+LINESZ
 mov dword [cursor_line], 55+LINESZ
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ
 mov dword [ge_mouse_column], eax
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 call redraw_screen

 mov dword [usb_mouse_dnd], 0
 .ge_select_color_halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_Q
  je graphic_editor
  
  cmp byte [key_code], KEY_S
  je .tool_line
  
  cmp byte [key_code], KEY_D
  je .tool_square
  
  cmp byte [key_code], KEY_F
  je .tool_circle
  
  cmp byte [key_code], KEY_G
  je .tool_fill
  
  cmp byte [key_code], KEY_1
  je .empty_tool
  
  cmp byte [key_code], KEY_2
  je .full_tool
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  
  cmp dword [usb_mouse_data], 0
  jne .move_mouse
 jmp .ge_select_color_halt
 
 .tool_line:
  mov dword [ge_draw_object], GE_LINE_DRAW
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .ge_select_color_halt
 
 .tool_square:
  mov dword [ge_draw_object], GE_SQUARE
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call read_cursor_bg
  call draw_cursor
  call redraw_screen
 jmp .ge_select_color_halt
 
 .tool_circle:
  mov dword [ge_draw_object], GE_CIRCLE
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .tool_fill:
  mov dword [ge_draw_object], GE_FILL
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .empty_tool:
  mov dword [ge_tool_option], GE_EMPTY
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .full_tool:
  mov dword [ge_tool_option], GE_FULL
  mov dword [ge_drawing_state], 0
  push dword [cursor_line]
  push dword [cursor_column]
  call graphic_editor_draw_panel
  pop dword [cursor_column]
  pop dword [cursor_line]
  call redraw_screen
 jmp .graphic_editor_halt
 
 .move_mouse:
  mov eax, dword [ge_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ge_mouse_column]
  mov dword [cursor_column], eax
  
  mov eax, dword [ge_panel_column]
  add eax, COLUMNSZ
  mov ebx, eax
  add ebx, 16*8
  mov dword [mcursor_left_side], eax
  mov dword [mcursor_right_side], ebx
  mov dword [mcursor_up_side], 55+LINESZ
  mov dword [mcursor_down_side], 55+LINESZ+16*8
  call move_mouse_cursor
  
  mov eax, dword [cursor_line]
  mov dword [ge_mouse_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_mouse_column], eax
 jmp .ge_select_color_halt

 .mouse_click:
  mov eax, dword [ge_panel_column]
  add eax, COLUMNSZ
  mov ebx, eax
  add ebx, 16*8
  TEST_CLICK_ZONE color_table_zone, 55+LINESZ, 55+LINESZ+16*8, eax, ebx
  cmp eax, 0
  je .ge_select_color_halt
  
  mov esi, ge_color_table
  
  mov eax, dword [ge_mouse_line]
  sub eax, 55+LINESZ
  mov ebx, 16
  mov edx, 0
  div ebx
  mov ebx, 8
  mul ebx
  mov ecx, eax
  
  mov eax, dword [ge_mouse_column]
  sub eax, dword [ge_panel_column]
  sub eax, COLUMNSZ
  mov ebx, 16
  mov edx, 0
  div ebx
  add eax, ecx
  
  mov ebx, 4
  mul ebx
  add eax, ge_color_table
  
  mov ebx, dword [eax]
  mov dword [ge_selected_color], ebx
  
  call graphic_editor_draw_panel
  mov eax, dword [ge_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [ge_mouse_column]
  mov dword [cursor_column], eax
  call draw_cursor
  call redraw_screen
 jmp .ge_select_color_halt
