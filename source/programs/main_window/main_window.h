//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MW_DOCUMENT_EDITOR 1
#define MW_TEXT_EDITOR 2
#define MW_GRAPHIC_EDITOR 3
#define MW_MEDIA_VIEWER 4
#define MW_INTERNET_BROWSER 5
#define MW_FILE_MANAGER 6
#define MW_RUN_BINARY_PROGRAMS 7

#define MW_CALCULATOR 8
#define MW_SCREENSHOOTER 9

#define MW_SYSTEM_BOARD 10
#define MW_CHANGE_KEYBOARD_LAYOUT 11
#define MW_SHUTDOWN 12

#define MW_MONITOR_BACKLIGHT 100
#define MW_SOUND_VOLUME 101
#define MW_ENABLE_DISABLE_TOUCHPAD 102
#define MW_EJECT_OPTICAL_DISK_DRIVE 103

#define MW_KEYBOARD_LAYOUT_ENGLISH 1
#define MW_KEYBOARD_LAYOUT_SLOVAK 2

#define MW_RUN_BINARY_PROGRAMS_OPEN_FILE 1
#define MW_RUN_BINARY_PROGRAMS_GO_BACK 2

struct main_window_click_zone {
 byte_t type;
 word_t up;
 word_t down;
 word_t left;
 word_t right;
}__attribute__((packed));
struct main_window_click_zone main_window_click_zones[10];

dword_t main_window_zones_pointer;
dword_t bleskos_main_window_drawing_line, bleskos_main_window_drawing_column;
byte_t bleskos_main_window_time_redraw = 0;

void bleskos_main_window_print_item(byte_t *string);
void bleskos_main_window_draw_item(byte_t *string, dword_t color, byte_t type);
void bleskos_main_window_redraw_time(void);
void bleskos_main_window_redraw_sound_volume(void);
void bleskos_main_window_draw_background(void);
void bleskos_main_window_redraw(void);
void bleskos_main_window(void);
void bleskos_main_window_change_keyboard_layout(void);
void bleskos_main_window_shutdown(void);
void bleskos_main_window_enable_disable_touchpad(void);
void bleskos_main_window_run_binary_programs(void);