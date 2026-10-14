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
#include <kernel/libc/string.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/cpu/initialize.h>
#include <kernel/hardware/devices/cpu/lapic.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/interrupt_controllers/pic.h>
#include <kernel/hardware/devices/interrupt_controllers/apic.h>
#include <kernel/firmware/main.h>
#include <kernel/firmware/acpi/madt.h>
#include <kernel/firmware/mp_tables/mp_tables.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/controllers/pci/pci.h>

/* global variables */
int interrupt_mode;

/* local variables */
isa_irq_override_t isa_irq_override[16];
pci_irq_routing_t *pci_irq_routing;
int number_of_pci_routing_entries;
uint32_t pci_idt_vector;
uint32_t pci_idt_max_vector;

/* functions */
void initialize_interrupt_controllers(void) {
    memset(isa_irq_override, 0, sizeof(isa_irq_override_t) * 16);
    pci_irq_routing = NULL;
    number_of_pci_routing_entries = 0;
    pci_idt_vector = 0x30;
    pci_idt_max_vector = 0xEF;

    if(lapic_base != NULL || firmware_info.madt != NULL || firmware_info.mpct != NULL) {
        // read info about IO APICs
        io_apics = kalloc(sizeof(io_apic_info_t) * MAX_NUMBER_OF_IOAPICS_ON_SYSTEM);
        if(firmware_info.madt != NULL) {
            madt_read_all_io_apics(io_apics, &number_of_io_apics);
        }
        else if(firmware_info.mpct != NULL) {
            mp_tables_read_all_io_apics(io_apics, &number_of_io_apics);
        }
        io_apics = krealloc(io_apics, sizeof(io_apic_info_t) * number_of_io_apics);

        if(number_of_io_apics != 0) {
            // read ISA IRQ remappings
            if(firmware_info.madt != NULL) {
                madt_read_isa_irq_overrides(isa_irq_override);
            }
            else if(firmware_info.mpct != NULL) {
                mp_tables_read_isa_irq_overrides(isa_irq_override);
            }

            // read PCI IRQ routing
            // TODO: read PCI IRQ routing from DSDT table if present
            if(firmware_info.mpct != NULL) {
                mp_tables_read_pci_irq_routing(&pci_irq_routing, &number_of_pci_routing_entries);
            }

            // disable PIC
            disable_pic();

            // enable APIC mode on old systems
            if(firmware_info.mpfp != NULL && ((mp_floating_pointer_table_t *) firmware_info.mpfp)->imcr_present == 1) {
                outb(0x22, 0x70); // select Interrupt Managment Control Register
                outb(0x23, 0x01); // route interrupts to APIC
            }

            // initialize IO APICs
            interrupt_mode = INTERRUPTS_THROUGH_APIC;
            for(int i = 0; i < number_of_io_apics; i++) {
                initialize_apic(&io_apics[i]);
            }

            // enable interrupts
            sti();

            return;
        }
    }

    // initialize PIC mode instead
    interrupt_mode = INTERRUPTS_THROUGH_PIC;
    initialize_pic();

    // enable interrupts
    sti();
}

interrupt_description_t interrupt_controller_enable_isa_interrupt(uint8_t isa_interrupt_number) {
    interrupt_description_t interrupt;
    interrupt.idt_vector = 0x20 + isa_interrupt_number;
    if(interrupt_mode == INTERRUPTS_THROUGH_PIC) {
        interrupt.processor_index = bootstrap_processor_index;
        pic_enable_interrupt(isa_interrupt_number);
        log("\n[INTERRUPT PIC] ISA interrupt enabled for IRQ %d", isa_interrupt_number);
        interrupt.success = SUCCESS;
        return interrupt;
    }
    else if(interrupt_mode == INTERRUPTS_THROUGH_APIC) {
        interrupt.processor_index = bootstrap_processor_index;
        connect_gsi_to_idt_through_apic(
            isa_irq_override[isa_interrupt_number].gsi,
            0x20 + isa_interrupt_number,
            APIC_DELIVERY_MODE_NORMAL,
            APIC_PHYSICAL_DESTINATION,
            isa_irq_override[isa_interrupt_number].polarity,
            isa_irq_override[isa_interrupt_number].trigger_mode,
            bootstrap_processor_id
        );
        log("\n[INTERRUPT APIC] ISA interrupt enabled for IRQ %d on GSI %d", isa_interrupt_number, isa_irq_override[isa_interrupt_number].gsi);
        interrupt.success = SUCCESS;
        return interrupt;
    }
    interrupt.success = ERROR;
    return interrupt;
}

interrupt_description_t interrupt_controller_enable_pci_interrupt(hardware_t *device) {
    interrupt_description_t interrupt;
    pci_device_data_t *data = device->data_from_controller;
    uint8_t bus = data->bus;
    uint8_t device_id = data->device;
    uint8_t pin = data->interrupt_pin;
    uint8_t pin_char = (pin + 'A' - 1);
    if(interrupt_mode == INTERRUPTS_THROUGH_PIC) {
        if(data->interrupt_line == 0 || data->interrupt_line >= 16) {
            log("\n[INTERRUPT PIC] ERROR: Invalid PCI interrupt line %d", data->interrupt_line);
            interrupt.success = ERROR;
            return interrupt;
        }
        interrupt.idt_vector = data->interrupt_line;
        interrupt.processor_index = bootstrap_processor_index;
        pic_enable_interrupt(data->interrupt_line);
        log("\n[INTERRUPT PIC] PCI device %d:%d:%c has interrupts enabled for IRQ %d", bus, device_id, pin_char, data->interrupt_line);
        interrupt.success = SUCCESS;
        return interrupt;
    }
    else if(interrupt_mode == INTERRUPTS_THROUGH_APIC) {
        if(pci_idt_vector == pci_idt_max_vector) {
            // TODO: add support for multiple functions on one IDT vector
            log("\n[INTERRUPT APIC] ERROR: No more IDT vectors available for PCI interrupts");
            interrupt.success = ERROR;
            return interrupt;
        }
        uint32_t vector = pci_idt_vector++;
        for(int i = 0; i < number_of_pci_routing_entries; i++) {
            if(pci_irq_routing[i].bus == bus && pci_irq_routing[i].device == device_id && pci_irq_routing[i].pin == pin) {
                interrupt.idt_vector = vector;
                interrupt.processor_index = bootstrap_processor_index;
                connect_gsi_to_idt_through_apic(
                    pci_irq_routing[i].gsi,
                    vector,
                    APIC_DELIVERY_MODE_NORMAL,
                    APIC_PHYSICAL_DESTINATION,
                    pci_irq_routing[i].polarity,
                    pci_irq_routing[i].trigger_mode,
                    bootstrap_processor_id
                );
                log("\n[INTERRUPT APIC] PCI device %d:%d:%c has interrupts enabled on GSI %d to IDT vector 0x%02X", bus, device_id, pin_char, pci_irq_routing[i].gsi, vector);
                interrupt.success = SUCCESS;
                return interrupt;
            }
        }
        log("\n[INTERRUPT APIC] Routing for PCI device %d:%d:%c was not found", bus, device_id, pin_char);
        interrupt.success = ERROR;
        return interrupt;
    }
    interrupt.success = ERROR;
    return interrupt;
}