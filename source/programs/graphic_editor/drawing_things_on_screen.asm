;BleskOS

ge_draw_heigth dd 0
ge_draw_length dd 0
ge_panel_column dd 0

ge_color_table:
dd 0xFFFFFF, 0xE0E0E0, 0xC0C0C0, 0xA0A0A0, 0x808080, 0x606060, 0x404040, 0x000000 ;WHITE to BLACK
dd 0xFFCCCC, 0xFF9999, 0xFF6666, 0xFF3333, 0xFF0000, 0xCC0000, 0x990000, 0x660000 ;RED
dd 0xFFE5CC, 0xFFCC99, 0xFFB266, 0xFF9933, 0xFF8000, 0xCC6600, 0x994C00, 0x663300 ;ORANGE to BROWN
dd 0xFFFFCC, 0xFFFF99, 0xFFFF66, 0xFFFF33, 0xFFFF00, 0xCCCC00, 0x999900, 0x333300 ;YELLOW
dd 0xE5FFCC, 0xCCFF99, 0xB2FF66, 0x99FF33, 0x80FF00, 0x00CC00, 0x009900, 0x006600 ;GREEN
dd 0xCCFFFF, 0x99FFFF, 0x66FFFF, 0x33FFFF, 0x00FFFF, 0x0088FF, 0x0000FF, 0x000099 ;BLUE
dd 0xE5CCFF, 0xCC99FF, 0xB266FF, 0x9933FF, 0x7F00FF, 0x6600CC, 0x4C0099, 0x330066 ;PURPLE
dd 0xFFCCFF, 0xFF99FF, 0xFF66FF, 0xFF33FF, 0xFF00FF, 0xCC00CC, 0x990099, 0x660066 ;PINK

graphic_editor_draw_panel:
 ;draw background
 SCREEN_X_SUB eax, COLUMNSZ*2+16*8
 mov dword [ge_panel_column], eax
 SCREEN_Y_SUB ebx, 40
 mov dword [ge_draw_heigth], ebx
 DRAW_SQUARE 20, eax, COLUMNSZ*2+16*8, ebx, 0x999999
 
 ;draw column between draw zone
 mov eax, dword [ge_panel_column]
 mov ebx, dword [ge_draw_heigth]
 DRAW_COLUMN 20, eax, ebx, BLACK
 
 ;print first string
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ
 PRINT '[w] Select color', ge_select_color_str, 25, eax
 
 ;draw square with selected color
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ
 mov edx, dword [ge_selected_color]
 DRAW_SQUARE 25+COLUMNSZ+5, eax, 30, 20, edx
 DRAW_EMPTY_SQUARE 25+COLUMNSZ+5, eax, 30, 20, BLACK
 
 ;print hex value of selected color
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2+30
 mov ecx, dword [ge_selected_color]
 PRINT_HEX ecx, 25+COLUMNSZ+11, eax
 
 ;draw table of colors
 mov dword [square_heigth], 16
 mov dword [square_length], 16
 mov dword [cursor_line], 55+LINESZ
 mov ebp, ge_color_table
 mov ecx, 8
 .draw_color_table:
 push ecx
  mov eax, dword [ge_panel_column]
  add eax, COLUMNSZ
  mov dword [cursor_column], eax
  
  mov ecx, 8
  .draw_color_table_line:
  push ecx
   mov eax, dword [ebp]
   mov dword [color], eax
   call draw_square
   
   add ebp, 4
   add dword [cursor_column], 16
  pop ecx
  loop .draw_color_table_line
  
  add dword [cursor_line], 16 ;next line
 pop ecx
 loop .draw_color_table
 
 ;draw square for selected tool
 mov eax, dword [ge_draw_object]
 mov ebx, 20
 mul ebx
 add eax, 200
 mov dword [cursor_line], eax
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ
 mov dword [cursor_column], eax
 mov dword [square_length], 16*8
 mov dword [square_heigth], 15
 mov dword [color], 0xFF0000
 call draw_square
 mov dword [color], BLACK
 call draw_empty_square
 cmp dword [ge_draw_object], GE_SQUARE
 jb .if_draw_empty_full
 cmp dword [ge_draw_object], GE_CIRCLE
 ja .if_draw_empty_full
  mov eax, dword [ge_tool_option]
  mov ebx, 20
  mul ebx
  add eax, 200+20+20+20+20+20
  mov dword [cursor_line], eax
  mov eax, dword [ge_panel_column]
  add eax, COLUMNSZ
  mov dword [cursor_column], eax
  mov dword [square_length], 16*8
  mov dword [square_heigth], 15
  mov dword [color], 0x00FF00
  call draw_square
  mov dword [color], BLACK
  call draw_empty_square
 .if_draw_empty_full:
 
 ;print tools
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[a] Pen', ge_pen_str, 200+4, eax
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[s] Line', ge_line_str, 200+20+4, eax
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[d] Square', ge_square_str, 200+20+20+4, eax
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[f] Circle', ge_circle_str, 200+20+20+20+4, eax
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[g] Fill', ge_fill_str, 200+20+20+20+20+4, eax
 
 cmp dword [ge_draw_object], GE_SQUARE
 jb .if_print_empty_full
 cmp dword [ge_draw_object], GE_CIRCLE
 ja .if_print_empty_full
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[1] Empty', ge_full_square_str, 200+20+20+20+20+20+4, eax
 mov eax, dword [ge_panel_column]
 add eax, COLUMNSZ*2
 PRINT '[2] Full', ge_full_circle_str, 200+20+20+20+20+20+20+4, eax
 .if_print_empty_full:
 
 ret

graphic_editor_draw_image:
 mov eax, dword [ge_image_pointer]
 mov dword [li_source_memory], eax
 mov eax, dword [ge_image_width]
 mov dword [li_source_width], eax
 mov eax, dword [ge_image_heigth]
 mov dword [li_source_heigth], eax
 
 mov eax, dword [ge_image_first_show_line]
 mov dword [li_source_line], eax
 mov eax, dword [ge_image_first_show_column]
 mov dword [li_source_column], eax
 
 mov eax, dword [ge_image_show_heigth]
 mov dword [li_image_heigth], eax
 mov eax, dword [ge_image_show_width]
 mov dword [li_image_width], eax
 
 mov dword [li_destination_memory], LI_MONITOR
 mov dword [li_destination_line], 20
 mov dword [li_destination_column], 0

 call transfer_image
 
 ret
