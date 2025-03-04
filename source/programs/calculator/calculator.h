//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define CALCULATOR_PROGRAM_TYPE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)

#define CALCULATOR_TYPE_BASIC 0
#define CALCULATOR_TYPE_CONVERSION 1

#define CALCULATOR_CLICK_ZONE_TEXT_AREA_FIRST_VALUE 1000
#define CALCULATOR_CLICK_ZONE_TEXT_AREA_SECOND_VALUE 1001
#define CALCULATOR_CLICK_ZONE_TEXT_AREA_RESULT 1002
#define CALCULATOR_CLICK_ZONE_BUTTON_PLUS 1100
#define CALCULATOR_CLICK_ZONE_BUTTON_MINUS 1101
#define CALCULATOR_CLICK_ZONE_BUTTON_MULTIPLY 1102
#define CALCULATOR_CLICK_ZONE_BUTTON_DIVIDE 1103
#define CALCULATOR_CLICK_ZONE_BUTTON_CALCULATE 1104

dword_t calculator_program_interface_memory = 0;
dword_t calculator_first_value_text_area_mem = 0, calculator_second_value_text_area_mem = 0, calculator_result_text_area_mem = 0, calculator_result_remainder = 0;
dword_t calculator_basic_type_button_colors[4];
dword_t calculator_selected_operation = 0;

void initalize_calculator(void);
void calculator(void);
void draw_calculator(void);
void calculator_button_plus(void);
void calculator_button_minus(void);
void calculator_button_multiply(void);
void calculator_button_divide(void);
void calculator_button_calculate(void);
void calculator_reset_color_of_buttons_in_basic_mode(void);
void calculator_redraw_buttons_in_basic_mode(void);
void calculator_disable_cursor_in_all_text_areas(void);
void calculator_redraw_text_areas_in_basic_mode(void);