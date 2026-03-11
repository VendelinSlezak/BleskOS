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

/* global variables */
uint32_t try_to_detect_default_graphic_controller = true;

/* functions */
uint32_t is_default_graphic_controller_present(void) {
    standardized_graphic_output_t *standardized_graphic_output = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    if(standardized_graphic_output->type == STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE) {
        return true;
    }
    else {
        return false;
    }
}

void initialize_default_graphic_controller(void) {
    if(try_to_detect_default_graphic_controller == false) {
        return;
    }
    if(is_default_graphic_controller_present() == false) {
        return;
    }
    new_uninitialized_device();
    is_there_graphic_output_device = true;

    standardized_graphic_output_t *standardized_graphic_output = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    uint32_t width = standardized_graphic_output->width;
    uint32_t height = standardized_graphic_output->height;
    uint32_t bpp = standardized_graphic_output->bpp;
    uint32_t bytes_per_line = standardized_graphic_output->bytes_per_line;
    monitor_mode_t modes[1];
    modes[0].active_width = standardized_graphic_output->width;
    modes[0].active_height = standardized_graphic_output->height;
    modes[0].bpp = standardized_graphic_output->bpp;
    modes[0].bytes_per_line = (standardized_graphic_output->width * standardized_graphic_output->bpp);
    modes[0].linear_frame_buffer = perm_phy_alloc(standardized_graphic_output->linear_frame_buffer, modes[0].bytes_per_line * modes[0].active_height, VM_KERNEL | VM_WRITE_COMBINED);
    monitor_mode_add_functions(&modes[0]);

    add_monitor_device( 100,
                        NULL,
                        1,
                        1,
                        1,
                        modes,
                        NULL
                    );
    
    device_initialized();
}