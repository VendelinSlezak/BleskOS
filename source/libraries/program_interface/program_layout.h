//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t program_layout_x = 0, program_layout_y = 0, program_layout_width = 0, program_layout_height = 0, program_layout_draw_x = 0, program_layout_draw_y = 0;

void program_layout_set_dimensions(dword_t x, dword_t y, dword_t width, dword_t height);
void program_layout_set_dimensions_window(dword_t width, dword_t height);
void program_layout_add_element(dword_t height);
void program_layout_add_text_line(void);
void program_layout_add_button(void);

#define PROGRAM_ELEMENT_LAYOUT_MAX_NUM_OF_AREAS 5

#define FIRST_AREA 0
#define SECOND_AREA 1
#define THIRD_AREA 2
#define FOURTH_AREA 3
#define FIFTH_AREA 4

#define IN_PIXELS 0
#define IN_PERCENTS 1

#define ELEMENT_LEFT_ALIGNMENT 0
#define ELEMENT_MIDDLE_ALIGNMENT 1
#define ELEMENT_RIGHT_ALIGNMENT 2

struct program_element_layout_area_info {
 dword_t x;
 dword_t y;
 dword_t width;
 dword_t height;
 dword_t actual_element_x;
 dword_t actual_element_y;
};

struct program_element_layout_area_info *program_element_layout_areas_info;
dword_t program_element_layout_number_of_areas, program_element_layout_actual_area;
dword_t element_x, element_y, element_width, element_height;

void program_element_layout_initalize_for_program(void);
void program_element_layout_initalize_for_window(dword_t width, dword_t height);
void program_element_layout_initalize_for_area(dword_t x, dword_t y, dword_t width, dword_t height);

void program_element_layout_split_area_vertically(byte_t area_number, byte_t unit_type, dword_t number);
void program_element_layout_split_area_horizontally(byte_t area_number, byte_t unit_type, dword_t number);

void program_element_layout_add_border_to_area(byte_t area_number, dword_t border);

void program_element_layout_calculate_element_position(byte_t area_number, byte_t alignment, dword_t width, dword_t height);
void program_element_move_vertically(dword_t border);
void program_element_move_horizontally(dword_t border);

void add_label(byte_t area_number, byte_t alignment, byte_t *string);
void add_button(byte_t area_number, byte_t alignment, byte_t width, byte_t *string, dword_t click_zone);