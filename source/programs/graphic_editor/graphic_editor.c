//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_graphic_editor(void) {
 graphic_editor_program_interface_memory = create_program_interface_memory(((dword_t)&draw_graphic_editor), 0);

 graphic_editor_pen_size_text_area_info_memory = create_text_area(TEXT_AREA_NUMBER_INPUT, 3, 8, 0, 160, 10);
 graphic_editor_width_text_area_info_memory = create_text_area(TEXT_AREA_NUMBER_INPUT, 4, screen_x_center-24, screen_y_center-50+25, 48, 10);
 graphic_editor_height_text_area_info_memory = create_text_area(TEXT_AREA_NUMBER_INPUT, 4, screen_x_center-24, screen_y_center-50+55, 48, 10);

 graphic_editor_image_area_width = (screen_width-GRAPHIC_EDITOR_SIDE_PANEL_WIDTH);
 graphic_editor_image_area_width_center = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+(graphic_editor_image_area_width/2);
 graphic_editor_image_area_height = (screen_height-41-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH);
 graphic_editor_image_area_height_center = 21+(graphic_editor_image_area_height/2);

 graphic_editor_color = BLACK;
 graphic_editor_tool = GRAPHIC_EDITOR_TOOL_PEN;
 graphic_editor_tool_state = GRAPHIC_EDITOR_TOOL_STATE_EMPTY;
 graphic_editor_pen_size = 1;

 graphic_editor_image_saved_to_preview = STATUS_TRUE;
}

void graphic_editor(void) {
 set_program_interface(graphic_editor_program_interface_memory);
 program_interface_add_keyboard_event(KEY_F1, (dword_t)graphic_editor_open_file);
 program_interface_add_keyboard_event(KEY_F2, (dword_t)graphic_editor_save_file);
 program_interface_add_keyboard_event(KEY_F3, (dword_t)graphic_editor_new_file);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)graphic_editor_close_file);
 program_interface_add_keyboard_event(KEY_F7, (dword_t)graphic_editor_key_f7_event);
 program_interface_add_keyboard_event(KEY_UP, (dword_t)graphic_editor_key_up_event);
 program_interface_add_keyboard_event(KEY_DOWN, (dword_t)graphic_editor_key_down_event);
 program_interface_add_keyboard_event(KEY_LEFT, (dword_t)graphic_editor_key_left_event);
 program_interface_add_keyboard_event(KEY_RIGHT, (dword_t)graphic_editor_key_right_event);
 program_interface_add_keyboard_event(KEY_A, (dword_t)graphic_editor_key_a_event);
 program_interface_add_keyboard_event(KEY_S, (dword_t)graphic_editor_key_s_event);
 program_interface_add_keyboard_event(KEY_D, (dword_t)graphic_editor_key_d_event);
 program_interface_add_keyboard_event(KEY_G, (dword_t)graphic_editor_key_g_event);
 program_interface_add_keyboard_event(KEY_C, (dword_t)graphic_editor_key_c_event);
 program_interface_add_keyboard_event(KEY_E, (dword_t)graphic_editor_key_e_event);
 program_interface_add_keyboard_event(KEY_F, (dword_t)graphic_editor_key_f_event);
 program_interface_add_keyboard_event(KEY_T, (dword_t)graphic_editor_key_t_event);
 program_interface_add_keyboard_event(KEY_1, (dword_t)graphic_editor_key_1_event);
 program_interface_add_keyboard_event(KEY_2, (dword_t)graphic_editor_key_2_event);
 program_interface_add_keyboard_event(KEY_3, (dword_t)graphic_editor_key_3_event);
 program_interface_add_keyboard_event(KEY_5, (dword_t)graphic_editor_key_5_event);
 program_interface_add_keyboard_event(KEY_0, (dword_t)graphic_editor_key_0_event);
 program_interface_add_keyboard_event(KEY_ENTER, (dword_t)graphic_editor_key_enter_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_EDIT_IMAGE, (dword_t)graphic_editor_key_f7_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_BUTTON_TAKE_COLOR, (dword_t)graphic_editor_key_t_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_COLOR_PALETTE, (dword_t)graphic_editor_click_zone_event_palette);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_PEN, (dword_t)graphic_editor_key_a_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_LINE, (dword_t)graphic_editor_key_s_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_SQUARE, (dword_t)graphic_editor_key_d_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_CIRCLE, (dword_t)graphic_editor_key_g_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_FILL, (dword_t)graphic_editor_key_c_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_STATE_EMPTY, (dword_t)graphic_editor_key_e_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_TOOL_STATE_FULL, (dword_t)graphic_editor_key_f_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_1, (dword_t)graphic_editor_key_1_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_2, (dword_t)graphic_editor_key_2_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_3, (dword_t)graphic_editor_key_3_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_5, (dword_t)graphic_editor_key_5_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_10, (dword_t)graphic_editor_key_0_event);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_20, (dword_t)graphic_editor_click_zone_event_pen_size_20);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_25, (dword_t)graphic_editor_click_zone_event_zoom_25);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_50, (dword_t)graphic_editor_click_zone_event_zoom_50);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_100, (dword_t)graphic_editor_click_zone_event_zoom_100);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_200, (dword_t)graphic_editor_click_zone_event_zoom_200);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_300, (dword_t)graphic_editor_click_zone_event_zoom_300);
 program_interface_add_click_zone_event(GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_400, (dword_t)graphic_editor_click_zone_event_zoom_400);
 program_interface_redraw();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //close program
  if(keyboard_value==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && get_mouse_cursor_click_board_value()==CLICK_ZONE_BACK)) {
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //draw on image
  if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
   if(mouse_click_button_state==MOUSE_NO_DRAG && graphic_editor_image_saved_to_preview==STATUS_FALSE) {
    graphic_editor_copy_image_to_preview();
    graphic_editor_image_saved_to_preview = STATUS_TRUE;
   }
   else if(mouse_click_button_state==MOUSE_CLICK && get_program_value(PROGRAM_INTERFACE_SELECTED_CLICK_ZONE)==GRAPHIC_EDITOR_CLICK_ZONE_IMAGE) {
    //get and save mouse position
    get_mouse_coordinates_on_image(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
    graphic_editor_mouse_cursor_previous_image_x = image_mouse_x;
    graphic_editor_mouse_cursor_previous_image_y = image_mouse_y;
    if(image_mouse_x==0xFFFFFFFF || image_mouse_y==0xFFFFFFFF) {
     continue;
    }

    //process tool drawing
    if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_PEN) {
     graphic_editor_prepare_drawing_by_tool();
     draw_pixel(image_mouse_x, image_mouse_y, graphic_editor_color);
     graphic_editor_finish_drawing_by_tool();
    }
    else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_FILL) {
     graphic_editor_prepare_drawing_by_tool();
     fill_area(image_mouse_x, image_mouse_y, graphic_editor_color);
     graphic_editor_finish_drawing_by_tool();
    }
   }
   else if(mouse_click_button_state==MOUSE_DRAG && get_program_value(PROGRAM_INTERFACE_SELECTED_CLICK_ZONE)==GRAPHIC_EDITOR_CLICK_ZONE_IMAGE) {
    //get mouse position
    get_mouse_coordinates_on_image(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
    if(image_mouse_x==0xFFFFFFFF || image_mouse_y==0xFFFFFFFF) {
     continue;
    }

    //calculate redraw square
    //TODO: use it to faster redrawing
    dword_t ge_redraw_square_x = 0;
    dword_t ge_redraw_square_y = 0;
    dword_t ge_redraw_square_width = abs(image_mouse_x-graphic_editor_mouse_cursor_previous_image_x);
    dword_t ge_redraw_square_height = abs(image_mouse_y-graphic_editor_mouse_cursor_previous_image_y);
    if(image_mouse_x>graphic_editor_mouse_cursor_previous_image_x) {
     ge_redraw_square_x = graphic_editor_mouse_cursor_previous_image_x;
    }
    else {
     ge_redraw_square_x = image_mouse_x;
    }
    if(image_mouse_y>graphic_editor_mouse_cursor_previous_image_y) {
     ge_redraw_square_y = graphic_editor_mouse_cursor_previous_image_y;
    }
    else {
     ge_redraw_square_y = image_mouse_y;
    }

    //process tool drawing
    if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_PEN) {
     graphic_editor_prepare_drawing_by_tool_without_copying_preview();
     draw_line(image_mouse_x, image_mouse_y, graphic_editor_mouse_cursor_previous_image_x, graphic_editor_mouse_cursor_previous_image_y, graphic_editor_color);
     graphic_editor_finish_drawing_by_tool();

     //save actual mouse position
     graphic_editor_mouse_cursor_previous_image_x = image_mouse_x;
     graphic_editor_mouse_cursor_previous_image_y = image_mouse_y;
    }
    else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_LINE) {
     graphic_editor_prepare_drawing_by_tool();
     draw_line(image_mouse_x, image_mouse_y, graphic_editor_mouse_cursor_previous_image_x, graphic_editor_mouse_cursor_previous_image_y, graphic_editor_color);
     graphic_editor_finish_drawing_by_tool();
    }
    else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_SQUARE) {
     graphic_editor_prepare_drawing_by_tool();
     if(graphic_editor_tool_state==GRAPHIC_EDITOR_TOOL_STATE_EMPTY) {
      draw_empty_square(ge_redraw_square_x, ge_redraw_square_y, ge_redraw_square_width, ge_redraw_square_height, graphic_editor_color);
     }
     else if(graphic_editor_tool_state==GRAPHIC_EDITOR_TOOL_STATE_FULL) {
      draw_full_square(ge_redraw_square_x, ge_redraw_square_y, ge_redraw_square_width, ge_redraw_square_height, graphic_editor_color);
     }
     graphic_editor_finish_drawing_by_tool();
    }
    else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_CIRCLE) {
     graphic_editor_prepare_drawing_by_tool();
     if(graphic_editor_tool_state==GRAPHIC_EDITOR_TOOL_STATE_EMPTY) {
      draw_empty_ellipse(image_mouse_x, image_mouse_y, graphic_editor_mouse_cursor_previous_image_x, graphic_editor_mouse_cursor_previous_image_y, graphic_editor_color);
     }
     else if(graphic_editor_tool_state==GRAPHIC_EDITOR_TOOL_STATE_FULL) {
      draw_full_ellipse(image_mouse_x, image_mouse_y, graphic_editor_mouse_cursor_previous_image_x, graphic_editor_mouse_cursor_previous_image_y, graphic_editor_color);
     }
     graphic_editor_finish_drawing_by_tool();
    }
   }
  }
 }
}

void draw_graphic_editor(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("Graphic editor", "", 0xFFAE29, 0xBBBBBB);
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //draw side panel
  draw_full_square(0, 21, GRAPHIC_EDITOR_SIDE_PANEL_WIDTH, screen_height-41, 0x999999);

  //draw color area
  print("Color", 8, GRAPHIC_EDITOR_COLOR_AREA_LINE+6, BLACK);
  draw_full_square(8, GRAPHIC_EDITOR_COLOR_AREA_LINE+20, 40, 20, graphic_editor_color);
  draw_button("[t] Get color", 8+40+8, GRAPHIC_EDITOR_COLOR_AREA_LINE+20, 112, 20);
  add_zone_to_click_board(8+40+8, GRAPHIC_EDITOR_COLOR_AREA_LINE+20, 112, 20, GRAPHIC_EDITOR_CLICK_ZONE_BUTTON_TAKE_COLOR);
  for(int line=0; line<8; line++) {
   for(int column=0; column<8; column++) {
    draw_full_square(8+column*20, GRAPHIC_EDITOR_COLOR_AREA_LINE+20+25+line*20, 20, 20, graphic_editor_pallette[line*8+column]);
   }
  }
  add_zone_to_click_board(8, GRAPHIC_EDITOR_COLOR_AREA_LINE+20+25, 160, 160, GRAPHIC_EDITOR_CLICK_ZONE_COLOR_PALETTE);
  
  //draw tools area
  print("Tools", 8, GRAPHIC_EDITOR_TOOLS_AREA_LINE+6, BLACK);
  dword_t graphic_editor_tool_line = 0;
  if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_PEN) {
   graphic_editor_tool_line = GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*0;
  }
  else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_LINE) {
   graphic_editor_tool_line = GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*1;
  }
  else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_SQUARE) {
   graphic_editor_tool_line = GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*2;
  }
  else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_CIRCLE) {
   graphic_editor_tool_line = GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*3;
  }
  else if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_FILL) {
   graphic_editor_tool_line = GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*4;
  }
  if(graphic_editor_tool_line!=0) {
   draw_full_square(8, graphic_editor_tool_line, 160, 20, RED);
   draw_empty_square(8, graphic_editor_tool_line, 160, 20, BLACK);
  }
  print("[a] Pen", 16, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*0+6, BLACK);
  add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*0, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_PEN);
  print("[s] Line", 16, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*1+6, BLACK);
  add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*1, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_LINE);
  print("[d] Square", 16, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*2+6, BLACK);
  add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*2, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_SQUARE);
  print("[g] Circle", 16, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*3+6, BLACK);
  add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*3, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_CIRCLE);
  print("[c] Fill", 16, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*4+6, BLACK);
  add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOLS_AREA_LINE+20+20*4, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_FILL);
  
  //tool state
  dword_t pen_size_area_line = GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE;
  if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_SQUARE || graphic_editor_tool==GRAPHIC_EDITOR_TOOL_CIRCLE) {
   print("Tool state", 8, GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+6, BLACK);
   dword_t graphic_editor_tool_state_line = 0;
   if(graphic_editor_tool_state==GRAPHIC_EDITOR_TOOL_STATE_EMPTY) {
    graphic_editor_tool_state_line = GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+20+20*0;
   }
   else if(graphic_editor_tool_state==GRAPHIC_EDITOR_TOOL_STATE_FULL) {
    graphic_editor_tool_state_line = GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+20+20*1;
   }
   if(graphic_editor_tool_state_line!=0) {
    draw_full_square(8, graphic_editor_tool_state_line, 160, 20, 0x00D000);
    draw_empty_square(8, graphic_editor_tool_state_line, 160, 20, BLACK);
   }
   print("[e] Empty", 16, GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+20+20*0+6, BLACK);
   add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+20+20*0, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_STATE_EMPTY);
   print("[f] Full", 16, GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+20+20*1+6, BLACK);
   add_zone_to_click_board(8, GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE+20+20*1, 160, 20, GRAPHIC_EDITOR_CLICK_ZONE_TOOL_STATE_FULL);
   
   pen_size_area_line += 20+20*2;
  }
  
  //draw pen size area
  dword_t *pen_size_text_area = (dword_t *) (graphic_editor_pen_size_text_area_info_memory);
  print("Pen size", 8, pen_size_area_line+6, BLACK);
  if(graphic_editor_pen_size<=3) {
   draw_full_square(8+graphic_editor_pen_size*24-24, pen_size_area_line+20, 23, 20, RED);
   draw_empty_square(8+graphic_editor_pen_size*24-24, pen_size_area_line+20, 23, 20, BLACK);
  }
  else if(graphic_editor_pen_size==5) {
   draw_full_square(8+3*24, pen_size_area_line+20, 23, 20, RED);
   draw_empty_square(8+3*24, pen_size_area_line+20, 23, 20, BLACK);
  }
  else if(graphic_editor_pen_size==10) {
   draw_full_square(8+4*24, pen_size_area_line+20, 31, 20, RED);
   draw_empty_square(8+4*24, pen_size_area_line+20, 31, 20, BLACK);
  }
  else if(graphic_editor_pen_size==20) {
   draw_full_square(8+4*24+32, pen_size_area_line+20, 31, 20, RED);
   draw_empty_square(8+4*24+32, pen_size_area_line+20, 31, 20, BLACK);
  }
  print("1  2  3  5  10  20", 16, pen_size_area_line+20+6, BLACK);
  add_zone_to_click_board(8+24*0, pen_size_area_line+20, 24, 20, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_1);
  add_zone_to_click_board(8+24*1, pen_size_area_line+20, 24, 20, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_2);
  add_zone_to_click_board(8+24*2, pen_size_area_line+20, 24, 20, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_3);
  add_zone_to_click_board(8+24*3, pen_size_area_line+20, 24, 20, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_5);
  add_zone_to_click_board(8+24*4, pen_size_area_line+20, 32, 20, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_10);
  add_zone_to_click_board(8+24*4+32, pen_size_area_line+20, 32, 20, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_20);

  //draw pen size text area
  pen_size_text_area[TEXT_AREA_INFO_Y] = pen_size_area_line+40+6;
  pen_size_text_area[TEXT_AREA_INFO_CURSOR_POSITION] = 0xFFFFFFFF;
  clear_memory(pen_size_text_area[TEXT_AREA_INFO_MEMORY], 6);
  convert_number_to_word_string(graphic_editor_pen_size, pen_size_text_area[TEXT_AREA_INFO_MEMORY]);
  draw_text_area(graphic_editor_pen_size_text_area_info_memory);
  add_zone_to_click_board(8, pen_size_area_line+40+6, 160, 10, GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_TEXT_AREA);
  program_interface_add_text_area(GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_TEXT_AREA, graphic_editor_pen_size_text_area_info_memory);

  //draw bottom panel
  draw_full_square(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, screen_width-GRAPHIC_EDITOR_SIDE_PANEL_WIDTH, GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 0xAAAAAA);
  dword_t zoom = get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_ZOOM), zoom_background_x = 0, zoom_background_width = 0;
  if(zoom==25) {
   zoom_background_x = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+4;
   zoom_background_width = 32;
  }
  else if(zoom==50) {
   zoom_background_x = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+4+40;
   zoom_background_width = 32;
  }
  else if(zoom==100) {
   zoom_background_x = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+4+40+40;
   zoom_background_width = 40;
  }
  else if(zoom==200) {
   zoom_background_x = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+4+40+40+48;
   zoom_background_width = 40;
  }
  else if(zoom==300) {
   zoom_background_x = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+4+40+40+48+48;
   zoom_background_width = 40;
  }
  else if(zoom==400) {
   zoom_background_x = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+4+40+40+48+48+48;
   zoom_background_width = 40;
  }
  draw_full_square(zoom_background_x, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH+2, zoom_background_width, 16, RED);
  draw_empty_square(zoom_background_x, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH+2, zoom_background_width, 16, BLACK);
  print(" 25%  50%  100%  200%  300%  400%", GRAPHIC_EDITOR_SIDE_PANEL_WIDTH, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH+6, BLACK);
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 40, 20, GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_25);
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+40, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 40, 20, GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_50);
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+40+40, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 48, 20, GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_100);
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+40+40+48, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 48, 20, GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_200);
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+40+40+48+48, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 48, 20, GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_300);
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH+40+40+48+48+48, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH, 48, 20, GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_400);
  
  //print image dimensions
  dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
  dword_t size_of_digits_of_width = (get_number_of_digits_in_number(image_info[IMAGE_INFO_REAL_WIDTH])*8), size_of_digits_of_height = (get_number_of_digits_in_number(image_info[IMAGE_INFO_REAL_HEIGTH])*8);
  print_var(image_info[IMAGE_INFO_REAL_WIDTH], screen_width-8-size_of_digits_of_height-8-size_of_digits_of_width, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH+6, BLACK);
  print("x", screen_width-8-size_of_digits_of_height-8, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH+6, BLACK);
  print_var(image_info[IMAGE_INFO_REAL_HEIGTH], screen_width-8-size_of_digits_of_height, screen_height-20-GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGTH+6, BLACK);

  //draw image
  add_zone_to_click_board(GRAPHIC_EDITOR_SIDE_PANEL_WIDTH, 20, graphic_editor_image_area_width, graphic_editor_image_area_height, NO_CLICK); //clear click board
  draw_resized_image(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
  add_zone_to_click_board(image_info[IMAGE_INFO_SCREEN_X], image_info[IMAGE_INFO_SCREEN_Y], image_info[IMAGE_INFO_DRAW_WIDTH], image_info[IMAGE_INFO_DRAW_HEIGTH], GRAPHIC_EDITOR_CLICK_ZONE_IMAGE); //add image click zone

  //add scrollbars
  if(image_info[IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE]!=0) {
   program_interface_add_vertical_scrollbar(GRAPHIC_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR, ((dword_t)image_info)+IMAGE_INFO_DRAW_HEIGTH*4, ((dword_t)image_info)+IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION*4, ((dword_t)image_info)+IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE*4, ((dword_t)&graphic_editor_image_vertical_scrollbar_event));
   add_zone_to_click_board(image_info[IMAGE_INFO_SCREEN_X]+image_info[IMAGE_INFO_DRAW_WIDTH], image_info[IMAGE_INFO_SCREEN_Y], 10, image_info[IMAGE_INFO_DRAW_HEIGTH], GRAPHIC_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR);
  }
  if(image_info[IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE]!=0) {
   program_interface_add_horizontal_scrollbar(GRAPHIC_EDITOR_CLICK_ZONE_HORIZONTAL_SCROLLBAR, ((dword_t)image_info)+IMAGE_INFO_DRAW_WIDTH*4, ((dword_t)image_info)+IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION*4, ((dword_t)image_info)+IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE*4, ((dword_t)&graphic_editor_image_horizontal_scrollbar_event));
   add_zone_to_click_board(image_info[IMAGE_INFO_SCREEN_X], image_info[IMAGE_INFO_SCREEN_Y]+image_info[IMAGE_INFO_DRAW_HEIGTH], image_info[IMAGE_INFO_DRAW_WIDTH], 10, GRAPHIC_EDITOR_CLICK_ZONE_HORIZONTAL_SCROLLBAR);
  }

  //draw bottom buttons
  set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, 0);
  draw_bottom_line_button("[F7] Edit image", GRAPHIC_EDITOR_CLICK_ZONE_EDIT_IMAGE);
 }
}

void graphic_editor_redraw_image(void) {
 //draw image
 draw_resized_image(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));

 //redraw image
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 redraw_part_of_screen(image_info[IMAGE_INFO_SCREEN_X], image_info[IMAGE_INFO_SCREEN_Y], image_info[IMAGE_INFO_WIDTH]+10, image_info[IMAGE_INFO_HEIGTH]+10); //+10 include scrollbars
 redraw_mouse_cursor();
}

void graphic_editor_open_file(void) {
 //open file
 file_dialog_open_file_extensions_clear_mem();
 file_dialog_open_file_add_extension("qoi");
 file_dialog_open_file_add_extension("bmp");
 file_dialog_open_file_add_extension("png");
 file_dialog_open_file_add_extension("gif");
 dword_t new_file_mem = file_dialog_open();
 if(new_file_mem==0) {
  return; //file not loaded
 }

 //add file entry
 set_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE));
 add_file((word_t *)file_dialog_file_name, 0, 0, 0, 0, 0);
 
 //convert image
 if(is_loaded_file_extension("qoi")==STATUS_TRUE) {
  set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY, convert_qoi_to_image_data(new_file_mem));
 }
 else if(is_loaded_file_extension("bmp")==STATUS_TRUE) {
  set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY, convert_bmp_to_image_data(new_file_mem));
 }
 else if(is_loaded_file_extension("png")==STATUS_TRUE) {
  set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY, convert_png_to_image_data(new_file_mem, file_dialog_file_size));
 }
 else if(is_loaded_file_extension("gif")==STATUS_TRUE) {
  set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY, convert_gif_to_image_data(new_file_mem, file_dialog_file_size));
 }
 free(new_file_mem);
 if(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY)==STATUS_ERROR) {
  remove_file(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE));
  set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE));
  error_window("Error during decoding file, more info in system log");
  return;
 }

 //set variables
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_ZOOM, 100);
 set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_PREVIEW_MEMORY, malloc(image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]*4));
 graphic_editor_copy_image_to_preview();
 graphic_editor_image_saved_to_preview = STATUS_TRUE;
 set_program_value(PROGRAM_INTERFACE_SELECTED_CLICK_ZONE, 0);

 //calculate image dimensions, position on screen and scrollbars
 graphic_editor_image_recalculate_zoom();
}

void graphic_editor_save_file(void) {
 dword_t file_format_number = window_for_choosing_file_format(2, "[b] BMP\0[q] QOI");
 if(file_format_number==0xFFFFFFFF) {
  return;
 }
 else if(file_format_number==0) {
  show_message_window("Converting image...");
  file_dialog_save_set_extension("bmp");
  convert_image_data_to_bmp(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 }
 else if(file_format_number==1) {
  show_message_window("Converting image...");
  file_dialog_save_set_extension("qoi");
  convert_image_data_to_qoi(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 }
 if(file_dialog_save(converted_file_memory, converted_file_size)==STATUS_GOOD) {
  set_file_value(PROGRAM_INTERFACE_FILE_FLAGS, (get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) | PROGRAM_INTERFACE_FILE_FLAG_SAVED));
  set_file_name_from_file_dialog();
 }
 free(converted_file_memory);
}

void graphic_editor_new_file(void) {
 dword_t width = 640, height = 480;
 dword_t *width_text_area = (dword_t *) (graphic_editor_width_text_area_info_memory);
 dword_t *height_text_area = (dword_t *) (graphic_editor_height_text_area_info_memory);

 //create dialog window
 clear_program_interface_before_drawing();
 set_program_value(PROGRAM_INTERFACE_FLAGS, (get_program_value(PROGRAM_INTERFACE_FLAGS) | PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED));
 draw_message_window(64, 90);

 text_area_disable_cursor(graphic_editor_width_text_area_info_memory);
 clear_memory(width_text_area[TEXT_AREA_INFO_MEMORY], 8);
 convert_number_to_word_string(width, width_text_area[TEXT_AREA_INFO_MEMORY]);
 print("Width", screen_x_center-24, screen_y_center-50+5+6, BLACK);
 draw_text_area(graphic_editor_width_text_area_info_memory);
 program_interface_add_text_area(GRAPHIC_EDITOR_CLICK_ZONE_TEXT_AREA_WIDTH, graphic_editor_width_text_area_info_memory);
 add_zone_to_click_board(screen_x_center-24, screen_y_center-25, 48, 10, GRAPHIC_EDITOR_CLICK_ZONE_TEXT_AREA_WIDTH);

 text_area_disable_cursor(graphic_editor_height_text_area_info_memory);
 clear_memory(height_text_area[TEXT_AREA_INFO_MEMORY], 8);
 convert_number_to_word_string(height, height_text_area[TEXT_AREA_INFO_MEMORY]);
 print("Heigth", screen_x_center-24, screen_y_center-50+35+6, BLACK);
 draw_text_area(graphic_editor_height_text_area_info_memory);
 program_interface_add_text_area(GRAPHIC_EDITOR_CLICK_ZONE_TEXT_AREA_HEIGTH, graphic_editor_height_text_area_info_memory);
 add_zone_to_click_board(screen_x_center-24, screen_y_center+5, 48, 10, GRAPHIC_EDITOR_CLICK_ZONE_TEXT_AREA_HEIGTH);

 draw_button("OK", screen_x_center-24, screen_y_center+20, 48, 20);
 redraw_screen();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //do not create new file
  if(keyboard_value==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && is_mouse_in_zone(screen_y_center-50, screen_y_center+50, screen_x_center-32, screen_x_center+32)==STATUS_FALSE)) {
   set_program_value(PROGRAM_INTERFACE_FLAGS, (get_program_value(PROGRAM_INTERFACE_FLAGS) & ~PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED));
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //create file
  if(keyboard_value==KEY_ENTER || (mouse_click_button_state==MOUSE_CLICK && is_mouse_in_zone(screen_y_center+20, screen_y_center+40, screen_x_center-24, screen_x_center+24)==STATUS_TRUE)) {
   //get image size
   width = convert_word_string_to_number(width_text_area[TEXT_AREA_INFO_MEMORY]);
   height = convert_word_string_to_number(height_text_area[TEXT_AREA_INFO_MEMORY]);
   if(width==0 || height==0) {
    set_program_value(PROGRAM_INTERFACE_FLAGS, (get_program_value(PROGRAM_INTERFACE_FLAGS) & ~PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED));
    return;
   }
   if(width>4000) {
    width = 4000;
   }
   if(height>4000) {
    height = 4000;
   }

   //create new file
   add_file((word_t *)"N\0e\0w\0 \0i\0m\0a\0g\0e\0\0\0", 0, 0, 0, 0, 0);
   set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY, create_image(width, height));
   set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_ZOOM, 100);
   set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_PREVIEW_MEMORY, malloc(width*height*4));
   set_program_value(PROGRAM_INTERFACE_SELECTED_CLICK_ZONE, 0);

   //white image
   dword_t *new_image_memory = (dword_t *) (get_image_data_memory(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY)));
   for(int i=0; i<(width*height); i++) {
    *new_image_memory = WHITE;
    new_image_memory++;
   }
   graphic_editor_copy_image_to_preview();
   graphic_editor_image_saved_to_preview = STATUS_TRUE;
 
   //calculate image dimensions, position on screen and scrollbars
   graphic_editor_image_recalculate_zoom();
   set_program_value(PROGRAM_INTERFACE_FLAGS, (get_program_value(PROGRAM_INTERFACE_FLAGS) & ~PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED));
   return;
  }
 }
}

void graphic_editor_close_file(void) {
 delete_image(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 free(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_PREVIEW_MEMORY));
}

void graphic_editor_key_f7_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  draw_menu_list("[l] Turn left\n[r] Turn right\n[h] Reverse left-right\n[v] Reverse up-down", COLUMN_OF_FIRST_BUTTON_ON_BOTTOM_LINE, 0);

  while(1) {
   wait_for_user_input();
   move_mouse_cursor();

   //close menu
   if(keyboard_value==KEY_ESC || keyboard_value==KEY_F7) {
    program_interface_redraw();
    return;
   }

   //perform action
   if(keyboard_value==KEY_L) {
    image_turn_left(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
    goto finalize_image_action;
   }
   else if(keyboard_value==KEY_R) {
    image_turn_right(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
    goto finalize_image_action;
   }
   else if(keyboard_value==KEY_H) {
    image_reverse_horizontally(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
    goto finalize_image_action;
   }
   else if(keyboard_value==KEY_V) {
    image_reverse_vertically(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
    goto finalize_image_action;
   }

   if(mouse_click_button_state==MOUSE_CLICK) {
    if(is_mouse_in_zone(screen_height-20-4*20, screen_height-20, 8+10*8+8, 8+10*8+8+200)==STATUS_TRUE) {
     dword_t selected_item = get_number_of_clicked_item_from_menu_list(4);

     if(selected_item==0) {
      image_turn_left(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
     }
     else if(selected_item==1) {
      image_turn_right(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
     }
     else if(selected_item==2) {
      image_reverse_horizontally(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
     }
     else if(selected_item==3) {
      image_reverse_vertically(get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
     }
    }

    finalize_image_action:
    graphic_editor_copy_image_to_preview();
    graphic_editor_image_recalculate_zoom();
    program_interface_redraw();
    return;
   }
  }
 }
}

void graphic_editor_key_up_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
  image_info[IMAGE_INFO_DRAW_Y]-=50;
  if(image_info[IMAGE_INFO_DRAW_Y]>0x80000000) {
   image_info[IMAGE_INFO_DRAW_Y] = 0;
  }
  graphic_editor_image_recalculate_scrollbars();
  graphic_editor_redraw_image();
 }
}

void graphic_editor_key_down_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
  graphic_editor_image_recalculate_scrollbars();
  image_info[IMAGE_INFO_DRAW_Y]+=50;
  if(image_info[IMAGE_INFO_DRAW_Y]>(image_info[IMAGE_INFO_HEIGTH]-image_info[IMAGE_INFO_DRAW_HEIGTH])) {
   image_info[IMAGE_INFO_DRAW_Y] = (image_info[IMAGE_INFO_HEIGTH]-image_info[IMAGE_INFO_DRAW_HEIGTH]);
  }
  graphic_editor_image_recalculate_scrollbars();
  graphic_editor_redraw_image();
 }
}

void graphic_editor_key_left_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
  image_info[IMAGE_INFO_DRAW_X]-=50;
  if(image_info[IMAGE_INFO_DRAW_X]>0x80000000) {
   image_info[IMAGE_INFO_DRAW_X] = 0;
  }
  graphic_editor_image_recalculate_scrollbars();
  graphic_editor_redraw_image();
 }
}

void graphic_editor_key_right_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
  graphic_editor_image_recalculate_scrollbars();
  image_info[IMAGE_INFO_DRAW_X]+=50;
  if(image_info[IMAGE_INFO_DRAW_X]>(image_info[IMAGE_INFO_WIDTH]-image_info[IMAGE_INFO_DRAW_WIDTH])) {
   image_info[IMAGE_INFO_DRAW_X] = (image_info[IMAGE_INFO_WIDTH]-image_info[IMAGE_INFO_DRAW_WIDTH]);
  }
  graphic_editor_image_recalculate_scrollbars();
  graphic_editor_redraw_image();
 }
}

void graphic_editor_key_a_event(void) {
 graphic_editor_tool = GRAPHIC_EDITOR_TOOL_PEN;
 program_interface_redraw();
}

void graphic_editor_key_s_event(void) {
 graphic_editor_tool = GRAPHIC_EDITOR_TOOL_LINE;
 program_interface_redraw();
}

void graphic_editor_key_d_event(void) {
 graphic_editor_tool = GRAPHIC_EDITOR_TOOL_SQUARE;
 program_interface_redraw();
}

void graphic_editor_key_g_event(void) {
 graphic_editor_tool = GRAPHIC_EDITOR_TOOL_CIRCLE;
 program_interface_redraw();
}

void graphic_editor_key_c_event(void) {
 graphic_editor_tool = GRAPHIC_EDITOR_TOOL_FILL;
 program_interface_redraw();
}

void graphic_editor_key_e_event(void) {
 if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_SQUARE || graphic_editor_tool==GRAPHIC_EDITOR_TOOL_CIRCLE) {
  graphic_editor_tool_state = GRAPHIC_EDITOR_TOOL_STATE_EMPTY;
  program_interface_redraw();
 }
}

void graphic_editor_key_f_event(void) {
 if(graphic_editor_tool==GRAPHIC_EDITOR_TOOL_SQUARE || graphic_editor_tool==GRAPHIC_EDITOR_TOOL_CIRCLE) {
  graphic_editor_tool_state = GRAPHIC_EDITOR_TOOL_STATE_FULL;
  program_interface_redraw();
 }
}

void graphic_editor_key_t_event(void) {
 dword_t new_color = 0;
    
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();
  new_color = get_mouse_cursor_pixel_color();
  
  draw_full_square(8, GRAPHIC_EDITOR_COLOR_AREA_LINE+20, 40, 20, new_color);
  redraw_part_of_screen(8, GRAPHIC_EDITOR_COLOR_AREA_LINE+20, 40, 20);
  
  if(keyboard_value==KEY_ESC) {
   program_interface_redraw();
   return;
  }

  if(mouse_click_button_state==MOUSE_CLICK || keyboard_value==KEY_ENTER) {
   graphic_editor_color = new_color;
   program_interface_redraw();
   return;
  }
 }
}

void graphic_editor_key_1_event(void) {
 if(program_interface_element_with_focus==0xFFFFFFFF) {
  graphic_editor_pen_size = 1;
  program_interface_redraw();
 }
}

void graphic_editor_key_2_event(void) {
 if(program_interface_element_with_focus==0xFFFFFFFF) {
  graphic_editor_pen_size = 2;
  program_interface_redraw();
 }
}

void graphic_editor_key_3_event(void) {
 if(program_interface_element_with_focus==0xFFFFFFFF) {
  graphic_editor_pen_size = 3;
  program_interface_redraw();
 }
}

void graphic_editor_key_5_event(void) {
 if(program_interface_element_with_focus==0xFFFFFFFF) {
  graphic_editor_pen_size = 5;
  program_interface_redraw();
 }
}

void graphic_editor_key_0_event(void) {
 if(program_interface_element_with_focus==0xFFFFFFFF) {
  graphic_editor_pen_size = 10;
  program_interface_redraw();
 }
}

void graphic_editor_key_enter_event(void) {
 if(program_interface_element_with_focus!=0xFFFFFFFF && get_program_value(PROGRAM_INTERFACE_SELECTED_CLICK_ZONE)==GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_TEXT_AREA) {
  text_area_disable_cursor(graphic_editor_pen_size_text_area_info_memory);

  dword_t *pen_size_text_area = (dword_t *) (graphic_editor_pen_size_text_area_info_memory);
  graphic_editor_pen_size = convert_word_string_to_number(pen_size_text_area[TEXT_AREA_INFO_MEMORY]);
  if(graphic_editor_pen_size==0) {
   graphic_editor_pen_size = 1;
  }
  else if(graphic_editor_pen_size>100) {
   graphic_editor_pen_size = 100;
  }

  program_interface_redraw();
 }
}

void graphic_editor_click_zone_event_palette(void) {
 graphic_editor_color = get_mouse_cursor_pixel_color();
 program_interface_redraw();
}

void graphic_editor_click_zone_event_pen_size_20(void) {
 graphic_editor_pen_size = 20;
 program_interface_redraw();
}

void graphic_editor_click_zone_event_zoom_25(void) {
 graphic_editor_set_image_zoom(25);
}

void graphic_editor_click_zone_event_zoom_50(void) {
 graphic_editor_set_image_zoom(50);
}

void graphic_editor_click_zone_event_zoom_100(void) {
 graphic_editor_set_image_zoom(100);
}

void graphic_editor_click_zone_event_zoom_200(void) {
 graphic_editor_set_image_zoom(200);
}

void graphic_editor_click_zone_event_zoom_300(void) {
 graphic_editor_set_image_zoom(300);
}

void graphic_editor_click_zone_event_zoom_400(void) {
 graphic_editor_set_image_zoom(400);
}

void graphic_editor_image_vertical_scrollbar_event(void) {
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 image_info[IMAGE_INFO_DRAW_Y] = get_scrollbar_rider_value(image_info[IMAGE_INFO_DRAW_HEIGTH], image_info[IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE], image_info[IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION], image_info[IMAGE_INFO_HEIGTH], image_info[IMAGE_INFO_DRAW_HEIGTH]);
 graphic_editor_redraw_image();
}

void graphic_editor_image_horizontal_scrollbar_event(void) {
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 image_info[IMAGE_INFO_DRAW_X] = get_scrollbar_rider_value(image_info[IMAGE_INFO_DRAW_WIDTH], image_info[IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE], image_info[IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION], image_info[IMAGE_INFO_WIDTH], image_info[IMAGE_INFO_DRAW_WIDTH]);
 graphic_editor_redraw_image();
}

void graphic_editor_set_image_zoom(dword_t zoom) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  set_file_value(GRAPHIC_EDITOR_FILE_IMAGE_ZOOM, zoom);
  graphic_editor_image_recalculate_zoom();
  program_interface_redraw();
 }
}

void graphic_editor_image_recalculate_scrollbars(void) {
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));

 //calculate scrollbars
 calculate_image_scrollbars((dword_t)image_info);

 //calculate size of image with scrollbars
 if(image_info[IMAGE_INFO_WIDTH]<graphic_editor_image_area_width) {
  image_info[IMAGE_INFO_DRAW_WIDTH] = image_info[IMAGE_INFO_WIDTH];
 }
 else {
  image_info[IMAGE_INFO_DRAW_WIDTH] = graphic_editor_image_area_width;
 }
 if(image_info[IMAGE_INFO_HEIGTH]<graphic_editor_image_area_height) {
  image_info[IMAGE_INFO_DRAW_HEIGTH] = image_info[IMAGE_INFO_HEIGTH];
 }
 else {
  image_info[IMAGE_INFO_DRAW_HEIGTH] = graphic_editor_image_area_height;
 }
 if(image_info[IMAGE_INFO_WIDTH]>graphic_editor_image_area_width && image_info[IMAGE_INFO_HEIGTH]>graphic_editor_image_area_height) { //both scrollbars
  image_info[IMAGE_INFO_DRAW_HEIGTH] -= 10;
  image_info[IMAGE_INFO_DRAW_WIDTH] -= 10;
 }
 else if(image_info[IMAGE_INFO_HEIGTH]>graphic_editor_image_area_height && image_info[IMAGE_INFO_WIDTH]<=graphic_editor_image_area_width) { //vertical scrollbar
  if((graphic_editor_image_area_width-image_info[IMAGE_INFO_WIDTH])<20 || (graphic_editor_image_area_width-image_info[IMAGE_INFO_WIDTH])>0x80000000) { //also horizontal scrollbar
   image_info[IMAGE_INFO_DRAW_HEIGTH] -= 10;
   image_info[IMAGE_INFO_DRAW_WIDTH] -= 10;
  }
 }
 else if(image_info[IMAGE_INFO_HEIGTH]<=graphic_editor_image_area_height && image_info[IMAGE_INFO_WIDTH]>graphic_editor_image_area_width) { //horizontal scrollbar
  if((graphic_editor_image_area_height-image_info[IMAGE_INFO_HEIGTH])<20 || (graphic_editor_image_area_height-image_info[IMAGE_INFO_HEIGTH])>0x80000000) { //also vertical scrollbar
   image_info[IMAGE_INFO_DRAW_HEIGTH] -= 10;
   image_info[IMAGE_INFO_DRAW_WIDTH] -= 10;
  }
 }

 //calculate scrollbars after size change
 calculate_image_scrollbars((dword_t)image_info);
}

void graphic_editor_image_recalculate_zoom(void) {
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 dword_t zoom = get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_ZOOM);
 
 //calculate width and height by zoom
 image_info[IMAGE_INFO_WIDTH]=(image_info[IMAGE_INFO_REAL_WIDTH]*zoom/100);
 image_info[IMAGE_INFO_HEIGTH]=(image_info[IMAGE_INFO_REAL_HEIGTH]*zoom/100);
 image_info[IMAGE_INFO_DRAW_WIDTH]=image_info[IMAGE_INFO_WIDTH];
 image_info[IMAGE_INFO_DRAW_HEIGTH]=image_info[IMAGE_INFO_HEIGTH];

 //set dimensions of image for screen
 if(image_info[IMAGE_INFO_WIDTH]<graphic_editor_image_area_width) {
  image_info[IMAGE_INFO_SCREEN_X] = (graphic_editor_image_area_width_center-(image_info[IMAGE_INFO_WIDTH]/2));
  image_info[IMAGE_INFO_DRAW_X] = 0;
 }
 else {
  image_info[IMAGE_INFO_DRAW_WIDTH] = graphic_editor_image_area_width;
  image_info[IMAGE_INFO_SCREEN_X] = GRAPHIC_EDITOR_SIDE_PANEL_WIDTH;
  if(image_info[IMAGE_INFO_DRAW_X]>(image_info[IMAGE_INFO_WIDTH]-graphic_editor_image_area_width)) {
   image_info[IMAGE_INFO_DRAW_X] = (image_info[IMAGE_INFO_WIDTH]-graphic_editor_image_area_width);
  }
 }
 if(image_info[IMAGE_INFO_HEIGTH]<graphic_editor_image_area_height) {
  image_info[IMAGE_INFO_SCREEN_Y] = (graphic_editor_image_area_height_center-(image_info[IMAGE_INFO_HEIGTH]/2));
  image_info[IMAGE_INFO_DRAW_Y] = 0;
 }
 else {
  image_info[IMAGE_INFO_DRAW_HEIGTH] = graphic_editor_image_area_height;
  image_info[IMAGE_INFO_SCREEN_Y] = 21;
  if(image_info[IMAGE_INFO_DRAW_Y]>(image_info[IMAGE_INFO_HEIGTH]-graphic_editor_image_area_height)) {
   image_info[IMAGE_INFO_DRAW_Y] = (image_info[IMAGE_INFO_HEIGTH]-graphic_editor_image_area_height);
  }
 }
 
 //set scrollbars
 graphic_editor_image_recalculate_scrollbars();
}

void graphic_editor_copy_image_to_preview(void) {
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 dword_t *image_data = (dword_t *) (get_image_data_memory((dword_t)image_info));
 dword_t *preview_data = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_PREVIEW_MEMORY));
 
 for(int i=0; i<(image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]); i++) {
  *preview_data = *image_data;
  preview_data++;
  image_data++;
 }
}

void graphic_editor_copy_preview_to_image(void) {
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 dword_t *image_data = (dword_t *) (get_image_data_memory((dword_t)image_info));
 dword_t *preview_data = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_PREVIEW_MEMORY));
 
 for(int i=0; i<(image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]); i++) {
  *image_data = *preview_data;
  image_data++;
  preview_data++;
 }
}

void graphic_editor_prepare_drawing_by_tool_without_copying_preview(void) {
 //save monitor screen
 screen_save_variables();

 //switch to image as screen
 dword_t *image_info = (dword_t *) (get_file_value(GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY));
 screen_double_buffer_memory_pointer = (byte_t *) get_image_data_memory((dword_t)image_info);
 screen_width = image_info[IMAGE_INFO_REAL_WIDTH];
 screen_height = image_info[IMAGE_INFO_REAL_HEIGTH];
 screen_double_buffer_bytes_per_line = (screen_width*4);

 //set pen size
 set_pen_width(graphic_editor_pen_size, graphic_editor_color);
}

void graphic_editor_prepare_drawing_by_tool(void) {
 //copy preview
 graphic_editor_copy_preview_to_image();

 //other stuff
 graphic_editor_prepare_drawing_by_tool_without_copying_preview();
}

void graphic_editor_finish_drawing_by_tool(void) {
 //restore pen size
 set_pen_width(1, graphic_editor_color);

 //switch back to monitor screen
 screen_restore_variables();

 //redraw image
 graphic_editor_redraw_image();

 //check if we need to set "unsaved" flag
 if((get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & PROGRAM_INTERFACE_FILE_FLAG_SAVED)==PROGRAM_INTERFACE_FILE_FLAG_SAVED) {
  set_file_value(PROGRAM_INTERFACE_FILE_FLAGS, (get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & ~PROGRAM_INTERFACE_FILE_FLAG_SAVED));
  program_interface_redraw();
 }
 graphic_editor_image_saved_to_preview = STATUS_FALSE;
}