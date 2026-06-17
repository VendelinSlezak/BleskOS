/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/interrupt.h>
#include <libraries/screen.h>
#include <libraries/bitmap_font.h>

typedef struct syslib {
    uint32_t version;
    uint32_t num_of_logical_processors;
    uint32_t userspace_start;
    uint32_t userspace_size;

    uint32_t window_width;
    uint32_t window_height;

    void (*initialize)(struct syslib *syslib_ptr);

    void (*log)(char *string);
    void (*logf)(char *string, ...);
    
    uint64_t (*get_time_in_microseconds)(void);

    void *(*malloc)(uint32_t size);
    void *(*calloc)(uint32_t number, uint32_t size);
    void *(*realloc)(void *ptr, uint32_t size);
    void (*free)(void *ptr);

    screen_buffer_t *(*create_window)(void);
    void (*redraw_window)(void);

    screen_buffer_t *(*create_screen_buffer)(uint32_t width, uint32_t height);
    void (*destroy_screen_buffer)(screen_buffer_t *buffer);

    bitmap_font_t *(*load_bitmap_font)(void *file, uint32_t size);
    void (*draw_bitmap_char)(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, uint32_t character, uint32_t color);
    void (*draw_bitmap_string)(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, char *string, uint32_t color);
    void (*destroy_bitmap_font)(bitmap_font_t *font);

    screen_buffer_t *(*initialize_gui)(void);
    void (*add_canvas_component)(uint32_t area_number, void (*redraw)(uint32_t screen_x, uint32_t screen_y, uint32_t screen_width, uint32_t screen_height, uint32_t x_offset, uint32_t y_offset));
    void (*register_pressed_key_event_handler)(uint32_t key, void (*handler)(void));
    void (*redraw_gui)(void);

    /*

    void (*copy_screen_buffer)(screen_buffer_t *dst, uint32_t dst_x, uint32_t dst_y, screen_buffer_t *src, uint32_t src_x, uint32_t src_y, uint32_t width, uint32_t height);

    void (*redraw_part_of_window)(uint32_t window_x, uint32_t window_y, screen_buffer_t *buffer, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height);

    */
} syslib_t;

extern syslib_t system_syslib;