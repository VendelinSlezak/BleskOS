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
#include <kernel/x86/system_call.h>
#include <hardware/devices/monitor/monitor.h>

/* functions for initialization */
dword_t is_default_graphic_card_present(void) {
    standardized_graphic_output_t *bootloader_graphic_info = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    if(bootloader_graphic_info->type == STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void initialize_default_graphic_card_entity(void) {
    dword_t entity_number = create_entity("Default Graphic Card", E_TYPE_KERNEL_RING);
    create_command_without_ca_without_fin(entity_number, initialize_dgc);
}

void initialize_dgc(void) {
    e_dgc_attr_t *attr = command_info->entity_attributes;
    command_t *command = command_get_ptr(command_info->command_number);

    // log
    log("\n[DGC] Initializing entity %d...", command->entity);

    // read information about available graphic modes
    standardized_graphic_output_t *bootloader_graphic_info = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    attr->mode.active_width = bootloader_graphic_info->width;
    attr->mode.active_height = bootloader_graphic_info->height;
    attr->mode.bpp = bootloader_graphic_info->bpp;
    attr->mode.bytes_per_line = bootloader_graphic_info->bytes_per_line;
    attr->mode.linear_frame_buffer = bootloader_graphic_info->linear_frame_buffer;

    // add device to list
    size_t dgc_functions_for_devices[] = {
        (size_t)dgc_change_monitor_resolution
    };
    attr->devices[0] = create_monitor_device_entity(command->entity, dgc_functions_for_devices, 1, &attr->mode, 0);
    attr->number_of_devices = 1;
    dword_t initialize_monitor_command_number = create_command_without_ca_without_fin(attr->devices[0], initialize_monitor_device);
    SC_WAIT_FOR_COMMAND(initialize_monitor_command_number);
}

/* functions for working with device */
dword_t dgc_change_monitor_resolution(monitor_mode_t mode) {
    e_dgc_attr_t *attr = command_info->entity_attributes;
    if(    mode.active_width == attr->mode.active_width
        && mode.active_height == attr->mode.active_height
        && mode.bpp == attr->mode.bpp
        && mode.bytes_per_line == attr->mode.bytes_per_line) {
        return SUCCESS;
    }
    else {
        return ERROR;
    }
}