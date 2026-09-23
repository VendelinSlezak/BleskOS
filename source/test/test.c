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

/* local variables */
syslib_t *syslib;
virtual_hardware_t *virtual_hardware;

/* functions */
void main(syslib_t *syslib_ptr, virtual_hardware_t *virtual_hardware_ptr) {
    syslib = syslib_ptr;
    virtual_hardware = virtual_hardware_ptr;
    syslib->initialize(virtual_hardware);
    syscall_create_thread(refresh_screen_thread, 0x10000000, delete_signal_handler);
    while(1) {
        if(virtual_hardware_ptr->consumer != virtual_hardware_ptr->producer) {
            vh_human_input_event_t *event = &virtual_hardware_ptr->events[virtual_hardware_ptr->consumer];
            if(event->type == VH_HUMAN_INPUT_EVENT_KEY_PRESSED) {
                syslib->log("Key %d pressed, unicode %c", event->argument1, event->argument2);
            }
            else if(event->type == VH_HUMAN_INPUT_EVENT_KEY_RELEASED) {
                syslib->log("Key %d released, unicode %c", event->argument1, event->argument2);
            }
            else if(event->type == VH_HUMAN_INPUT_EVENT_BUTTON_PRESSED) {
                syslib->log("Button %d pressed at %d %d", event->argument1, event->argument2, event->argument3);
            }
            else if(event->type == VH_HUMAN_INPUT_EVENT_BUTTON_RELEASED) {
                syslib->log("Button %d released at %d %d", event->argument1, event->argument2, event->argument3);
            }
            else if(event->type == VH_HUMAN_INPUT_EVENT_MOUSE_MOVEMENT) {
                syslib->log("Mouse moved to %d %d wheel %d", event->argument1, event->argument2, event->argument3);
            }
            virtual_hardware_ptr->consumer = (virtual_hardware_ptr->consumer + 1) % VH_HUMAN_INPUT_SIZE_OF_RING;
        }
    }
}

void refresh_screen_thread(void) {
    while(1) {
        if(virtual_hardware->flag_window_is_different_from_buffer == 1) {
            if(    virtual_hardware->buffer_width != virtual_hardware->window_width
                || virtual_hardware->buffer_height != virtual_hardware->window_height) {
                virtual_hardware->buffer_width = virtual_hardware->window_width;
                virtual_hardware->buffer_height = virtual_hardware->window_height;
                uint32_t pixels = virtual_hardware->buffer_width * virtual_hardware->buffer_height;
                for(int i = 0; i < pixels; i++) {
                    virtual_hardware->buffer[i] = 0xFF00FF00;
                }
            }
            syscall_virtual_hardware(VIRTUAL_HARDWARE_SCREEN_ID, VH_SCREEN_DEMAND_WINDOW_CHANGE); // buffer updated after window change
        }
        if(virtual_hardware->flag_buffer_is_on_screen == 0) {
            syscall_virtual_hardware(VIRTUAL_HARDWARE_SCREEN_ID, VH_SCREEN_DEMAND_REDRAW); // copy buffer on screen
        }
    }
}

void delete_signal_handler(void) {
    syslib->log("Hello from delete signal thread");
    syscall_close_thread();
}