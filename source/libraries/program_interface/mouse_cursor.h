//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MOUSE_CURSOR_WIDTH 7
#define MOUSE_CURSOR_HEIGHT 10

dword_t mouse_cursor_img[MOUSE_CURSOR_WIDTH*MOUSE_CURSOR_HEIGHT] = {
BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
BLACK, BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
BLACK, WHITE, BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
BLACK, WHITE, WHITE, BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
BLACK, WHITE, WHITE, WHITE, BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
BLACK, WHITE, WHITE, WHITE, WHITE, BLACK, TRANSPARENT_COLOR,
BLACK, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK,
BLACK, WHITE, WHITE, WHITE, BLACK, BLACK, TRANSPARENT_COLOR,
BLACK, WHITE, BLACK, BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
BLACK, BLACK, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR, TRANSPARENT_COLOR,
};

dword_t mouse_cursor_background;
dword_t mouse_cursor_x, mouse_cursor_y, mouse_cursor_x_click, mouse_cursor_y_click, mouse_cursor_x_dnd, mouse_cursor_y_dnd, mouse_cursor_x_previous_dnd, mouse_cursor_y_previous_dnd;

void draw_mouse_cursor(dword_t x, dword_t y);
void redraw_mouse_cursor(void);
void mouse_cursor_save_background(dword_t x, dword_t y);
void mouse_cursor_restore_background(dword_t x, dword_t y);
void move_mouse_cursor(void);
byte_t is_mouse_in_zone(dword_t up, dword_t down, dword_t left, dword_t right);
dword_t get_mouse_cursor_pixel_color(void);