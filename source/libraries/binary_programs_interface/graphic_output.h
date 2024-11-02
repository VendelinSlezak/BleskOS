//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct text_mode_screen_char_t {
 word_t character;
 dword_t character_color;
 dword_t background_color;
}__attribute__((packed));

struct text_mode_screen_char_t *text_mode_screen;

dword_t text_mode_screen_lines, text_mode_screen_columns, text_mode_screen_pixels;
dword_t text_mode_cursor_line, text_mode_cursor_column;
dword_t text_mode_x_on_screen, text_mode_y_on_screen;

byte_t text_mode_cursor_is_visible;
byte_t text_mode_show_changes_on_screen;

void redraw_text_mode_screen(void);
void redraw_text_mode_pixel(dword_t column, dword_t line);