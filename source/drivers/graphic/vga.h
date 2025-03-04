//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define VGA_TEXT_MODE 0
#define VGA_GRAPHIC_MODE 1

struct component_info_vga_t {
 dword_t mode_type;
 dword_t mode_number;
 dword_t width;
 dword_t height;
};

void vga_text_mode_clear_screen(byte_t color);
void vga_text_mode_put_char(byte_t line, byte_t column, byte_t character);
void vga_text_mode_print(byte_t line, byte_t column, byte_t *string);
void vga_text_mode_draw_square(byte_t line, byte_t column, byte_t width, byte_t height, byte_t color);
void vga_text_mode_move_cursor(byte_t line, byte_t column);