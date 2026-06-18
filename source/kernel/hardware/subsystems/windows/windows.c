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
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/software/syscall.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
screen_layout_t *screen_layout;
window_t *current_window;

/* functions */
void initialize_windows_subsystem(void) {
    // TODO: support for more windows
    screen_layout = kalloc(sizeof(screen_layout_t));
    add_virtual_device_to_hardware_list(VIRTUAL_HARDWARE_WINDOW);
}

/* userspace functions */
void window_subsystem_process_userspace_command(windows_subsystem_command_t *command) {
    if(return_validated_pointer(command, sizeof(windows_subsystem_command_t) + sizeof(uint32_t)) == NULL) {
        return;
    }

    switch(command->type) {
        case WINDOWS_SUBSYSTEM_CREATE_WINDOW: {
            if(return_validated_pointer(command, sizeof(windows_subsystem_command_t) + sizeof(uint32_t) * 3) == NULL) {
                break;
            }
            if(get_current_logical_processor_struct()->current_program->window != NULL) {
                return;
            }
            window_t *window = kalloc(sizeof(window_t));
            window->program = get_current_logical_processor_struct()->current_program;
            window->width = get_output_width();
            window->height = get_output_height();
            window->x = 0;
            window->y = 0;
            window->real_inside_width = command->argument[0];
            window->real_inside_height = command->argument[1];
            get_current_logical_processor_struct()->current_program->window = (void *) window;
            screen_layout->window[0] = window;
            screen_layout->num_of_windows = 1;
            current_window = window;

            // return actual size of window
            command->argument[0] = window->width;
            command->argument[1] = window->height;
            break;
        }
        case WINDOWS_SUBSYSTEM_REDRAW_WINDOW: {
            if(return_validated_pointer(command, sizeof(windows_subsystem_command_t) + sizeof(uint32_t) * 2) == NULL) {
                return;
            }
            window_t *process_window = (window_t *) get_current_logical_processor_struct()->current_program->window;
            for(int i = 0; i < screen_layout->num_of_windows; i++) {
                if(screen_layout->window[i] == process_window) {
                    // TODO: support other sizes of window not only fullscreen mode
                    void *double_buffer = (void *) command->argument[0];
                    if(return_validated_pointer(double_buffer, get_size_of_double_buffer()) == NULL) {
                        return;
                    }
                    redraw_full_screen(double_buffer);
                    return;
                }
            }
            break;
        }
    }
}