//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_MONITORS 4
struct monitor_info_t {
    dword_t connected;
    dword_t card_number;

    struct edid_t edid;

    dword_t width;
    dword_t height;
    dword_t bpp;
    dword_t x_center;
    dword_t y_center;
    byte_t *linear_frame_buffer;
    dword_t linear_frame_buffer_bpl;
    byte_t *double_buffer;
    dword_t double_buffer_bpl;
    void (*change_mode)(dword_t monitor_number, byte_t value);

    dword_t backlight;
    void (*change_backlight)(dword_t monitor_number, byte_t value);
};
struct monitor_info_t monitors[MAX_NUMBER_OF_MONITORS];

#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define RED 0xFF0000
#define BLESKOS_GREEN 0x00C000
#define TRANSPARENT_COLOR 0x88000000

// TODO: rewrite this concept to something better
byte_t *save_screen_double_buffer_memory_pointer;
dword_t save_screen_width;
dword_t save_screen_height;
dword_t save_screen_double_buffer_bytes_per_line;

void initalize_graphic(void);
void redraw_framebuffer_32_bpp(void);
void redraw_framebuffer_32_bpp_without_padding(void);
void redraw_framebuffer_24_bpp(void);
void redraw_framebuffer_24_bpp_without_padding(void);
void redraw_framebuffer_16_bpp(void);
void redraw_framebuffer_16_bpp_without_padding(void);
void redraw_screen(void);
void redraw_part_of_framebuffer_32_bpp(dword_t x, dword_t y, dword_t width, dword_t height);
void redraw_part_of_framebuffer_24_bpp(dword_t x, dword_t y, dword_t width, dword_t height);
void redraw_part_of_framebuffer_16_bpp(dword_t x, dword_t y, dword_t width, dword_t height);
void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t height);
void add_mouse_to_screen_double_buffer(void);
void remove_mouse_from_screen_double_buffer(void);
void screen_save_variables(void);
void screen_restore_variables(void);

void (*redraw_framebuffer)(void);
void (*redraw_part_of_framebuffer)(dword_t x, dword_t y, dword_t width, dword_t height);