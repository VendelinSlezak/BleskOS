//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_calculator(void) {
 calculator_program_interface_memory = create_program_interface_memory(((dword_t)&draw_calculator), (PROGRAM_INTERFACE_FLAG_NO_OPEN_AND_SAVE_BUTTON | PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON));
 calculator_first_value_text_area_mem = create_text_area(TEXT_AREA_NUMBER_INPUT, 9, 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10, 200, 10);
 calculator_second_value_text_area_mem = create_text_area(TEXT_AREA_NUMBER_INPUT, 9, 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10, 200, 10);
 calculator_result_text_area_mem = create_text_area(TEXT_AREA_NUMBER_INPUT, 11, 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10, 200, 10);
 calculator_disable_cursor_in_all_text_areas();
 calculator_reset_color_of_buttons_in_basic_mode();
 calculator_basic_type_button_colors[0] = RED;
 calculator_selected_operation = CALCULATOR_CLICK_ZONE_BUTTON_PLUS;
 calculator_result_remainder = 0;
}

void calculator(void) {
 set_program_interface(calculator_program_interface_memory);
 program_interface_add_keyboard_event(KEY_F9, (dword_t)calculator_button_plus);
 program_interface_add_keyboard_event(KEY_F10, (dword_t)calculator_button_minus);
 program_interface_add_keyboard_event(KEY_F11, (dword_t)calculator_button_multiply);
 program_interface_add_keyboard_event(KEY_F12, (dword_t)calculator_button_divide);
 program_interface_add_keyboard_event(KEY_ENTER, (dword_t)calculator_button_calculate);
 program_interface_add_click_zone_event(CALCULATOR_CLICK_ZONE_BUTTON_PLUS, (dword_t)calculator_button_plus);
 program_interface_add_click_zone_event(CALCULATOR_CLICK_ZONE_BUTTON_MINUS, (dword_t)calculator_button_minus);
 program_interface_add_click_zone_event(CALCULATOR_CLICK_ZONE_BUTTON_MULTIPLY, (dword_t)calculator_button_multiply);
 program_interface_add_click_zone_event(CALCULATOR_CLICK_ZONE_BUTTON_DIVIDE, (dword_t)calculator_button_divide);
 program_interface_add_click_zone_event(CALCULATOR_CLICK_ZONE_BUTTON_CALCULATE, (dword_t)calculator_button_calculate);
 program_interface_redraw();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //close program
  if(keyboard_code_of_pressed_key==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && get_mouse_cursor_click_board_value()==CLICK_ZONE_BACK)) {
   text_area_disable_cursor(calculator_first_value_text_area_mem);
   text_area_disable_cursor(calculator_second_value_text_area_mem);
   text_area_disable_cursor(calculator_result_text_area_mem);
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();
 }
}

void draw_calculator(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("Calculator", "", 0x888888, 0x444444);
 if(get_program_value(CALCULATOR_PROGRAM_TYPE)==CALCULATOR_TYPE_BASIC) {
  print("First value", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10, WHITE);
  draw_text_area(calculator_first_value_text_area_mem);
  add_zone_to_click_board(10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10, 200, 10, CALCULATOR_CLICK_ZONE_TEXT_AREA_FIRST_VALUE);
  program_interface_add_text_area(CALCULATOR_CLICK_ZONE_TEXT_AREA_FIRST_VALUE, calculator_first_value_text_area_mem);

  draw_button_with_specific_color_and_click_zone("+", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[0], CALCULATOR_CLICK_ZONE_BUTTON_PLUS);
  draw_button_with_specific_color_and_click_zone("-", 10+20+10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[1], CALCULATOR_CLICK_ZONE_BUTTON_MINUS);
  draw_button_with_specific_color_and_click_zone("*", 10+20+10+20+10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[2], CALCULATOR_CLICK_ZONE_BUTTON_MULTIPLY);
  draw_button_with_specific_color_and_click_zone("/", 10+20+10+20+10+20+10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[3], CALCULATOR_CLICK_ZONE_BUTTON_DIVIDE);
  
  print("Second value", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10, WHITE);
  draw_text_area(calculator_second_value_text_area_mem);
  add_zone_to_click_board(10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10, 200, 10, CALCULATOR_CLICK_ZONE_TEXT_AREA_SECOND_VALUE);
  program_interface_add_text_area(CALCULATOR_CLICK_ZONE_TEXT_AREA_SECOND_VALUE, calculator_second_value_text_area_mem);
 
  draw_button_with_click_zone("[enter] Calculate", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10, 200, 20, CALCULATOR_CLICK_ZONE_BUTTON_CALCULATE);

  print("Result", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10, WHITE);
  draw_text_area(calculator_result_text_area_mem);
  add_zone_to_click_board(10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10, 200, 10, CALCULATOR_CLICK_ZONE_TEXT_AREA_RESULT);
  program_interface_add_text_area(CALCULATOR_CLICK_ZONE_TEXT_AREA_RESULT, calculator_result_text_area_mem);

  if(calculator_result_remainder!=0) {
   print("Remainder:", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10+10+10, WHITE);
   print_var(calculator_result_remainder, 10+11*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10+10+10, WHITE);
  }

  print("You can use F9-F12 buttons to select operation", 10, screen_height-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH-20, BLACK);
 }
}

void calculator_button_plus(void) {
 calculator_reset_color_of_buttons_in_basic_mode();
 calculator_basic_type_button_colors[0] = RED;
 calculator_selected_operation = CALCULATOR_CLICK_ZONE_BUTTON_PLUS;
 calculator_redraw_buttons_in_basic_mode();
}

void calculator_button_minus(void) {
 calculator_reset_color_of_buttons_in_basic_mode();
 calculator_basic_type_button_colors[1] = RED;
 calculator_selected_operation = CALCULATOR_CLICK_ZONE_BUTTON_MINUS;
 calculator_redraw_buttons_in_basic_mode();
}

void calculator_button_multiply(void) {
 calculator_reset_color_of_buttons_in_basic_mode();
 calculator_basic_type_button_colors[2] = RED;
 calculator_selected_operation = CALCULATOR_CLICK_ZONE_BUTTON_MULTIPLY;
 calculator_redraw_buttons_in_basic_mode();
}

void calculator_button_divide(void) {
 calculator_reset_color_of_buttons_in_basic_mode();
 calculator_basic_type_button_colors[3] = RED;
 calculator_selected_operation = CALCULATOR_CLICK_ZONE_BUTTON_DIVIDE;
 calculator_redraw_buttons_in_basic_mode();
}

void calculator_button_calculate(void) {
 //TODO: calculate not by processor methods that are limited to dword size, but by digits in string
 //TODO: negative numbers in values
 dword_t first_value = text_area_return_written_number_in_dword(calculator_first_value_text_area_mem);
 dword_t second_value = text_area_return_written_number_in_dword(calculator_second_value_text_area_mem);
 dword_t result = 0;
 calculator_result_remainder = 0;

 //clear result area
 dword_t *result_text_area_info = (dword_t *) (calculator_result_text_area_mem);
 word_t *result_text_area_data = (word_t *) (result_text_area_info[TEXT_AREA_INFO_MEMORY]);
 clear_memory(result_text_area_info[TEXT_AREA_INFO_MEMORY], 10*2);

 //calculate
 dword_t *first_value_text_area_info = (dword_t *) (calculator_first_value_text_area_mem);
 dword_t *second_value_text_area_info = (dword_t *) (calculator_second_value_text_area_mem);
 if(calculator_selected_operation==CALCULATOR_CLICK_ZONE_BUTTON_PLUS) {
  result = (first_value+second_value);
  if(result>0x80000000) { //negative number
   error_window("Too big numbers");
   goto error_window_end;
  }
 }
 else if(calculator_selected_operation==CALCULATOR_CLICK_ZONE_BUTTON_MINUS) {
  result = (first_value-second_value);
  if((second_value>first_value) && result<0x80000000) { //positive number
   error_window("Too big numbers");
   goto error_window_end;
  }
 }
 else if(calculator_selected_operation==CALCULATOR_CLICK_ZONE_BUTTON_MULTIPLY) {
  if(second_value>first_value) {
   dword_t x = first_value;
   first_value = second_value;
   second_value = x;
  }
  for(dword_t i=0; i<second_value; i++) {
   result += first_value;
   if(result>0x80000000) { //negative number
    error_window("Result is too big");
    goto error_window_end;
   }
  }
 }
 else if(calculator_selected_operation==CALCULATOR_CLICK_ZONE_BUTTON_DIVIDE) {
  if(second_value==0) {
   error_window("You can not divide by zero");
   goto error_window_end;
  }
  else {
   result = (first_value/second_value);
   calculator_result_remainder = (first_value%second_value);
  }
 }

 //put result into text area
 if(result>0x80000000) { //negative number
  result_text_area_data[0] = '-';
  result_text_area_data++;
  result = abs(result);
 }
 convert_number_to_word_string(result, (dword_t)(result_text_area_data));

 //redraw text areas
 calculator_redraw_text_areas_in_basic_mode();
 draw_full_square(10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10+10+10, 21*8, 8, 0x444444);
 if(calculator_result_remainder!=0) {
  print("Remainder:", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10+10+10, WHITE);
  print_var(calculator_result_remainder, 10+11*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10+10+10, WHITE);
 }
 redraw_part_of_screen(10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10+20+10+8+10+10+10+20+10+8+10+10+10, 21*8, 8);
 return;

 error_window_end:
 program_interface_redraw();
}

void calculator_reset_color_of_buttons_in_basic_mode(void) {
 for(dword_t i=0; i<4; i++) {
  calculator_basic_type_button_colors[i] = BUTTON_COLOR;
 }
}

void calculator_redraw_buttons_in_basic_mode(void) {
 draw_button_with_specific_color_and_click_zone("+", 10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[0], CALCULATOR_CLICK_ZONE_BUTTON_PLUS);
 draw_button_with_specific_color_and_click_zone("-", 10+20+10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[1], CALCULATOR_CLICK_ZONE_BUTTON_MINUS);
 draw_button_with_specific_color_and_click_zone("*", 10+20+10+20+10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[2], CALCULATOR_CLICK_ZONE_BUTTON_MULTIPLY);
 draw_button_with_specific_color_and_click_zone("/", 10+20+10+20+10+20+10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 20, 20, calculator_basic_type_button_colors[3], CALCULATOR_CLICK_ZONE_BUTTON_DIVIDE);
 redraw_part_of_screen(10, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+10+8+10+10+10, 110, 20);
}

void calculator_disable_cursor_in_all_text_areas(void) {
 text_area_disable_cursor(calculator_first_value_text_area_mem);
 text_area_disable_cursor(calculator_second_value_text_area_mem);
 text_area_disable_cursor(calculator_result_text_area_mem);
}

void calculator_redraw_text_areas_in_basic_mode(void) {
 calculator_disable_cursor_in_all_text_areas();
 draw_text_area(calculator_first_value_text_area_mem);
 draw_text_area(calculator_second_value_text_area_mem);
 draw_text_area(calculator_result_text_area_mem);
 redraw_text_area(calculator_first_value_text_area_mem);
 redraw_text_area(calculator_second_value_text_area_mem);
 redraw_text_area(calculator_result_text_area_mem);
}