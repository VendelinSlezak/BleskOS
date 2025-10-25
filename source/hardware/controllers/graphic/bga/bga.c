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
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/system_call.h>
#include <kernel/x86/hardware/initialization.h>
#include <hardware/controllers/pci/pci.h>
#include <hardware/controllers/pci/device.h>
#include <hardware/devices/monitor/monitor.h>
#include <hardware/controllers/graphic/default_graphic_card/default_graphic_card.h>

/* global variables */
pci_supported_classic_devices_by_driver_t pci_bga_devices[] = {
    { 0x1234, 0x1111 },
    { NULL, NULL }
};

/* functions for communication */
word_t bga_read(byte_t index) {
    outw(BGA_REGISTER_INDEX, index);
    return inw(BGA_REGISTER_DATA);
}

void bga_write(byte_t index, word_t value) {
    outw(BGA_REGISTER_INDEX, index);
    outw(BGA_REGISTER_DATA, value);
}

/* functions for device initialization */
dword_t is_bga_controller_present(void) {
    word_t id = bga_read(BGA_INDEX_ID);
    if(id >= 0xB0C2 && id <= 0xB0C5) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void initialize_bga_controller_entity(void) {
    remove_driver_from_static_driver_list(is_default_graphic_card_present);
    dword_t entity_number = create_entity("Bochs Graphic Adapter", E_TYPE_KERNEL_RING);
    create_command_without_ca_without_fin(entity_number, initialize_bga_controller);
}
void initialize_bga_controller(void) {
    initialize_bga_controller_with_lfb((void *) 0xE0000000);
}

void initialize_pci_bga_controller_entity(pci_device_info_t device) {
    remove_driver_from_static_driver_list(is_default_graphic_card_present);
    remove_driver_from_static_driver_list(is_bga_controller_present);

    dword_t entity_number = create_entity("Bochs Graphic Adapter", E_TYPE_KERNEL_RING);
    
    e_bga_attr_t *attr = entity_get_attr_ptr(entity_number);
    memcpy(&attr->pci_device, &device, sizeof(pci_device_info_t));

    create_command_without_ca_without_fin(entity_number, initialize_pci_bga_controller);
}
void initialize_pci_bga_controller(void) {
    e_bga_attr_t *attr = command_info->entity_attributes;
    initialize_bga_controller_with_lfb((void *) pci_get_mmio(attr->pci_device, PCI_BAR0));
}

void initialize_bga_controller_with_lfb(void *linear_frame_buffer) {
    e_bga_attr_t *attr = command_info->entity_attributes;
    command_t *command = command_get_ptr(command_info->command_number);

    // log
    log("\n[BGA] Initializing entity %d...", command->entity);

    // read version
    attr->version = (bga_read(BGA_INDEX_ID) & 0xF);

    // read information about available graphic modes
    void *lfb = (void *) linear_frame_buffer;
    // TODO: lfb for modes
    monitor_mode_t modes[5];
    dword_t number_of_modes = 3;
    modes[0].active_width = 640; modes[0].active_height = 480;
    modes[1].active_width = 800; modes[1].active_height = 600;
    modes[2].active_width = 1024; modes[2].active_height = 768;
    if(attr->version == 4) {
        modes[3].active_width = 1600; modes[3].active_height = 1200;
        number_of_modes++;
    }
    else if(attr->version == 5) {
        modes[3].active_width = 1600; modes[3].active_height = 1200;
        modes[4].active_width = 2560; modes[4].active_height = 1600;
        number_of_modes += 2;
    }
    for(int i = 0; i < number_of_modes; i++) {
        modes[i].bpp = 24;
        modes[i].bytes_per_line = (modes[i].active_width * 3);
        modes[i].linear_frame_buffer = linear_frame_buffer;
    }

    // set DAC pallette 0bRRRGGGBB for 8 bpp modes
    for(int i = 0; i < 256; i++){
        outb(0x3C8, i); // index
        outb(0x3C9, ((i >> 5) & 7) * 9); // red
        outb(0x3C9, ((i >> 2) & 7) * 9); // green
        outb(0x3C9, (i & 3) * 21); // blue
    }

    // add device to list
    size_t bga_functions_for_devices[] = {
        (size_t)bga_controller_change_monitor_resolution
    };
    attr->devices[0] = create_monitor_device_entity(command->entity, bga_functions_for_devices, number_of_modes, modes, 1);
    attr->number_of_devices = 1;
    dword_t initialize_monitor_command_number = create_command_without_ca_without_fin(attr->devices[0], initialize_monitor_device);
    SC_WAIT_FOR_COMMAND(initialize_monitor_command_number);
}

/* functions for working with device */
dword_t bga_controller_change_monitor_resolution(monitor_mode_t mode) {
    // disable BGA
    bga_write(BGA_INDEX_ENABLE, 0x0000);

    // set graphic mode
    bga_write(BGA_INDEX_WIDTH, mode.active_width);
    bga_write(BGA_INDEX_HEIGHT, mode.active_height);
    bga_write(BGA_INDEX_BPP, mode.bpp);

    // enable BGA with Linear Frame Buffer
    bga_write(BGA_INDEX_ENABLE, 0x0001 | 0x0040);

    // log
    log("\n[BGA] Monitor mode successfully changed to %dx%dx%d", mode.active_width, mode.active_height, mode.bpp);

    return SUCCESS;
}