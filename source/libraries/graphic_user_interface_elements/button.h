//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define BUTTON_COLOR 0x0782D1

void draw_button_with_specific_color(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height, dword_t color);
void draw_button(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height);
void draw_button_with_specific_color_and_click_zone(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height, dword_t color, dword_t click_zone);
void draw_button_with_click_zone(byte_t *string, dword_t x, dword_t y, dword_t width, dword_t height, dword_t click_zone);