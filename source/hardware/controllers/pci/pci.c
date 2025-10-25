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
#include <kernel/x86/acpi/main.h>
#include <kernel/x86/acpi/mcfg.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/stdlib.h>
#include <hardware/controllers/pci/device.h>
#include <hardware/controllers/pci/vendors.h>
#include <hardware/controllers/pci/supported_devices.h>

/* global variables */
pci_access_type_t pci_access_type = io;

/* functions for communication with PCI */
// TODO: separate io and mmio access
byte_t pci_inb(pci_device_info_t device, dword_t offset) {
    if(pci_access_type == mmio) {
        return *((byte_t *) (device.pci_mmio_start + offset));
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        return (byte_t) (ind(0xCFC) >> ((offset & 0x3)*8));
    }
}

word_t pci_inw(pci_device_info_t device, dword_t offset) {
    if(pci_access_type == mmio) {
        return *((word_t *) (device.pci_mmio_start + offset));
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        return (word_t) (ind(0xCFC) >> ((offset & 0x3)*8));
    }
}

dword_t pci_ind(pci_device_info_t device, dword_t offset) {
    if(pci_access_type == mmio) {
        return *((dword_t *) (device.pci_mmio_start + offset));
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | offset));
        return ind(0xCFC);
    }
}

void pci_outb(pci_device_info_t device, dword_t offset, byte_t value) {
    if(pci_access_type == mmio) {
        *((byte_t *) (device.pci_mmio_start + offset)) = value;
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        dword_t temp = ind(0xCFC);
        temp &= ~(0xFF << ((offset & 0x3) * 8));
        temp |= (value << ((offset & 0x3) * 8));
        outd(0xCFC, temp);
    }
}

void pci_outw(pci_device_info_t device, dword_t offset, word_t value) {
    if(pci_access_type == mmio) {
        *((word_t *) (device.pci_mmio_start + offset)) = value;
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        word_t temp = ind(0xCFC);
        temp &= ~(0xFFFF << ((offset & 0x3) * 8));
        temp |= (value << ((offset & 0x3) * 8));
        outd(0xCFC, temp);
    }
}

void pci_outd(pci_device_info_t device, dword_t offset, dword_t value) {
    if(pci_access_type == mmio) {
        *((dword_t *) (device.pci_mmio_start + offset)) = value;
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | offset));
        outd(0xCFC, value);
    }
}

void pci_set_bits(pci_device_info_t device, dword_t offset, dword_t bits) {
    pci_outd(device, offset, pci_ind(device, offset) | bits);
}

void pci_clear_bits(pci_device_info_t device, dword_t offset, dword_t bits) {
    pci_outd(device, offset, pci_ind(device, offset) & ~bits);
}

/* functions for parsing PCI data */
byte_t *pci_get_vendor_name(word_t vendor_id) {
    for(dword_t i = 0; pci_vendor_list[i].vendor_name != 0; i++) {
        if(pci_vendor_list[i].vendor_id == vendor_id) {
            return pci_vendor_list[i].vendor_name;
        }
    }

    static byte_t str[7];
    htoan(vendor_id, str, 4);
    return str;
}

byte_t *pci_get_device_type_string(dword_t type) {
    for(dword_t i = 0; pci_device_type_list[i].description != 0; i++) {
        if(pci_device_type_list[i].type == type) {
            return pci_device_type_list[i].description;
        }
    }

    static byte_t str[9];
    htoan(type >> 8, str, 6);
    return str;
}

dword_t pci_get_bar_type(pci_device_info_t device, dword_t bar) {
    return (pci_ind(device, bar) & 0x1);
}

word_t pci_get_io(pci_device_info_t device, dword_t bar) {
    return (pci_inw(device, bar) & 0xFFFC);
}

dword_t pci_get_mmio(pci_device_info_t device, dword_t bar) {
    return (pci_ind(device, bar) & 0xFFFFFFF0);
}

dword_t pci_get_64_bit_mmio(pci_device_info_t device, dword_t bar) {
    if((pci_ind(device, bar+0x04) & 0xFFFFFFF0) != 0) {
        return 0;
    }
    else {
        return (pci_ind(device, bar) & 0xFFFFFFF0);
    }
}

/* functions for initializing PCI */
void initialize_pci_controller(void) {
    if(kernel_attr->pm_mcfg != INVALID) {
        log("\n[PCI] Access: MMIO");
        pci_access_type = mmio;

        void *vm_of_mcfg_table = kpalloc(kernel_attr->pm_mcfg, kernel_attr->size_of_mcfg, VM_FLAG_WRITE_BACK);
        mcfg_table_t *mcfg = (mcfg_table_t *) (vm_of_mcfg_table + (kernel_attr->pm_mcfg & 0xFFF));

        dword_t number_of_segments = ((kernel_attr->size_of_mcfg - sizeof(acpi_table_header_t) - 8) / sizeof(mcfg_pci_segment_info_t));
        log("\n[PCI] Number of segments: %d", number_of_segments);

        pci_device_info_t pci_device;
        for(dword_t i = 0; i < number_of_segments; i++) {
            pci_device.segment = mcfg->segments[i].segment;
            dword_t number_of_buses = (mcfg->segments[i].last_bus - mcfg->segments[i].first_bus + 1);
            dword_t vm_of_segment_mem = (dword_t) kpalloc(mcfg->segments[i].base, number_of_buses * 32 * 8 * 0x1000, VM_FLAG_UNCACHEABLE);
            dword_t start_of_vm_of_segment_mem = (vm_of_segment_mem + (mcfg->segments[i].base & 0xFFF));

            for(dword_t bus = mcfg->segments[i].first_bus; bus <= mcfg->segments[i].last_bus; bus++) {
                pci_device.bus = bus;

                for(dword_t device = 0; device < 32; device++) {
                    pci_device.device = device;
                    pci_device.function = 0;
                    pci_device.pci_mmio_start = (start_of_vm_of_segment_mem + (((pci_device.bus - mcfg->segments[i].first_bus) << 20) | (pci_device.device << 15) | (pci_device.function << 12)));

                    // read informations about device on primary function
                    if(pci_inw(pci_device, 0x00) != 0xFFFF) {
                        scan_pci_device(pci_device);

                        // check if this is multifunctional device
                        if((pci_inb(pci_device, 0x0E) & (1 << 7)) == (1 << 7)) {
                            for(dword_t function = 1; function < 8; function++) {
                                pci_device.function = function;
                                pci_device.pci_mmio_start = (start_of_vm_of_segment_mem + (((pci_device.bus - mcfg->segments[i].first_bus) << 20) | (pci_device.device << 15) | (pci_device.function << 12)));

                                // read informations about device on other functions
                                if(pci_inw(pci_device, 0x00) != 0xFFFF) {
                                    scan_pci_device(pci_device);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        log("\n[PCI] Access: IO");
        pci_access_type = io;

        dword_t buses[256];
        buses[0] = 0;
        dword_t number_of_buses = 1;

        pci_device_info_t pci_device;
        pci_device.segment = 0;
        pci_device.pci_mmio_start = INVALID;
        for(dword_t i = 0; i < number_of_buses; i++) {
            pci_device.bus = buses[i];

            for(dword_t device = 0; device < 32; device++) {
                pci_device.device = device;
                pci_device.function = 0;

                // read informations about device on primary function
                if(pci_inw(pci_device, 0x00) != 0xFFFF) {
                    scan_pci_device(pci_device);

                    // if it is PCI-to-PCI bridge, read secondary bus
                    if(pci_inw(pci_device, 0x0A) == 0x0604 && number_of_buses < 256) {
                        buses[number_of_buses] = pci_inb(pci_device, 0x19);
                        number_of_buses++;
                    }

                    // check if this is multifunctional device
                    if((pci_inb(pci_device, 0x0E) & (1 << 7)) == (1 << 7)) {
                        for(dword_t function = 1; function < 8; function++) {
                            pci_device.function = function;

                            // read informations about device on other functions
                            if(pci_inw(pci_device, 0x00) != 0xFFFF) {
                                scan_pci_device(pci_device);

                                // if it is PCI-to-PCI bridge, read secondary bus
                                if(pci_inw(pci_device, 0x0A) == 0x0604 && number_of_buses < 256) {
                                    buses[number_of_buses] = pci_inb(pci_device, 0x19);
                                    number_of_buses++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void scan_pci_device(pci_device_info_t device) {
    // read basic device informations
    dword_t header = pci_inb(device, 0x0D);
    device.vendor_id = pci_inw(device, 0x00);
    device.device_id = pci_inw(device, 0x02);
    if(header == 0) {
        device.subsystem_vendor_id = pci_inw(device, 0x2E);
        device.subsystem_id = pci_inw(device, 0x2C);
    }
    else if(header == 2) {
        device.subsystem_vendor_id = pci_inw(device, 0x40);
        device.subsystem_id = pci_inw(device, 0x42);
    }
    else {
        device.subsystem_vendor_id = 0;
        device.subsystem_id = 0;
    }
    device.interrupt_line = pci_inb(device, 0x3C);
    device.interrupt_pin = pci_inb(device, 0x3D);
    device.msi_register = 0;
    device.msi_x_register = 0;
    byte_t progif = pci_inb(device, 0x09);
    byte_t subclass = pci_inb(device, 0x0A);
    byte_t class = pci_inb(device, 0x0B);
    dword_t type = (pci_ind(device, 0x08) & 0xFFFFFF00);

    // log basic device informations
    log("\n[PCI] Device at %d:%d:%d:%d | Type: %s | Vendor: %s | Device ID: 0x%04x | Subsystem Vendor: %s | Subsystem ID: 0x%04x",
        device.segment, device.bus, device.device, device.function,
        pci_get_device_type_string(type),
        pci_get_vendor_name(device.vendor_id),
        device.device_id,
        pci_get_vendor_name(device.subsystem_vendor_id),
        device.subsystem_id);

    // run driver of device
    for(dword_t i = 0; pci_device_type_list[i].description != 0; i++) {
        if(pci_device_type_list[i].type == type) {
            pci_drivers_for_type_t *device_drivers = pci_device_type_list[i].drivers;

            if((dword_t)device_drivers->driver_subsystem_devices != NULL) {
                pci_supported_subsystem_devices_by_driver_t *driver_subsystem_devices = device_drivers->driver_subsystem_devices;
                for(int j = 0; driver_subsystem_devices[j].vendor_id != NULL; j++) {
                    if(device.vendor_id == driver_subsystem_devices[j].vendor_id
                        && device.device_id == driver_subsystem_devices[j].device_id
                        && device.subsystem_vendor_id == driver_subsystem_devices[j].subsystem_vendor_id
                        && device.subsystem_id == driver_subsystem_devices[j].subsystem_id) {
                            device_drivers->initialize(device);
                        }
                }
            }

            if((dword_t)device_drivers->driver_classic_devices != NULL) {
                pci_supported_classic_devices_by_driver_t *driver_classic_devices = device_drivers->driver_classic_devices;
                for(int j = 0; driver_classic_devices[j].vendor_id != NULL; j++) {
                    if(device.vendor_id == driver_classic_devices[j].vendor_id
                        && device.device_id == driver_classic_devices[j].device_id) {
                            device_drivers->initialize(device);
                        }
                }
            }

        }
    }
}