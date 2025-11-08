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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/interrupt_controllers/pic.h>
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/acpi/main.h>

#include <hardware/controllers/pci/pci.h>
#include <hardware/controllers/pci/device.h>

/* global variables */
void (*connect_legacy_interrupt_to_handler)(dword_t irq, void (*handler)(void));

/* local variables */
pir_table_t *pir;
dword_t pir_number_of_entries;

/* functions */
void initialize_interrupt_controller(void) {
    // search for PCI Interrupt Routing table
    void *bios_memory = kpalloc(0xF0000, 0x10000, VM_FLAG_WRITE_BACK);
    pir = bios_memory;
    for(int i = 0; i < (0x10000 - sizeof(pir_table_t) - sizeof(pir_table_entry_t)); i++, pir = (pir_table_t *) ((dword_t)pir + 1)) {
        if(pir->signature == ACPI_SIGNATURE_32("$PIR") && pir->table_size <= 1024 && acpi_table_is_checksum_valid(pir, pir->table_size)) {
            if((i + pir->table_size) >= 0x10000) {
                continue;
            }
            pir_number_of_entries = (pir->table_size - sizeof(pir_table_t)) / sizeof(pir_table_entry_t);
            log("\n[$PIR] PIR table founded at 0x%x with %d entries", (dword_t)pir - (dword_t)bios_memory + 0xF0000, pir_number_of_entries);
            for(int i = 0; i < pir_number_of_entries; i++) {
                byte_t bus = pir->entries[i].bus;
                byte_t dev = (pir->entries[i].devfunc >> 3) & 0x1F;
                byte_t func = pir->entries[i].devfunc & 0x07;
                log("\n[$PIR] Entry %u: bus=%u dev=%u func=%u slot=%u", i, bus, dev, func, pir->entries[i].slot);
                for(int j = 0; j < 4; j++) {
                    if(pir->entries[i].link[j] != 0) {
                        log("\n INT%c: link=0x%02x bitmap=0x%04x", 'A'+j, pir->entries[i].link[j], pir->entries[i].bitmap[j]);
                    }
                    else {
                        log("\n INT%c: not connected", 'A'+j);
                    }
                }
            }
            break;
        }
    }
    if((dword_t)pir == ((dword_t)bios_memory + (0x10000 - sizeof(pir_table_t) - sizeof(pir_table_entry_t)))) {
        unmap(bios_memory);
        pir = (pir_table_t *) INVALID;
    }

    // TODO: check if is APIC available
    // TODO: if yes, initialize APIC

    initialize_pic();
    enable_interrupts();
}

void connect_pci_device_interrupt_to_handler(pci_device_info_t pci_device, void (*handler)(void)) {
    // TODO: parse DSDT for GSI
    // TODO: use MSI if available

    connect_legacy_interrupt_to_handler(pci_device.interrupt_line, handler);
}

void enable_interrupts(void) {
    core_lock_level = 0;
    sti();
}

void disable_interrupts(void) {
    cli();
    core_lock_level = 0;
}

void dump_pci_device_gsi(pci_device_info_t pci_device) {
    if((dword_t)pir == INVALID || pci_device.interrupt_pin == 0 || pci_device.interrupt_pin > 4) {
        return;
    }

    for(int i = 0; i < pir_number_of_entries; i++) {
        byte_t bus = pir->entries[i].bus;
        byte_t dev = (pir->entries[i].devfunc >> 3) & 0x1F;
        byte_t func = pir->entries[i].devfunc & 0x07;
        if(pci_device.bus == bus && pci_device.device == dev && pci_device.function == func) {
            byte_t link = pir->entries[i].link[pci_device.interrupt_pin-1];
            log("Link: 0x%02x ", link);
            outd(0xCF8, (0x80000000 | (pir->router_bus << 16) | (((pir->router_devfunc >> 3) & 0x1F) << 11) | ((pir->router_devfunc & 0x07) << 8) | (link & ~0x3)));
            byte_t irq = (ind(0xCFC) >> ((link & 0x3) * 8));
            if(irq == 0x00 || irq == 0xFF) {
                log("IRQ is not mapped");
            }
            else if((irq & 0x80) == 0x80) {
                log("IRQ is disabled in bridge");
            }
            else {
                log("%d", irq);
            }
            log(" Interrupt Line: %d", pci_device.interrupt_line);
            return;
        }
    }
}