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
#include <syslib.h>
#include <kernel/hardware/groups/human_input/human_input.h>

/* local variables */
syslib_t *syslib;
uint32_t **start_of_lines;
int first_shown_line;
uint32_t last_line;

/* functions */
void main(syslib_t *syslib_ptr) {
    syslib = syslib_ptr;
    // syslib->initialize_screen(redraw_screen);
    // syslib->add_key_event(KEY_UP_ARROW, VH_HUMAN_INPUT_EVENT_KEY_PRESSED, key_pressed, KEY_UP_ARROW);
    // syslib->add_key_event(KEY_DOWN_ARROW, VH_HUMAN_INPUT_EVENT_KEY_PRESSED, key_pressed, KEY_DOWN_ARROW);

    // // count number of lines
    // last_line = 0;
    // uint32_t *log = (uint32_t *) 0xB1000000;
    // while(*log != '\0') {
    //     if(*log == '\n') {
    //         last_line++;
    //     }
    //     log++;
    // }

    // // parse lines
    // start_of_lines = syslib->malloc(sizeof(uint32_t *) * (last_line + 1));
    // log = (uint32_t *) 0xB1000000;
    // uint32_t p = 0;
    // start_of_lines[p] = log;
    // while(*log != '\0') {
    //     if(*log == '\n') {
    //         log++;
    //         start_of_lines[p++] = log;
    //     }
    //     else {
    //         log++;
    //     }
    // }

    // syslib->event_loop();
}

void redraw_screen(uint32_t width, uint32_t height) {
    syslib->draw_window_background_color(0xFFFFFFFF);
    uint32_t *log = start_of_lines[first_shown_line];
    uint32_t x = 0;
    uint32_t y = 0;
    while(*log != '\0') {
        if(*log == '\n') {
            log++;
            x = 0;
            y += 16;
        }
        else {
            syslib->draw_char(*log, x, y, 0xFF000000);
            log++;
            x += 8;
        }
    }
}

void key_pressed(uint32_t identifier, uint32_t unicode) {
    if(identifier == KEY_UP_ARROW) {
        first_shown_line -= 5;
    }
    else if(identifier == KEY_DOWN_ARROW) {
        first_shown_line += 5;
    }
    if(first_shown_line < 0) {
        first_shown_line = 0;
    }
    else if(first_shown_line >= last_line) {
        first_shown_line = (last_line - 1);
    }
    syslib->update_screen();
}