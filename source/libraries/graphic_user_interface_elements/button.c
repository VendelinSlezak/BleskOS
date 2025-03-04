//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_button_with_specific_color(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height, dword_t color) {
 draw_full_square(x, y, width, height, color);
 
 byte_t string_length=0;
 for(int i=0; i<100; i++) {
  if(string[i]==0) {
   break;
  }
  string_length++;
 }
 
 print(string, x+(width/2)-(string_length*4), y+((height-8)/2), BLACK);
}

void draw_button(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height) {
 draw_button_with_specific_color(string, x, y, width, height, BUTTON_COLOR);
}

void draw_button_with_specific_color_and_click_zone(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height, dword_t color, dword_t click_zone) {
 draw_button_with_specific_color(string, x, y, width, height, color);
 add_zone_to_click_board(x, y, width, height, click_zone);
}

void draw_button_with_click_zone(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height, dword_t click_zone) {
 draw_button(string, x, y, width, height);
 add_zone_to_click_board(x, y, width, height, click_zone);
}