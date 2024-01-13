//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define TEXT_AREA_NORMAL 0
#define TEXT_AREA_NORMAL_DARK 1
#define TEXT_AREA_INPUT_LINE 2
#define TEXT_AREA_NUMBER_INPUT 3

#define TEXT_AREA_INFO_MEMORY 0
#define TEXT_AREA_INFO_NUM_OF_CHARS 1
#define TEXT_AREA_INFO_MEMORY_LAST_BYTE 2
#define TEXT_AREA_INFO_X 3
#define TEXT_AREA_INFO_Y 4
#define TEXT_AREA_INFO_WIDTH 5
#define TEXT_AREA_INFO_HEIGTH 6
#define TEXT_AREA_INFO_FIRST_SHOW_LINE 7
#define TEXT_AREA_INFO_FIRST_SHOW_COLUMN 8
#define TEXT_AREA_INFO_CURSOR_POSITION 9
#define TEXT_AREA_INFO_SELECTED_AREA_POINTER 10
#define TEXT_AREA_INFO_TYPE 11
#define TEXT_AREA_INFO_NUMBER_OF_LINES 12
#define TEXT_AREA_INFO_NUMBER_OF_COLUMNS 13
#define TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION 14
#define TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE 15
#define TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION 16
#define TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE 17
#define TEXT_AREA_INFO_FLAGS 18
#define TEXT_AREA_INFO_REAL_WIDTH 19
#define TEXT_AREA_INFO_REAL_HEIGTH 20
#define TEXT_AREA_INFO_REDRAW_X 21
#define TEXT_AREA_INFO_REDRAW_Y 22
#define TEXT_AREA_INFO_REDRAW_WIDTH 23
#define TEXT_AREA_INFO_REDRAW_HEIGTH 24
#define TEXT_AREA_INFO_CHANGES_LIST 25
#define TEXT_AREA_INFO_CHANGES_LIST_POINTER 26
#define TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER 27

#define TEXT_AREA_TEXT_CHANGE_FLAG 0x1
#define TEXT_AREA_COPY_TOGGLE_FLAG 0x2

#define TEXT_AREA_NO_CHANGE 0
#define TEXT_AREA_TEXT_CHANGE 1

#define TEXT_AREA_MAX_NUMBER_OF_CHANGES_IN_LIST 10000
#define TEXT_AREA_CHANGE_ADD_CHAR 1
#define TEXT_AREA_CHANGE_REMOVE_CHAR_BY_DELETE 2
#define TEXT_AREA_CHANGE_REMOVE_CHAR_BY_BACKSPACE 3
#define TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0 4
#define TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1 5
#define TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0 6
#define TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1 7

dword_t text_area_absoulte_cursor_line = 0, text_area_absoulte_cursor_column = 0, text_area_change_type = 0, text_area_mouse_cursor_char_memory = 0;
dword_t text_area_copy_memory = 0, text_area_copy_memory_length = 0, text_area_change_command = 0;

dword_t create_text_area(dword_t type, dword_t length_in_chars, dword_t x, dword_t y, dword_t width, dword_t height);
void delete_text_area(dword_t text_area_info_mem);
void draw_text_area(dword_t text_area_info_mem);
void redraw_text_area(dword_t text_area_info_mem);
void text_area_set_show_line_and_column(dword_t text_area_info_mem);
void text_area_calculate_cursor_position(dword_t text_area_info_mem);
void text_area_calculate_number_of_lines_and_columns(dword_t text_area_info_mem);
void text_area_keyboard_event(dword_t text_area_info_mem);
void text_area_mouse_event(dword_t text_area_info_mem);
void text_area_disable_cursor(dword_t text_area_info_mem);
void text_area_move_cursor_to_start(dword_t text_area_info_mem);
void text_area_add_change_to_list(dword_t text_area_info_mem, dword_t change_type, dword_t unicode_char, dword_t cursor_position);
void text_area_undo(dword_t text_area_info_mem);
void text_area_redo(dword_t text_area_info_mem);
dword_t text_area_return_written_number_in_dword(dword_t text_area_info_mem);