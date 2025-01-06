//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t system_call(dword_t call, dword_t args[]) {
 extern void kill_binary_program(void);

 //this should be first call of program
 if(call == NUMBER_OF_CALL_SET_PROGRAM_MODE) {
  //check version of BleskOS System Calls used in program
  if(args[1]<0x20241102) {
   error_window("FATAL ERROR: program is using old unsupported calls");
   kill_binary_program(); //this will not return here, but to where was binary program called from
  }
  else if(args[1]>0x20241102) {
   error_window("FATAL ERROR: program is using calls that are not implemented, update BleskOS");
   kill_binary_program(); //this will not return here, but to where was binary program called from
  }

  //set program mode
  if(args[0] == PROGRAM_TEXT_MODE) {
   binary_program_mode = PROGRAM_TEXT_MODE;

   system_call_reset_text_mode_screen();
  }
  else {
   error_window("FATAL ERROR: program is running in mode not supported by this version of BleskOS");
   kill_binary_program(); //this will not return here, but to where was binary program called from
  }

  //reset variables
  pressed_key_value = 0;
  pressed_key_unicode_value = 0;
  system_calls_is_execution_in_repeating_method = STATUS_FALSE;

  return SYSTEM_CALL_SUCCESS;
 }

 //system_call called without setting program mode, kill program immediately
 if(binary_program_mode == PROGRAM_NO_MODE) {
  error_window("FATAL ERROR: system_call invoked without setting program mode");
  kill_binary_program(); //this will not return here, but to where was binary program called from
 }

 //call proper method
 if(binary_program_mode == PROGRAM_TEXT_MODE) {
  //whole screen calls
  if(call == NUMBER_OF_CALL_RESET_SCREEN) {
   return system_call_reset_text_mode_screen();
  }
  else if(call == NUMBER_OF_CALL_CLEAR_SCREEN) {
   return system_call_clear_text_mode_screen(args[0], args[1]);
  }
  else if(call == NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_COLUMNS) {
   return system_call_text_mode_get_number_of_columns();
  }
  else if(call == NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_LINES) {
   return system_call_text_mode_get_number_of_lines();
  }
  else if(call == NUMBER_OF_CALL_HIDE_CHANGES_ON_SCREEN) {
   return system_call_text_mode_hide_changes_on_screen();
  }
  else if(call == NUMBER_OF_CALL_REDRAW_SCREEN) {
   return system_call_text_mode_redraw_screen();
  }
  else if(call == NUMBER_OF_CALL_REDRAW_PART_OF_SCREEN) {
   return system_call_text_mode_redraw_part_of_screen(args[0], args[1], args[2], args[3]);
  }
  else if(call == NUMBER_OF_CALL_SHOW_CHANGES_ON_SCREEN) {
   return system_call_text_mode_show_changes_on_screen();
  }

  //cursor calls
  else if(call == NUMBER_OF_CALL_SHOW_CURSOR) {
   return system_call_text_mode_show_cursor();
  }
  else if(call == NUMBER_OF_CALL_HIDE_CURSOR) {
   return system_call_text_mode_hide_cursor();
  }
  else if(call == NUMBER_OF_CALL_MOVE_CURSOR_TO_COLUMN) {
   return system_call_text_mode_move_cursor_to_column(args[0]);
  }
  else if(call == NUMBER_OF_CALL_MOVE_CURSOR_TO_LINE) {
   return system_call_text_mode_move_cursor_to_line(args[0]);
  }
  else if(call == NUMBER_OF_CALL_GET_CURSOR_COLUMN) {
   return system_call_text_mode_get_cursor_column();
  }
  else if(call == NUMBER_OF_CALL_GET_CURSOR_LINE) {
   return system_call_text_mode_get_cursor_column();
  }

  //pixel calls
  else if(call == NUMBER_OF_CALL_SET_PIXEL) {
   return system_call_text_mode_set_pixel(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_SET_CURSOR_PIXEL_CHARACTER) {
   return system_call_text_mode_set_cursor_pixel_character(args[0]);
  }
  else if(call == NUMBER_OF_CALL_SET_CURSOR_PIXEL_CH_COLOR) {
   return system_call_text_mode_set_cursor_pixel_ch_color(args[0]);
  }
  else if(call == NUMBER_OF_CALL_SET_CURSOR_PIXEL_BG_COLOR) {
   return system_call_text_mode_set_cursor_pixel_bg_color(args[0]);
  }
  else if(call == NUMBER_OF_CALL_SET_CURSOR_PIXEL) {
   return system_call_text_mode_set_cursor_pixel(args[0], args[1], args[2]);
  }
  else if(call == NUMBER_OF_CALL_PUT_CHARACTER_ON_CURSOR) {
   return system_call_text_mode_put_character_on_cursor(args[0]);
  }
  else if(call == NUMBER_OF_CALL_PUT_PIXEL_ON_CURSOR) {
   return system_call_text_mode_put_pixel_on_cursor(args[0], args[1], args[2]);
  }
  else if(call == NUMBER_OF_CALL_GET_PIXEL_CHARACTER) {
   return system_call_text_mode_get_pixel_character(args[0], args[1]);
  }
  else if(call == NUMBER_OF_CALL_GET_PIXEL_CH_COLOR) {
   return system_call_text_mode_get_pixel_character(args[0], args[1]);
  }
  else if(call == NUMBER_OF_CALL_GET_PIXEL_BG_COLOR) {
   return system_call_text_mode_get_pixel_character(args[0], args[1]);
  }
  else if(call == NUMBER_OF_CALL_GET_CURSOR_PIXEL_CHARACTER) {
   return system_call_text_mode_get_cursor_pixel_character();
  }
  else if(call == NUMBER_OF_CALL_GET_CURSOR_PIXEL_CH_COLOR) {
   return system_call_text_mode_get_cursor_pixel_ch_color();
  }
  else if(call == NUMBER_OF_CALL_GET_CURSOR_PIXEL_BG_COLOR) {
   return system_call_text_mode_get_cursor_pixel_bg_color();
  }

  //print calls
  else if(call == NUMBER_OF_CALL_PRINT) {
   return system_call_text_mode_print(args[0], args[1], (byte_t *)(args[2]));
  }
  else if(call == NUMBER_OF_CALL_PRINT_WITH_CH_COLOR) {
   return system_call_text_mode_print_with_ch_color(args[0], args[1], (byte_t *)(args[2]), args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_WITH_BG_COLOR) {
   return system_call_text_mode_print_with_bg_color(args[0], args[1], (byte_t *)(args[2]), args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_NUMBER) {
   return system_call_text_mode_print_number(args[0], args[1], args[2]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_NUMBER_WITH_CH_COLOR) {
   return system_call_text_mode_print_number_with_ch_color(args[0], args[1], args[2], args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_NUMBER_WITH_BG_COLOR) {
   return system_call_text_mode_print_number_with_bg_color(args[0], args[1], args[2], args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_FLOAT_NUMBER) {
   return system_call_text_mode_print_float_number(args[0], args[1], *(float*)&(args[2]));
  }
  else if(call == NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_CH_COLOR) {
   return system_call_text_mode_print_float_number_with_ch_color(args[0], args[1], *(float*)&(args[2]), args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_BG_COLOR) {
   return system_call_text_mode_print_float_number_with_bg_color(args[0], args[1], *(float*)&(args[2]), args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_HEX_NUMBER) {
   return system_call_text_mode_print_hex_number(args[0], args[1], args[2], args[3]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_CH_COLOR) {
   return system_call_text_mode_print_hex_number_with_ch_color(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_BG_COLOR) {
   return system_call_text_mode_print_hex_number_with_bg_color(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_ON_CURSOR) {
   return system_call_text_mode_print_on_cursor((byte_t *)(args[0]));
  }
  else if(call == NUMBER_OF_CALL_PRINT_NUMBER_ON_CURSOR) {
   return system_call_text_mode_print_number_on_cursor(args[0]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_ON_CURSOR) {
   return system_call_text_mode_print_float_number_on_cursor(*(float*)&(args[0]));
  }
  else if(call == NUMBER_OF_CALL_PRINT_HEX_NUMBER_ON_CURSOR) {
   return system_call_text_mode_print_hex_number_on_cursor(args[0], args[1]);
  }
  else if(call == NUMBER_OF_CALL_PRINT_TO_BLOCK) {
   return system_call_text_mode_print_to_block(args[0], args[1], args[2], args[3], (byte_t *)(args[4]));
  }

  //block calls
  else if(call == NUMBER_OF_CALL_SET_CHARACTER_ON_BLOCK) {
   return system_call_text_mode_set_character_on_block(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_SET_CH_COLOR_ON_BLOCK) {
   return system_call_text_mode_set_ch_color_on_block(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_SET_BG_COLOR_ON_BLOCK) {
   return system_call_text_mode_set_bg_color_on_block(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_SET_COLORS_ON_BLOCK) {
   return system_call_text_mode_set_colors_on_block(args[0], args[1], args[2], args[3], args[4], args[5]);
  }
  else if(call == NUMBER_OF_CALL_DRAW_FULL_SQUARE) {
   return system_call_text_mode_draw_full_square(args[0], args[1], args[2], args[3], args[4]);
  }
  else if(call == NUMBER_OF_CALL_DRAW_EMPTY_SQUARE) {
   return system_call_text_mode_draw_empty_square(args[0], args[1], args[2], args[3], args[4]);
  }

  //keyboard calls
  else if(call == NUMBER_OF_CALL_WAIT_FOR_KEYBOARD) {
   return system_call_wait_for_keyboard();
  }
  else if(call == NUMBER_OF_CALL_GET_KEY_VALUE) {
   return system_call_get_key_value();
  }
  else if(call == NUMBER_OF_CALL_GET_KEY_UNICODE_VALUE) {
   return system_call_get_key_unicode_value();
  }
  else if(call == NUMBER_OF_CALL_GET_ACTUAL_KEY_VALUE) {
   return system_call_get_actual_key_value();
  }
  else if(call == NUMBER_OF_CALL_GET_ACTUAL_KEY_UNICODE_VALUE) {
   return system_call_get_actual_key_unicode_value();
  }
  else if(call == NUMBER_OF_CALL_IS_THIS_KEY_PRESSED) {
   return system_call_is_this_key_pressed(args[0]);
  }

  //input calls
  else if(call == NUMBER_OF_CALL_CURSOR_LINE_INPUT) {
   return system_call_text_mode_cursor_line_input(args[0], args[1], args[2]);
  }
  else if(call == NUMBER_OF_CALL_GET_INPUT_STRING_POINTER) {
   return system_call_get_input_string_pointer();
  }
  else if(call == NUMBER_OF_CALL_GET_INPUT_STRING_NUMBER) {
   return system_call_get_input_string_number();
  }
  else if(call == NUMBER_OF_CALL_GET_INPUT_STRING_FLOAT_NUMBER) {
   return *(unsigned int*)&(float){system_call_get_input_string_float_number()};
  }

  //other calls
  else if(call == NUMBER_OF_CALL_GET_RANDOM_NUMBER) {
   return system_call_get_random_number(args[0], args[1], args[2]);
  }
  else if(call == NUMBER_OF_CALL_WAIT) {
   return system_call_wait(args[0]);
  }
  else if(call == NUMBER_OF_CALL_COMPARE_MEMORY) {
   return system_call_compare_memory((byte_t *)(args[0]), (byte_t *)(args[1]), args[2]);
  }
  else if(call == NUMBER_OF_CALL_MOVE_EXECUTION_TO_METHOD) {
   return system_call_move_execution_to_method((void (*)(unsigned int (*)(unsigned int,  unsigned int *)))args[0], args[1]);
  }
  else if(call == NUMBER_OF_CALL_EXIT_EXECUTION_OF_METHOD) {
   return system_call_exit_execution_of_method();
  }

  //unknown call
  else {
   return SYSTEM_CALL_ERROR;
  }
 }
 else {
  return SYSTEM_CALL_ERROR;
 }
}

// METHOD FOR OUTPUT ON TEXT SCREEN
dword_t system_call_reset_text_mode_screen(void) {
 //clear screen
 for(dword_t i=0; i<text_mode_screen_pixels; i++) {
  text_mode_screen[i].character = 0;
  text_mode_screen[i].background_color = BLACK;
  text_mode_screen[i].character_color = WHITE;
 }

 //move cursor to top
 text_mode_cursor_is_visible = STATUS_TRUE;
 text_mode_cursor_line = 0;
 text_mode_cursor_column = 0;

 //show changes
 redraw_text_mode_screen();

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_hide_changes_on_screen(void) {
 text_mode_show_changes_on_screen = STATUS_FALSE;

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_redraw_screen(void) {
 if(text_mode_show_changes_on_screen == STATUS_FALSE) {
  text_mode_show_changes_on_screen = STATUS_TRUE;
  redraw_text_mode_screen();
  text_mode_show_changes_on_screen = STATUS_FALSE;
 }
 //else screen is already updated

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_redraw_part_of_screen(dword_t column, dword_t line, dword_t width, dword_t height) {
 //check if parameters are valid
 if(width==0 || width>text_mode_screen_columns || height==0 || height>text_mode_screen_lines || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }
 
 if(text_mode_show_changes_on_screen == STATUS_FALSE) {
  text_mode_show_changes_on_screen = STATUS_TRUE;
  for(dword_t i=0; i<height; i++) {
   for(dword_t j=0; j<width; j++) {
    redraw_text_mode_pixel(line+i, column+j);
   }
  }
  text_mode_show_changes_on_screen = STATUS_FALSE;
 }
 //else screen is already updated

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_show_changes_on_screen(void) {
 text_mode_show_changes_on_screen = STATUS_TRUE;

 redraw_text_mode_screen();

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_clear_text_mode_screen(dword_t background_color, dword_t character_color) {
 //clear screen
 for(dword_t i=0; i<text_mode_screen_pixels; i++) {
  text_mode_screen[i].character = 0;
  text_mode_screen[i].background_color = background_color;
  text_mode_screen[i].character_color = character_color;
 }

 //show changes
 redraw_text_mode_screen();

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_get_number_of_columns(void) {
 return text_mode_screen_columns;
}

dword_t system_call_text_mode_get_number_of_lines(void) {
 return text_mode_screen_lines;
}

dword_t system_call_text_mode_show_cursor(void) {
 if(text_mode_cursor_is_visible == STATUS_FALSE) {
  text_mode_cursor_is_visible = STATUS_TRUE;
  redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);
 }
}

dword_t system_call_text_mode_hide_cursor(void) {
 if(text_mode_cursor_is_visible == STATUS_TRUE) {
  text_mode_cursor_is_visible = STATUS_FALSE;
  redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);
 }
}

dword_t system_call_text_mode_move_cursor(dword_t column, dword_t line) {
 //do not move above screen
 if(column>text_mode_screen_columns || line>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //do not move cursor to same pixel
 if(text_mode_cursor_column==column && text_mode_cursor_line==line) {
  return SYSTEM_CALL_SUCCESS;
 }

 //save old cursor location
 dword_t old_cursor_column = text_mode_cursor_column;
 dword_t old_cursor_line = text_mode_cursor_line;

 //move cursor
 text_mode_cursor_column = column;
 text_mode_cursor_line = line;

 //show changes
 redraw_text_mode_pixel(old_cursor_column, old_cursor_line);
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_move_cursor_to_column(dword_t column) {
 //do not move above screen
 if(column>text_mode_screen_columns) {
  return SYSTEM_CALL_ERROR;
 }

 //do not move cursor to same pixel
 if(text_mode_cursor_column==column) {
  return SYSTEM_CALL_SUCCESS;
 }

 //save old cursor location
 dword_t old_cursor_column = text_mode_cursor_column;

 //move cursor
 text_mode_cursor_column = column;

 //show changes
 redraw_text_mode_pixel(old_cursor_column, text_mode_cursor_line);
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_move_cursor_to_line(dword_t line) {
 //do not move above screen
 if(line>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //do not move cursor to same pixel
 if(text_mode_cursor_line==line) {
  return SYSTEM_CALL_SUCCESS;
 }

 //save old cursor location
 dword_t old_cursor_line = text_mode_cursor_line;

 //move cursor
 text_mode_cursor_line = line;

 //show changes
 redraw_text_mode_pixel(text_mode_cursor_column, old_cursor_line);
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_get_cursor_column(void) {
 return text_mode_cursor_column;
}

dword_t system_call_text_mode_get_cursor_line(void) {
 return text_mode_cursor_line;
}

dword_t system_call_text_mode_set_pixel(dword_t column, dword_t line, dword_t character, dword_t ch_color, dword_t bg_color) {
 if((line*text_mode_screen_columns+column)<text_mode_screen_pixels) {
  //set pixel
  text_mode_screen[(line*text_mode_screen_columns+column)].character = character;
  text_mode_screen[(line*text_mode_screen_columns+column)].character_color = ch_color;
  text_mode_screen[(line*text_mode_screen_columns+column)].background_color = bg_color;

  //show changes on screen
  redraw_text_mode_pixel(column, line);

  return SYSTEM_CALL_SUCCESS;
 }
 else {
  return SYSTEM_CALL_ERROR;
 }
}

dword_t system_call_text_mode_set_cursor_pixel_character(dword_t character) {
 //set pixel
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character = character;

 //show changes on screen
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_cursor_pixel_ch_color(dword_t ch_color) {
 //set pixel
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character_color = ch_color;

 //show changes on screen
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_cursor_pixel_bg_color(dword_t bg_color) {
 //set pixel
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].background_color = bg_color;

 //show changes on screen
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_cursor_pixel(dword_t character, dword_t ch_color, dword_t bg_color) {
 //set pixel
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character = character;
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character_color = ch_color;
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].background_color = bg_color;

 //show changes on screen
 redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);

 return SYSTEM_CALL_SUCCESS;
}

void text_mode_move_cursor_one_column_forward(void) { //help method used in system_call_text_mode_put_character_on_cursor() and system_call_text_mode_put_pixel_on_cursor()
 //move cursor
 text_mode_cursor_column++;
 if(text_mode_cursor_column==text_mode_screen_columns) {
  if(text_mode_cursor_line==(text_mode_screen_lines-1)) {
   //end of screen, move screen content
   for(dword_t i=0; i<(text_mode_screen_pixels-text_mode_screen_columns); i++) {
    text_mode_screen[i].character = text_mode_screen[i+text_mode_screen_columns].character;
   }
   for(dword_t i=0; i<text_mode_screen_columns; i++) {
    text_mode_screen[(text_mode_screen_pixels-text_mode_screen_columns+i)].character = 0;
   }
   text_mode_cursor_column = 0;

   redraw_text_mode_screen();
  }
  else {
   //move cursor to next line
   text_mode_cursor_column = 0;
   text_mode_cursor_line++;

   redraw_text_mode_pixel(text_mode_screen_columns-1, text_mode_cursor_line-1);
   redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);
  }
 }
 else {
  redraw_text_mode_pixel(text_mode_cursor_column-1, text_mode_cursor_line);
  redraw_text_mode_pixel(text_mode_cursor_column, text_mode_cursor_line);
 }
}

dword_t system_call_text_mode_put_character_on_cursor(dword_t character) {
 //set character of pixel
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character = character;

 //move cursor
 text_mode_move_cursor_one_column_forward();

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_put_pixel_on_cursor(dword_t character, dword_t ch_color, dword_t bg_color) {
 //set pixel
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character = character;
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character_color = ch_color;
 text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].background_color = bg_color;

 //move cursor
 text_mode_move_cursor_one_column_forward();

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_get_pixel_character(dword_t column, dword_t line) {
 if((line*text_mode_screen_columns+column)<text_mode_screen_pixels) {
  return text_mode_screen[(line*text_mode_screen_columns+column)].character;
 }
 else {
  return SYSTEM_CALL_ERROR;
 }
}

dword_t system_call_text_mode_get_pixel_ch_color(dword_t column, dword_t line) {
 if((line*text_mode_screen_columns+column)<text_mode_screen_pixels) {
  return text_mode_screen[(line*text_mode_screen_columns+column)].character_color;
 }
 else {
  return SYSTEM_CALL_ERROR;
 }
}

dword_t system_call_text_mode_get_pixel_bg_color(dword_t column, dword_t line) {
 if((line*text_mode_screen_columns+column)<text_mode_screen_pixels) {
  return text_mode_screen[(line*text_mode_screen_columns+column)].background_color;
 }
 else {
  return SYSTEM_CALL_ERROR;
 }
}

dword_t system_call_text_mode_get_cursor_pixel_character(void) {
 return text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character;
}

dword_t system_call_text_mode_get_cursor_pixel_ch_color(void) {
 return text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].character_color;
}

dword_t system_call_text_mode_get_cursor_pixel_bg_color(void) {
 return text_mode_screen[(text_mode_cursor_line*text_mode_screen_columns+text_mode_cursor_column)].background_color;
}

dword_t system_call_text_mode_print(dword_t column, dword_t line, byte_t *string) {
 while(*string!=0) {
  if((line*text_mode_screen_columns+column)<text_mode_screen_pixels) {
   //set pixel
   text_mode_screen[(line*text_mode_screen_columns+column)].character = *string;

   //show changes on screen
   redraw_text_mode_pixel(column, line);

   //move to next character in string
   string++;
   column++;
  }
  else { //above screen
   return SYSTEM_CALL_ERROR;
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_print_with_ch_color(dword_t column, dword_t line, byte_t *string, dword_t ch_color) {
 while (*string != 0) {
  if((line * text_mode_screen_columns + column) < text_mode_screen_pixels) {
   //set pixel
   text_mode_screen[(line * text_mode_screen_columns + column)].character = *string;
   text_mode_screen[(line * text_mode_screen_columns + column)].character_color = ch_color;

   //show changes on screen
   redraw_text_mode_pixel(column, line);

   //move to next character in string
   string++;
   column++;
  }
  else { //above screen
   return SYSTEM_CALL_ERROR;
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_print_with_bg_color(dword_t column, dword_t line, byte_t *string, dword_t bg_color) {
 while (*string != 0) {
  if((line * text_mode_screen_columns + column) < text_mode_screen_pixels) {
   //set pixel
   text_mode_screen[(line * text_mode_screen_columns + column)].character = *string;
   text_mode_screen[(line * text_mode_screen_columns + column)].background_color = bg_color;

   //show changes on screen
   redraw_text_mode_pixel(column, line);

   //move to next character in string
   string++;
   column++;
  }
  else { //above screen
   return SYSTEM_CALL_ERROR;
  }
 }
 
 return SYSTEM_CALL_SUCCESS;
}

dword_t parse_digits_from_int_number(byte_t *number_string, int number) {
 byte_t number_string_2[11];
 byte_t number_of_digits = 0;

 if(number == 0) {
  number_string_2[0] = '0';
  number_of_digits = 1;
 }
 else if(number<0) {
  number *= -1;

  while(number != 0) {
   number_string_2[number_of_digits] = ((number % 10)+'0');
   number_of_digits++;
   number /= 10;
  }

  number_string_2[number_of_digits] = '-';
  number_of_digits++;
 }
 else {
  while(number != 0) {
   number_string_2[number_of_digits] = ((number % 10)+'0');
   number_of_digits++;
   number /= 10;
  }
 }

 for(dword_t i=0; i<number_of_digits; i++) {
  number_string[i] = number_string_2[number_of_digits-1-i];
 }
 number_string[number_of_digits] = 0;

 return number_of_digits;
}

dword_t parse_digits_from_float_number(byte_t *number_string, float number) {
 byte_t number_string_2[50];
 byte_t number_of_digits = 0;

 int whole_part = (int)number;
 number_of_digits = parse_digits_from_int_number(number_string_2, whole_part);

 if((number - whole_part) != 0) {
  number_string_2[number_of_digits] = '.';
  number_of_digits++;

  float decimal_part = (number - whole_part);
  for(dword_t i=0; i<6; i++) {
   if(decimal_part == 0) {
    break;
   }
   decimal_part *= 10;
   number_string_2[number_of_digits] = ('0' + ((int)decimal_part));
   number_of_digits++;
   decimal_part -= ((int)decimal_part);
  }
 }

 // Kopírovanie do cieľového bufferu
 for (dword_t i=0; i<number_of_digits; i++) {
  number_string[i] = number_string_2[i];
 }
 number_string[number_of_digits] = 0;

 return number_of_digits;
}

void parse_digits_from_hex_number(byte_t *number_string, int number, byte_t number_of_digits) {
 number_string[0] = '0';
 number_string[1] = 'x';

 for(dword_t i=2, digit=0, shift=(number_of_digits*4-4); i<(number_of_digits+2); i++, shift-=4) {
  digit = ((number>>shift) & 0xF);
  number_string[i] = ((digit<10) ? (digit+'0') : (digit+'A'-10));
 }
 number_string[number_of_digits+2] = 0;
}

dword_t system_call_text_mode_print_number(dword_t column, dword_t line, int number) {
 byte_t number_string[12];
 parse_digits_from_int_number(number_string, number);
 return system_call_text_mode_print(column, line, number_string);
}

dword_t system_call_text_mode_print_number_with_ch_color(dword_t column, dword_t line, dword_t number, dword_t ch_color) {
 byte_t number_string[12];
 parse_digits_from_int_number(number_string, number);
 return system_call_text_mode_print_with_ch_color(column, line, number_string, ch_color);
}

dword_t system_call_text_mode_print_number_with_bg_color(dword_t column, dword_t line, dword_t number, dword_t bg_color) {
 byte_t number_string[12];
 parse_digits_from_int_number(number_string, number);
 return system_call_text_mode_print_with_bg_color(column, line, number_string, bg_color);
}

dword_t system_call_text_mode_print_float_number(dword_t column, dword_t line, float number) {
 byte_t number_string[50];
 parse_digits_from_float_number(number_string, number);
 return system_call_text_mode_print(column, line, number_string);
}

dword_t system_call_text_mode_print_float_number_with_ch_color(dword_t column, dword_t line, float number, dword_t ch_color) {
 byte_t number_string[12];
 parse_digits_from_float_number(number_string, number);
 return system_call_text_mode_print_with_ch_color(column, line, number_string, ch_color);
}

dword_t system_call_text_mode_print_float_number_with_bg_color(dword_t column, dword_t line, float number, dword_t bg_color) {
 byte_t number_string[12];
 parse_digits_from_float_number(number_string, number);
 return system_call_text_mode_print_with_bg_color(column, line, number_string, bg_color);
}

dword_t system_call_text_mode_print_hex_number(dword_t column, dword_t line, dword_t number, dword_t number_of_digits) {
 if(number_of_digits>8) {
  return SYSTEM_CALL_ERROR;
 }

 byte_t number_string[11];
 parse_digits_from_hex_number(number_string, number, number_of_digits);
 return system_call_text_mode_print(column, line, number_string);
}

dword_t system_call_text_mode_print_hex_number_with_ch_color(dword_t column, dword_t line, dword_t number, dword_t number_of_digits, dword_t ch_color) {
 if(number_of_digits>8) {
  return SYSTEM_CALL_ERROR;
 }

 byte_t number_string[11];
 parse_digits_from_hex_number(number_string, number, number_of_digits);
 return system_call_text_mode_print_with_ch_color(column, line, number_string, ch_color);
}

dword_t system_call_text_mode_print_hex_number_with_bg_color(dword_t column, dword_t line, dword_t number, dword_t number_of_digits, dword_t bg_color) {
 if(number_of_digits>8) {
  return SYSTEM_CALL_ERROR;
 }
 
 byte_t number_string[11];
 parse_digits_from_hex_number(number_string, number, number_of_digits);
 return system_call_text_mode_print_with_bg_color(column, line, number_string, bg_color);
}

dword_t system_call_text_mode_print_on_cursor(byte_t *string) {
 while (*string != 0) {
  //set character at cursor position
  text_mode_screen[(text_mode_cursor_line * text_mode_screen_columns + text_mode_cursor_column)].character = *string;

  //move cursor
  text_mode_move_cursor_one_column_forward();

  //move to next character in string
  string++;
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_print_number_on_cursor(int number) {
 byte_t number_string[12];
 parse_digits_from_int_number(number_string, number);
 return system_call_text_mode_print_on_cursor(number_string);
}

dword_t system_call_text_mode_print_float_number_on_cursor(float number) {
 byte_t number_string[50];
 parse_digits_from_float_number(number_string, number);
 return system_call_text_mode_print_on_cursor(number_string);
}

dword_t system_call_text_mode_print_hex_number_on_cursor(dword_t number, dword_t number_of_digits) {
 if(number_of_digits>8) {
  return SYSTEM_CALL_ERROR;
 }

 byte_t number_string[11];
 parse_digits_from_hex_number(number_string, number, number_of_digits);
 return system_call_text_mode_print_on_cursor(number_string);
}

dword_t system_call_text_mode_print_to_block(dword_t column, dword_t line, dword_t width, dword_t align, byte_t *string) {
 //above screen
 if(width==0 || width>text_mode_screen_columns || (column+width)>text_mode_screen_columns || line>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 dword_t original_column = column;
 
 while(*string != 0) {
  //count how many characters could fit into line
  dword_t characters_in_line = 0, last_space_position = -1;
  for(dword_t i=0; i<width; i++) {
   //end of line
   if(string[i]=='\n' || string[i]==0) {
    goto print_line;
   }

   //space
   if(string[i]==' ') {
    last_space_position = i;
   }

   characters_in_line++;
  }

  //remove spaces
  if(last_space_position!=-1) {
   characters_in_line = last_space_position;

   while(characters_in_line>0) {
    if(string[characters_in_line]==' ') {
     characters_in_line--;
    }
    else {
     break;
    }
   }
  }
  
  //print line according to align
  print_line:
  if(string[characters_in_line]!=0) {
   characters_in_line++;
  }
  if(align == 0) { //TODO: left
   column = original_column;
  }
  else if(align == 1) { //TODO: center
   column = ((original_column+(width/2)) - (characters_in_line/2));
  }
  else { //TODO: right
   column = (original_column+width-characters_in_line);
  }
  for(dword_t i=0; i<characters_in_line; i++) {
   text_mode_screen[(line * text_mode_screen_columns + column)].character = *string;

   redraw_text_mode_pixel(column, line);
   
   string++;
   column++;
  }
  line++;

  //above screen
  if(line==text_mode_screen_lines) {
   return SYSTEM_CALL_ERROR;
  }

  //skip spaces at end of line
  while(*string==' ') {
   string++;
  }
  if(*string=='\n') {
   string++;
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_character_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t character) {
 //check if parameters are valid
 if(width==0 || width>text_mode_screen_columns || height==0 || height>text_mode_screen_lines || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //set pixels
 for(dword_t i=0; i<height; i++) {
  for(dword_t j=0; j<width; j++) {
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].character = character;
   redraw_text_mode_pixel(column+j, line+i);
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_ch_color_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t ch_color) {
 //check if parameters are valid
 if(width==0 || width>text_mode_screen_columns || height==0 || height>text_mode_screen_lines || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //set pixels
 for(dword_t i=0; i<height; i++) {
  for(dword_t j=0; j<width; j++) {
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].character_color = ch_color;
   redraw_text_mode_pixel(column+j, line+i);
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_bg_color_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t bg_color) {
 //check if parameters are valid
 if(width==0 || width>text_mode_screen_columns || height==0 || height>text_mode_screen_lines || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //set pixels
 for(dword_t i=0; i<height; i++) {
  for(dword_t j=0; j<width; j++) {
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].background_color = bg_color;
   redraw_text_mode_pixel(column+j, line+i);
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_set_colors_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t ch_color, dword_t bg_color) {
 //check if parameters are valid
 if(width==0 || width>text_mode_screen_columns || height==0 || height>text_mode_screen_lines || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //set pixels
 for(dword_t i=0; i<height; i++) {
  for(dword_t j=0; j<width; j++) {
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].character_color = ch_color;
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].background_color = bg_color;
   redraw_text_mode_pixel(column+j, line+i);
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_draw_full_square(dword_t column, dword_t line, dword_t width, dword_t height, dword_t color) {
 //check if parameters are valid
 if(width==0 || width>text_mode_screen_columns || height==0 || height>text_mode_screen_lines || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //set pixels
 for(dword_t i=0; i<height; i++) {
  for(dword_t j=0; j<width; j++) {
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].character = ' ';
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].character_color = color;
   text_mode_screen[((line+i) * text_mode_screen_columns + (column+j))].background_color = color;
   redraw_text_mode_pixel(column+j, line+i);
  }
 }

 return SYSTEM_CALL_SUCCESS;
}

dword_t system_call_text_mode_draw_empty_square(dword_t column, dword_t line, dword_t width, dword_t height, dword_t color) {
 //check if parameters are valid
 if(width==0 || height==0 || (column+width)>text_mode_screen_columns || (line+height)>text_mode_screen_lines) {
  return SYSTEM_CALL_ERROR;
 }

 //draw empty square
 system_call_text_mode_draw_full_square(column, line, width, 1, color);
 system_call_text_mode_draw_full_square(column, line, 1, height, color);
 system_call_text_mode_draw_full_square(column+width-1, line, 1, height, color);
 system_call_text_mode_draw_full_square(column, line+height-1, width, 1, color);
}

// METHODS FOR INPUT FROM KEYBOARD
dword_t system_call_wait_for_keyboard(void) {
 while(1) {
  wait_for_user_input();

  if(keyboard_code_of_pressed_key!=0 && (keyboard_code_of_pressed_key & 0xFF)<0x80) {
   pressed_key_value = keyboard_code_of_pressed_key;
   pressed_key_unicode_value = keyboard_unicode_value_of_pressed_key;
   return SYSTEM_CALL_SUCCESS;
  }
 }
}

dword_t system_call_get_key_value(void) {
 return pressed_key_value;
}

dword_t system_call_get_key_unicode_value(void) {
 return pressed_key_unicode_value;
}

dword_t system_call_get_actual_key_value(void) {
 return keyboard_code_of_pressed_key;
}

dword_t system_call_get_actual_key_unicode_value(void) {
 return keyboard_unicode_value_of_pressed_key;
}

dword_t system_call_is_this_key_pressed(dword_t key_value) {
 return keyboard_is_key_pressed(key_value);
}

dword_t system_call_text_mode_cursor_line_input(dword_t column, dword_t line, dword_t number_of_chars) {
 if(line>=text_mode_screen_lines || (column+number_of_chars)>=text_mode_screen_columns) {
  return SYSTEM_CALL_ERROR;
 }

 //move cursor to start of input
 system_call_text_mode_move_cursor(column, line);

 while(1) {
  system_call_wait_for_keyboard();

  if(keyboard_code_of_pressed_key==KEY_ENTER) {
   return SYSTEM_CALL_SUCCESS;
  }
  else if(keyboard_code_of_pressed_key==KEY_ESC) {
   return SYSTEM_CALL_ERROR;
  }
  else if(keyboard_code_of_pressed_key==KEY_UP) { //move cursor to start of line
   system_call_text_mode_move_cursor(column, line);
  }
  else if(keyboard_code_of_pressed_key==KEY_DOWN) { //move cursor to end of line
   system_call_text_mode_move_cursor((column+number_of_chars-1), line);
  }
  else if(keyboard_code_of_pressed_key==KEY_LEFT && text_mode_cursor_column>column) {
   system_call_text_mode_move_cursor(text_mode_cursor_column-1, line);
  }
  else if(keyboard_code_of_pressed_key==KEY_RIGHT && text_mode_cursor_column<(column+number_of_chars-1)) {
   system_call_text_mode_move_cursor(text_mode_cursor_column+1, line);
  }
  else if(keyboard_code_of_pressed_key==KEY_BACKSPACE && text_mode_cursor_column>column) {
   text_mode_cursor_column--;
   remove_space_from_memory_area((dword_t)cursor_line_input_buffer, number_of_chars*2, ((dword_t)cursor_line_input_buffer)+(text_mode_cursor_column-column)*2, 2);
   for(dword_t i=0; i<number_of_chars; i++) {
    system_call_text_mode_set_character_on_block(column+i, line, 1, 1, cursor_line_input_buffer[i]);
   }
  }
  else if(keyboard_code_of_pressed_key==KEY_DELETE) {
   remove_space_from_memory_area((dword_t)cursor_line_input_buffer, number_of_chars*2, ((dword_t)cursor_line_input_buffer)+(text_mode_cursor_column-column)*2, 2);
   for(dword_t i=0; i<number_of_chars; i++) {
    system_call_text_mode_set_character_on_block(column+i, line, 1, 1, cursor_line_input_buffer[i]);
   }
  }
  else if(keyboard_unicode_value_of_pressed_key!=0 && text_mode_cursor_column<(column+number_of_chars-1)) {
   add_space_to_memory_area((dword_t)cursor_line_input_buffer, number_of_chars*2, ((dword_t)cursor_line_input_buffer)+(text_mode_cursor_column-column)*2, 2);
   cursor_line_input_buffer[(text_mode_cursor_column-column)] = keyboard_unicode_value_of_pressed_key;
   text_mode_cursor_column++;

   for(dword_t i=0; i<number_of_chars; i++) {
    system_call_text_mode_set_character_on_block(column+i, line, 1, 1, cursor_line_input_buffer[i]);
   }
  }
 }
}

dword_t system_call_get_input_string_pointer(void) {
 return (dword_t)cursor_line_input_buffer;
}

dword_t system_call_get_input_string_number(void) {
 return convert_word_string_to_number((dword_t)cursor_line_input_buffer);
}

float system_call_get_input_string_float_number(void) {
 return convert_word_string_to_float_number((dword_t)cursor_line_input_buffer);
}

// OTHER METHODS
dword_t system_call_get_random_number(int first_value, int last_value, int step) {
 system_calls_random_number += get_timer_value_in_milliseconds();

 if(first_value > last_value) {
  dword_t a = first_value;
  first_value = last_value;
  last_value = a;
 }

 int number = 0;
 number = (system_calls_random_number % (last_value-first_value)); //value between 0 - input scale
 number -= (number%step); //value between 0 - input scale that fits into step
 number += first_value; //value calculated from frist step
 return number;
}

dword_t system_call_wait(dword_t milliseconds) {
 //do not allow wait longer than 5 seconds
 if(milliseconds > 5000) {
  return SYSTEM_CALL_ERROR;
 }

 wait(milliseconds);
}

dword_t system_call_compare_memory(byte_t *memory_1, byte_t *memory_2, dword_t size) {
 return is_memory_equal_with_memory(memory_1, memory_2, size);
}

dword_t system_call_move_execution_to_method(void (*method)(unsigned int (*system_call)(unsigned int call, unsigned int args[])), dword_t calls_per_second) {
 if(calls_per_second==0 || calls_per_second>100 || (dword_t)method<0x10000 || (dword_t)method>(0x10000+256*1024) || system_calls_is_execution_in_repeating_method==STATUS_TRUE) {
  return SYSTEM_CALL_ERROR;
 }

 system_calls_is_execution_in_repeating_method = STATUS_TRUE;
 reset_timer();

 while(system_calls_is_execution_in_repeating_method==STATUS_TRUE) {
  asm("hlt");

  if(get_timer_value_in_milliseconds()>(1000/calls_per_second)) {
   (*method)(system_call);
   reset_timer();
  }
 }
}

dword_t system_call_exit_execution_of_method(void) {
 system_calls_is_execution_in_repeating_method = STATUS_FALSE;
}