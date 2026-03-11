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
#include <kernel/memory/memory_allocators.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
uint32_t number_of_io_apics;
io_apic_info_t *io_apics;

/* functions */
void initialize_apic(io_apic_info_t *apic) {
    if(apic->mmio_address == NULL) {
        log("\n[APIC] Error: IO APIC MMIO address is NULL");
        return;
    }

    // map APIC MMIO ports
    void *base = perm_phy_alloc(apic->mmio_address, 0x10 + (256 * 8), VM_KERNEL | VM_UNCACHEABLE);
    apic->port_register = (uint32_t volatile *)((uint32_t)base + 0x00);
    apic->port_data = (uint32_t volatile *)((uint32_t)base + 0x10);

    // read APIC info
    apic->redirection_entries = ((read_apic(apic, 0x01) >> 16) & 0xFF) + 1;

    // mask all redirection entries
    for(int i = 0; i < apic->redirection_entries; i++) {
        apic_mask_redirection_entry(apic, i);
    }

    // log APIC info
    log("\n[APIC] IO APIC ID 0x%02x", (read_apic(apic, 0x00) >> 24) & 0xFF);
    log("\n Version: 0x%02x", read_apic(apic, 0x01) & 0xFF);
    log("\n Redirection entries: %d", apic->redirection_entries);
    log("\n Arbitration ID: 0x%02x", (read_apic(apic, 0x02) >> 24) & 0xFF);
}

uint32_t read_apic(io_apic_info_t *apic, uint8_t reg) {
    *(apic->port_register) = reg;
    return *(apic->port_data);
}

void write_apic(io_apic_info_t *apic, uint8_t reg, uint32_t value) {
    *(apic->port_register) = reg;
    *(apic->port_data) = value;
}

void apic_mask_redirection_entry(io_apic_info_t *apic, uint8_t entry) {
    uint32_t low = read_apic(apic, 0x10 + (entry * 2));
    low |= (1 << 16);
    write_apic(apic, 0x10 + (entry * 2), low);
}

void apic_set_redirection_entry(io_apic_info_t *apic, uint8_t entry, uint8_t vector, uint8_t delivery_mode, uint8_t dest_mode, uint8_t polarity, uint8_t trigger_mode, uint8_t destination) {
    apic_mask_redirection_entry(apic, entry);
    
    uint32_t low = 0;
    low |= vector;
    low |= (delivery_mode & 0x7) << 8;
    low |= (dest_mode & 0x1) << 11;
    low |= (polarity & 0x1) << 13;
    low |= (trigger_mode & 0x1) << 15;
    uint32_t high = ((uint32_t)destination) << 24;

    write_apic(apic, 0x10 + (entry * 2) + 1, high);
    write_apic(apic, 0x10 + (entry * 2), low); // this unmasks entry
}

int connect_gsi_to_idt_through_apic(uint32_t gsi, uint8_t vector, uint8_t delivery_mode, uint8_t dest_mode, uint8_t polarity, uint8_t trigger_mode, uint8_t destination) {
    // find IO APIC for GSI
    for(int i = 0; i < number_of_io_apics; i++) {
        io_apic_info_t *apic = &io_apics[i];
        if(gsi >= apic->base_gsi && gsi < (apic->base_gsi + apic->redirection_entries)) {
            uint8_t entry = gsi - apic->base_gsi;
            apic_set_redirection_entry(apic, entry, vector, delivery_mode, dest_mode, polarity, trigger_mode, destination);
            return SUCCESS;
        }
    }
    return ERROR; // GSI not found
}