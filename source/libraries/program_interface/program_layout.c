//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void program_layout_set_dimensions(dword_t x, dword_t y, dword_t width, dword_t height) {
 program_layout_draw_x = program_layout_x = x;
 program_layout_draw_y = program_layout_y = y;
 program_layout_width = width;
 program_layout_height = height;
}

void program_layout_set_dimensions_window(dword_t width, dword_t height) {
 program_layout_draw_x = program_layout_x = (screen_x_center-(width/2)+10);
 program_layout_draw_y = program_layout_y = (screen_y_center-(height/2)+10);
 program_layout_width = (width-20);
 program_layout_height = (height-20);
}

void program_layout_add_element(dword_t height) {
 program_layout_draw_x = program_layout_x;
 program_layout_draw_y += (height+10);
}

void program_layout_add_text_line(void) {
 program_layout_add_element(10);
}

void program_layout_add_button(void) {
 program_layout_add_element(20);
}

//////////////////////////////

void program_element_layout_initalize_for_program(void) {
 program_element_layout_number_of_areas = 1;
 program_element_layout_areas_info[FIRST_AREA].x = 0;
 program_element_layout_areas_info[FIRST_AREA].y = PROGRAM_INTERFACE_TOP_LINE_HEIGHT;
 program_element_layout_areas_info[FIRST_AREA].width = screen_width;
 program_element_layout_areas_info[FIRST_AREA].height = (screen_height-PROGRAM_INTERFACE_TOP_LINE_HEIGHT-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT);
 program_element_layout_areas_info[FIRST_AREA].actual_element_x = program_element_layout_areas_info[FIRST_AREA].x;
 program_element_layout_areas_info[FIRST_AREA].actual_element_y = program_element_layout_areas_info[FIRST_AREA].y;
 program_element_layout_bottom_line_left_x = 0;
 program_element_layout_bottom_line_right_x = screen_width;
}

void program_element_layout_initalize_for_window(dword_t width, dword_t height) {
 program_element_layout_number_of_areas = 1;
 program_element_layout_areas_info[FIRST_AREA].x = screen_x_center-(width/2);
 program_element_layout_areas_info[FIRST_AREA].y = screen_y_center-(height/2);
 program_element_layout_areas_info[FIRST_AREA].width = width;
 program_element_layout_areas_info[FIRST_AREA].height = height;
 program_element_layout_areas_info[FIRST_AREA].actual_element_x = program_element_layout_areas_info[FIRST_AREA].x;
 program_element_layout_areas_info[FIRST_AREA].actual_element_y = program_element_layout_areas_info[FIRST_AREA].y;
}

void program_element_layout_initalize_for_area(dword_t x, dword_t y, dword_t width, dword_t height) {
 program_element_layout_number_of_areas = 1;
 program_element_layout_areas_info[FIRST_AREA].x = x;
 program_element_layout_areas_info[FIRST_AREA].y = y;
 program_element_layout_areas_info[FIRST_AREA].width = width;
 program_element_layout_areas_info[FIRST_AREA].height = height;
 program_element_layout_areas_info[FIRST_AREA].actual_element_x = program_element_layout_areas_info[FIRST_AREA].x;
 program_element_layout_areas_info[FIRST_AREA].actual_element_y = program_element_layout_areas_info[FIRST_AREA].y;
}

void program_element_layout_split_area_vertically(byte_t area_number, byte_t unit_type, dword_t number) {
 if(program_element_layout_number_of_areas>=PROGRAM_ELEMENT_LAYOUT_MAX_NUM_OF_AREAS) {
  return;
 }

 if(unit_type==IN_PIXELS) {
  //create new area
  program_element_layout_areas_info[program_element_layout_number_of_areas].x = program_element_layout_areas_info[area_number].x+number;
  program_element_layout_areas_info[program_element_layout_number_of_areas].y = program_element_layout_areas_info[area_number].y;
  program_element_layout_areas_info[program_element_layout_number_of_areas].width = program_element_layout_areas_info[area_number].width-number;
  program_element_layout_areas_info[program_element_layout_number_of_areas].height = program_element_layout_areas_info[area_number].height;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_x = program_element_layout_areas_info[area_number].x;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_y = program_element_layout_areas_info[area_number].y;
 
  //recalculate previous area
  program_element_layout_areas_info[area_number].width = number;
 }
 else if(unit_type==IN_PERCENTS) {
  //create new area
  program_element_layout_areas_info[program_element_layout_number_of_areas].x = program_element_layout_areas_info[area_number].x+(program_element_layout_areas_info[area_number].width*number/100);
  program_element_layout_areas_info[program_element_layout_number_of_areas].y = program_element_layout_areas_info[area_number].y;
  program_element_layout_areas_info[program_element_layout_number_of_areas].width = program_element_layout_areas_info[area_number].width-(program_element_layout_areas_info[area_number].width*number/100);
  program_element_layout_areas_info[program_element_layout_number_of_areas].height = program_element_layout_areas_info[area_number].height;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_x = program_element_layout_areas_info[area_number].x;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_y = program_element_layout_areas_info[area_number].y;
 
  //recalculate previous area
  program_element_layout_areas_info[area_number].width = (program_element_layout_areas_info[area_number].width*number/100);
 }

 program_element_layout_number_of_areas++;
}

void program_element_layout_split_area_horizontally(byte_t area_number, byte_t unit_type, dword_t number) {
 if(program_element_layout_number_of_areas>=PROGRAM_ELEMENT_LAYOUT_MAX_NUM_OF_AREAS) {
  return;
 }

 if(unit_type==IN_PIXELS) {
  //create new area
  program_element_layout_areas_info[program_element_layout_number_of_areas].x = program_element_layout_areas_info[area_number].x;
  program_element_layout_areas_info[program_element_layout_number_of_areas].y = program_element_layout_areas_info[area_number].y+number;
  program_element_layout_areas_info[program_element_layout_number_of_areas].width = program_element_layout_areas_info[program_element_layout_number_of_areas].width;
  program_element_layout_areas_info[program_element_layout_number_of_areas].height = program_element_layout_areas_info[program_element_layout_number_of_areas].height-number;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_x = program_element_layout_areas_info[program_element_layout_number_of_areas].x;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_y = program_element_layout_areas_info[program_element_layout_number_of_areas].y;
 
  //recalculate previous area
  program_element_layout_areas_info[area_number].height = number;
 }
 else if(unit_type==IN_PERCENTS) {
  //create new area
  program_element_layout_areas_info[program_element_layout_number_of_areas].x = program_element_layout_areas_info[area_number].x;
  program_element_layout_areas_info[program_element_layout_number_of_areas].y = program_element_layout_areas_info[area_number].y+(program_element_layout_areas_info[area_number].height*number/100);
  program_element_layout_areas_info[program_element_layout_number_of_areas].width = program_element_layout_areas_info[program_element_layout_number_of_areas].width;
  program_element_layout_areas_info[program_element_layout_number_of_areas].height = program_element_layout_areas_info[program_element_layout_number_of_areas].height-(program_element_layout_areas_info[area_number].height*number/100);
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_x = program_element_layout_areas_info[program_element_layout_number_of_areas].x;
  program_element_layout_areas_info[program_element_layout_number_of_areas].actual_element_y = program_element_layout_areas_info[program_element_layout_number_of_areas].y;
 
  //recalculate previous area
  program_element_layout_areas_info[area_number].height = (program_element_layout_areas_info[area_number].height*number/100);
 }

 program_element_layout_number_of_areas++;
}

void program_element_layout_add_border_to_area(byte_t area_number, dword_t border) {
 program_element_layout_areas_info[area_number].x += border;
 program_element_layout_areas_info[area_number].y += border;
 program_element_layout_areas_info[area_number].width -= border*2;
 program_element_layout_areas_info[area_number].height -= border*2;
 program_element_layout_areas_info[area_number].actual_element_x = program_element_layout_areas_info[area_number].x;
 program_element_layout_areas_info[area_number].actual_element_y = program_element_layout_areas_info[area_number].y;
}

void program_element_layout_add_borders_to_area(byte_t area_number, dword_t top_border, dword_t left_border, dword_t bottom_border, dword_t right_border) {
 program_element_layout_areas_info[area_number].x += left_border;
 program_element_layout_areas_info[area_number].y += top_border;
 program_element_layout_areas_info[area_number].width -= (left_border+right_border);
 program_element_layout_areas_info[area_number].height -= (top_border+bottom_border);
 program_element_layout_areas_info[area_number].actual_element_x = program_element_layout_areas_info[area_number].x;
 program_element_layout_areas_info[area_number].actual_element_y = program_element_layout_areas_info[area_number].y;
}

void program_element_layout_calculate_element_position(byte_t area_number, byte_t alignment, dword_t width, dword_t height) {
 if(alignment==ELEMENT_LEFT_ALIGNMENT) {
  element_x = program_element_layout_areas_info[area_number].actual_element_x;
 }
 else if(alignment==ELEMENT_MIDDLE_ALIGNMENT) {
  element_x = program_element_layout_areas_info[area_number].actual_element_x+(program_element_layout_areas_info[area_number].width/2)-(width/2);
 }
 else if(alignment==ELEMENT_RIGHT_ALIGNMENT) {
  element_x = program_element_layout_areas_info[area_number].actual_element_x+program_element_layout_areas_info[area_number].width-width;
 }
 element_y = program_element_layout_areas_info[area_number].actual_element_y;
 element_width = width;
 element_height = height;
}

void program_element_layout_calculate_floating_element_position(byte_t area_number, dword_t area_width, dword_t area_height, byte_t horizontal_alignment, byte_t vertical_alignment, dword_t width, dword_t height) {
 //this method calculate only element_x and element_y

 if(horizontal_alignment==ELEMENT_LEFT_ALIGNMENT) {
  element_x = program_element_layout_areas_info[area_number].actual_element_x;
 }
 else if(horizontal_alignment==ELEMENT_MIDDLE_ALIGNMENT) {
  element_x = program_element_layout_areas_info[area_number].actual_element_x+(area_width/2)-(width/2);
 }
 else if(horizontal_alignment==ELEMENT_RIGHT_ALIGNMENT) {
  element_x = program_element_layout_areas_info[area_number].actual_element_x+area_width-width;
 }

 if(vertical_alignment==ELEMENT_TOP_ALIGNMENT) {
  element_y = program_element_layout_areas_info[area_number].actual_element_y;
 }
 else if(vertical_alignment==ELEMENT_CENTER_ALIGNMENT) {
  element_y = program_element_layout_areas_info[area_number].actual_element_y+(area_height/2)-(height/2);
 }
 else if(vertical_alignment==ELEMENT_BOTTOM_ALIGNMENT) {
  element_y = program_element_layout_areas_info[area_number].actual_element_y+area_height-height;
 }
}

void program_element_move_vertically(byte_t area_number, dword_t border) {
 program_element_layout_areas_info[area_number].actual_element_x += element_width+border;
}

void program_element_layout_skip_space_vertically(byte_t area_number, dword_t border) {
 program_element_layout_areas_info[area_number].actual_element_x += border;
}

void program_element_move_horizontally(byte_t area_number, dword_t border) {
 program_element_layout_areas_info[area_number].actual_element_x = program_element_layout_areas_info[area_number].x;
 program_element_layout_areas_info[area_number].actual_element_y += element_height+border;
}

void program_element_layout_skip_space_horizontally(byte_t area_number, dword_t border) {
 program_element_layout_areas_info[area_number].actual_element_x = program_element_layout_areas_info[area_number].x;
 program_element_layout_areas_info[area_number].actual_element_y += border;
}

void add_label(byte_t area_number, byte_t alignment, byte_t *string) {
 program_element_layout_calculate_element_position(area_number, alignment, get_number_of_chars_in_ascii_string(string)*8, 8);
 print(string, element_x, element_y, BLACK);
}

void add_button(byte_t area_number, byte_t alignment, byte_t width, byte_t *string, dword_t click_zone) {
 program_element_layout_calculate_element_position(area_number, alignment, width, 20);
 draw_button_with_click_zone(string, element_x, element_y, width, 20, click_zone);
}

void add_list_entry(byte_t area_number, byte_t alignment, byte_t width, byte_t *string, dword_t click_zone, dword_t entry_number) {
 program_element_layout_calculate_element_position(area_number, alignment, width, 20);
 if(entry_number==SELECTED_LIST_ENTRY) {
  draw_full_square(element_x, element_y, width, 20, RED);
 }
 else if((entry_number & 0x1)==0) {
  draw_full_square(element_x, element_y, width, 20, 0xAAAAAA);
 }
 else {
  draw_full_square(element_x, element_y, width, 20, 0x888888);
 }
 print(string, element_x+8, element_y+6, BLACK);
 add_zone_to_click_board(element_x, element_y, width, 20, click_zone);
}

void add_button_with_specific_color(byte_t area_number, byte_t alignment, byte_t width, byte_t *string, dword_t color, dword_t click_zone) {
 program_element_layout_calculate_element_position(area_number, alignment, width, 20);
 draw_button_with_specific_color_and_click_zone(string, element_x, element_y, width, 20, color, click_zone);
}

void add_button_to_bottom_line_from_left(byte_t *string, dword_t click_zone) {
 add_zone_to_click_board(program_element_layout_bottom_line_left_x, screen_height-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, 8+get_number_of_chars_in_ascii_string(string)*8+8, PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, click_zone);
 program_element_layout_bottom_line_left_x += 8;
 print(string, program_element_layout_bottom_line_left_x, screen_height-13, BLACK);
 program_element_layout_bottom_line_left_x += (get_number_of_chars_in_ascii_string(string)*8+8);
 draw_straigth_column(program_element_layout_bottom_line_left_x, screen_height-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, BLACK);
}

void add_button_to_bottom_line_from_right(byte_t *string, dword_t click_zone) {
 add_zone_to_click_board(program_element_layout_bottom_line_right_x-8-get_number_of_chars_in_ascii_string(string)*8-8, screen_height-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, 8+get_number_of_chars_in_ascii_string(string)*8+8, PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, click_zone);
 program_element_layout_bottom_line_right_x -= (get_number_of_chars_in_ascii_string(string)*8+8);
 print(string, program_element_layout_bottom_line_right_x, screen_height-13, BLACK);
 program_element_layout_bottom_line_right_x -= 8;
 draw_straigth_column(program_element_layout_bottom_line_right_x, screen_height-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT, BLACK);
}

void program_element_layout_draw_background_of_area(byte_t area_number, dword_t color) {
 draw_full_square(program_element_layout_areas_info[area_number].x,
                  program_element_layout_areas_info[area_number].y,
                  program_element_layout_areas_info[area_number].width,
                  program_element_layout_areas_info[area_number].height,
                  color);
}