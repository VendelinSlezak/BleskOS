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
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/monitor/monitor.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
uint32_t is_there_graphic_output_device = false;

/* local variables */
graphic_output_group_t *graphic_output_group;

/* functions */
void initialize_graphic_group(void) {
    graphic_output_group = kalloc(sizeof(graphic_output_group_t));
}

void add_graphic_output_device(hardware_t *device, graphic_output_group_device_functions_t *functions) {
    if(graphic_output_group->number_of_devices >= MAX_NUMBER_OF_LOGGING_GROUP_DEVICES) {
        return;
    }
    graphic_output_group->devices[graphic_output_group->number_of_devices].device = device;
    graphic_output_group->devices[graphic_output_group->number_of_devices].functions = functions;
    graphic_output_group->number_of_devices++;
}

void remove_graphic_output_device(hardware_t *device) {
    for(int i = 0; i < graphic_output_group->number_of_devices; i++) {
        if(graphic_output_group->devices[i].device == device) {
            for(int j = i; j < graphic_output_group->number_of_devices - 1; j++) {
                graphic_output_group->devices[j] = graphic_output_group->devices[j + 1];
            }
            graphic_output_group->number_of_devices--;
            break;
        }
    }
}

hardware_t *get_graphic_output_first_monitor_device(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return NULL;
    }
    return graphic_output_group->devices[0].device;
}

void *get_output_linear_frame_buffer(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return NULL;
    }
    return graphic_output_group->devices[0].functions->get_linear_frame_buffer(graphic_output_group->devices[0].device);
}

uint32_t get_number_of_graphic_output_devices(void) {
    return graphic_output_group->number_of_devices;
}

void redraw_full_screen(void *double_buffer) {
    if(graphic_output_group->number_of_devices == 0) {
        return;
    }
    graphic_output_group->devices[0].functions->redraw_full_screen(graphic_output_group->devices[0].device, double_buffer);
}

void redraw_part_of_screen(uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    if(graphic_output_group->number_of_devices == 0) {
        return;
    }
    if(monitor_x >= get_output_width() || monitor_y >= get_output_height()) {
        return;
    }
    if((monitor_x + width) > get_output_width()) {
        width = get_output_width() - monitor_x;
    }
    if((monitor_y + height) > get_output_height()) {
        height = get_output_height() - monitor_y;
    }
    graphic_output_group->devices[0].functions->redraw_part_of_screen(graphic_output_group->devices[0].device, monitor_x, monitor_y, double_buffer, buffer_pixels_per_line, buffer_x, buffer_y, width, height);
}

uint32_t get_size_of_double_buffer(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    return graphic_output_group->devices[0].functions->get_size_of_output_buffer(graphic_output_group->devices[0].device);
}

uint32_t get_output_width(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    return graphic_output_group->devices[0].functions->get_output_width(graphic_output_group->devices[0].device);
}

uint32_t get_output_height(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    return graphic_output_group->devices[0].functions->get_output_height(graphic_output_group->devices[0].device);
}

uint32_t get_output_bpp(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    return graphic_output_group->devices[0].functions->get_output_bpp(graphic_output_group->devices[0].device);
}

uint32_t get_output_bytes_per_line(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    return graphic_output_group->devices[0].functions->get_bytes_per_line(graphic_output_group->devices[0].device);
}