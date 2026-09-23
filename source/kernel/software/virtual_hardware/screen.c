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
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/software/running_executables.h>
#include <kernel/software/spawning_template.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/main_panel.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>

/* functions */
void vh_screen_copy_buffer(uint32_t *screen, uint32_t screen_width, uint32_t *buffer, uint32_t buffer_width, uint32_t buffer_height) {
    for(uint32_t y = 0; y < buffer_height; y++) {
        for(uint32_t x = 0; x < buffer_width; x++) {
            screen[(y * screen_width) + x] = *buffer++;
        }
    }
}

void vh_screen_update_window_resolution(running_executable_t *re, uint32_t width, uint32_t height) {
    if(re == NULL || re->page_directory_of_virtual_hardware == 0) {
        return;
    }
    move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
    spawning_template_t *template = re->template;
    virtual_hardware_t *virtual_hardware = template->virtual_hardware;
    virtual_hardware->window_width = width;
    virtual_hardware->window_height = height;
    virtual_hardware->flag_window_is_different_from_buffer = 1;
    virtual_hardware->flag_buffer_is_on_screen = 0;
    move_back_to_previous_virtual_space();
}

void vh_screen_demand_redraw_from_program(running_executable_t *re) {
    if(re == NULL || re->page_directory_of_virtual_hardware == 0) {
        return;
    }
    move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
    spawning_template_t *template = re->template;
    virtual_hardware_t *virtual_hardware = template->virtual_hardware;
    virtual_hardware->flag_buffer_is_on_screen = 0;
    move_back_to_previous_virtual_space();
}

void vh_screen_doorbell(uint32_t demand) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    program_t *program = lpdata->current_program;
    running_executable_t *re = program->running_executable;

    switch(demand) {
        case VH_SCREEN_DEMAND_WINDOW_CHANGE: {
            move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
            spawning_template_t *template = re->template;
            virtual_hardware_t *virtual_hardware = template->virtual_hardware;
            if(virtual_hardware->buffer_width == virtual_hardware->window_width && virtual_hardware->buffer_height == virtual_hardware->window_height) {
                virtual_hardware->size_of_buffer = (virtual_hardware->buffer_width * virtual_hardware->buffer_height * sizeof(uint32_t));
                virtual_hardware->flag_window_is_different_from_buffer = 0;
                screen_part_t *part = re->part;
                view_t *view = part->view;
                vh_screen_copy_buffer(view->buffer + (view->width * 4 * (part->y + 31)) + (4 * part->x), view->width, virtual_hardware->buffer, part->width, part->height - 60);
                redraw_full_screen(view->buffer);
                virtual_hardware->flag_buffer_is_on_screen = 1;
            }
            move_back_to_previous_virtual_space();
            break;
        }
        case VH_SCREEN_DEMAND_REDRAW: {
            move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
            spawning_template_t *template = re->template;
            virtual_hardware_t *virtual_hardware = template->virtual_hardware;
            screen_part_t *part = re->part;
            view_t *view = part->view;
            vh_screen_copy_buffer(view->buffer + (view->width * 4 * (part->y + 31)) + (4 * part->x), view->width, virtual_hardware->buffer, part->width, part->height - 60);
            redraw_full_screen(view->buffer);
            virtual_hardware->flag_buffer_is_on_screen = 1;
            move_back_to_previous_virtual_space();
            break;
        }
    }
}