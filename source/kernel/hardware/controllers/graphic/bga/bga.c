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
#include <kernel/cpu/scheduler.h>
#include <kernel/hardware/devices/monitor/monitor.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <libc/string.h>
#include <kernel/hardware/controllers/pci/pci.h>
#include <kernel/hardware/controllers/graphic/default/default.h>
#include <kernel/hardware/main.h>

/* global variables */
uint32_t try_to_detect_bga_on_isa = true;

/* local variables */
monitor_t *bga_monitor;

/* functions for communication */
uint16_t bga_read(uint8_t index) {
    outw(BGA_REGISTER_INDEX, index);
    return inw(BGA_REGISTER_DATA);
}

void bga_write(uint8_t index, uint16_t value) {
    outw(BGA_REGISTER_INDEX, index);
    outw(BGA_REGISTER_DATA, value);
}

/* functions for device initialization */
void initialize_bga_controller_on_pci(pci_device_t device) {
    new_uninitialized_device();
    try_to_detect_bga_on_isa = false;
    pci_enable_busmastering(device);
    try_to_detect_default_graphic_controller = false;
    is_there_graphic_output_device = true;
    create_kernel_thread((uint32_t)initialize_bga_controller_with_lfb, (uint32_t []) { pci_get_mmio(device, PCI_BAR0) }, 1);
}

uint32_t is_bga_controller_present(void) {
    uint16_t id = bga_read(BGA_INDEX_ID);
    if(id >= 0xB0C2 && id <= 0xB0C5) {
        return true;
    }
    else {
        return false;
    }
}

void initialize_bga_controller_on_isa(void) {
    if(try_to_detect_bga_on_isa == false) {
        return;
    }
    if(is_bga_controller_present() == false) {
        return;
    }
    new_uninitialized_device();
    try_to_detect_default_graphic_controller = false;
    is_there_graphic_output_device = true;
    create_kernel_thread((uint32_t)initialize_bga_controller_with_lfb, (uint32_t []) { 0xE0000000 }, 1);
}

void initialize_bga_controller_with_lfb(void *linear_frame_buffer) {
    // read version
    uint32_t version = (bga_read(BGA_INDEX_ID) & 0xF);

    // set informations about available graphic modes
    void *lfb = (void *) linear_frame_buffer;
    monitor_mode_t modes[5];
    uint32_t number_of_modes = 3;
    modes[0].active_width = 640; modes[0].active_height = 480;
    modes[1].active_width = 800; modes[1].active_height = 600;
    modes[2].active_width = 1024; modes[2].active_height = 768;
    if(version == 4) {
        modes[3].active_width = 1600; modes[3].active_height = 1200;
        number_of_modes++;
    }
    else if(version == 5) {
        modes[3].active_width = 1600; modes[3].active_height = 1200;
        modes[4].active_width = 2560; modes[4].active_height = 1600;
        number_of_modes += 2;
    }
    for(int i = 0; i < number_of_modes; i++) {
        modes[i].bpp = 32;
        modes[i].bytes_per_line = (modes[i].active_width * 4);
        modes[i].redraw_cycles = (modes[i].active_width * modes[i].active_height);
        monitor_mode_add_functions(&modes[i]);
    }

    // // set DAC pallette 0bRRRGGGBB for 8 bpp modes
    // for(int i = 0; i < 256; i++){
    //     outb(0x3C8, i); // index
    //     outb(0x3C9, ((i >> 5) & 7) * 9); // red
    //     outb(0x3C9, ((i >> 2) & 7) * 9); // green
    //     outb(0x3C9, (i & 3) * 21); // blue
    // }

    // allocate memory for linear frame buffer
    uint32_t index_of_biggest_mode = (number_of_modes - 1);
    void *vm_linear_frame_buffer = perm_phy_alloc((uint32_t)linear_frame_buffer, modes[index_of_biggest_mode].bytes_per_line * modes[index_of_biggest_mode].active_height, VM_KERNEL | VM_WRITE_COMBINED);
    for(int i = 0; i < number_of_modes; i++) {
        modes[i].linear_frame_buffer = vm_linear_frame_buffer;
    }

    // add device to list
    bga_monitor = add_monitor_device(   100,
                                        NULL,
                                        NULL,
                                        3,
                                        number_of_modes,
                                        modes,
                                        bga_change_monitor_resolution
                                    );
    
    device_initialized();
}

/* functions for working with device */
uint32_t bga_change_monitor_resolution(monitor_t *monitor, monitor_mode_t *mode) {
    // disable BGA
    bga_write(BGA_INDEX_ENABLE, 0x0000);

    // set graphic mode
    bga_write(BGA_INDEX_WIDTH, mode->active_width);
    bga_write(BGA_INDEX_HEIGHT, mode->active_height);
    bga_write(BGA_INDEX_BPP, mode->bpp);

    // enable BGA with Linear Frame Buffer
    bga_write(BGA_INDEX_ENABLE, 0x0001 | 0x0040);

    // log
    log("\n[BGA] Monitor mode changed to %dx%dx%d", mode->active_width, mode->active_height, mode->bpp);

    return SUCCESS;
}