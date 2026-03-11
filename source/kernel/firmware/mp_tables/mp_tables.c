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
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/interrupt_controller/main.h>
#include <kernel/interrupt_controller/apic.h>
#include <kernel/cpu/initialize.h>
#include <kernel/cpu/info.h>
#include <kernel/hardware/groups/logging/logging.h>

/* local variables */
uint32_t isa_bus_id;
uint8_t is_bus_pci[256];

/* functions */
void search_for_mp_floating_pointer_table(void) {
    mp_floating_pointer_table_t *mpfp = NULL;

    // search in first KB of EBDA or last KB of conventional memory
    if(firmware_info.ebda != NULL) {
        mpfp = search_for_mp_floating_pointer_table_in_memory(firmware_info.ebda, 0x400);
    }
    else {
        mpfp = search_for_mp_floating_pointer_table_in_memory(firmware_info.last_kb_of_conventional_memory, 0x400);
    }

    // search in 0xE0000 - 0xFFFFF
    if(mpfp == NULL) {
        mpfp = search_for_mp_floating_pointer_table_in_memory(firmware_info.bios_code + 0x00000, 0x20000);
    }

    // if we did not found anything, return
    if(mpfp == NULL) {
        return;
    }
    firmware_info.mpfp = (uint8_t *) mpfp;

    // log table content
    log("\nMP Floating Pointer");
    log("\n Version: %d", mpfp->specification_revision);
    log("\n MP Configuration Table physical address: 0x%x", mpfp->mp_configuration_table_pm);
    log("\n System configuration type: %d", mpfp->mp_system_configuration_type);
    log("\n IMCR: %s", (mpfp->imcr_present == 1) ? "PIC mode" : "Virtual Wire mode");

    // MP Configuration Table
    // TODO: add static configuration types
    if(mpfp->mp_system_configuration_type != 0) {
        log("\n Warning: Unsupported Configuration type");
    }
    if(mpfp->mp_configuration_table_pm == 0) {
        return;
    }
    mp_configuration_table_header_t *mpct = temp_phy_alloc(mpfp->mp_configuration_table_pm, sizeof(mp_configuration_table_header_t), VM_KERNEL | VM_UNCACHEABLE);
    if(mpct->signature != 0x504D4350) { // "PCMP"
        log("\n Error: MP Configuration Table has invalid signature");
        return;
    }
    mpct = temp_phy_alloc(mpfp->mp_configuration_table_pm, mpct->base_table_length, VM_KERNEL | VM_UNCACHEABLE);
    if(!firmware_table_checksum_validation((uint8_t *) mpct, mpct->base_table_length)) {
        log("\n Error: MP Configuration Table has invalid checksum");
        return;
    }
    firmware_info.mpct = (uint8_t *) mpct;
    log("\nMP Configuration Table | Revision: %d | OEM: %08s | Product ID: %012s", mpct->specification_revision, mpct->oem_id, mpct->product_id);
    log("\n LAPIC address: 0x%x", mpct->mmio_lapic_address);

    // TODO: print OEM table

    // go through all base entries
    uint8_t *entry_ptr = (uint8_t *) mpct + sizeof(mp_configuration_table_header_t);
    uint8_t *mpct_end = (uint8_t *) mpct + mpct->base_table_length;
    isa_bus_id = INVALID;
    memset(is_bus_pci, false, sizeof(is_bus_pci));
    for(int i = 0; i < mpct->entry_count; i++) {
        if(entry_ptr >= mpct_end) {
            log("\n Error: MP Configuration Table entries exceed table length");
            break;
        }

        uint8_t entry_type = entry_ptr[0];
        if(entry_type == 0) {
            log("\n Processor");
            mp_processor_entry_t *entry = (mp_processor_entry_t *) entry_ptr;
            log("\n  LAPIC ID: %d", entry->lapic_id);
            log("\n  LAPIC Version: 0x%x", entry->lapic_version);
            log("\n  CPU Enabled: %s", (entry->cpu_enabled) ? "Yes" : "No");
            log("\n  CPU Bootstrap Processor: %s", (entry->cpu_bootstrap) ? "Yes" : "No");
            log("\n  CPU Family: %d", entry->cpu_family);
            log("\n  CPU Model: %d", entry->cpu_model);
            log("\n  CPU Stepping: %d", entry->cpu_stepping);
            log("\n  FPU Present: %s", (entry->fpu_present) ? "Yes" : "No");
            log("\n  Machine Check Exception Present: %s", (entry->machine_check_exception_present) ? "Yes" : "No");
            log("\n  CMPXCHG8B Present: %s", (entry->cmpxchg8b_present) ? "Yes" : "No");
            log("\n  APIC Present: %s", (entry->apic_present) ? "Yes" : "No");
            entry_ptr += sizeof(mp_processor_entry_t);
        }
        else if(entry_type == 1) {
            log("\n Bus");
            mp_bus_entry_t *entry = (mp_bus_entry_t *) entry_ptr;
            log("\n  Bus ID: %d", entry->bus_id);
            log("\n  Bus Type: %06s", entry->bus_type_string);
            if(strncmp((char *) entry->bus_type_string, "ISA   ", 6) == 0) {
                if(isa_bus_id == INVALID) {
                    isa_bus_id = entry->bus_id;
                }
                else {
                    log("\n Warning: Multiple ISA buses found, using first one");
                }
            }
            else if(strncmp((char *) entry->bus_type_string, "PCI   ", 6) == 0) {
                is_bus_pci[entry->bus_id] = true;
            }
            entry_ptr += sizeof(mp_bus_entry_t);
        }
        else if(entry_type == 2) {
            log("\n I/O APIC");
            mp_io_apic_entry_t *entry = (mp_io_apic_entry_t *) entry_ptr;
            log("\n  I/O APIC ID: %d", entry->io_apic_id);
            log("\n  I/O APIC Version: 0x%x", entry->io_apic_version);
            log("\n  I/O APIC Address: 0x%x", entry->io_apic_address);
            log("\n  I/O APIC Usable: %s", (entry->io_apic_usable) ? "Yes" : "No");
            entry_ptr += sizeof(mp_io_apic_entry_t);
        }
        else if(entry_type == 3) {
            log("\n I/O Interrupt Assignment");
            mp_interrupt_assignment_entry_t *entry = (mp_interrupt_assignment_entry_t *) entry_ptr;
            log(" from Bus %d ", entry->source_bus_id);
            if(is_bus_pci[entry->source_bus_id] == true) {
                log("Device %d PIN %c ", (entry->source_bus_irq >> 2) & 0x1F, 'A' + (entry->source_bus_irq & 0b11));
            }
            else {
                log("IRQ %d ", entry->source_bus_irq);
            }
            switch(entry->interrupt_type) {
                case 0b00: log("INT"); break;
                case 0b01: log("NMI"); break;
                case 0b10: log("SMI"); break;
                case 0b11: log("ExtINT"); break;
            }
            log(" / ");
            switch(entry->interrupt_polarity) {
                case 0b00: log("Conforms to bus"); break;
                case 0b01: log("Active High"); break;
                case 0b10: log("Reserved"); break;
                case 0b11: log("Active Low"); break;
            }
            log(" / ");
            switch(entry->interrupt_trigger_mode) {
                case 0b00: log("Conforms to bus"); break;
                case 0b01: log("Edge Triggered"); break;
                case 0b10: log("Reserved"); break;
                case 0b11: log("Level Triggered"); break;
            }
            log(" to ");
            if(entry->destination_io_apic_id != 0xFF) {
                log("I/O APIC %d", entry->destination_io_apic_id);
            }
            else {
                log("all I/O APICs");
            }
            log(" pin %d", entry->destination_io_apic_intin);
            entry_ptr += sizeof(mp_interrupt_assignment_entry_t);
        }
        else if(entry_type == 4) {
            log("\n Local Interrupt Assignment");
            mp_local_interrupt_assignment_entry_t *entry = (mp_local_interrupt_assignment_entry_t *) entry_ptr;
            log(" from Bus %d IRQ %d ", entry->source_bus_id, entry->source_bus_irq);
            switch(entry->interrupt_type) {
                case 0b00: log("INT"); break;
                case 0b01: log("NMI"); break;
                case 0b10: log("SMI"); break;
                case 0b11: log("ExtINT"); break;
            }
            log(" / ");
            switch(entry->interrupt_polarity) {
                case 0b00: log("Conforms to bus"); break;
                case 0b01: log("Active High"); break;
                case 0b10: log("Reserved"); break;
                case 0b11: log("Active Low"); break;
            }
            log(" / ");
            switch(entry->interrupt_trigger_mode) {
                case 0b00: log("Conforms to bus"); break;
                case 0b01: log("Edge Triggered"); break;
                case 0b10: log("Reserved"); break;
                case 0b11: log("Level Triggered"); break;
            }
            log(" to ");
            if(entry->destination_lapic_id != 0xFF) {
                log("I/O APIC %d", entry->destination_lapic_id);
            }
            else {
                log("all I/O APICs");
            }
            log(" pin %d", entry->destination_lapic_intin);
            entry_ptr += sizeof(mp_local_interrupt_assignment_entry_t);
        }
        else {
            log("\n Unknown entry type: %d", entry_type);
            break;
        }
    }

    // TODO: print all extended entries
}

mp_floating_pointer_table_t *search_for_mp_floating_pointer_table_in_memory(void *start, uint32_t size) {    
    for(uint32_t offset = 0; offset < (size - sizeof(mp_floating_pointer_table_t)); offset += 16) {
        mp_floating_pointer_table_t *mpfp = (mp_floating_pointer_table_t *) (start + offset);
        if(    mpfp->signature != 0x5F504D5F  // "_MP_"
            || (offset + sizeof(mp_floating_pointer_table_t)) > size
            || mpfp->length != 0x01
            || !firmware_table_checksum_validation((uint8_t *) mpfp, sizeof(mp_floating_pointer_table_t))) {
            continue;
        }

        return mpfp;
    }

    return NULL;
}

void mp_tables_read_all_io_apics(io_apic_info_t *io_apics, uint32_t *number_of_io_apics) {
    *number_of_io_apics = 0;

    if(firmware_info.mpct == NULL) {
        return;
    }

    // parse info from table
    mp_configuration_table_header_t *mpct = (mp_configuration_table_header_t *) firmware_info.mpct;
    uint8_t *entry_ptr = (uint8_t *) mpct + sizeof(mp_configuration_table_header_t);
    uint8_t *mpct_end = (uint8_t *) mpct + mpct->base_table_length;
    uint32_t base_gsi = 0;
    for(int i = 0; i < mpct->entry_count; i++) {
        if(entry_ptr >= mpct_end) {
            break;
        }

        uint8_t entry_type = entry_ptr[0];
        if(entry_type == 2) {
            mp_io_apic_entry_t *entry = (mp_io_apic_entry_t *) entry_ptr;
            if(entry->io_apic_usable == 1) {
                io_apics[*number_of_io_apics].id = entry->io_apic_id;
                io_apics[*number_of_io_apics].mmio_address = entry->io_apic_address;
                io_apics[*number_of_io_apics].base_gsi = base_gsi; // MP tables do not provide GSI base info, but we reserve 256 GSI for each IO APIC
                base_gsi += 256;
                (*number_of_io_apics)++;
            }
        }

        // move to next entry
        switch(entry_type) {
            case 0: entry_ptr += sizeof(mp_processor_entry_t); break;
            case 1: entry_ptr += sizeof(mp_bus_entry_t); break;
            case 2: entry_ptr += sizeof(mp_io_apic_entry_t); break;
            case 3: entry_ptr += sizeof(mp_interrupt_assignment_entry_t); break;
            case 4: entry_ptr += sizeof(mp_local_interrupt_assignment_entry_t); break;
            default: return;
        }
    }
}

void mp_tables_read_isa_irq_overrides(isa_irq_override_t *overrides) {
    memset(overrides, 0, sizeof(isa_irq_override_t) * 16);

    if(firmware_info.mpct == NULL || isa_bus_id == INVALID) {
        return;
    }

    // parse info from table
    mp_configuration_table_header_t *mpct = (mp_configuration_table_header_t *) firmware_info.mpct;
    uint8_t *entry_ptr = (uint8_t *) mpct + sizeof(mp_configuration_table_header_t);
    uint8_t *mpct_end = (uint8_t *) mpct + mpct->base_table_length;
    for(int i = 0; i < mpct->entry_count; i++) {
        if(entry_ptr >= mpct_end) {
            break;
        }

        uint8_t entry_type = entry_ptr[0];
        if(entry_type == 3) {
            mp_interrupt_assignment_entry_t *entry = (mp_interrupt_assignment_entry_t *) entry_ptr;
            if(entry->source_bus_id == isa_bus_id && entry->source_bus_irq < 16) {
                overrides[entry->source_bus_irq].is_overridden = true;

                // search for GSI base of IO APIC
                overrides[entry->source_bus_irq].gsi = INVALID;
                for(int j = 0; j < number_of_io_apics; j++) {
                    if(io_apics[j].id == entry->destination_io_apic_id) {
                        overrides[entry->source_bus_irq].gsi = io_apics[j].base_gsi + entry->destination_io_apic_intin; // set correct GSI
                        break;
                    }
                }
                if(overrides[entry->source_bus_irq].gsi == INVALID) {
                    overrides[entry->source_bus_irq].is_overridden = false;
                    log("\n Warning: Could not find IO APIC for ISA IRQ %d override", entry->source_bus_irq);
                }
                else {
                    // set interrupt polarity
                    if(entry->interrupt_polarity == 0b00) {
                        overrides[entry->source_bus_irq].polarity = 0; // conforms to bus = active high
                    }
                    else if(entry->interrupt_polarity == 0b01) {
                        overrides[entry->source_bus_irq].polarity = 0; // active high
                    }
                    else if(entry->interrupt_polarity == 0b11) {
                        overrides[entry->source_bus_irq].polarity = 1; // active low
                    }
                    else {
                        overrides[entry->source_bus_irq].polarity = 0; // reserved, treat as conforms to bus
                    }

                    // set interrupt trigger mode
                    if(entry->interrupt_trigger_mode == 0b00) {
                        overrides[entry->source_bus_irq].trigger_mode = 0; // conforms to bus = edge triggered
                    }
                    else if(entry->interrupt_trigger_mode == 0b01) {
                        overrides[entry->source_bus_irq].trigger_mode = 0; // edge triggered
                    }
                    else if(entry->interrupt_trigger_mode == 0b11) {
                        overrides[entry->source_bus_irq].trigger_mode = 1; // level triggered
                    }
                    else {
                        overrides[entry->source_bus_irq].trigger_mode = 0; // reserved, treat as conforms to bus
                    }
                }
            }
        }

        // move to next entry
        switch(entry_type) {
            case 0: entry_ptr += sizeof(mp_processor_entry_t); break;
            case 1: entry_ptr += sizeof(mp_bus_entry_t); break;
            case 2: entry_ptr += sizeof(mp_io_apic_entry_t); break;
            case 3: entry_ptr += sizeof(mp_interrupt_assignment_entry_t); break;
            case 4: entry_ptr += sizeof(mp_local_interrupt_assignment_entry_t); break;
            default: return;
        }
    }
}

void mp_tables_read_pci_irq_routing(pci_irq_routing_t **routing_ptr, int *number_of_entries) {
    *number_of_entries = 0;

    if(firmware_info.mpct == NULL) {
        return;
    }

    // parse info from table
    mp_configuration_table_header_t *mpct = (mp_configuration_table_header_t *) firmware_info.mpct;
    uint8_t *entry_ptr = (uint8_t *) mpct + sizeof(mp_configuration_table_header_t);
    uint8_t *mpct_end = (uint8_t *) mpct + mpct->base_table_length;
    for(int i = 0; i < mpct->entry_count; i++) {
        if(entry_ptr >= mpct_end) {
            break;
        }

        uint8_t entry_type = entry_ptr[0];
        if(entry_type == 3) {
            mp_interrupt_assignment_entry_t *entry = (mp_interrupt_assignment_entry_t *) entry_ptr;
            if(is_bus_pci[entry->source_bus_id] == true) {
                // search for GSI base of IO APIC
                uint32_t gsi = INVALID;
                for(int j = 0; j < number_of_io_apics; j++) {
                    if(io_apics[j].id == entry->destination_io_apic_id) {
                        gsi = io_apics[j].base_gsi + entry->destination_io_apic_intin; // set correct GSI
                        break;
                    }
                }
                if(gsi != INVALID) {
                    *routing_ptr = krealloc(*routing_ptr, sizeof(pci_irq_routing_t) * (*number_of_entries + 1));
                    pci_irq_routing_t *routing = *routing_ptr;
                    routing[*number_of_entries].bus = entry->source_bus_id;
                    routing[*number_of_entries].device = (entry->source_bus_irq >> 2) & 0x1F;
                    routing[*number_of_entries].pin = (entry->source_bus_irq & 0b11) + 1; // convert to INT A-D
                    routing[*number_of_entries].gsi = gsi;
                    (*number_of_entries)++;
                }
            }
        }

        // move to next entry
        switch(entry_type) {
            case 0: entry_ptr += sizeof(mp_processor_entry_t); break;
            case 1: entry_ptr += sizeof(mp_bus_entry_t); break;
            case 2: entry_ptr += sizeof(mp_io_apic_entry_t); break;
            case 3: entry_ptr += sizeof(mp_interrupt_assignment_entry_t); break;
            case 4: entry_ptr += sizeof(mp_local_interrupt_assignment_entry_t); break;
            default: return;
        }
    }
}

void mp_tables_read_info_about_logical_processors(uint32_t *number_of_logical_processors, logical_processor_info_t *logical_processors) {
    *number_of_logical_processors = 0;

    if(firmware_info.mpct == NULL) {
        return;
    }

    mp_configuration_table_header_t *mpct = (mp_configuration_table_header_t *) firmware_info.mpct;
    uint8_t *entry_ptr = (uint8_t *) mpct + sizeof(mp_configuration_table_header_t);
    uint8_t *mpct_end = (uint8_t *) mpct + mpct->base_table_length;
    for(int i = 0; i < mpct->entry_count; i++) {
        if(entry_ptr >= mpct_end) {
            break;
        }

        uint8_t entry_type = entry_ptr[0];
        if(entry_type == 0) {
            mp_processor_entry_t *entry = (mp_processor_entry_t *) entry_ptr;
            if(entry->cpu_enabled == 1) {
                logical_processors[*number_of_logical_processors].hardware_id = entry->lapic_id;
                (*number_of_logical_processors)++;
                if(*number_of_logical_processors >= MAX_NUMBER_OF_LOGICAL_PROCESSORS_ON_SYSTEM) {
                    return;
                }
            }
        }

        // move to next entry
        switch(entry_type) {
            case 0: entry_ptr += sizeof(mp_processor_entry_t); break;
            case 1: entry_ptr += sizeof(mp_bus_entry_t); break;
            case 2: entry_ptr += sizeof(mp_io_apic_entry_t); break;
            case 3: entry_ptr += sizeof(mp_interrupt_assignment_entry_t); break;
            case 4: entry_ptr += sizeof(mp_local_interrupt_assignment_entry_t); break;
            default: return;
        }
    }
}