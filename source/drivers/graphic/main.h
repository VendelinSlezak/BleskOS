//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define RED 0xFF0000
#define TRANSPARENT_COLOR 0xFF000000

dword_t screen_mem = 0;
dword_t graphic_screen_lfb = 0;
dword_t graphic_screen_x = 0;
dword_t graphic_screen_y = 0;
dword_t graphic_screen_bpp = 0;
dword_t graphic_screen_bytes_per_line = 0;
dword_t screen_bytes_per_line = 0;
dword_t graphic_screen_x_center = 0;
dword_t graphic_screen_y_center = 0;

dword_t save_screen_mem = 0;
dword_t save_screen_x = 0;
dword_t save_screen_y = 0;
dword_t save_screen_bytes_per_line = 0;

void initalize_graphic(void);
void draw_mouse_to_double_framebuffer_before_redraw(void);
void restore_mouse_area_in_double_framebuffer_after_redraw(void);
void redraw_screen(void);
void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t heigth);
void screen_save_variabiles(void);
void screen_restore_variables(void);