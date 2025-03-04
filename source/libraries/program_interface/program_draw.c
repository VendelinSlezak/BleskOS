//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_program_interface(byte_t *program_name, byte_t *down_string, dword_t border_color, dword_t background_color) {
 //clear click board
 clear_click_board();

 //draw background
 clear_screen(background_color);
 set_pen_width(1, BLACK);

 //draw top border
 draw_full_square(0, 0, screen_width, 20, border_color);
 draw_straigth_line(0, 20, screen_width, BLACK);

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
 dword_t draw_column = screen_width;
 if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON)==STATUS_FALSE) {
  draw_straigth_line(screen_width-2-15, 10, 15, BLACK);
  draw_straigth_column(screen_width-2-8, 2, 16, BLACK);
  draw_straigth_column(screen_width-20, 0, 20, BLACK);
  add_zone_to_click_board(screen_width-20, 0, 20, 20, CLICK_ZONE_NEW);

  draw_column -= 20;
 }

 //draw file entries
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //calculate values
  dword_t size_of_file_area = (screen_width-20-size_of_program_name-16);
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
   draw_full_square((screen_width-20-(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)*size_of_file_entry)), 0, size_of_file_entry, 20, RED);
  }
  else {
   draw_full_square((screen_width-(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)*size_of_file_entry)), 0, size_of_file_entry, 20, RED);
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
 draw_full_square(0, screen_height-19, screen_width, 19, border_color);
 draw_straigth_line(0, screen_height-20, screen_width, BLACK);

 //draw "BACK" button
 print("[esc] Back", 8, screen_height-6-7, BLACK);
 draw_straigth_column(8+10*8+8, screen_height-20, 20, BLACK);
 add_zone_to_click_board(0, screen_height-20, 8+10*8+8, 20, CLICK_ZONE_BACK);

 //print down string
 print(down_string, 8+10*8+8+8, screen_height-6-7, BLACK);

 //draw "OPEN" and "SAVE" button
 if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_OPEN_AND_SAVE_BUTTON)!=PROGRAM_INTERFACE_FLAG_NO_OPEN_AND_SAVE_BUTTON) {
  draw_column = (screen_width-8-9*8-8);
  if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON)==PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON) {
   //draw "OPEN" button
   print("[F1] Open", draw_column+8, screen_height-6-7, BLACK);
   draw_straigth_column(draw_column, screen_height-20, 20, BLACK);
   add_zone_to_click_board(draw_column, screen_height-20, 8+9*8+8, 20, CLICK_ZONE_OPEN);
  }
  else {
   //draw "SAVE" button
   print("[F2] Save", draw_column+8, screen_height-6-7, BLACK);
   draw_straigth_column(draw_column, screen_height-20, 20, BLACK);
   add_zone_to_click_board(draw_column, screen_height-20, 8+9*8+8, 20, CLICK_ZONE_SAVE);
   draw_column -= (8+9*8+8);

   //draw "OPEN" button
   print("[F1] Open", draw_column+8, screen_height-6-7, BLACK);
   draw_straigth_column(draw_column, screen_height-20, 20, BLACK);
   add_zone_to_click_board(draw_column, screen_height-20, 8+9*8+8, 20, CLICK_ZONE_OPEN);
  }
 }
}

void draw_dialog_yes_no(byte_t *string) {
 dword_t num_of_chars = get_number_of_chars_in_ascii_string(string);
 dword_t dialog_width = ((num_of_chars*8)+32+2);
 if(dialog_width<200) {
  dialog_width = 200;
 }
 draw_full_square(screen_x_center-(dialog_width/2), screen_y_center-38, dialog_width, 2+16+7+16+2+5+7+5+16, 0xFF7000);
 draw_empty_square(screen_x_center-(dialog_width/2), screen_y_center-38, dialog_width, 2+16+7+16+2+5+7+5+16, BLACK);
 print(string, screen_x_center-(num_of_chars*4), screen_y_center-38+2+16, BLACK);
 draw_full_square(screen_x_center-10-80, screen_y_center-38+2+16+7+16, 80, 17, 0xFF9000);
 draw_empty_square(screen_x_center-10-80, screen_y_center-38+2+16+7+16, 80, 17, BLACK);
 print("Yes", screen_x_center-10-40-12, screen_y_center-38+2+16+7+16+1+4, BLACK);
 draw_full_square(screen_x_center+10, screen_y_center-38+2+16+7+16, 80, 17, 0xFF9000);
 draw_empty_square(screen_x_center+10, screen_y_center-38+2+16+7+16, 80, 17, BLACK);
 print("No", screen_x_center+10+40-8, screen_y_center-38+2+16+7+16+1+4, BLACK);
}

void draw_bottom_line_button(byte_t *string, dword_t click_zone) {
 dword_t number_of_chars = get_number_of_chars_in_ascii_string(string);
 dword_t x = (get_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN)+8+10*8+8);
 draw_straigth_column(x, screen_height-20, 20, BLACK);
 print(string, x+8, screen_height-6-7, BLACK);
 draw_straigth_column(x+8+number_of_chars*8+8, screen_height-20, 20, BLACK);
 add_zone_to_click_board(x, screen_height-20, 8+number_of_chars*8+8, 20, click_zone);
 set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, get_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN)+8+number_of_chars*8+8);
}

void draw_menu_list(byte_t *items_string, dword_t x, dword_t click_zone) {
 byte_t string[100];

 //count number of items
 dword_t number_of_items = 1, length_of_actual_item = 0, longest_item = 0, pointer = 0;
 while(items_string[pointer]!=0) {
  if(items_string[pointer]=='\n') {
   length_of_actual_item = 0;
   number_of_items++;
   pointer++;
   continue;
  }
  else {
   pointer++;
   length_of_actual_item+=8;
   if(length_of_actual_item>longest_item) {
    longest_item = length_of_actual_item;
   }
  }
 }
 if(length_of_actual_item>longest_item) {
  longest_item = length_of_actual_item;
 }
 longest_item += 16;

 //draw menu list
 
 list_item_line = (screen_height-20-number_of_items*20);
 list_item_column = x;
 draw_list_background(list_item_column, list_item_line, longest_item, number_of_items);
 for(dword_t i=0; i<number_of_items; i++) {
  for(dword_t j=0; j<100; j++) {
   string[j] = items_string[j];
   if(string[j]==0) {
    draw_list_item(string);
    add_zone_to_click_board(x, screen_height-20-number_of_items*20, longest_item, number_of_items*20, click_zone);
    redraw_part_of_screen(x, screen_height-20-number_of_items*20, longest_item, number_of_items*20);
    return;
   }
   else if(string[j]=='\n') {
    string[j]=0;
    draw_list_item(string);
    items_string+=(j+1);
    break;
   }
  }
 }
}