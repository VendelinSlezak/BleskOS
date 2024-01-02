//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define CHAR_COMMA_IN_MIDDLE_OF_LINE 31

dword_t binary_font_memory = 0;
dword_t debug_line = 0;

void draw_char(word_t char_val, dword_t x, dword_t y, dword_t color);
void draw_part_of_char(word_t char_val, dword_t x, dword_t y, dword_t first_char_column, dword_t last_char_column, dword_t first_char_line, dword_t last_char_line, dword_t color);
void print(byte_t *string, dword_t x, dword_t y, dword_t color);
void print_ascii(byte_t *string, dword_t x, dword_t y, dword_t color);
void print_unicode(word_t *string, dword_t x, dword_t y, dword_t color);
void print_var(dword_t value, dword_t x, dword_t y, dword_t color);
void print_hex(dword_t value, dword_t x, dword_t y, dword_t color);

void pstr(byte_t *string);
void pvar(dword_t value);
void phex(dword_t value);
