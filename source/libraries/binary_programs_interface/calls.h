//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SYSTEM_CALL_SUCCESS 0
#define SYSTEM_CALL_ERROR -1

#define NUMBER_OF_CALL_SET_PROGRAM_MODE 0
enum {
 NUMBER_OF_CALL_RESET_SCREEN = 1,
 NUMBER_OF_CALL_CLEAR_SCREEN,
 NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_COLUMNS,
 NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_LINES,
 NUMBER_OF_CALL_HIDE_CHANGES_ON_SCREEN,
 NUMBER_OF_CALL_REDRAW_SCREEN,
 NUMBER_OF_CALL_REDRAW_PART_OF_SCREEN,
 NUMBER_OF_CALL_SHOW_CHANGES_ON_SCREEN,

 NUMBER_OF_CALL_SHOW_CURSOR,
 NUMBER_OF_CALL_HIDE_CURSOR,
 NUMBER_OF_CALL_MOVE_CURSOR,
 NUMBER_OF_CALL_MOVE_CURSOR_TO_COLUMN,
 NUMBER_OF_CALL_MOVE_CURSOR_TO_LINE,
 NUMBER_OF_CALL_GET_CURSOR_COLUMN,
 NUMBER_OF_CALL_GET_CURSOR_LINE,

 NUMBER_OF_CALL_SET_PIXEL,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL_CHARACTER,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL_CH_COLOR,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL_BG_COLOR,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL,
 NUMBER_OF_CALL_PUT_CHARACTER_ON_CURSOR,
 NUMBER_OF_CALL_PUT_PIXEL_ON_CURSOR,
 NUMBER_OF_CALL_GET_PIXEL_CHARACTER,
 NUMBER_OF_CALL_GET_PIXEL_CH_COLOR,
 NUMBER_OF_CALL_GET_PIXEL_BG_COLOR,
 NUMBER_OF_CALL_GET_CURSOR_PIXEL_CHARACTER,
 NUMBER_OF_CALL_GET_CURSOR_PIXEL_CH_COLOR,
 NUMBER_OF_CALL_GET_CURSOR_PIXEL_BG_COLOR,

 NUMBER_OF_CALL_PRINT,
 NUMBER_OF_CALL_PRINT_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_NUMBER,
 NUMBER_OF_CALL_PRINT_NUMBER_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_NUMBER_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_NUMBER_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_TO_BLOCK,

 NUMBER_OF_CALL_SET_CHARACTER_ON_BLOCK,
 NUMBER_OF_CALL_SET_CH_COLOR_ON_BLOCK,
 NUMBER_OF_CALL_SET_BG_COLOR_ON_BLOCK,
 NUMBER_OF_CALL_SET_COLORS_ON_BLOCK,
 NUMBER_OF_CALL_DRAW_FULL_SQUARE,
 NUMBER_OF_CALL_DRAW_EMPTY_SQUARE,
  
 NUMBER_OF_CALL_WAIT_FOR_KEYBOARD,
 NUMBER_OF_CALL_GET_KEY_VALUE,
 NUMBER_OF_CALL_GET_KEY_UNICODE_VALUE,
 NUMBER_OF_CALL_GET_ACTUAL_KEY_VALUE,
 NUMBER_OF_CALL_GET_ACTUAL_KEY_UNICODE_VALUE,
 NUMBER_OF_CALL_IS_THIS_KEY_PRESSED,

 NUMBER_OF_CALL_CURSOR_LINE_INPUT,
 NUMBER_OF_CALL_GET_INPUT_STRING_POINTER,
 NUMBER_OF_CALL_GET_INPUT_STRING_NUMBER,
 NUMBER_OF_CALL_GET_INPUT_STRING_FLOAT_NUMBER,

 NUMBER_OF_CALL_GET_RANDOM_NUMBER,
 NUMBER_OF_CALL_WAIT,
 NUMBER_OF_CALL_COMPARE_MEMORY,
 NUMBER_OF_CALL_MOVE_EXECUTION_TO_METHOD,
 NUMBER_OF_CALL_EXIT_EXECUTION_OF_METHOD,
};

dword_t pressed_key_value, pressed_key_unicode_value;
dword_t system_calls_random_number;
word_t *cursor_line_input_buffer;
byte_t system_calls_is_execution_in_repeating_method;

dword_t system_call(dword_t call, dword_t args[]);

dword_t system_call_reset_text_mode_screen(void);
dword_t system_call_clear_text_mode_screen(dword_t background_color, dword_t character_color);
dword_t system_call_text_mode_get_number_of_columns(void);
dword_t system_call_text_mode_get_number_of_lines(void);
dword_t system_call_text_mode_hide_changes_on_screen(void);
dword_t system_call_text_mode_redraw_screen(void);
dword_t system_call_text_mode_redraw_part_of_screen(dword_t column, dword_t line, dword_t width, dword_t height);
dword_t system_call_text_mode_show_changes_on_screen(void);

dword_t system_call_text_mode_show_cursor(void);
dword_t system_call_text_mode_hide_cursor(void);
dword_t system_call_text_mode_move_cursor(dword_t column, dword_t line);
dword_t system_call_text_mode_move_cursor_to_column(dword_t column);
dword_t system_call_text_mode_move_cursor_to_line(dword_t line);
dword_t system_call_text_mode_get_cursor_column(void);
dword_t system_call_text_mode_get_cursor_line(void);

dword_t system_call_text_mode_set_pixel_character(dword_t column, dword_t line, dword_t character);
dword_t system_call_text_mode_set_pixel_ch_color(dword_t column, dword_t line, dword_t ch_color);
dword_t system_call_text_mode_set_pixel_bg_color(dword_t column, dword_t line, dword_t bg_color);
dword_t system_call_text_mode_set_pixel(dword_t column, dword_t line, dword_t character, dword_t ch_color, dword_t bg_color);
dword_t system_call_text_mode_set_cursor_pixel_character(dword_t character);
dword_t system_call_text_mode_set_cursor_pixel_ch_color(dword_t ch_color);
dword_t system_call_text_mode_set_cursor_pixel_bg_color(dword_t bg_color);
dword_t system_call_text_mode_set_cursor_pixel(dword_t character, dword_t ch_color, dword_t bg_color);
void text_mode_move_cursor_one_column_forward(void);
dword_t system_call_text_mode_put_character_on_cursor(dword_t character);
dword_t system_call_text_mode_put_pixel_on_cursor(dword_t character, dword_t ch_color, dword_t bg_color);
dword_t system_call_text_mode_get_pixel_character(dword_t column, dword_t line);
dword_t system_call_text_mode_get_pixel_ch_color(dword_t column, dword_t line);
dword_t system_call_text_mode_get_pixel_bg_color(dword_t column, dword_t line);
dword_t system_call_text_mode_get_cursor_pixel_character(void);
dword_t system_call_text_mode_get_cursor_pixel_ch_color(void);
dword_t system_call_text_mode_get_cursor_pixel_bg_color(void);

dword_t system_call_text_mode_print(dword_t column, dword_t line, byte_t *string);
dword_t system_call_text_mode_print_with_ch_color(dword_t column, dword_t line, byte_t *string, dword_t ch_color);
dword_t system_call_text_mode_print_with_bg_color(dword_t column, dword_t line, byte_t *string, dword_t bg_color);
dword_t parse_digits_from_int_number(byte_t *number_string, int number);
dword_t parse_digits_from_float_number(byte_t *number_string, float number);
void parse_digits_from_hex_number(byte_t *number_string, int number, byte_t number_of_digits);
dword_t system_call_text_mode_print_number(dword_t column, dword_t line, int number);
dword_t system_call_text_mode_print_number_with_ch_color(dword_t column, dword_t line, dword_t number, dword_t ch_color);
dword_t system_call_text_mode_print_number_with_bg_color(dword_t column, dword_t line, dword_t number, dword_t bg_color);
dword_t system_call_text_mode_print_float_number(dword_t column, dword_t line, float number);
dword_t system_call_text_mode_print_float_number_with_ch_color(dword_t column, dword_t line, float number, dword_t ch_color);
dword_t system_call_text_mode_print_float_number_with_bg_color(dword_t column, dword_t line, float number, dword_t bg_color);
dword_t system_call_text_mode_print_hex_number(dword_t column, dword_t line, dword_t number, dword_t number_of_digits);
dword_t system_call_text_mode_print_hex_number_with_ch_color(dword_t column, dword_t line, dword_t number, dword_t number_of_digits, dword_t ch_color);
dword_t system_call_text_mode_print_hex_number_with_bg_color(dword_t column, dword_t line, dword_t number, dword_t number_of_digits, dword_t bg_color);
dword_t system_call_text_mode_print_on_cursor(byte_t *string);
dword_t system_call_text_mode_print_number_on_cursor(int number);
dword_t system_call_text_mode_print_float_number_on_cursor(float number);
dword_t system_call_text_mode_print_hex_number_on_cursor(dword_t number, dword_t number_of_digits);
#define PARAMETER_ALIGN_LEFT 0
#define PARAMETER_ALIGN_CENTER 1
#define PARAMETER_ALIGN_RIGHT 2
dword_t system_call_text_mode_print_to_block(dword_t column, dword_t line, dword_t width, dword_t align, byte_t *string);

dword_t system_call_text_mode_set_character_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t character);
dword_t system_call_text_mode_set_ch_color_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t ch_color);
dword_t system_call_text_mode_set_bg_color_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t bg_color);
dword_t system_call_text_mode_set_colors_on_block(dword_t column, dword_t line, dword_t width, dword_t height, dword_t ch_color, dword_t bg_color);
dword_t system_call_text_mode_draw_full_square(dword_t column, dword_t line, dword_t width, dword_t height, dword_t color);
dword_t system_call_text_mode_draw_empty_square(dword_t column, dword_t line, dword_t width, dword_t height, dword_t color);

dword_t system_call_wait_for_keyboard(void);
dword_t system_call_get_key_value(void);
dword_t system_call_get_key_unicode_value(void);
dword_t system_call_get_actual_key_value(void);
dword_t system_call_get_actual_key_unicode_value(void);
dword_t system_call_is_this_key_pressed(dword_t key_value);

dword_t system_call_text_mode_cursor_line_input(dword_t column, dword_t line, dword_t number_of_chars);
dword_t system_call_get_input_string_pointer(void);
dword_t system_call_get_input_string_number(void);
float system_call_get_input_string_float_number(void);

dword_t system_call_get_random_number(int first_value, int last_value, int step);
dword_t system_call_wait(dword_t milliseconds);
dword_t system_call_compare_memory(byte_t *memory_1, byte_t *memory_2, dword_t size);
dword_t system_call_move_execution_to_method(void (*method)(unsigned int (*system_call)(unsigned int call, unsigned int args[])), dword_t calls_per_second);
dword_t system_call_exit_execution_of_method(void);