/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <userspace_library/libc/math.h>
#include <userspace_library/logging.h>

/* global variables */
uint32_t *screen_buffer;
int screen_width;
int screen_height;

/* functions */
void update_screen_buffer(uint32_t *new_buffer, int new_width, int new_height) {
    screen_buffer = new_buffer;
    screen_width = new_width;
    screen_height = new_height;
}

position_t calculate_real_position_in_box(buffer_box_t *box, int x, int y, int width, int height) {
    position_t pos;
    pos.drawable = false;
    int box_width = box->width;
    int box_height = box->height;

    pos.top = y;
    if(pos.top >= box_height) {
        return pos;
    }
    pos.bottom = y + height;
    if(pos.bottom <= 0) {
        return pos;
    }

    pos.first_line = 0;
    if(pos.top < 0) {
        pos.first_line = (pos.top * -1);
        pos.top = 0;
    }
    pos.last_line = height;
    if(pos.bottom >= box_height) {
        pos.last_line = height - (pos.bottom - box_height);
        pos.bottom = box_height;
    }
    pos.height = pos.last_line - pos.first_line;

    pos.left = x;
    if(pos.left >= box_width) {
        return pos;
    }
    pos.right = x + width;
    if(pos.right <= 0) {
        return pos;
    }

    pos.first_column = 0;
    if(pos.left < 0) {
        pos.first_column = (pos.left * -1);
        pos.left = 0;
    }
    pos.last_column = width;
    if(pos.right >= box_width) {
        pos.last_column = width - (pos.right - box_width);
        pos.right = box_width;
    }
    pos.width = pos.last_column - pos.first_column;

    pos.drawable = true;
    return pos;
}

void draw_pixel(buffer_box_t *box, int x, int y, uint32_t color) {
    if(x < 0 || x >= box->width || y < 0 || y >= box->height) {
        return;
    }
    box->buffer[((box->y + y) * box->buffer_width) + (box->x + x)] = color;
}

void draw_square(buffer_box_t *box, int x, int y, int width, int height, uint32_t color) {
    position_t pos = calculate_real_position_in_box(box, x, y, width, height);
    if(pos.drawable == false) {
        return;
    }
    for(int i = pos.top; i < pos.bottom; i++) {
        for(int j = pos.left; j < pos.right; j++) {
            draw_pixel(box, j, i, color);
        }
    }
}

void draw_line(buffer_box_t *box, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx =  abs(x1 - x0);
    int sx = (x0 < x1 ? 1 : -1);
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1 ? 1 : -1);
    int err = (dx + dy);
    int e2;

    if(x0 == x1 && y0 == y1) {
        draw_pixel(box, x0, y0, color);
        return;
    }
 
    while(true) {
        if(x0 == x1 && y0 == y1) {
            return;
        }
        if(x0 > box->width || y0 > box->height) {
            return;
        }

        if(x0 >= 0 && y0 >= 0) {
            draw_pixel(box, x0, y0, color);
        }
        
        e2 = (2 * err);
        if(e2 >= dy) { 
            err += dy; 
            x0 += sx; 
        }
        if(e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void copy_buffer_between_boxes(buffer_box_t *from, buffer_box_t *to) {
    uint32_t *buffer_from = (uint32_t *) ((uint32_t) from->buffer + (from->y * from->buffer_width * 4) + (from->x * 4));
    uint32_t *buffer_to = (uint32_t *) ((uint32_t) to->buffer + (to->y * to->buffer_width * 4) + (to->x * 4));
    uint32_t copy_width = (from->width < to->width) ? from->width : to->width;
    uint32_t copy_height = (from->height < to->height) ? from->height : to->height;
    for(uint32_t i = 0; i < copy_height; i++) {
        for(uint32_t j = 0; j < copy_width; j++) {
            buffer_to[i * to->buffer_width + j] = buffer_from[i * from->buffer_width + j];
        }
    }
}