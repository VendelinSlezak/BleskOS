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

/* functions */
void main(syslib_t *syslib_ptr) {
    syslib = syslib_ptr;

    // TODO: load log from file

    // count dimensions
    uint32_t *log = (uint32_t *) 0xB1000000; // TODO: this is hardcoded for now
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t line_width = 0;
    while(*log != '\0') {
        if(*log == '\n') {
            log++;
            height += 16;
            if(line_width > width) {
                width = line_width;
            }
            line_width = 0;
        }
        else {
            line_width += 8;
            log++;
        }
    }
    height += 16;
    if(line_width > width) {
        width = line_width;
    }

    // create gui
    void *area = syslib->create_draw_area(redraw_area, width, height, process_wheel);
    syslib->element_enable_scrollbars(area);
    syslib->add_key_event(area, KEY_UP_ARROW, key_pressed);
    syslib->add_key_event(area, KEY_DOWN_ARROW, key_pressed);
    syslib->add_key_event(area, KEY_LEFT_ARROW, key_pressed);
    syslib->add_key_event(area, KEY_RIGHT_ARROW, key_pressed);
    syslib->give_focus_to_element(area);

    // start event loop
    syslib->event_loop(area);
}

void redraw_area(void *element, void *box, int x_offset, int y_offset) {
    int x = 0;
    int y = 0;
    uint32_t *log = (uint32_t *) 0xB1000000;
    while(*log != '\0') {
        if(*log == '\n') {
            x = 0;
            y += 16;
        }
        else {
            syslib->draw_bitmap_char(box, (x_offset * -1) + x, (y_offset * -1) + y, *log, 0xFFFFFFFF);
            x += 8;
        }
        log++;
    }
}

uint32_t process_wheel(void *element, int movement) {
    return syslib->element_move_content_inside(element, 0, movement * 16 * 3);
}

uint32_t key_pressed(void *element, uint32_t key, uint32_t unicode) {
    int x_movement = 0;
    int y_movement = 0;
    switch(key) {
        case KEY_UP_ARROW: {
            y_movement = -1;
            break;
        }
        case KEY_DOWN_ARROW: {
            y_movement = 1;
            break;
        }
        case KEY_LEFT_ARROW: {
            x_movement = -1;
            break;
        }
        case KEY_RIGHT_ARROW: {
            x_movement = 1;
            break;
        }
    }
    return syslib->element_move_content_inside(element, x_movement * 16 * 3, y_movement * 16 * 3);
}