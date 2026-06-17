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
#include <kernel/hardware/controllers/graphic/bga/bga.h>
#include <kernel/hardware/controllers/pci/pci.h>
#include <kernel/hardware/controllers/isa/isa.h>
#include <kernel/hardware/devices/logging/e9_device.h>
#include <kernel/hardware/devices/graphic_cards/vga.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/groups/human_input/human_input.h>
#include <kernel/hardware/subsystems/windows/windows.h>
#include <kernel/cpu/mutex.h>
#include <kernel/cpu/scheduler.h>

/* global variables */
virtual_hardware_list_t *virtual_hardware_list;
hardware_t *motherboard;

/* local variables */
uint32_t hardware_id_counter = 1;
mutex_t hardware_list_spinlock;

/* functions */
void initialize_hardware_structs(void) {
    virtual_hardware_list = (virtual_hardware_list_t *) kalloc(sizeof(virtual_hardware_list_t) + (sizeof(virtual_hardware_list_entry_t) * MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST));
    motherboard = kalloc(sizeof(hardware_t));
    motherboard->id = get_unique_hardware_id();
    motherboard->name = "Motherboard";
    motherboard->is_initialized = true;
}

uint32_t get_unique_hardware_id(void) {
    return __atomic_fetch_add(&hardware_id_counter, 1, __ATOMIC_SEQ_CST);
}

void release_unique_hardware_id(uint32_t id) {
    // TODO:
    return;
}

void add_virtual_device_to_hardware_list(uint32_t type) {
    if(virtual_hardware_list->number_of_entries >= MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST) {
        return;
    }
    virtual_hardware_list->entries[virtual_hardware_list->number_of_entries].type = type;
    virtual_hardware_list->number_of_entries++;
}

uint32_t does_virtual_device_exist(uint32_t type) {
    for(int i = 0; i < virtual_hardware_list->number_of_entries; i++) {
        if(virtual_hardware_list->entries[i].type == type) {
            return true;
        }
    }
    return false;
}

void initialize_logging(void) {
    initialize_logging_group();
    if(does_e9_device_exist() == true) {
        hardware_t *e9_device = add_hardware(motherboard, "E9 logging device", NULL, NULL, NULL, NULL);
        initialize_e9_device(e9_device);
    }
    if(does_text_mode_vga_device_exist() == true) {
        hardware_t *vga_device = add_hardware(motherboard, "VGA text mode", NULL, NULL, NULL, NULL);
        initialize_text_mode_vga_device(vga_device);
    }
}

void initialize_hardware_list(void) {
    // initialize groups
    initialize_graphic_group();
    initialize_human_input_group();

    // initialize subsystems
    initialize_windows_subsystem();

    // TODO: add all controllers based on data from ACPI tables
    pci_controller = add_hardware(motherboard, "PCI controller", NULL, NULL, initialize_pci_controller, NULL);
    isa_controller = add_hardware(motherboard, "ISA controller", NULL, NULL, initialize_isa_controller, NULL);
    init_hardware(pci_controller);
    init_hardware(isa_controller);

    while(pci_controller->is_initialized == false) {
        asm volatile("pause");
        switch_to_another_thread();
    }
    if(is_bga_controller_on_pci == false && is_bga_controller_present() == true) {
        hardware_t *bga = add_hardware(isa_controller, "BGA", NULL, NULL, initialize_bga_controller, NULL);
        init_hardware(bga);
        while(bga->is_initialized == false) {
            asm volatile("pause");
            switch_to_another_thread();
        }
    }
    if(try_to_detect_default_graphic_controller == true && is_default_graphic_controller_present() == true) {
        hardware_t *dgc = add_hardware(motherboard, "Default graphic controller", NULL, NULL, initialize_default_graphic_controller, NULL);
        init_hardware(dgc);
        while(dgc->is_initialized == false) {
            asm volatile("pause");
            switch_to_another_thread();
        }
    }
}

hardware_t *add_hardware(hardware_t *controller, uint8_t *name, void *communication_functions, void *data_from_controller, void (*init)(hardware_t *self), void (*remove)(hardware_t *self)) {
    log("\n[HARDWARE] New hardware %s", name);
    
    hardware_t *new_hardware = kalloc(sizeof(hardware_t));
    new_hardware->id = get_unique_hardware_id();
    new_hardware->name = name;
    new_hardware->controller = controller;
    new_hardware->communication_functions = communication_functions;
    new_hardware->data_from_controller = data_from_controller;
    new_hardware->init = init;
    new_hardware->remove = remove;

    LOCK_MUTEX(&hardware_list_spinlock);
    if(controller->first_device == NULL) {
        controller->first_device = new_hardware;
    }
    else {
        hardware_t *current_hardware = controller->first_device;
        while(current_hardware->next != NULL) {
            current_hardware = current_hardware->next;
        }
        current_hardware->next = new_hardware;
        new_hardware->prev = current_hardware;
    }
    UNLOCK_MUTEX(&hardware_list_spinlock);

    if(new_hardware->init == NULL) {
        new_hardware->is_initialized = true;
    }

    return new_hardware;
}

void init_hardware(hardware_t *hardware) {
    if(hardware->init != NULL) {
        create_kernel_thread((uint32_t) hardware->init, (uint32_t []) { (uint32_t) hardware }, 1);
    }
}

void synchronous_init_hardware(hardware_t *hardware) {
    if(hardware->init != NULL) {
        hardware->init(hardware);
    }
}

void remove_hardware(hardware_t *hardware) {
    LOCK_MUTEX(&hardware_list_spinlock);
    static hardware_t first_hardware = {0};
    first_hardware.first_device = hardware;
    if(hardware->prev == NULL) {
        hardware->controller->first_device = hardware->next;
        if(hardware->next != NULL) {
            hardware->next->prev = NULL;
        }
    }
    else {
        hardware->prev->next = hardware->next;
        if(hardware->next != NULL) {
            hardware->next->prev = hardware->prev;
        }
    }
    hardware->controller = (hardware_t *) &first_hardware;
    hardware_t *current_hardware = (hardware_t *) &first_hardware;
    do {
        if(current_hardware->first_device != NULL) {
            current_hardware = current_hardware->first_device;
        }
        else {
            hardware_t *hardware_struct_to_remove = current_hardware;
            if(hardware_struct_to_remove->remove != NULL) {
                hardware_struct_to_remove->remove(hardware_struct_to_remove);
            }

            release_unique_hardware_id(hardware_struct_to_remove->id);
            if(hardware_struct_to_remove->data != NULL) {
                kfree(hardware_struct_to_remove->data);
            }
            if(hardware_struct_to_remove->data_from_controller != NULL) {
                kfree(hardware_struct_to_remove->data_from_controller);
            }

            if(hardware_struct_to_remove->next != NULL) {
                current_hardware = hardware_struct_to_remove->next;
            }
            else {
                hardware_struct_to_remove->controller->first_device = NULL;
                current_hardware = hardware_struct_to_remove->controller;
            }

            kfree(hardware_struct_to_remove);
        }
    } while(current_hardware != (hardware_t *) &first_hardware);
    UNLOCK_MUTEX(&hardware_list_spinlock);
}

uint32_t is_all_hardware_initialized(void) {
    hardware_t *current_hardware = motherboard;
    while(current_hardware != NULL) {
        if(current_hardware->is_initialized == false) {
            return false;
        }
        if(current_hardware->first_device != NULL) {
            current_hardware = current_hardware->first_device;
        }
        else if(current_hardware->next != NULL) {
            current_hardware = current_hardware->next;
        }
        else {
            current_hardware = current_hardware->controller->next;
        }
    }
    return true;
}

void dump_hardware_list(void) {
    hardware_t *current_hardware = motherboard;
    uint32_t depth = 0;
    while(current_hardware != NULL) {
        log("\n");
        for(int i = 0; i < depth; i++) {
            log("  ");
        }
        log("%s", current_hardware->name);
        if(current_hardware->first_device != NULL) {
            depth++;
            current_hardware = current_hardware->first_device;
        }
        else if(current_hardware->next != NULL) {
            current_hardware = current_hardware->next;
        }
        else {
            hardware_t *controller = current_hardware->controller;
            current_hardware = controller;
            while(controller != NULL) {
                if(controller->next != NULL) {
                    current_hardware = controller->next;
                    break;
                }
                else {
                    controller = controller->controller;
                    current_hardware = controller;
                    depth--;
                }
            }
            depth--;
        }
    }
}

uint32_t how_many_devices_are_uninitalized(void) {
    LOCK_MUTEX(&hardware_list_spinlock);
    uint32_t count = 0;
    hardware_t *current_hardware = motherboard;
    while(current_hardware != NULL) {
        if(current_hardware->is_initialized == false) {
            count++;
        }
        if(current_hardware->first_device != NULL) {
            current_hardware = current_hardware->first_device;
        }
        else if(current_hardware->next != NULL) {
            current_hardware = current_hardware->next;
        }
        else {
            hardware_t *controller = current_hardware->controller;
            current_hardware = controller;
            while(controller != NULL) {
                if(controller->next != NULL) {
                    current_hardware = controller->next;
                    break;
                }
                else {
                    controller = controller->controller;
                    current_hardware = controller;
                }
            }
        }
    }
    UNLOCK_MUTEX(&hardware_list_spinlock);
    // log("\nThere are %d uninitialized devices", count);
    return count;
}