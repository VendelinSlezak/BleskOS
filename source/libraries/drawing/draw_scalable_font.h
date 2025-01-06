//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SF_CHAR_END 0
#define SF_CHAR_LINE 1
#define SF_CHAR_BEZIER 2
#define SF_CHAR_FULL_ELLIPSE 3
#define SF_CHAR_EMPTY_ELLIPSE 4
#define SF_CHAR_JUMP 5

#define SF_EMPHASIS_NONE 0
#define SF_EMPHASIS_BOLD 0x1
#define SF_EMPHASIS_UNDERLINE 0x2
#define SF_EMPHASIS_STRIKE 0x4

#define SF_DOT_IN_MIDDLE_OF_LINE 127
#define SF_NBSP 0xA0

word_t *scalable_font_pixel_distance;

dword_t scalable_font_pixel_distance_mem, scalable_font_char_mem, scalable_font_x, scalable_font_y, scalable_font_char_size, scalable_font_char_emphasis;
dword_t scalable_font_char_board_mem, scalable_font_predraw_size_10_mem;

void initalize_scalable_font(void);
void set_scalable_char_size(dword_t size);
void draw_scalable_char_without_emphasis(word_t char_val, dword_t x, dword_t y, dword_t color);
void draw_scalable_char(word_t char_val, dword_t x, dword_t y, dword_t color);
void draw_part_of_scalable_char(word_t char_val, dword_t x, dword_t y, dword_t char_column, dword_t char_line, dword_t char_width, dword_t char_height, dword_t color);
void scalable_font_print(byte_t *string, dword_t x, dword_t y, dword_t color);
