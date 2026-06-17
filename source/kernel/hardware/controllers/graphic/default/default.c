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
#include <kernel/kernel.h>
#include <kernel/hardware/devices/monitor/monitor.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/main.h>
#include <libc/string.h>

/* global variables */
uint32_t try_to_detect_default_graphic_controller = true;

/* local variables */
hardware_t *default_graphic_controller;
hardware_t *default_graphic_controller_monitor;
static monitor_communication_functions_t dgc_monitor_funcs = {
    .change_resolution = dgc_change_monitor_resolution,
    .change_brightness = dgc_change_monitor_brightness
};

/* functions for device initialization */
uint32_t is_default_graphic_controller_present(void) {
    standardized_graphic_output_t *standardized_graphic_output = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    if(standardized_graphic_output->type == STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE) {
        return true;
    }
    else {
        return false;
    }
}

void initialize_default_graphic_controller(hardware_t *device) {
    log("\n[DEFAULT GRAPHIC CONTROLLER] Initialize default graphic controller");
    default_graphic_controller = device;
    is_there_graphic_output_device = true;

    standardized_graphic_output_t *standardized_graphic_output = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    monitor_mode_t modes[1];
    modes[0].active_width = standardized_graphic_output->width;
    modes[0].active_height = standardized_graphic_output->height;
    modes[0].bpp = standardized_graphic_output->bpp;
    modes[0].bytes_per_line = standardized_graphic_output->bytes_per_line;
    modes[0].linear_frame_buffer = perm_phy_alloc(standardized_graphic_output->linear_frame_buffer, modes[0].bytes_per_line * modes[0].active_height, VM_KERNEL | VM_WRITE_COMBINED);
    monitor_mode_add_functions(&modes[0]);

    monitor_controller_data_t *controller_data = (monitor_controller_data_t *) kalloc(sizeof(monitor_controller_data_t) + (sizeof(monitor_mode_t) * 1));
    controller_data->brightness = 100;
    memcpy(controller_data->supported_modes, modes, sizeof(monitor_mode_t) * 1);
    controller_data->best_mode = &controller_data->supported_modes[0];

    default_graphic_controller_monitor = add_hardware(
        default_graphic_controller,
        "Monitor", 
        &dgc_monitor_funcs,
        controller_data,
        initialize_monitor_device,
        NULL
    );
    init_hardware(default_graphic_controller_monitor);

    device->is_initialized = true;
}

/* functions for working with device */
uint32_t dgc_change_monitor_resolution(hardware_t *monitor, monitor_mode_t *mode) {
    monitor_controller_data_t *data = (monitor_controller_data_t *) monitor->data_from_controller;
    monitor_mode_t *best_mode = data->best_mode;
    if(best_mode->active_width == mode->active_width && best_mode->active_height == mode->active_height && best_mode->bpp == mode->bpp) {
        log("\n[DGC] Monitor mode changed to %dx%dx%d", mode->active_width, mode->active_height, mode->bpp);
        return SUCCESS;
    }
    else {
        return ERROR;
    }
}

uint32_t dgc_change_monitor_brightness(hardware_t *monitor, uint32_t brightness) {
    if(brightness == 100) {
        return SUCCESS;
    }
    else {
        return ERROR;
    }
}