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
#include <libc/string.h>
#include <kernel/cpu/commands.h>
#include <kernel/cpu/initialize.h>
#include <kernel/cpu/lapic.h>
#include <kernel/interrupt_controller/pic.h>
#include <kernel/interrupt_controller/apic.h>
#include <kernel/firmware/main.h>
#include <kernel/firmware/acpi/madt.h>
#include <kernel/firmware/mp_tables/mp_tables.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
int interrupt_mode;

/* local variables */
isa_irq_override_t isa_irq_override[16];
pci_irq_routing_t *pci_irq_routing;
int number_of_pci_routing_entries;

/* functions */
void initialize_interrupt_controllers(void) {
    memset(isa_irq_override, 0, sizeof(isa_irq_override_t) * 16);
    pci_irq_routing = NULL;
    number_of_pci_routing_entries = 0;

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

void interrupt_controller_enable_isa_interrupt(uint8_t isa_interrupt_number) {
    if(interrupt_mode == INTERRUPTS_THROUGH_PIC) {
        pic_enable_interrupt(isa_interrupt_number);
        log("\n[INTERRUPT PIC] ISA interrupt enabled for IRQ %d", isa_interrupt_number);
    }
    else if(interrupt_mode == INTERRUPTS_THROUGH_APIC) {
        connect_gsi_to_idt_through_apic(
            isa_irq_override[isa_interrupt_number].gsi,
            isa_interrupt_number + 0x20,
            APIC_DELIVERY_MODE_NORMAL,
            APIC_PHYSICAL_DESTINATION,
            isa_irq_override[isa_interrupt_number].polarity,
            isa_irq_override[isa_interrupt_number].trigger_mode,
            bootstrap_processor_id
        );
        log("\n[INTERRUPT APIC] ISA interrupt enabled for IRQ %d on GSI %d", isa_interrupt_number, isa_irq_override[isa_interrupt_number].gsi);
    }
}