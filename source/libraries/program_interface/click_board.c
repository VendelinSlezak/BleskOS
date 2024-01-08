//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_click_board(void) {
 click_board_memory = malloc(graphic_screen_x*graphic_screen_y*4);
}

void clear_click_board(void) {
 dword_t *click_board = (dword_t *) click_board_memory;
 
 for(int i=0; i<(graphic_screen_x*graphic_screen_y); i++) {
  *click_board = NO_CLICK;
  click_board++;
 }
}

void add_zone_to_click_board(dword_t x, dword_t y, dword_t width, dword_t height, dword_t click_value) {
 dword_t first_line_pixel_pointer = (click_board_memory + (y*screen_bytes_per_line) + (x<<2));
 dword_t *click_board = (dword_t *) first_line_pixel_pointer;
 
 for(int i=0; i<height; i++) {
  for(int j=0; j<width; j++) {
   *click_board = click_value;
   click_board++;
  }
  
  first_line_pixel_pointer += screen_bytes_per_line;
  click_board = (dword_t *) first_line_pixel_pointer;
 }
}

dword_t get_mouse_cursor_click_board_value(void) {
 dword_t *click_board = (dword_t *) (click_board_memory + (mouse_cursor_y*screen_bytes_per_line) + (mouse_cursor_x<<2));
 return *click_board;
}