//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_program_interface(byte_t *program_name, byte_t *down_string, dword_t border_color, dword_t background_color) {
 //clear click board
 clear_click_board();

 //draw background
 clear_screen(background_color);
 global_color = BLACK;
 set_pen_width(1);

 //draw top border
 draw_full_square(0, 0, graphic_screen_x, 20, border_color);
 draw_straigth_line(0, 20, graphic_screen_x);

 //draw program name
 print(program_name, 8, 6, BLACK);
 dword_t size_of_program_name = 0;
 for(dword_t i=0; i<1000; i++) {
  if(program_name[i]!=0) {
   size_of_program_name += 8;
  }
  else {
   break;
  }
 }
 add_zone_to_click_board(0, 0, size_of_program_name+16, 20, CLICK_ZONE_PROGRAM_NAME);

 //draw "NEW" button
 dword_t draw_column = graphic_screen_x;
 if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON)==STATUS_FALSE) {
  draw_straigth_line(graphic_screen_x-2-15, 10, 15);
  draw_straigth_column(graphic_screen_x-2-8, 2, 16);
  draw_straigth_column(graphic_screen_x-20, 0, 20);
  add_zone_to_click_board(graphic_screen_x-20, 0, 20, 20, CLICK_ZONE_NEW);

  draw_column -= 20;
 }

 //draw file entries
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //calculate values
  dword_t size_of_file_area = (graphic_screen_x-20-size_of_program_name-16);
  if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON)==PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON) {
   size_of_file_area += 20;
  }
  dword_t size_of_file_entry = 0, number_of_file_entry_chars = 0;
  if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)*193<=size_of_file_area) {
   size_of_file_entry = 193;
   number_of_file_entry_chars = 20;
  }
  else {
   size_of_file_entry = (size_of_file_area/get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES));
   number_of_file_entry_chars = ((size_of_file_entry-24-9)/8);
  }

  //draw selected file background
  if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON)==STATUS_FALSE) {
   draw_full_square((graphic_screen_x-20-(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)*size_of_file_entry)), 0, size_of_file_entry, 20, RED);
  }
  else {
   draw_full_square((graphic_screen_x-(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)*size_of_file_entry)), 0, size_of_file_entry, 20, RED);
  }

  //draw all entries
  set_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE));
  draw_column -= size_of_file_entry;
  for(dword_t i=0; i<get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES); i++, draw_column-=size_of_file_entry) {
   //file
   set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, (i+1));
   add_zone_to_click_board(draw_column, 0, size_of_file_entry, 20, (CLICK_ZONE_FIRST_FILE+i));

   //closing square
   draw_empty_square((draw_column+size_of_file_entry-8-9), 5, 10, 10, BLACK);
   draw_line((draw_column+size_of_file_entry-8-9), 5, (draw_column+size_of_file_entry-8), 5+9, BLACK);
   draw_line((draw_column+size_of_file_entry-8-9), 5+9, (draw_column+size_of_file_entry-8), 5, BLACK);
   add_zone_to_click_board((draw_column+size_of_file_entry-8-9), 5, 9, 9, (CLICK_ZONE_CLOSE_FIRST_FILE+i));

   //name
   if((get_specific_file_value((i+1), PROGRAM_INTERFACE_FILE_FLAGS) & PROGRAM_INTERFACE_FILE_FLAG_SAVED)==STATUS_FALSE) { //unsaved file
    draw_char('*', (draw_column+8), 6, BLACK);
    for(dword_t j=0; j<(number_of_file_entry_chars-1); j++) {
     if(get_char_of_file_name(j)==0) {
      break;
     }
     draw_char(get_char_of_file_name(j), (draw_column+16+j*8), 6, BLACK);
    } 
   }
   else { //saved file
    for(dword_t j=0; j<number_of_file_entry_chars; j++) {
     if(get_char_of_file_name(j)==0) {
      break;
     }
     draw_char(get_char_of_file_name(j), (draw_column+8+j*8), 6, BLACK);
    } 
   }
  }
  set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE));
 }

 //draw bottom border
 draw_full_square(0, graphic_screen_y-19, graphic_screen_x, 19, border_color);
 draw_straigth_line(0, graphic_screen_y-20, graphic_screen_x);

 //draw "BACK" button
 print("[esc] Back", 8, graphic_screen_y-6-7, BLACK);
 draw_straigth_column(8+10*8+8, graphic_screen_y-20, 20);
 add_zone_to_click_board(0, graphic_screen_y-20, 8+10*8+8, 20, CLICK_ZONE_BACK);

 //print down string
 print(down_string, 8+10*8+8+8, graphic_screen_y-6-7, BLACK);

 //draw "OPEN" and "SAVE" button
 draw_column = (graphic_screen_x-8-9*8-8);
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON)==PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON) {
  //draw "OPEN" button
  print("[F1] Open", draw_column+8, graphic_screen_y-6-7, BLACK);
  draw_straigth_column(draw_column, graphic_screen_y-20, 20);
  add_zone_to_click_board(draw_column, graphic_screen_y-20, 8+9*8+8, 20, CLICK_ZONE_OPEN);
 }
 else {
  //draw "SAVE" button
  print("[F2] Save", draw_column+8, graphic_screen_y-6-7, BLACK);
  draw_straigth_column(draw_column, graphic_screen_y-20, 20);
  add_zone_to_click_board(draw_column, graphic_screen_y-20, 8+9*8+8, 20, CLICK_ZONE_SAVE);
  draw_column -= (8+9*8+8);

  //draw "OPEN" button
  print("[F1] Open", draw_column+8, graphic_screen_y-6-7, BLACK);
  draw_straigth_column(draw_column, graphic_screen_y-20, 20);
  add_zone_to_click_board(draw_column, graphic_screen_y-20, 8+9*8+8, 20, CLICK_ZONE_OPEN);
 }
}

void draw_dialog_yes_no(byte_t *string) {
 dword_t num_of_chars = get_number_of_chars_in_ascii_string(string);
 dword_t dialog_width = ((num_of_chars*8)+32+2);
 if(dialog_width<200) {
  dialog_width = 200;
 }
 draw_full_square(graphic_screen_x_center-(dialog_width/2), graphic_screen_y_center-38, dialog_width, 2+16+7+16+2+5+7+5+16, 0xFF7000);
 draw_empty_square(graphic_screen_x_center-(dialog_width/2), graphic_screen_y_center-38, dialog_width, 2+16+7+16+2+5+7+5+16, BLACK);
 print(string, graphic_screen_x_center-(num_of_chars*4), graphic_screen_y_center-38+2+16, BLACK);
 draw_full_square(graphic_screen_x_center-10-80, graphic_screen_y_center-38+2+16+7+16, 80, 17, 0xFF9000);
 draw_empty_square(graphic_screen_x_center-10-80, graphic_screen_y_center-38+2+16+7+16, 80, 17, BLACK);
 print("Yes", graphic_screen_x_center-10-40-12, graphic_screen_y_center-38+2+16+7+16+1+4, BLACK);
 draw_full_square(graphic_screen_x_center+10, graphic_screen_y_center-38+2+16+7+16, 80, 17, 0xFF9000);
 draw_empty_square(graphic_screen_x_center+10, graphic_screen_y_center-38+2+16+7+16, 80, 17, BLACK);
 print("No", graphic_screen_x_center+10+40-8, graphic_screen_y_center-38+2+16+7+16+1+4, BLACK);
}

void draw_bottom_line_button(byte_t *string, dword_t click_zone) {
 dword_t number_of_chars = get_number_of_chars_in_ascii_string(string);
 dword_t x = (get_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN)+8+10*8+8);
 global_color = BLACK;
 draw_straigth_column(x, graphic_screen_y-20, 20);
 print(string, x+8, graphic_screen_y-6-7, BLACK);
 draw_straigth_column(x+8+number_of_chars*8+8, graphic_screen_y-20, 20);
 add_zone_to_click_board(x, graphic_screen_y-20, 8+number_of_chars*8+8, 20, click_zone);
 set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, get_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN)+8+number_of_chars*8+8);
}