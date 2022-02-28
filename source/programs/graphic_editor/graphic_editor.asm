;BleskOS

graphic_editor_up_str db 'Graphic editor', 0
graphic_editor_down_str db '[q] Mouse on image', 0

ge_selected_color dd BLACK
ge_drawing_state dd 0
%define GE_FREE_DRAW 0
%define GE_LINE_DRAW 1
%define GE_SQUARE 2
ge_draw_object dd GE_SQUARE
ge_draw_click1_line dd 0
ge_draw_click1_column dd 0
ge_draw_click2_line dd 0
ge_draw_click2_column dd 0
ge_mouse_img_line dd 21
ge_mouse_img_column dd 1
ge_mouse_line dd 0
ge_mouse_column dd 0

ge_img_width dd 640
ge_img_heigth dd 480
ge_image_pointer dd 0

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
 mov dword [mcursor_left_side], 1
 mov dword [mcursor_right_side], 642
 mov dword [mcursor_up_side], 21
 mov dword [mcursor_down_side], 21+481
 call redraw_screen
 
 mov dword [usb_mouse_dnd], 0
 .graphic_editor_halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_E
  je ge_select_color
  
  cmp dword [usb_mouse_dnd], 0x0
  je .mouse_end_of_click
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  cmp dword [usb_mouse_dnd], 0x2
  je .mouse_drag_and_drop
  
  .test_mouse_data:
  cmp dword [usb_mouse_data], 0
  jne .move_mouse
 jmp .graphic_editor_halt
 
 .mouse_end_of_click:
  cmp dword [ge_drawing_state], 0
  je .test_mouse_data
  
  push dword [screen_mem_pointer]
  push dword [screen_x]
  push dword [screen_y]
  push dword [screen_pixels_per_line]
  mov eax, dword [ge_image_pointer]
  mov dword [screen_mem_pointer], eax
  mov dword [screen_x], 640
  mov dword [screen_y], 480
  mov dword [screen_pixels_per_line], 640*4
  
  mov eax, dword [ge_selected_color]
  mov dword [color], eax
  
  call ge_draw_object_on_screen
  
  pop dword [screen_pixels_per_line]
  pop dword [screen_y]
  pop dword [screen_x]
  pop dword [screen_mem_pointer]
  
  mov dword [ge_drawing_state], 0
 jmp .test_mouse_data
 
 .mouse_click:
  mov dword [ge_drawing_state], 1
  
  mov eax, dword [cursor_line]
  mov dword [ge_draw_click1_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_draw_click1_column], eax
 jmp .test_mouse_data
 
 .mouse_drag_and_drop:
  mov eax, dword [cursor_line]
  mov dword [ge_draw_click2_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_draw_click2_column], eax
  
  push dword [cursor_line]
  push dword [cursor_column]
  
  call graphic_editor_draw_image
  mov eax, dword [ge_selected_color]
  mov dword [color], eax
  
  call ge_draw_object_on_screen 
  
  pop dword [cursor_column]
  pop dword [cursor_line]
  
  call redraw_screen
 jmp .test_mouse_data
 
 .move_mouse:
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [ge_mouse_img_line], eax
  mov eax, dword [cursor_column]
  mov dword [ge_mouse_img_column], eax
 jmp .graphic_editor_halt
 
ge_draw_object_on_screen:
 cmp dword [ge_draw_object], GE_LINE_DRAW
 jne .if_draw_free
  mov eax, dword [ge_draw_click1_line]
  mov dword [y1], eax
  mov eax, dword [ge_draw_click1_column]
  mov dword [x1], eax
  mov eax, dword [ge_draw_click2_line]
  mov dword [y2], eax
  mov eax, dword [ge_draw_click2_column]
  mov dword [x2], eax
  call draw_line_all
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
   mov eax, dword [ge_draw_click1_line]
   sub eax, dword [ge_draw_click2_line]
  .if_draw_square_reverse_heigth:
  mov dword [square_heigth], eax
  cmp eax, 0
  je .if_draw_square
  
  mov eax, dword [ge_draw_click2_column]
  sub eax, dword [ge_draw_click1_column]
  cmp dword [ge_draw_click2_column], eax
  ja .if_draw_square_reverse_length
   mov eax, dword [ge_draw_click1_column]
   sub eax, dword [ge_draw_click2_column]
  .if_draw_square_reverse_length:
  mov dword [square_length], eax
  cmp eax, 0
  je .if_draw_square
  
  call draw_square
 .if_draw_square:
 
 ret

ge_select_color:
 call graphic_editor_draw_image
 mov dword [ge_mouse_line], 20
 mov dword [cursor_line], 20
 mov eax, dword [ge_panel_column]
 mov dword [ge_mouse_column], eax
 mov dword [cursor_column], eax
 call read_cursor_bg
 call redraw_screen

 mov dword [usb_mouse_dnd], 0
 .ge_select_color_halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp byte [key_code], KEY_Q
  je graphic_editor
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  
  cmp dword [usb_mouse_data], 0
  jne .move_mouse
 jmp .ge_select_color_halt
 
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
