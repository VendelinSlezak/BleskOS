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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/system_call.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/libc/stdio.h>
#include <kernel/x86/ramdisk/ramdisk.h>

#include <hardware/groups/graphic_output/graphic_output.h>
#include <hardware/groups/human_input/human_input.h>
#include <hardware/controllers/pci/pci.h>
#include <hardware/controllers/graphic/bga/bga.h>
#include <hardware/controllers/graphic/default_graphic_card/default_graphic_card.h>
#include <hardware/controllers/controller_8042/controller_8042.h>

#include <software/ramdisk_programs/convBMPtoI/main.h>
#include <software/ramdisk_programs/internal_image/main.h>
#include <software/ramdisk_programs/draw/draw.h>

/* local variables */
void (*initialize_group[])(void) = {
    initialize_graphic_output_group,
    initialize_human_input_group
};

controller_device_t io_controller_devices[] = {
    { is_bga_controller_present, initialize_bga_controller_entity },
    { is_default_graphic_card_present, initialize_default_graphic_card_entity },
    { is_controller_8042_present, initialize_controller_8042_entity },
};

/* functions */
void initialize_hardware(void) {
    // initialize groups
    for(int i = 0; i < (sizeof(initialize_group) / sizeof(size_t)); i++) {
        (*initialize_group[i])();
    }

    // initialize controllers and devices on PCI controller
    initialize_pci_controller();

    // initialize rest of controllers and devices
    dword_t number_of_io_devices_in_list = (sizeof(io_controller_devices) / sizeof(controller_device_t));
    log("\n[HARDWARE] Start of initialization");
    for(int i = 0; i < number_of_io_devices_in_list; i++) {
        if(io_controller_devices[i].check_presence == NULL) {
            continue;
        }
        if(io_controller_devices[i].check_presence() == TRUE) {
            io_controller_devices[i].initialize();
        }
    }

    // wait for graphic device to initialize
    volatile command_list_t *command_list = (command_list_t *) command_info->vm_of_command_list;
    while(1) {
        if(go_get_num_of_devices() != 0) {
            dword_t monitor_id = go_get_device_id(0);
            dword_t mode = go_get_actual_mode(monitor_id);
            dword_t width = go_get_width_of_mode(monitor_id, mode);
            dword_t height = go_get_height_of_mode(monitor_id, mode);
            dword_t *buff = go_get_double_buffer(monitor_id);

            memch_program_output_t logo_img = memch_run_and_complete("convBMPtoI", MEMCH_INPUT_ALLOCATION, get_ramdisk_file_ptr("logo.bmp"), get_ramdisk_file_size("logo.bmp"));
            output_image_t *img = get_memch_program_output_ptr(&logo_img);
            dword_t start_x = (width/2) - (img->width/2);
            dword_t start_y = (height/2) - (img->height/2);
            memch_program_output_t draw = memch_run_and_complete("drawIonScreen", &(drawIonScreen_param_t){ width, height, start_x, start_y }, MEMCH_INPUT_MEMCH_OUTPUT, logo_img, buff, width*height*4);
            go_redraw_screen(monitor_id);

            break;
        }

        if(command_list->number_of_commands == 0) {
            break;
        }

        SC_SWITCH();
    }

    // wait for all devices to initialize
    while(1) {
        if(command_list->number_of_commands == 0) {
            break;
        }
        else {
            SC_SWITCH();
        }
    }

    log("\n[HARDWARE] All detected hardware was initialized");
}

void remove_driver_from_static_driver_list(dword_t (*check_presence)(void)) {
    dword_t number_of_io_devices_in_list = (sizeof(io_controller_devices) / sizeof(controller_device_t));
    for(int i = 0; i < number_of_io_devices_in_list; i++) {
        if(io_controller_devices[i].check_presence == check_presence) {
            io_controller_devices[i].check_presence = NULL;
            io_controller_devices[i].initialize = NULL;
        }
    }
}