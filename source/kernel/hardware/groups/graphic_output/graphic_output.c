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
#include <kernel/cpu/commands.h>
#include <kernel/memory/memory_allocators.h>
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

void add_graphic_output_device(uint32_t type, void *structure) {
    if(graphic_output_group->number_of_devices >= MAX_NUMBER_OF_GRAPHIC_OUTPUT_DEVICES) {
        return;
    }
    graphic_output_group->devices[graphic_output_group->number_of_devices].type = type;
    graphic_output_group->devices[graphic_output_group->number_of_devices].structure = structure;
    graphic_output_group->number_of_devices++;
}

void remove_graphic_output_device(uint32_t id) {
    for(int i = 0; i < graphic_output_group->number_of_devices; i++) {
        uint32_t device_id = 0;
        if(graphic_output_group->devices[i].type == GRAPHIC_OUTPUT_DEVICE_TYPE_MONITOR) {
            device_id = ((monitor_t *) graphic_output_group->devices[i].structure)->id;
        }
        if(device_id == id) {
            for(int j = i; j < graphic_output_group->number_of_devices - 1; j++) {
                graphic_output_group->devices[j] = graphic_output_group->devices[j + 1];
            }
            graphic_output_group->number_of_devices--;
            break;
        }
    }
}

uint32_t get_number_of_graphic_output_devices(void) {
    return graphic_output_group->number_of_devices;
}

void redraw_full_screen(void *double_buffer) {
    if(graphic_output_group->number_of_devices == 0) {
        return;
    }

    // TODO: add usage for more devices
    if(graphic_output_group->devices[0].type == GRAPHIC_OUTPUT_DEVICE_TYPE_MONITOR) {
        monitor_t *monitor = (monitor_t *) graphic_output_group->devices[0].structure;
        if(monitor->actual_mode->redraw_full_screen == NULL) {
            return;
        }
        monitor->actual_mode->redraw_full_screen(monitor, double_buffer);
    }
}

void redraw_part_of_screen(uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    if(graphic_output_group->number_of_devices == 0) {
        return;
    }

    // TODO: add usage for more devices
    if(graphic_output_group->devices[0].type == GRAPHIC_OUTPUT_DEVICE_TYPE_MONITOR) {
        monitor_t *monitor = (monitor_t *) graphic_output_group->devices[0].structure;
        if(monitor->actual_mode->redraw_part_of_screen == NULL) {
            return;
        }
        monitor->actual_mode->redraw_part_of_screen(monitor, monitor_x, monitor_y, double_buffer, buffer_pixels_per_line, buffer_x, buffer_y, width, height);
    }
}

uint32_t get_size_of_double_buffer(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    if(graphic_output_group->devices[0].type == GRAPHIC_OUTPUT_DEVICE_TYPE_MONITOR) {
        monitor_t *monitor = (monitor_t *) graphic_output_group->devices[0].structure;
        return monitor->actual_mode->active_width * monitor->actual_mode->active_height * 4;
    }
    return 0;
}

uint32_t get_output_width(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    if(graphic_output_group->devices[0].type == GRAPHIC_OUTPUT_DEVICE_TYPE_MONITOR) {
        monitor_t *monitor = (monitor_t *) graphic_output_group->devices[0].structure;
        return monitor->actual_mode->active_width;
    }
    return 0;
}

uint32_t get_output_height(void) {
    if(graphic_output_group->number_of_devices == 0) {
        return 0;
    }
    if(graphic_output_group->devices[0].type == GRAPHIC_OUTPUT_DEVICE_TYPE_MONITOR) {
        monitor_t *monitor = (monitor_t *) graphic_output_group->devices[0].structure;
        return monitor->actual_mode->active_height;
    }
    return 0;
}