/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes*/
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/controllers/graphic/default/default.h>
#include <kernel/hardware/controllers/pci/pci.h>
#include <kernel/hardware/controllers/isa/isa.h>
#include <kernel/hardware/devices/logging/e9_device.h>
#include <kernel/hardware/devices/graphic_cards/vga.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/groups/human_input/human_input.h>
#include <kernel/hardware/subsystems/windows/windows.h>

/* global variables */
hardware_list_t *hardware_list;
uint32_t number_of_unintialized_devices = 0;

/* local variables */
uint32_t hardware_id_counter = 1;

/* functions */
void initialize_hardware(void) {
    hardware_list = (hardware_list_t *) kalloc(sizeof(hardware_list_t) + (sizeof(hardware_list_entry_t) * MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST));
}

uint32_t get_unique_hardware_id(void) {
    return __atomic_fetch_add(&hardware_id_counter, 1, __ATOMIC_SEQ_CST);
}

void add_device_to_hardware_list(uint32_t id, uint32_t type) {
    if(hardware_list->number_of_entries >= MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST) {
        return;
    }
    hardware_list->entries[hardware_list->number_of_entries].id = id;
    hardware_list->entries[hardware_list->number_of_entries].type = type;
    hardware_list->number_of_entries++;
}

void new_uninitialized_device(void) {
    __atomic_fetch_add(&number_of_unintialized_devices, 1, __ATOMIC_SEQ_CST);
}

void device_initialized(void) {
    __atomic_fetch_sub(&number_of_unintialized_devices, 1, __ATOMIC_SEQ_CST);
}

void initialize_logging(void) {
    initialize_hardware();
    initialize_logging_group();
    initialize_e9_device();
    initialize_text_mode_vga_device();
}

void initialize_hardware_list(void) {
    // initialize groups
    initialize_graphic_group();
    initialize_human_input_group();

    // initialize subsystems
    initialize_windows_subsystem();

    // initialize controllers - they will initialize devices
    initialize_pci_controller();
    initialize_default_graphic_controller();
    initialize_isa_controller();
}