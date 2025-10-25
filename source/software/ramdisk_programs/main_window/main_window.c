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
#include <kernel/x86/kernel.h>
#include <kernel/x86/system_call.h>
#include <kernel/x86/libc/stdlib.h>

#include <hardware/groups/logging/logging.h>
#include <hardware/groups/graphic_output/graphic_output.h>
#include <hardware/groups/human_input/human_input.h>

#include <software/ramdisk_programs/draw/draw.h>
#include <software/ramdisk_programs/draw_font_psf/draw_font_psf.h>

/* local variables */
int line_start_offset;
int line_column_offset;

/* functions */
void main_window(void) {
    line_start_offset = 0;
    line_column_offset = 0;
    redraw();

    while(1) {
        if(human_input_has_value() == TRUE) {
            human_input_pop_event();
            dword_t pressed_key = human_input_last_event_get_pressed_key();
            if(pressed_key != 0) {
                if(pressed_key == KEY_UP_ARROW) {
                    if(line_start_offset == 0) {
                        continue;
                    }
                    byte_t *logs_ptr = (byte_t *) (get_start_of_log_stream() + line_start_offset);
                    logs_ptr -= 2;
                    line_start_offset -= 2;
                    while(line_start_offset > 0 && *logs_ptr != '\n') {
                        logs_ptr--;
                        line_start_offset--;
                    }
                    if(line_start_offset > 0) {
                        logs_ptr++;
                        line_start_offset++;
                    }
                    else {
                        line_start_offset = 0;
                    }
                    redraw();
                }
                else if(pressed_key == KEY_DOWN_ARROW) {
                    byte_t *logs_ptr = (byte_t *) (get_start_of_log_stream() + line_start_offset);
                    while(*logs_ptr != 0x0A && *logs_ptr != 0x00) {
                        logs_ptr++;
                        line_start_offset++;
                    }
                    if(*logs_ptr == 0x0A) {
                        line_start_offset++;
                    }
                    redraw();
                }
                else if(pressed_key == KEY_PAGE_UP) {
                    for(int i = 0; i < 10; i++) {
                        if(line_start_offset == 0) {
                            continue;
                        }
                        byte_t *logs_ptr = (byte_t *) (get_start_of_log_stream() + line_start_offset);
                        logs_ptr -= 2;
                        line_start_offset -= 2;
                        while(line_start_offset > 0 && *logs_ptr != '\n') {
                            logs_ptr--;
                            line_start_offset--;
                        }
                        if(line_start_offset > 0) {
                            logs_ptr++;
                            line_start_offset++;
                        }
                        else {
                            line_start_offset = 0;
                        }
                    }
                    redraw();
                }
                else if(pressed_key == KEY_PAGE_DOWN) {
                    for(int i = 0; i < 10; i++) {
                        byte_t *logs_ptr = (byte_t *) (get_start_of_log_stream() + line_start_offset);
                        while(*logs_ptr != 0x0A && *logs_ptr != 0x00) {
                            logs_ptr++;
                            line_start_offset++;
                        }
                        if(*logs_ptr == 0x0A) {
                            line_start_offset++;
                        }
                    }
                    redraw();
                }
                else if(pressed_key == KEY_RIGHT_ARROW && line_column_offset < 1000) {
                    line_column_offset++;
                    redraw();
                }
                else if(pressed_key == KEY_LEFT_ARROW && line_column_offset > 0) {
                    line_column_offset--;
                    redraw();
                }
                else if(pressed_key == KEY_HOME) {
                    line_column_offset = 0;
                    redraw();
                }
            }
        }
        asm volatile("hlt");
    }
}

void redraw(void) {
    dword_t default_monitor_id = draw_get_default_monitor_id();
    dword_t width = go_get_width_of_actual_mode(default_monitor_id);
    dword_t height = go_get_height_of_actual_mode(default_monitor_id);

    draw_clear_screen(default_monitor_id, 0x888888);
    print_psf(default_monitor_id, "Welcome to BleskOS\n\nThis is version under active development. There is not much to do now.\nIf you want to use last usable version, download v2025u10. Here you can\nscroll logs by arrows, page up/down and home.", 10, 10, 0x000000);

    byte_t *logs_ptr = (byte_t *) (get_start_of_log_stream() + line_start_offset);
    dword_t x = 10;
    dword_t y = 70;
    dword_t line_column_offset_countdown = line_column_offset;
    while(y < (height - 20)) {
        if(*logs_ptr == 0x00) {
            break;
        }
        else if(*logs_ptr == 0x0A) {
            x = 10;
            y += 10;
            line_column_offset_countdown = line_column_offset;
        }
        else if(line_column_offset_countdown == 0) {
            draw_char_psf(default_monitor_id, *logs_ptr, x, y, 0x000000);
            x += 8;
        }
        else {
            line_column_offset_countdown--;
        }
        logs_ptr++;
    }
    draw_update_default_monitor_screen();
}