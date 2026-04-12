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
#include <kernel/firmware/main.h>
#include <kernel/interrupt_controller/main.h>
#include <kernel/interrupt_controller/apic.h>
#include <kernel/cpu/initialize.h>
#include <kernel/cpu/info.h>
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void dump_madt_table(void) {
    if(firmware_info.madt == NULL) {
        log("\n No MADT table found");
        return;
    }

    madt_table_t *madt = (madt_table_t *) firmware_info.madt;
    log("\n Local APIC address: 0x%x | Flags: %x", madt->local_apic_address, madt->flags);

    uint32_t offset = sizeof(madt_table_t);
    while(offset < madt->header.length) {
        madt_entry_header_t *entry = (madt_entry_header_t *) &madt->entries[offset - sizeof(madt_table_t)];
        if(entry->type == MADT_TYPE_LOCAL_APIC && (offset + sizeof(madt_local_apic_t)) <= madt->header.length) {
            madt_local_apic_t *local_apic = (madt_local_apic_t *) entry;
            log("\n Local APIC | ACPI processor ID: %d | APIC ID: %d | Enabled: %d | Online capable: %d", local_apic->acpi_processor_id, local_apic->apic_id, local_apic->enabled, local_apic->online_capable);
        }
        else if(entry->type == MADT_TYPE_IO_APIC && (offset + sizeof(madt_io_apic_t)) <= madt->header.length) {
            madt_io_apic_t *io_apic = (madt_io_apic_t *) entry;
            log("\n IO APIC | IO APIC ID: %d | IO APIC address: 0x%x | GSI base: %d", io_apic->io_apic_id, io_apic->io_apic_address, io_apic->global_system_interrupt_base);
        }
        else if(entry->type == MADT_TYPE_INTERRUPT_OVERRIDE && (offset + sizeof(madt_interrupt_override_t)) <= madt->header.length) {
            madt_interrupt_override_t *irq_override = (madt_interrupt_override_t *) entry;
            log("\n Interrupt override | Bus source: %d | IRQ source: %d | GSI: %d | Flags: %x", irq_override->bus_source, irq_override->irq_source, irq_override->global_system_interrupt, irq_override->flags);
        }
        else if(entry->type == MADT_TYPE_NMI_SOURCE && (offset + sizeof(madt_nmi_source_t)) <= madt->header.length) {
            madt_nmi_source_t *nmi_source = (madt_nmi_source_t *) entry;
            log("\n NMI source | Flags: %x | GSI: %d", nmi_source->flags, nmi_source->global_system_interrupt);
        }
        else if(entry->type == MADT_TYPE_LOCAL_APIC_NMI && (offset + sizeof(madt_local_apic_nmi_t)) <= madt->header.length) {
            madt_local_apic_nmi_t *local_apic_nmi = (madt_local_apic_nmi_t *) entry;
            log("\n Local APIC NMI | ACPI processor ID: %d | Flags: %x | LINT#: %d", local_apic_nmi->acpi_processor_id, local_apic_nmi->flags, local_apic_nmi->local_apic_lint);
        }
        else if(entry->type == MADT_TYPE_LOCAL_APIC_OVERRIDE && (offset + sizeof(madt_local_apic_override_t)) <= madt->header.length) {
            madt_local_apic_override_t *local_apic_override = (madt_local_apic_override_t *) entry;
            log("\n Local APIC override | Address: 0x%x%x", (uint32_t)(local_apic_override->local_apic_address >> 32), (uint32_t)local_apic_override->local_apic_address);
        }
        else {
            log("\n Unknown MADT entry type: %d", entry->type);
        }

        offset += entry->length;
    }
}

void madt_read_all_io_apics(io_apic_info_t *io_apics, uint32_t *number_of_io_apics) {
    *number_of_io_apics = 0;

    if(firmware_info.madt == NULL) {
        return;
    }

    madt_table_t *madt = (madt_table_t *) firmware_info.madt;
    uint32_t offset = sizeof(madt_table_t);
    while(offset < madt->header.length) {
        madt_entry_header_t *entry = (madt_entry_header_t *) &madt->entries[offset - sizeof(madt_table_t)];
        if(entry->type == MADT_TYPE_IO_APIC && (offset + sizeof(madt_io_apic_t)) <= madt->header.length) {
            madt_io_apic_t *io_apic = (madt_io_apic_t *) entry;
            io_apics[*number_of_io_apics].id = io_apic->io_apic_id;
            io_apics[*number_of_io_apics].mmio_address = io_apic->io_apic_address;
            io_apics[*number_of_io_apics].base_gsi = io_apic->global_system_interrupt_base;
            (*number_of_io_apics)++;
        }
        offset += entry->length;
    }
}

void madt_read_isa_irq_overrides(isa_irq_override_t *overrides) {
    memset(overrides, 0, sizeof(isa_irq_override_t) * 16);

    if(firmware_info.madt == NULL) {
        return;
    }

    // parse info from table
    madt_table_t *madt = (madt_table_t *) firmware_info.madt;
    uint32_t offset = sizeof(madt_table_t);
    while(offset < madt->header.length) {
        madt_entry_header_t *entry = (madt_entry_header_t *) &madt->entries[offset - sizeof(madt_table_t)];
        if(entry->type == MADT_TYPE_INTERRUPT_OVERRIDE && (offset + sizeof(madt_interrupt_override_t)) <= madt->header.length) {
            madt_interrupt_override_t *irq_override = (madt_interrupt_override_t *) entry;
            if(irq_override->irq_source < 16) {
                overrides[irq_override->irq_source].is_overridden = true;
                overrides[irq_override->irq_source].gsi = irq_override->global_system_interrupt;
                overrides[irq_override->irq_source].polarity = ((irq_override->flags >> 1) & 0x1);
                overrides[irq_override->irq_source].trigger_mode = ((irq_override->flags >> 3) & 0x1);
            }
        }
        offset += entry->length;
    }

    // set rest of interrupts to default state
    for(int i = 0; i < 16; i++) {
        if(overrides[i].is_overridden == false) {
            overrides[i].is_overridden = true;
            overrides[i].gsi = i;
            overrides[i].polarity = 0;      // high active
            overrides[i].trigger_mode = 0;  // edge triggered
        }
    }
}

void madt_read_info_about_logical_processors(uint32_t *number_of_logical_processors, logical_processor_info_t *logical_processors) {
    *number_of_logical_processors = 0;

    if(firmware_info.madt == NULL) {
        return;
    }

    madt_table_t *madt = (madt_table_t *) firmware_info.madt;
    uint32_t offset = sizeof(madt_table_t);
    while(offset < madt->header.length) {
        madt_entry_header_t *entry = (madt_entry_header_t *) &madt->entries[offset - sizeof(madt_table_t)];
        if(entry->type == MADT_TYPE_LOCAL_APIC && (offset + sizeof(madt_local_apic_t)) <= madt->header.length) {
            madt_local_apic_t *local_apic = (madt_local_apic_t *) entry;
            if(local_apic->enabled == 1 || local_apic->online_capable == 1) {
                logical_processors[*number_of_logical_processors].hardware_id = local_apic->apic_id;
                (*number_of_logical_processors)++;
                if(*number_of_logical_processors >= MAX_NUMBER_OF_LOGICAL_PROCESSORS_ON_SYSTEM) {
                    return;
                }
            }
        }
        offset += entry->length;
    }
}