//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define IMAGE_SIZE_OF_INFO_IN_BYTES 100

#define IMAGE_INFO_WIDTH 0
#define IMAGE_INFO_HEIGHT 1
#define IMAGE_INFO_DRAW_X 2
#define IMAGE_INFO_DRAW_Y 3
#define IMAGE_INFO_DRAW_WIDTH 4
#define IMAGE_INFO_DRAW_HEIGHT 5
#define IMAGE_INFO_SCREEN_X 6
#define IMAGE_INFO_SCREEN_Y 7
#define IMAGE_INFO_REAL_WIDTH 8
#define IMAGE_INFO_REAL_HEIGHT 9
#define IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE 10
#define IMAGE_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION 11
#define IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE 12
#define IMAGE_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION 13

dword_t image_resize_width_array_mem, image_resize_height_array_mem;
dword_t image_mouse_x, image_mouse_y;

void initalize_image_operations(void);
void copy_raw_image_data(dword_t source_memory, dword_t source_width, dword_t source_x, dword_t source_y, dword_t image_width, dword_t image_height, dword_t dest_memory, dword_t dest_width, dword_t dest_x, dword_t dest_y);
void copy_and_resize_raw_image_data(dword_t source_memory, dword_t source_width, dword_t source_height, dword_t resized_source_width, dword_t resized_source_height, dword_t source_x, dword_t source_y, dword_t image_width, dword_t image_height, dword_t dest_memory, dword_t dest_width, dword_t dest_x, dword_t dest_y);
dword_t create_image(dword_t width, dword_t height);
void delete_image(dword_t image_info_mem);
dword_t get_image_data_memory(dword_t image_info_mem);
void draw_image(dword_t image_info_mem);
void draw_resized_image(dword_t image_info_mem);
void calculate_image_scrollbars(dword_t image_info_mem);
void get_mouse_coordinates_on_image(dword_t image_info_mem);
void image_reverse_horizontally(dword_t image_info_mem);
void image_reverse_vertically(dword_t image_info_mem);
void image_turn_left(dword_t image_info_mem);
void image_turn_right(dword_t image_info_mem);
