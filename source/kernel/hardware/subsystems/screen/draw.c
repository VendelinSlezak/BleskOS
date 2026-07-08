/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/font.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>

/* local variables */
uint32_t show_mouse_cursor = true;
const uint8_t *cursor_data = 
"B\n"
"BB\n"
"BWB\n"
"BWWB\n"
"BWWWB\n"
"BWWWWB\n"
"BWWWWWB\n"
"BWWWWWWB\n"
"BWWWWWWWB\n"
"BWWWWWWWWB\n"
"BWWWWWWWWB\n"
"BWWWWWWBB\n"
"BWWWWBB\n"
"BWWBB\n"
"BBB\n";

/* functions */
calculated_area_t calculate_area_in_part(screen_part_t *part, int x, int y, int width, int height) {
    calculated_area_t area;
    area.drawable = false;

    int part_height = (int) part->height;
    int part_width = (int) part->width;

    area.top = y;
    if(area.top >= part_height) {
        return area;
    }
    area.bottom = y + height;
    if(area.bottom <= 0) {
        return area;
    }

    area.first_line = 0;
    if(area.top < 0) {
        area.first_line = (area.top * -1);
        area.top = 0;
    }
    area.last_line = height;
    if(area.bottom >= part_height) {
        area.last_line = height - (area.bottom - part_height);
        area.bottom = part_height;
    }
    area.height = area.last_line - area.first_line;

    area.left = x;
    if(area.left >= part_width) {
        return area;
    }
    area.right = x + width;
    if(area.right <= 0) {
        return area;
    }

    area.first_column = 0;
    if(area.left < 0) {
        area.first_column = (area.left * -1);
        area.left = 0;
    }
    area.last_column = width;
    if(area.right >= part_width) {
        area.last_column = width - (area.right - part_width);
        area.right = part_width;
    }
    area.width = area.last_column - area.first_column;

    area.drawable = true;
    return area;
}

screen_part_buffer_t *alloc_screen_part_buffer(void *screen, uint32_t screen_width, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    screen_part_buffer_t *buffer = kalloc(sizeof(screen_part_buffer_t) + (width * height * sizeof(uint32_t)));
    buffer->screen = screen;
    buffer->screen_ptr = (uint32_t *) ((uint32_t)screen + (y * screen_width * 4) + (x * 4));
    buffer->screen_width = screen_width;
    buffer->x = x;
    buffer->y = y;
    buffer->width = width;
    buffer->height = height;
    return buffer;
}

void move_screen_part_buffer(screen_part_buffer_t **buffer_ptr, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    screen_part_buffer_t *buffer = *buffer_ptr;
    if((width * height) != (buffer->width * buffer->height)) {
        buffer = krealloc(buffer, sizeof(screen_part_buffer_t) + (width * height * sizeof(uint32_t)));
        *buffer_ptr = buffer;
    }
    buffer->screen_ptr = (uint32_t *) ((uint32_t)buffer->screen + (y * buffer->screen_width * 4) + (x * 4));
    buffer->x = x;
    buffer->y = y;
    buffer->width = width;
    buffer->height = height;
}

void copy_from_screen_to_buffer(screen_part_buffer_t *buffer) {
    uint32_t *src = buffer->screen_ptr;
    uint32_t *dst = (uint32_t *) &buffer->buffer;
    uint32_t height = buffer->height;
    uint32_t width = buffer->width;
    uint32_t screen_width = buffer->screen_width;
    for(int i = 0; i < height; i++) {
        uint32_t *line_src = src;
        for(int j = 0; j < width; j++) {
            *dst++ = *line_src++;
        }
        src += screen_width;
    }
}

void copy_from_buffer_to_screen(screen_part_buffer_t *buffer) {
    uint32_t *src = (uint32_t *) &buffer->buffer;
    uint32_t *dst = buffer->screen_ptr;
    uint32_t height = buffer->height;
    uint32_t width = buffer->width;
    uint32_t screen_width = buffer->screen_width;
    for(int i = 0; i < height; i++) {
        uint32_t *line_dst = dst;
        for(int j = 0; j < width; j++) {
            *line_dst++ = *src++;
        }
        dst += screen_width;
    }
}

void draw_square(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    uint32_t output_width = get_output_width();

    if((color & 0xFF000000) == 0xFF000000) {
        uint32_t *dst = (uint32_t *) ((uint32_t)active_view->buffer + (y * output_width * 4) + (x * 4));
        for(uint32_t i = 0; i < height; i++) {
            for(uint32_t j = 0; j < width; j++) {
                dst[j] = color;
            }
            dst = (uint32_t *) ((uint32_t)dst + (output_width * 4));
        }
    }
    else {
        uint8_t *dst = (uint8_t *) ((uint32_t)active_view->buffer + (y * output_width * 4) + (x * 4));
        uint8_t alpha = (uint8_t)(color >> 24);
        uint8_t alpha_inverted = (255 - alpha);
        uint8_t red = (uint8_t)(color >> 16);
        uint8_t green = (uint8_t)(color >> 8);
        uint8_t blue = (uint8_t)(color >> 0);
        for(uint32_t i = 0; i < height; i++) {
            uint8_t *line_dst = dst;
            for(uint32_t j = 0; j < width; j++, line_dst += 4) {
                int temp = (red * alpha) + (line_dst[2] * alpha_inverted);
                line_dst[2] = ((temp + 1 + (temp >> 8)) >> 8);

                temp = (green * alpha) + (line_dst[1] * alpha_inverted);
                line_dst[1] = ((temp + 1 + (temp >> 8)) >> 8);

                temp = (blue * alpha) + (line_dst[0] * alpha_inverted);
                line_dst[0] = ((temp + 1 + (temp >> 8)) >> 8);
            }
            dst = (uint8_t *) ((uint32_t)dst + (output_width * 4));
        }
    }
}

void draw_square_in_part(screen_part_t *part, int x, int y, int width, int height, uint32_t color) {
    calculated_area_t area = calculate_area_in_part(part, x, y, width, height);
    if(area.drawable == false) {
        return;
    }
    draw_square(part->x + area.left, part->y + area.top, area.width, area.height, color);
}

void draw_bitmap(screen_part_t *part, uint32_t x, uint32_t y, uint32_t *data, uint32_t width, uint32_t height) {
    calculated_area_t area = calculate_area_in_part(part, x, y, width, height);
    if(area.drawable == false) {
        return;
    }
    uint32_t top = area.top;
    uint32_t left = area.left;
    uint32_t showed_width = area.width;
    uint32_t showed_height = area.height;
    uint32_t first_line = area.first_line;
    uint32_t first_column = area.first_column;

    view_t *view = part->view;
    uint32_t view_width = view->width;
    uint32_t *buffer = (uint32_t *) view->buffer;
    top += part->y;
    left += part->x;
    uint32_t *dst = (uint32_t *) &buffer[(top * view_width) + left];
    uint32_t *src = (uint32_t *) &data[(first_line * width) + first_column];
    for(uint32_t i = 0; i < showed_height; i++) {
        uint32_t *line_dst = dst;
        uint32_t *line_src = src;
        for(int j = 0; j < showed_width; j++) {
            if((*line_src & 0xFF000000) == 0x00000000) {
                line_src++;
                line_dst++;
            }
            else {
                *line_dst++ = *line_src++;
            }
        }
        dst += view_width;
        src += width;
    }
}

void draw_mouse_cursor(uint32_t x, uint32_t y) {
    if(show_mouse_cursor == false) {
        return;
    }
    redraw_part_of_screen_without_mouse(mouse_cursor_x, mouse_cursor_y, active_view->buffer, get_output_width(), mouse_cursor_x, mouse_cursor_y, 16, 16);
    uint32_t bpp = get_output_bpp();
    void *dst = (void *) ((uint32_t)get_output_linear_frame_buffer() + (y * get_output_bytes_per_line()) + (x * (bpp / 8)));
    uint32_t i = 0;
    uint32_t line = y;
    uint32_t column = 0;
    uint32_t output_width = get_output_width();
    uint32_t output_height = get_output_height();
    while(cursor_data[i] != 0) {
        if(cursor_data[i] == 'B') {
            if((x + column) >= output_width) {
                i++;
                continue;
            }
            if(bpp == 32) {
                ((uint32_t *)dst)[column] = 0xFF000000;
            }
            else if(bpp == 24) {
                ((uint8_t *)dst)[column * 3] = 0x00;
                ((uint8_t *)dst)[column * 3 + 1] = 0x00;
                ((uint8_t *)dst)[column * 3 + 2] = 0x00;
            }
            else if(bpp == 16 || bpp == 15) {
                ((uint16_t *)dst)[column] = 0x0000;
            }
            else if(bpp == 8) {
                ((uint8_t *)dst)[column] = 0x00;
            }
        }
        else if(cursor_data[i] == 'W') {
            if((x + column) >= output_width) {
                i++;
                continue;
            }
            if(bpp == 32) {
                ((uint32_t *)dst)[column] = 0xFFFFFFFF;
            }
            else if(bpp == 24) {
                ((uint8_t *)dst)[column * 3] = 0xFF;
                ((uint8_t *)dst)[column * 3 + 1] = 0xFF;
                ((uint8_t *)dst)[column * 3 + 2] = 0xFF;
            }
            else if(bpp == 16 || bpp == 15) {
                ((uint16_t *)dst)[column] = 0xFFFF;
            }
            else if(bpp == 8) {
                ((uint8_t *)dst)[column] = 0xFF;
            }
        }
        else if(cursor_data[i] == '\n') {
            line++;
            if(line >= (output_height - 1)) {
                break;
            }
            dst = (void *) ((uint32_t)dst + get_output_bytes_per_line());
            i++;
            column = 0;
            continue;
        }
        i++;
        column++;
    }
    mouse_cursor_x = x;
    mouse_cursor_y = y;
}