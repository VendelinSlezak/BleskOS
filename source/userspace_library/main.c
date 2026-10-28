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

#include <userspace_library/time.h>
#include <userspace_library/logging.h>
#include <userspace_library/bleskalloc.h>
#include <userspace_library/libc/string.h>
#include <userspace_library/screen.h>
#include <userspace_library/bitmap_font.h>
#include <userspace_library/gui/gui.h>
#include <userspace_library/got.h>

/* global variables */
virtual_hardware_t *virtual_hardware;

/* local variables */
buffer_box_t *screen_box;
element_t *root_element;

/* functions */
void main(virtual_hardware_t *virtual_hardware_ptr, int entry_point(const syslib_t *syslib)) {
    virtual_hardware = virtual_hardware_ptr;

    initialize_bleskalloc();

    virtual_hardware_ptr->consumer = 0;
    virtual_hardware_ptr->producer = 0;
    syscall_virtual_hardware(VIRTUAL_HARDWARE_HUMAN_INPUT_ID, VH_HUMAN_INPUT_DEMAND_ENABLE_STREAMING);

    screen_box = malloc(sizeof(buffer_box_t));
    screen_box->buffer = virtual_hardware_ptr->buffer;
    screen_box->buffer_width = screen_width;
    screen_box->buffer_height = screen_height;
    screen_box->x = 0;
    screen_box->y = 0;
    screen_box->width = screen_width;
    screen_box->height = screen_height;

    root_element = NULL;

    initialize_bitmap_font();
    if(syscall_create_thread(catch_virtual_hardware_events, aligned_malloc(4, 4096) + 4096, NULL) == ERROR) {
        log("Failed to create thread to catch virtual hardware events");
        syscall_close_thread();
    }

    /* testing */
    // element_t *hbox = create_hbox();
    // hbox->valign = ALIGN_BOTTOM;
    // box_t *box = hbox->data;
    // box->wrap = true;
    // element_set_content_dimensions(hbox, 0, 300, 0, ELEMENT_MAX_HEIGHT);
    // element_enable_scrollbars(hbox);

    // element_t *hb1 = create_button("Press me 1");
    // element_set_padding(hb1, 10, 10, 10, 10);
    // hb1->background_color = 0xFF00FF00;
    // add_element_to_box_from_start(hbox, hb1);

    // element_t *hb2 = create_button("Press me 2");
    // element_set_padding(hb2, 5, 5, 5, 5);
    // element_set_border(hb2, 3, 3, 1, 1, 0xFF000000);
    // element_set_margin(hb2, 5, 5, 5, 5);
    // hb2->background_color = 0xFF0000FF;
    // add_element_to_box_from_start(hbox, hb2);

    // element_t *hb3 = create_button("Press me 3");
    // element_set_padding(hb3, 20, 20, 20, 20);
    // hb3->background_color = 0xFF00FFFF;
    // add_element_to_box_from_end(hbox, hb3);

    // element_t *table = create_table(2, 2);

    // element_t *b1 = create_button("OK");
    // add_element_to_table_box(table, 0, 0, b1);
    // cell_set_alignment(table, 0, 0, ALIGN_CENTER, ALIGN_MIDDLE);

    // add_element_to_table_box(table, 1, 0, hbox);
    // cell_set_alignment(table, 0, 1, ALIGN_CENTER, ALIGN_MIDDLE);

    // element_t *b3 = create_button("Press me");
    // add_element_to_table_box(table, 0, 1, b3);
    // cell_set_alignment(table, 1, 0, ALIGN_CENTER, ALIGN_MIDDLE);

    // element_t *b4 = create_button("Press me 4");
    // add_element_to_table_box(table, 1, 1, b4);
    // cell_set_alignment(table, 1, 1, ALIGN_CENTER, ALIGN_MIDDLE);

    // event_loop(table);
    /* end of testing */

    entry_point(&syslib_got);
}

void update_screen(void) {
    draw_element(screen_box, root_element, 0, 0);
    syscall_virtual_hardware(VIRTUAL_HARDWARE_SCREEN_ID, VH_SCREEN_DEMAND_REDRAW);
}

void redraw_gui_main(uint32_t width, uint32_t height) {
    screen_box->width = width;
    screen_box->height = height;
    if(very_unlikely(root_element == NULL)) {
        return;
    }
    redraw_gui(root_element, screen_box, width, height);
}

void catch_virtual_hardware_events(void) {
    while(true) {
        asm volatile("pause");

        if(virtual_hardware->flag_window_is_different_from_buffer == 1) {
            if(virtual_hardware->buffer_width != virtual_hardware->window_width || virtual_hardware->buffer_height != virtual_hardware->window_height) {
                update_screen_buffer(virtual_hardware->buffer, virtual_hardware->window_width, virtual_hardware->window_height);
                virtual_hardware->buffer_width = virtual_hardware->window_width;
                virtual_hardware->buffer_height = virtual_hardware->window_height;
                screen_box->buffer_width = virtual_hardware->buffer_width;
                screen_box->buffer_height = virtual_hardware->buffer_height;
                screen_box->width = virtual_hardware->buffer_width;
                screen_box->height = virtual_hardware->buffer_height;
            }
            redraw_gui_main(virtual_hardware->buffer_width, virtual_hardware->buffer_height);
            syscall_virtual_hardware(VIRTUAL_HARDWARE_SCREEN_ID, VH_SCREEN_DEMAND_WINDOW_CHANGE); // buffer updated after window change
        }
        if(virtual_hardware->flag_buffer_is_on_screen == 0) {
            update_screen(); // copy buffer on screen
        }

        uint32_t events_processed = 0;
        uint32_t does_gui_need_redraw = false;
        while(virtual_hardware->consumer != virtual_hardware->producer) {
            vh_human_input_event_t *event = &virtual_hardware->events[virtual_hardware->consumer];
            if(process_event(event) == true) {
                does_gui_need_redraw = true;
            }
            virtual_hardware->consumer = (virtual_hardware->consumer + 1) % VH_HUMAN_INPUT_SIZE_OF_RING;
            events_processed++;
            if(events_processed == VH_HUMAN_INPUT_SIZE_OF_RING) {
                break;
            }
        }
        if(does_gui_need_redraw == true) {
            update_screen();
        }

        syscall_switch_thread();
    }
}

void event_loop(void *root_element_ptr) {
    root_element = root_element_ptr;
    while(true) {
        asm volatile("pause");
        syscall_switch_thread();
    }
}

// TODO: management of sessions