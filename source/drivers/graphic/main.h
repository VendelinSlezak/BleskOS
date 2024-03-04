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
#define BLESKOS_GREEN 0x00C000
#define TRANSPARENT_COLOR 0x88000000

#define MAX_NUMBER_OF_GRAPHIC_CARDS 10
struct graphic_card_info {
 word_t vendor_id;
 word_t device_id;
 dword_t mmio_base;
 word_t io_base;
 byte_t *linear_frame_buffer;
}__attribute__((packed));
struct graphic_card_info graphic_cards_info[MAX_NUMBER_OF_GRAPHIC_CARDS];
dword_t number_of_graphic_cards;
dword_t selected_graphic_card;
byte_t is_driver_for_graphic_card;
byte_t can_graphic_card_driver_change_backlight;

byte_t percent_of_backlight;

dword_t screen_mem;
dword_t screen_bytes_per_line;

dword_t save_screen_mem;
dword_t save_screen_x;
dword_t save_screen_y;
dword_t save_screen_bytes_per_line;

dword_t graphic_screen_lfb;
dword_t graphic_screen_x;
dword_t graphic_screen_y;
dword_t graphic_screen_bpp;
dword_t graphic_screen_bytes_per_line;
dword_t graphic_screen_x_center;
dword_t graphic_screen_y_center;

void initalize_graphic(void);
void redraw_framebuffer_32_bpp(void);
void redraw_framebuffer_24_bpp(void);
void redraw_framebuffer_16_bpp(void);
void redraw_screen(void);
void redraw_part_of_framebuffer_32_bpp(dword_t x, dword_t y, dword_t width, dword_t height);
void redraw_part_of_framebuffer_24_bpp(dword_t x, dword_t y, dword_t width, dword_t height);
void redraw_part_of_framebuffer_16_bpp(dword_t x, dword_t y, dword_t width, dword_t height);
void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t height);
void screen_save_variables(void);
void screen_restore_variables(void);
void monitor_change_backlight(byte_t value);

void (*redraw_framebuffer)(void);
void (*redraw_part_of_framebuffer)(dword_t x, dword_t y, dword_t width, dword_t height);
void (*graphic_card_driver_monitor_change_backlight)(byte_t value);