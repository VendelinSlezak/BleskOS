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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/entities/entities.h>
#include <hardware/devices/monitor/monitor.h>

/* local variables */
dword_t graphic_group_entity;
e_graphic_output_attr_t *attr;

/* functions */
void initialize_graphic_output_group(void) {
    graphic_group_entity = create_entity("Graphic Output group", E_TYPE_KERNEL_RING);
    attr = entity_get_attr_ptr(graphic_group_entity);
}

void graphic_output_group_add_device(dword_t device_entity, dword_t device_id, graphic_output_device_functions_t *functions) {
    // check if there is free space for device
    if(attr->number_of_devices >= GRAPHIC_OUTPUT_MAX_NUMBER_OF_DEVICES) {
        log("\n[ERROR] Too many Graphic Output devices");
        return;
    }

    // save number of new device
    dword_t arr_num = attr->number_of_devices;
    attr->number_of_devices++;

    // save functions
    attr->devices[arr_num] = device_entity;
    attr->ids[arr_num] = device_id;
    attr->functions[arr_num] = functions;
}

dword_t graphic_output_group_get_arr_num(dword_t device_id) {
    for(int i = 0; i < attr->number_of_devices; i++) {
        if(attr->ids[i] == device_id) {
            return i;
        }
    }
    return INVALID;
}

dword_t go_get_num_of_devices(void) {
    return attr->number_of_devices;
}

dword_t go_get_device_id(dword_t arr_num) {
    return attr->ids[arr_num];
}

dword_t go_is_device_connected(dword_t device_id) {
    if(graphic_output_group_get_arr_num(device_id) == INVALID) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

void *go_get_double_buffer(dword_t device_id) {
    dword_t d = graphic_output_group_get_arr_num(device_id);
    if(d == INVALID) { return (void *) INVALID; }
    return attr->functions[d]->get_double_buffer(attr->devices[d]);
}

void go_redraw_screen(dword_t device_id) {
    GRAPHIC_OUTPUT_FUNCTION_INIT();
    // dword_t start_time = kernel_attr->kernel_time;
    attr->functions[d]->redraw_screen(attr->devices[d]);

    // debug log
    // log("\nTime in ms: %d", kernel_attr->kernel_time - start_time);
}

dword_t go_get_number_of_available_modes(dword_t device_id) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);
    return monitor_attr->number_of_modes;
}

dword_t go_get_actual_mode(dword_t device_id) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);
    return monitor_attr->selected_mode;
}

dword_t go_get_width_of_mode(dword_t device_id, dword_t mode_number) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);

    if(monitor_attr->number_of_modes <= mode_number) {
        return INVALID;
    }

    return monitor_attr->modes[mode_number].active_width;
}

dword_t go_get_width_of_actual_mode(dword_t device_id) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);
    return monitor_attr->modes[monitor_attr->selected_mode].active_width;
}

dword_t go_get_height_of_mode(dword_t device_id, dword_t mode_number) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);

    if(monitor_attr->number_of_modes <= mode_number) {
        return INVALID;
    }

    return monitor_attr->modes[mode_number].active_height;
}

dword_t go_get_height_of_actual_mode(dword_t device_id) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);
    return monitor_attr->modes[monitor_attr->selected_mode].active_height;
}

dword_t go_get_bpp_of_mode(dword_t device_id, dword_t mode_number) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);

    if(monitor_attr->number_of_modes <= mode_number) {
        return INVALID;
    }

    return monitor_attr->modes[mode_number].bpp;
}

dword_t go_change_resolution(dword_t device_id, dword_t mode_number) {
    GRAPHIC_OUTPUT_FUNCTION_INIT(INVALID);
    e_monitor_attr_t *monitor_attr = entity_get_attr_ptr(attr->devices[d]);

    if(monitor_attr->number_of_modes <= mode_number) {
        return INVALID;
    }

    return monitor_attr->change_monitor_resolution(monitor_attr->modes[mode_number]);
}