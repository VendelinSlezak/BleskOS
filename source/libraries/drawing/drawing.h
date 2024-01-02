//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t global_color = 0;
dword_t pen_width = 1;
dword_t pen_img_pointer = 0;

dword_t draw_x = 0;
dword_t draw_y = 0;

dword_t fill_first_stack = 0;
dword_t fill_second_stack = 0;

void clear_screen(dword_t color);
void set_pen_width(dword_t width);
void set_pixel_shape_circle(void);
void set_pixel_shape_square(void);
void draw_pixel(dword_t x, dword_t y);
void draw_straigth_line(dword_t x, dword_t y, dword_t length);
void draw_straigth_column(dword_t x, dword_t y, dword_t heigth);
void draw_line(int x0, int y0, int x1, int y1, dword_t color);
void draw_quadratic_bezier(int x0, int y0, int x1, int y1, int x2, int y2, dword_t color);
void draw_empty_square(dword_t x, dword_t y, dword_t width, dword_t heigth, dword_t color);
void draw_full_square(dword_t x, dword_t y, dword_t width, dword_t heigth, dword_t color);
void draw_empty_circle_point(dword_t x, dword_t y);
void draw_empty_circle(dword_t x, dword_t y, dword_t radius, dword_t color);
void draw_full_circle_line(dword_t x, dword_t y);
void draw_full_circle(dword_t x, dword_t y, dword_t radius, dword_t color);
void draw_empty_ellipse(int x0, int y0, int x1, int y1, dword_t color);
void draw_parts_of_empty_ellipse(byte_t parts, int x0, int y0, int x1, int y1, dword_t color);
void draw_full_ellipse(int x0, int y0, int x1, int y1, dword_t color);
void fill_area(dword_t x, dword_t y, dword_t color);
