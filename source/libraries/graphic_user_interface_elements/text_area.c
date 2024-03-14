//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t create_text_area(dword_t type, dword_t length_in_chars, dword_t x, dword_t y, dword_t width, dword_t height) {
 dword_t text_area_memory = malloc(28*4);
 dword_t *text_area_info = (dword_t *) text_area_memory;

 text_area_info[TEXT_AREA_INFO_TYPE] = type;
 text_area_info[TEXT_AREA_INFO_MEMORY] = calloc((length_in_chars+1)*2); //allocate memory for text
 text_area_info[TEXT_AREA_INFO_NUM_OF_CHARS] = length_in_chars; //maximum number of chars in this text area
 text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE] = (text_area_info[TEXT_AREA_INFO_MEMORY] + length_in_chars*2); //last byte in text area memory
 text_area_info[TEXT_AREA_INFO_WIDTH] = width;
 text_area_info[TEXT_AREA_INFO_HEIGTH] = height;
 text_area_info[TEXT_AREA_INFO_REAL_WIDTH] = width;
 text_area_info[TEXT_AREA_INFO_REAL_HEIGTH] = height;
 text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN] = 0; //first column in pixels to show
 text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] = 0; //first line in pixels to show
 text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = text_area_info[TEXT_AREA_INFO_MEMORY]; //cursor position
 text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF; //cursor selected area pointer (from this point to cursor position is everything selected)
 text_area_info[TEXT_AREA_INFO_X] = x;
 text_area_info[TEXT_AREA_INFO_Y] = y;
 text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES] = 0;
 text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS] = 0;
 text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION] = 0;
 text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE] = 0;
 text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION] = 0;
 text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE] = 0;
 text_area_info[TEXT_AREA_INFO_FLAGS] = 0;
 text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
 text_area_info[TEXT_AREA_INFO_REDRAW_Y] = 0;
 text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = 0;
 text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 0;
 text_area_info[TEXT_AREA_INFO_CHANGES_LIST] = malloc(TEXT_AREA_MAX_NUMBER_OF_CHANGES_IN_LIST*8);
 text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER] = 0;
 text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER] = 0;
 
 return text_area_memory;
}

void delete_text_area(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 free(text_area_info[TEXT_AREA_INFO_MEMORY]);
 free(text_area_info[TEXT_AREA_INFO_CHANGES_LIST]);
 free(text_area_info_mem);
}

void draw_text_area(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 word_t *text_area_data = (word_t *) ((dword_t)text_area_info[TEXT_AREA_INFO_MEMORY]);
 dword_t char_drawing_line_up, char_drawing_line_down, char_drawing_column_left, char_drawing_column_right, background_color, chars_color;
 dword_t char_column, char_line, first_char_column, last_char_column, first_char_line, last_char_line;
 dword_t type = text_area_info[TEXT_AREA_INFO_TYPE];

 //draw background
 if(type==TEXT_AREA_NORMAL || type==TEXT_AREA_INPUT_LINE || type==TEXT_AREA_NUMBER_INPUT) {
  background_color = WHITE;
  chars_color = BLACK;
 }
 else if(type==TEXT_AREA_NORMAL_DARK) {
  background_color = BLACK;
  chars_color = WHITE;
 }
 if(type==TEXT_AREA_NORMAL || type==TEXT_AREA_NORMAL_DARK) {
  draw_full_square(text_area_info[TEXT_AREA_INFO_X], text_area_info[TEXT_AREA_INFO_Y], text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_HEIGTH], background_color);
 }
 else if(type==TEXT_AREA_INPUT_LINE || type==TEXT_AREA_NUMBER_INPUT) {
  draw_full_square(text_area_info[TEXT_AREA_INFO_X]+1, text_area_info[TEXT_AREA_INFO_Y]+1, text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_HEIGTH], background_color);
  draw_empty_square(text_area_info[TEXT_AREA_INFO_X], text_area_info[TEXT_AREA_INFO_Y], text_area_info[TEXT_AREA_INFO_WIDTH]+2, text_area_info[TEXT_AREA_INFO_HEIGTH]+2, chars_color);
  text_area_info[TEXT_AREA_INFO_X]++;
  text_area_info[TEXT_AREA_INFO_Y]++;
 }
 
 //set variables
 text_area_mouse_cursor_char_memory = text_area_info[TEXT_AREA_INFO_CURSOR_POSITION];
 char_drawing_line_up = 0;
 char_drawing_line_down = 10;
 char_drawing_column_left = 0;
 char_drawing_column_right = 8;
 char_line = text_area_info[TEXT_AREA_INFO_Y];
 char_column = text_area_info[TEXT_AREA_INFO_X];
 text_area_info[TEXT_AREA_INFO_WIDTH] = text_area_info[TEXT_AREA_INFO_REAL_WIDTH];
 text_area_info[TEXT_AREA_INFO_HEIGTH] = text_area_info[TEXT_AREA_INFO_REAL_HEIGTH];
 if(text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10>text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]) { //vertical scrollbar is present
  text_area_info[TEXT_AREA_INFO_WIDTH] = (text_area_info[TEXT_AREA_INFO_REAL_WIDTH]-10);
 }
 if(text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8>text_area_info[TEXT_AREA_INFO_REAL_WIDTH]) { //horizontal scrollbar is present
  text_area_info[TEXT_AREA_INFO_HEIGTH] = (text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]-10);
 }
 
 //draw text
 while(*text_area_data!=0) {
  //test if char is on screen
  if(char_drawing_line_down<=text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]) {
   goto next_char;
  }
  if(char_drawing_line_up>=(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH])) {
   break; //char is on line after visible part, so it mean we draw everything
  }
  if(char_drawing_column_right<=text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]) {
   goto next_char;
  }
  if(char_drawing_column_left>=(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]+text_area_info[TEXT_AREA_INFO_WIDTH])) {
   goto next_char;
  }
  
  //calculate char size
  if(char_drawing_column_left<text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]) {
   char_column = text_area_info[TEXT_AREA_INFO_X];
   first_char_column = (text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]-char_drawing_column_left);
   last_char_column = 8;
  }
  else if(char_drawing_column_right>(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]+text_area_info[TEXT_AREA_INFO_WIDTH])) {
   char_column = (text_area_info[TEXT_AREA_INFO_X]+(char_drawing_column_left-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]));
   first_char_column = 0;
   last_char_column = ((text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]+text_area_info[TEXT_AREA_INFO_WIDTH])-char_drawing_column_left);
  }
  else {
   char_column = (text_area_info[TEXT_AREA_INFO_X]+(char_drawing_column_left-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]));
   first_char_column = 0;
   last_char_column = 8;
  }
  
  if(char_drawing_line_up<text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]) {
   char_line = (text_area_info[TEXT_AREA_INFO_Y]);
   first_char_line = (text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]-(char_drawing_line_up+1));
   last_char_line = 8;
  }
  else if(char_drawing_line_down>(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH])) {
   char_line = (text_area_info[TEXT_AREA_INFO_Y]+(char_drawing_line_up-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]))+1;
   first_char_line = 0;
   last_char_line = ((text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH])-(char_drawing_line_up+1));
  }
  else {
   char_line = (text_area_info[TEXT_AREA_INFO_Y]+(char_drawing_line_up-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]))+1;
   first_char_line = 0;
   last_char_line = 8;
  }
  
  //check mouse cursor position
  if(mouse_cursor_y>=char_line && mouse_cursor_y<(char_line+10)) {
   if(mouse_cursor_x>=char_column) {
    if(*text_area_data==0xA) {
     text_area_mouse_cursor_char_memory = (dword_t)text_area_data;
    }
    else if(first_char_column==0 && (char_column+8)<screen_width) {
     if(mouse_cursor_x<(char_column+4)) {
      text_area_mouse_cursor_char_memory = (dword_t)text_area_data;
     }
     else if(mouse_cursor_x<(char_column+8)) {
      text_area_mouse_cursor_char_memory = (((dword_t)text_area_data)+2);
     }
    }
    else if(mouse_cursor_x<(char_column+8)) {
     text_area_mouse_cursor_char_memory = (dword_t)text_area_data;
    }
   }
  }
  
  //draw background of char
  if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]!=0xFFFFFFFF && text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]!=text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    if((dword_t)text_area_data>=text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] && (dword_t)text_area_data<text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) {
     draw_full_square(char_column, char_line-1, 8, (last_char_line-first_char_line)+2, 0x00A0FF);
    }
   }
   else {
    if((dword_t)text_area_data>=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] && (dword_t)text_area_data<text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
     draw_full_square(char_column, char_line-1, 8, (last_char_line-first_char_line)+2, 0x00A0FF);
    }
   }
  }
  
  //draw char
  if(first_char_line<8) {
   draw_part_of_char(*text_area_data, char_column, char_line, first_char_column, last_char_column, first_char_line, last_char_line, chars_color);
   
   //draw cursor
   if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] && char_drawing_column_left>=text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN] && (last_char_line-first_char_line)<=10) {
    draw_line(char_column, char_line-1, char_column, char_line+(last_char_line-first_char_line), chars_color);
   }
  }
  
  //move to next char
  next_char:
  if(*text_area_data==0xA) {
   char_drawing_line_up += 10;
   char_drawing_line_down += 10;
   char_drawing_column_left = 0;
   char_drawing_column_right = 8;
  }
  else {
   char_drawing_column_left += 8;
   char_drawing_column_right += 8;
  }
  text_area_data++;
 }
 
 //draw cursor if is on last char
 if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] && char_drawing_column_left>=text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]) {
  if(char_drawing_column_left<text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]) {
   char_column = text_area_info[TEXT_AREA_INFO_X];
  }
  else if(char_drawing_column_right>(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]+text_area_info[TEXT_AREA_INFO_WIDTH])) {
   char_column = (text_area_info[TEXT_AREA_INFO_X]+(char_drawing_column_left-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]));
  }
  else {
   char_column = (text_area_info[TEXT_AREA_INFO_X]+(char_drawing_column_left-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]));
  }
  
  if(char_drawing_line_up<text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]) {
   char_line = (text_area_info[TEXT_AREA_INFO_Y]+1);
   first_char_line = (text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]-char_drawing_line_up);
   last_char_line = 8;
  }
  else if(char_drawing_line_down>(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH])) {
   char_line = (text_area_info[TEXT_AREA_INFO_Y]+(char_drawing_line_up-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]))+1;
   first_char_line = 0;
   last_char_line = ((text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH])-char_drawing_line_up);
  }
  else {
   char_line = (text_area_info[TEXT_AREA_INFO_Y]+(char_drawing_line_up-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]))+1;
   first_char_line = 0;
   last_char_line = 8;
  }
 
  if((last_char_line-first_char_line)<=10) {
   draw_line(char_column, char_line-1, char_column, char_line+(last_char_line-first_char_line), chars_color);
  }
 }
 
 //check if mouse cursor position is on last char
 if((mouse_cursor_y>(char_line+10) || (mouse_cursor_y>char_line && mouse_cursor_x>char_column))) {
  text_area_mouse_cursor_char_memory = (dword_t)text_area_data;
 }

 //draw scrollbars
 text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE] = 0;
 if(text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10>text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]) { //vertical scrollbar is present
  text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE] = calculate_scrollbar_rider_size(text_area_info[TEXT_AREA_INFO_REAL_HEIGTH], text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10, text_area_info[TEXT_AREA_INFO_HEIGTH]);
  text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION] = calculate_scrollbar_rider_position(text_area_info[TEXT_AREA_INFO_REAL_HEIGTH], text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE], text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10, text_area_info[TEXT_AREA_INFO_HEIGTH], text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]);
  draw_vertical_scrollbar(text_area_info[TEXT_AREA_INFO_X]+text_area_info[TEXT_AREA_INFO_REAL_WIDTH]-10, text_area_info[TEXT_AREA_INFO_Y], text_area_info[TEXT_AREA_INFO_REAL_HEIGTH], text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION], text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE]);
 }
 text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE] = 0;
 if(text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8>text_area_info[TEXT_AREA_INFO_REAL_WIDTH]) { //horizontal scrollbar is present
  text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE] = calculate_scrollbar_rider_size(text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8, text_area_info[TEXT_AREA_INFO_WIDTH]);
  text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION] = calculate_scrollbar_rider_position(text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE], text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8, text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]);
  draw_horizontal_scrollbar(text_area_info[TEXT_AREA_INFO_X], text_area_info[TEXT_AREA_INFO_Y]+text_area_info[TEXT_AREA_INFO_HEIGTH], text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION], text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE]);
 }

 //move back to right place
 if(type==TEXT_AREA_INPUT_LINE || type==TEXT_AREA_NUMBER_INPUT) {
  text_area_info[TEXT_AREA_INFO_X]--;
  text_area_info[TEXT_AREA_INFO_Y]--;
 }
}

void redraw_text_area(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;

 //redraw input line
 dword_t type = text_area_info[TEXT_AREA_INFO_TYPE];
 if(type==TEXT_AREA_INPUT_LINE || type==TEXT_AREA_NUMBER_INPUT) {
  redraw_part_of_screen(text_area_info[TEXT_AREA_INFO_X]+1, text_area_info[TEXT_AREA_INFO_Y]+1, text_area_info[TEXT_AREA_INFO_REAL_WIDTH], text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]);
  return;
 }

 //redraw text area
 if(text_area_info[TEXT_AREA_INFO_REDRAW_X]==0xFFFFFFFF) { //redraw full text area
  redraw_part_of_screen(text_area_info[TEXT_AREA_INFO_X], text_area_info[TEXT_AREA_INFO_Y], text_area_info[TEXT_AREA_INFO_REAL_WIDTH], text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]);
 }
 else { //redraw part of text area
  redraw_part_of_screen(text_area_info[TEXT_AREA_INFO_REDRAW_X], text_area_info[TEXT_AREA_INFO_REDRAW_Y], text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH], text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH]);
 }
}

void text_area_set_show_line_and_column(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;

 text_area_calculate_cursor_position(text_area_info_mem);
 if(text_area_absoulte_cursor_line<text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]) {
  text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]=text_area_absoulte_cursor_line;
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
 else if(text_area_absoulte_cursor_line>(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH]-10)) {
  text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]=(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_HEIGTH]+10);
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
 if(text_area_absoulte_cursor_column<text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]) {
  text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]=text_area_absoulte_cursor_column;
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
 else if(text_area_absoulte_cursor_column>(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]+text_area_info[TEXT_AREA_INFO_WIDTH]-1)) {
  text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]=(text_area_absoulte_cursor_column-text_area_info[TEXT_AREA_INFO_WIDTH]+8);
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
 if((text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10>text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]) && (text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]+text_area_info[TEXT_AREA_INFO_HEIGTH])>text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10) {
  text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] = (text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10-text_area_info[TEXT_AREA_INFO_HEIGTH]);
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
 if((text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8>text_area_info[TEXT_AREA_INFO_REAL_WIDTH]) && (text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN]+text_area_info[TEXT_AREA_INFO_WIDTH])>text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8) {
  text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN] = (text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8-text_area_info[TEXT_AREA_INFO_WIDTH]);
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
}

void text_area_calculate_cursor_position(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 word_t *text_area_data = (word_t *) text_area_info[TEXT_AREA_INFO_MEMORY];
 dword_t line, column, first_char_line, last_char_line, num_of_lines, num_of_columns, skip_chars;
 
 text_area_absoulte_cursor_line = 0;
 text_area_absoulte_cursor_column = 0;

 while((dword_t)text_area_data<=text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]) {
  if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
   //we find cursor
   return;
  }
  else if(*text_area_data==0xA) {
   text_area_absoulte_cursor_column = 0;
   text_area_absoulte_cursor_line += 10;
   text_area_data++;
   continue;
  }
  else if(*text_area_data==0) { //cursor was not found in text area
   text_area_absoulte_cursor_column = 0xFFFFFFFF;
   text_area_absoulte_cursor_line = 0xFFFFFFFF;
  }
  
  text_area_absoulte_cursor_column += 8;
  text_area_data++;
 }
 
}

void text_area_calculate_number_of_lines_and_columns(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) (text_area_info_mem);
 word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
 dword_t number_of_lines = 1, number_of_columns = 0, number_of_columns_in_biggest_line = 0;

 while(*text_area_data!=0) {
  if(*text_area_data=='\n') {
   number_of_lines++;
   if(number_of_columns_in_biggest_line<number_of_columns) {
    number_of_columns_in_biggest_line = number_of_columns;
   }
   number_of_columns = 0;
  }
  else {
   number_of_columns++;
  }

  text_area_data++;
 }
 if(number_of_columns_in_biggest_line<number_of_columns) {
  number_of_columns_in_biggest_line = number_of_columns;
 }

 number_of_columns_in_biggest_line++; //add space for cursor at end of line
 text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES] = number_of_lines;
 text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS] = number_of_columns_in_biggest_line;
}

void text_area_keyboard_event(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 word_t *text_area_data = (word_t *) ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
 dword_t count_chars=0;
 dword_t type = text_area_info[TEXT_AREA_INFO_TYPE];

 if(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]==0xFFFFFFFF) {
  return; //this area is not selected
 }
 
 text_area_change_type = TEXT_AREA_NO_CHANGE;
 text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 
 if((keyboard_pressed_control_keys & KEYBOARD_CTRL)==KEYBOARD_CTRL) {
  if(keyboard_code_of_pressed_key==KEY_C) { //copy text
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]!=0xFFFFFFFF && text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]!=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) { //only if some text is selected
    //release previous copied text
    if(text_area_copy_memory!=0) {
     free(text_area_copy_memory);
    }

    //copy text
    if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
     text_area_copy_memory_length = (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
     text_area_copy_memory = malloc(text_area_copy_memory_length);
     copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_copy_memory), text_area_copy_memory_length);
    }
    else {
     text_area_copy_memory_length = (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]);
     text_area_copy_memory = malloc(text_area_copy_memory_length);
     copy_memory((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_copy_memory), text_area_copy_memory_length);
    }

    //calculate actual number of lines and columns
    text_area_calculate_number_of_lines_and_columns(text_area_info_mem);
   }
  }
  else if(keyboard_code_of_pressed_key==KEY_V) { //insert text
   //TODO: do not insert if is text area full of text

   //number of copied chars
   dword_t text_area_length_of_copied_chars = text_area_copy_memory_length;
   if(text_area_info[TEXT_AREA_INFO_TYPE]==TEXT_AREA_NUMBER_INPUT) {
    text_area_length_of_copied_chars = 0;

    //count how many numbers are in copied text
    word_t *copy_memory = (word_t *) (text_area_copy_memory);
    while(*copy_memory>='0' && *copy_memory<='9' && text_area_length_of_copied_chars<text_area_copy_memory_length) {
     text_area_length_of_copied_chars+=2;
     copy_memory++;
    }
   }
   
   if(text_area_copy_memory!=0 && text_area_length_of_copied_chars!=0 && (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]<=(text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_length_of_copied_chars))) {
    //shift chars
    copy_memory_back((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_length_of_copied_chars-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_length_of_copied_chars));
   
    //add chars
    copy_memory(((dword_t)text_area_copy_memory), ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), text_area_length_of_copied_chars);

    //add chars to change list
    text_area_change_command = TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0;
    if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
     text_area_change_command = TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1;
     text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
    }
    else {
     text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
    }
    word_t *text_area_copy_memory_data = (word_t *) (text_area_copy_memory);
    for(dword_t i=0, cursor_position = text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]; i<(text_area_length_of_copied_chars/2); i++) {
     text_area_add_change_to_list(text_area_info_mem, text_area_change_command, *text_area_copy_memory_data, cursor_position);
     text_area_copy_memory_data++;
     cursor_position+=2;
    }
  
    //move cursor
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=text_area_length_of_copied_chars;
    text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
  
    //set change
    text_area_change_type = TEXT_AREA_TEXT_CHANGE;
    text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;

    //calculate actual number of lines and columns
    text_area_calculate_number_of_lines_and_columns(text_area_info_mem);
   }
  }
  else if(keyboard_code_of_pressed_key==KEY_A) {
   //select from start
   text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=text_area_info[TEXT_AREA_INFO_MEMORY];
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_info[TEXT_AREA_INFO_MEMORY];

   //select all chars
   text_area_data = (word_t *) ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   while(*text_area_data!=0) {
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
    text_area_data++;
   }
  }
  else if(keyboard_code_of_pressed_key==KEY_Z) {
   //perform undo
   text_area_undo(text_area_info_mem);

   //set change
   text_area_change_type = TEXT_AREA_TEXT_CHANGE;
   text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;
  }
  else if(keyboard_code_of_pressed_key==KEY_Y) {
   //perform undo
   text_area_redo(text_area_info_mem);

   //set change
   text_area_change_type = TEXT_AREA_TEXT_CHANGE;
   text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;
  }
 }
 else if(keyboard_code_of_pressed_key==KEY_LEFT) {
  //move cursor
  if(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]>text_area_info[TEXT_AREA_INFO_MEMORY]) {
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-=2; //move cursor

   //find actual cursor position
   text_area_calculate_cursor_position(text_area_info_mem);

   //set what to redraw from screen
   if(text_area_data[-1]=='\n') {
    //redraw two lines where was and where is cursor now
    text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
    text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
    text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
    text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 20;
   }
   else {
    //redraw only line where is cursor
    text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
    text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
    text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
    text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 10;
   }
  }
  
  //disable selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 else if(keyboard_code_of_pressed_key==KEY_RIGHT) {
  //find actual cursor position
  text_area_calculate_cursor_position(text_area_info_mem);
  
  //set what to redraw from screen
  if(text_area_data[0]=='\n') {
   //redraw two lines where was and where is cursor now
   text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
   text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
   text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
   text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 20;
  }
  else {
   //redraw only line where is cursor
   text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
   text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
   text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
   text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 10;
  }

  //move cursor
  if(text_area_data[TEXT_AREA_INFO_MEMORY]!=0) {
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
  }
  
  //hide any selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 else if(keyboard_code_of_pressed_key==KEY_UP) {
  if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_MEMORY]) {
   goto move_text_area_to_cursor;
  }
  
  //compute number of chars to start of line
  count_chars = 0;
  if((dword_t)text_area_data!=text_area_info[TEXT_AREA_INFO_MEMORY]) {
   text_area_data--;
   
   while(*text_area_data!=0xA) {
    if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_MEMORY]) {
     goto key_up_move_area;
    }
    count_chars++;
    text_area_data--;
   }
  }
  
  //move cursor to start of next line
  if((dword_t)text_area_data!=text_area_info[TEXT_AREA_INFO_MEMORY]) {
   text_area_data--;
   
   while(*text_area_data!=0xA) {
    if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_MEMORY]) {
     text_area_data--;
     break;
    }
    text_area_data--;
   }
   
   text_area_data++;
  }
  
  //move cursor to right position
  while(count_chars>0) {
   if(*text_area_data==0xA || *text_area_data==0) {
    break;
   }
   text_area_data++;
   count_chars--;
  }
 
  key_up_move_area:
  //find actual cursor position
  text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=(dword_t)text_area_data;
  text_area_calculate_cursor_position(text_area_info_mem);

  //redraw two lines where was and where is cursor now
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
  text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
  text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
  text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 20;

  //hide any selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 else if(keyboard_code_of_pressed_key==KEY_DOWN) {
  if(*text_area_data==0) {
   goto move_text_area_to_cursor;
  }

  //find actual cursor position
  text_area_calculate_cursor_position(text_area_info_mem);

  //redraw two lines where was and will be cursor
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
  text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
  text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
  text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 20;
  
  //compute number of chars to start of line
  count_chars = 0;
  if((dword_t)text_area_data!=text_area_info[TEXT_AREA_INFO_MEMORY]) {
   text_area_data--;
   
   while(*text_area_data!=0xA) {
    count_chars++;
    if((dword_t)text_area_data==text_area_info[TEXT_AREA_INFO_MEMORY]) {
     break;
    }
    text_area_data--;
   }
  }
  
  //move cursor to start of next line
  text_area_data = (word_t *) ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
  while(*text_area_data!=0xA) {
   if(*text_area_data==0) {
    goto key_down_move_area;
   }
   text_area_data++;
  }
  text_area_data++;
  
  //move cursor to right position
  while(count_chars>0) {
   if(*text_area_data==0xA || *text_area_data==0) {
    break;
   }
   text_area_data++;
   count_chars--;
  }
  
  key_down_move_area:
  text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=(dword_t)text_area_data;

  //hide any selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 else if(keyboard_code_of_pressed_key==KEY_BACKSPACE) {
  if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]==0xFFFFFFFF || text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]==text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) {
   if(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]>text_area_info[TEXT_AREA_INFO_MEMORY]) {
    //move cursor
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-=2;

    //add change
    text_area_add_change_to_list(text_area_info_mem, TEXT_AREA_CHANGE_REMOVE_CHAR_BY_BACKSPACE, text_area_data[-1], text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   
    //delete one char
    copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+2), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), 2);
    
    //set change
    text_area_change_type = TEXT_AREA_TEXT_CHANGE;
    text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;

    //calculate actual number of lines and columns
    text_area_calculate_number_of_lines_and_columns(text_area_info_mem);
   }
  }
  else {
   //set change
   text_area_change_type = TEXT_AREA_TEXT_CHANGE;
   text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;
  
   goto key_delete_whole_selected_area;
  }
  
  //hide any selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 else if(keyboard_code_of_pressed_key==KEY_DELETE) {
  //set change
  text_area_change_type = TEXT_AREA_TEXT_CHANGE;
  text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;
  
  if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]==0xFFFFFFFF || text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]==text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) {
   //add change
   text_area_add_change_to_list(text_area_info_mem, TEXT_AREA_CHANGE_REMOVE_CHAR_BY_DELETE, text_area_data[0], text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   
   //delete one char
   copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+2), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
   clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), 2);

   //calculate actual number of lines and columns
   text_area_calculate_number_of_lines_and_columns(text_area_info_mem);
  }
  else {
   key_delete_whole_selected_area:
   //add deleted area data to change list
   text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0;
   if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
    text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1;
    text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
   }
   else {
    text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
   }
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2);
    for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2); i<((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])/2); i++) {
     text_area_add_change_to_list(text_area_info_mem, text_area_change_command, *text_area_data, cursor_position);
     text_area_data--;
     cursor_position-=2;
    }
   }
   else {
    text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2);
    for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2); i<((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])/2); i++) {
     text_area_add_change_to_list(text_area_info_mem, text_area_change_command, *text_area_data, cursor_position);
     text_area_data--;
     cursor_position-=2;
    }
   }

   //delete whole selected area
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    copy_memory((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]+2));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
   }
   else {
    copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+2));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]));
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER];
   }

   //calculate actual number of lines and columns
   text_area_calculate_number_of_lines_and_columns(text_area_info_mem);
  }
  
  //hide any selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 else if(keyboard_unicode_value_of_pressed_key!=0) {
  if(type==TEXT_AREA_NUMBER_INPUT && (keyboard_unicode_value_of_pressed_key<'0' || keyboard_unicode_value_of_pressed_key>'9')) { //in number input accept only numbers
   goto move_text_area_to_cursor;
  }
  if(type==TEXT_AREA_INPUT_LINE && keyboard_unicode_value_of_pressed_key==0xA) { //in one line input we can not use enter
   goto move_text_area_to_cursor;
  }
  
  dword_t keyboard_unicode_value_of_pressed_key_save = keyboard_unicode_value_of_pressed_key; //we need to save key value, because if user releases key before we finish copying memory, we will write 0 what will broke text area
  
  //delete selected area
  if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]!=0xFFFFFFFF && text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]!=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) {
   //add deleted area data to change list
   text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0;
   if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
    text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1;
    text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
   }
   else {
    text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
   }
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2);
    for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2); i<((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])/2); i++) {
     text_area_add_change_to_list(text_area_info_mem, text_area_change_command, *text_area_data, cursor_position);
     text_area_data--;
     cursor_position-=2;
    }
   }
   else {
    text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2);
    for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2); i<((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])/2); i++) {
     text_area_add_change_to_list(text_area_info_mem, text_area_change_command, *text_area_data, cursor_position);
     text_area_data--;
     cursor_position-=2;
    }
   }
   text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   
   //delete chars from text area
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    copy_memory((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]+2));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
   }
   else {
    copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+2));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]));
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER];
    text_area_data = (word_t *) ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   }
  }
  
  //do not insert if is text area full of text
  word_t *text_area_last_char_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2);
  if(*text_area_last_char_data!=0) {
   goto move_text_area_to_cursor;
  }
  
  //shift chars
  copy_memory_back((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-4), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2));
  
  //add char
  text_area_data[0] = keyboard_unicode_value_of_pressed_key_save;
  if(keyboard_unicode_value_of_pressed_key_save!='\n') {
   //find actual cursor position
   text_area_calculate_cursor_position(text_area_info_mem);

   //redraw line where is cursor
   text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0;
   text_area_info[TEXT_AREA_INFO_REDRAW_Y] = (text_area_info[TEXT_AREA_INFO_Y]+(text_area_absoulte_cursor_line-text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]));
   text_area_info[TEXT_AREA_INFO_REDRAW_WIDTH] = text_area_info[TEXT_AREA_INFO_WIDTH];
   text_area_info[TEXT_AREA_INFO_REDRAW_HEIGTH] = 10;

   //count number of chars in line
   while(text_area_data[-1]!='\n' && (dword_t)text_area_data>text_area_info[TEXT_AREA_INFO_MEMORY]) {
    text_area_data--;
   }
   while(text_area_data[0]!='\n' && text_area_data[0]!=0) {
    count_chars++;
    text_area_data++;
   }
   count_chars++; //add space for last char
   if(count_chars>text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]) {
    text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS] = count_chars; //this is now biggest line
   }
  }
  else {
   text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]++; //new line was inserted
  }

  //add change
  text_area_add_change_to_list(text_area_info_mem, TEXT_AREA_CHANGE_ADD_CHAR, keyboard_unicode_value_of_pressed_key_save, text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
  
  //move cursor
  text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
  
  //set change
  text_area_change_type = TEXT_AREA_TEXT_CHANGE;
  text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;
  
  //hide any selected area
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
 }
 
 move_text_area_to_cursor:
 text_area_set_show_line_and_column(text_area_info_mem);
}

void text_area_mouse_event(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 
 //change cursor position
 if(mouse_click_button_state==MOUSE_CLICK) {
  draw_text_area(text_area_info_mem); //get mouse cursor position
  word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
  if(text_area_mouse_cursor_char_memory==text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] && text_area_data[0]!=0) { //double click on word
   while((dword_t)text_area_data>text_area_info[TEXT_AREA_INFO_MEMORY] && text_area_data[-1]!=' ') {
    text_area_data--;
   }
   text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = ((dword_t)text_area_data);

   text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   while((dword_t)text_area_data<text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE] && text_area_data[0]!=' ' && text_area_data[0]!=0) {
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
    text_area_data++;
   }
  }
  else if(text_area_mouse_cursor_char_memory!=0xFFFFFFFF) {
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = text_area_mouse_cursor_char_memory; 
   text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = text_area_mouse_cursor_char_memory;
  }
  else {
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = text_area_info[TEXT_AREA_INFO_MEMORY]; 
   text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = text_area_info[TEXT_AREA_INFO_MEMORY];
  }
  text_area_set_show_line_and_column(text_area_info_mem);
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }
 else if(mouse_click_button_state==MOUSE_DRAG) {
  draw_text_area(text_area_info_mem); //get mouse cursor position
  text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = text_area_mouse_cursor_char_memory;
  text_area_set_show_line_and_column(text_area_info_mem);
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area
 }

 //mouse wheel
 if(mouse_wheel!=0) {
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF; //redraw whole text area

  if(mouse_wheel<0x80000000) {
   text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] -= 30;
   if(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]>0x80000000) {
    text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] = 0;
   }
  }
  else if(text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10>text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]) {
   text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] += 30;
   if(text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE]>(text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10-text_area_info[TEXT_AREA_INFO_HEIGTH])) {
    text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] = (text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10-text_area_info[TEXT_AREA_INFO_HEIGTH]);
   }
  }
 }
}

void text_area_disable_cursor(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;

 text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = 0xFFFFFFFF;
 text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF;
}

void text_area_move_cursor_to_start(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;

 text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = text_area_info[TEXT_AREA_INFO_MEMORY];
 text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF;
}

void text_area_add_change_to_list(dword_t text_area_info_mem, dword_t change_type, dword_t unicode_char, dword_t cursor_position) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 dword_t *text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]*8);

 if(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]<TEXT_AREA_MAX_NUMBER_OF_CHANGES_IN_LIST) {
  text_area_change_list[0] = (change_type | (unicode_char<<16));
  text_area_change_list[1] = cursor_position;
  text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]++;
 }
 else {
  //TODO: do not erase all changes
  clear_memory(text_area_info[TEXT_AREA_INFO_CHANGES_LIST], TEXT_AREA_MAX_NUMBER_OF_CHANGES_IN_LIST*8);
  text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER] = 0;
  text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]);
  text_area_change_list[0] = (change_type | (unicode_char<<16));
  text_area_change_list[1] = cursor_position;

  //TODO: something faster than this:
  // copy_memory(text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+8, text_area_info[TEXT_AREA_INFO_CHANGES_LIST], (TEXT_AREA_MAX_NUMBER_OF_CHANGES_IN_LIST-1)*8);
  // text_area_change_list[-2] = (change_type | (unicode_char<<16));
  // text_area_change_list[-1] = cursor_position;
 }

 text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER]=text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER];
}

void text_area_undo(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 if(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]>0) {
  dword_t *text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]-1)*8);
  dword_t change_type = (text_area_change_list[0] & 0xFFFF), cursor_position = text_area_change_list[1], how_many_entries = 0;

  //count how many chars should we insert/remove at one request
  if(change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1 || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1) {
   while((text_area_change_list[0] & 0xFFFF)==change_type) {
    how_many_entries++; //count entry
    if((dword_t)text_area_change_list==text_area_info[TEXT_AREA_INFO_CHANGES_LIST]) {
     break; //we are at start
    }
    text_area_change_list-=2; //previous entry
   }
  }
  else if((text_area_change_list[0]>>16)==' ') {
   how_many_entries++; //one entry with space
  }
  else {
   while((text_area_change_list[0] & 0xFFFF)==change_type && (text_area_change_list[1]==cursor_position) && (text_area_change_list[0]>>16)!=' ') {
    how_many_entries++; //count entry
    if((dword_t)text_area_change_list==text_area_info[TEXT_AREA_INFO_CHANGES_LIST]) {
     break; //we are at start
    }
    text_area_change_list-=2; //previous entry
    if(change_type==TEXT_AREA_CHANGE_ADD_CHAR) {
     cursor_position-=2;
    }
    else {
     cursor_position+=2;
    }
   }
  }

  //perform undo
  if(change_type==TEXT_AREA_CHANGE_ADD_CHAR || change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1) {
   //move cursor
   text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]-how_many_entries)*8);
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_change_list[1];

   //remove chars
   copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+how_many_entries*2), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
   clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-how_many_entries*2), how_many_entries*2);
   text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]-=how_many_entries;
  }
  else if(change_type==TEXT_AREA_CHANGE_REMOVE_CHAR_BY_BACKSPACE || change_type==TEXT_AREA_CHANGE_REMOVE_CHAR_BY_DELETE || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1) {
   //move cursor
   text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]-1)*8);
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_change_list[1];
   
   //shift memory
   copy_memory_back((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-how_many_entries*2-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-how_many_entries*2));

   //add chars
   word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   for(dword_t i=0; i<how_many_entries; i++) {
    *text_area_data = (text_area_change_list[0]>>16);
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
    text_area_data++;
    text_area_change_list-=2;
   }
   text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]-=how_many_entries;

   //move cursor
   if(change_type==TEXT_AREA_CHANGE_REMOVE_CHAR_BY_DELETE) {
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-=2;
   }
  }

  //calculate actual number of lines and columns
  text_area_calculate_number_of_lines_and_columns(text_area_info_mem);

  //redraw whole area
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF;

  //unselect if something was selected
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF;
 }
}

void text_area_redo(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) text_area_info_mem;
 if(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]<text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER]) {
  dword_t *text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]*8);
  dword_t change_type = (text_area_change_list[0] & 0xFFFF), cursor_position = text_area_change_list[1], how_many_entries = 0;

  //count how many chars should we insert/remove at one request
  if(change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1 || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1) {
   while((text_area_change_list[0] & 0xFFFF)==change_type) {
    how_many_entries++; //count entry
    text_area_change_list+=2; //next entry
    if((dword_t)text_area_change_list==(text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER]*8)) {
     break; //we are at end
    }
   }
  }
  else if((text_area_change_list[0]>>16)==' ') {
   how_many_entries++; //one entry with space
  }
  else {
   while((text_area_change_list[0] & 0xFFFF)==change_type && (text_area_change_list[1]==cursor_position) && (text_area_change_list[0]>>16)!=' ') {
    how_many_entries++; //count entry
    text_area_change_list+=2; //next entry
    if(change_type==TEXT_AREA_CHANGE_ADD_CHAR) {
     cursor_position+=2;
    }
    else {
     cursor_position-=2;
    }
    if((dword_t)text_area_change_list==(text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER]*8)) {
     break; //we are at end
    }
   }
  }

  //perform redo
  if(change_type==TEXT_AREA_CHANGE_ADD_CHAR || change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1) {
   //move cursor
   text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]*8);
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_change_list[1];
   
   //shift memory
   copy_memory_back((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-how_many_entries*2-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-how_many_entries*2));

   //add chars
   word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   for(dword_t i=0; i<how_many_entries; i++) {
    *text_area_data = (text_area_change_list[0]>>16);
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
    text_area_data++;
    text_area_change_list+=2;
   }
   text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]+=how_many_entries;
  }
  else if(change_type==TEXT_AREA_CHANGE_REMOVE_CHAR_BY_BACKSPACE || change_type==TEXT_AREA_CHANGE_REMOVE_CHAR_BY_DELETE || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0 || change_type==TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1) {
   //move cursor
   text_area_change_list = (dword_t *) (text_area_info[TEXT_AREA_INFO_CHANGES_LIST]+(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]+(how_many_entries-1))*8);
   text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_change_list[1];

   //remove chars
   copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+how_many_entries*2), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
   clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-how_many_entries*2), how_many_entries*2);
   text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]+=how_many_entries;
  }

  //calculate actual number of lines and columns
  text_area_calculate_number_of_lines_and_columns(text_area_info_mem);

  //redraw whole area
  text_area_info[TEXT_AREA_INFO_REDRAW_X] = 0xFFFFFFFF;

  //unselect if something was selected
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF;
 }
}

dword_t text_area_return_written_number_in_dword(dword_t text_area_info_mem) {
 dword_t *text_area_info = (dword_t *) (text_area_info_mem);
 return convert_word_string_to_number(text_area_info[TEXT_AREA_INFO_MEMORY]);
}