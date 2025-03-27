//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t pci_ind(struct pci_device_info_t device, dword_t offset) {
    if(components->pci.is_memory_access_supported == STATUS_TRUE) {
        return *((dword_t *) (device.segment_memory + ((device.bus << 20) | (device.device << 15) | (device.function << 12) | offset)));
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | offset));
        return ind(0xCFC);
    }
}

word_t pci_inw(struct pci_device_info_t device, dword_t offset) {
    if(components->pci.is_memory_access_supported == STATUS_TRUE) {
        return *((word_t *) (device.segment_memory + ((device.bus << 20) | (device.device << 15) | (device.function << 12) | offset)));
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        return (word_t) (ind(0xCFC) >> ((offset & 0x3)*8));
    }
}

byte_t pci_inb(struct pci_device_info_t device, dword_t offset) {
    if(components->pci.is_memory_access_supported == STATUS_TRUE) {
        return *((byte_t *) (device.segment_memory + ((device.bus << 20) | (device.device << 15) | (device.function << 12) | offset)));
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        return (byte_t) (ind(0xCFC) >> ((offset & 0x3)*8));
    }
}

void pci_outd(struct pci_device_info_t device, dword_t offset, dword_t value) {
    if(components->pci.is_memory_access_supported == STATUS_TRUE) {
        *((dword_t *) (device.segment_memory + ((device.bus << 20) | (device.device << 15) | (device.function << 12) | offset))) = value;
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | offset));
        outd(0xCFC, value);
    }
}

void pci_set_bits(struct pci_device_info_t device, dword_t offset, dword_t bits) {
    pci_outd(device, offset, pci_ind(device, offset) | bits);
}

void pci_clear_bits(struct pci_device_info_t device, dword_t offset, dword_t bits) {
    pci_outd(device, offset, pci_ind(device, offset) & ~bits);
}

void pci_outw(struct pci_device_info_t device, dword_t offset, word_t value) {
    if (components->pci.is_memory_access_supported == STATUS_TRUE) {
        *((word_t *)(device.segment_memory + ((device.bus << 20) | (device.device << 15) |  (device.function << 12) |  offset))) = value;
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        word_t temp = ind(0xCFC);
        temp &= ~(0xFFFF << ((offset & 0x3) * 8));
        temp |= (value << ((offset & 0x3) * 8));
        outd(0xCFC, temp);
    }
}

void pci_outb(struct pci_device_info_t device, dword_t offset, byte_t value) {
    if (components->pci.is_memory_access_supported == STATUS_TRUE) {
        *((byte_t *)(device.segment_memory + ((device.bus << 20) | (device.device << 15) | (device.function << 12) | offset))) = value;
    }
    else {
        outd(0xCF8, (0x80000000 | (device.bus << 16) | (device.device << 11) | (device.function << 8) | (offset & ~0x3)));
        dword_t temp = ind(0xCFC);
        temp &= ~(0xFF << ((offset & 0x3) * 8));
        temp |= (value << ((offset & 0x3) * 8));
        outd(0xCFC, temp);
    }
}

dword_t pci_get_bar_type(struct pci_device_info_t device, dword_t bar) {
    return (pci_ind(device, bar) & 0x1);
}

word_t pci_get_io(struct pci_device_info_t device, dword_t bar) {
    return (pci_inw(device, bar) & 0xFFFC);
}

dword_t pci_get_mmio(struct pci_device_info_t device, dword_t bar) {
    return (pci_ind(device, bar) & 0xFFFFFFF0);
}

dword_t pci_get_64_bit_mmio(struct pci_device_info_t device, dword_t bar) {
    if((pci_ind(device, bar+0x04) & 0xFFFFFFF0) != 0) {
        return 0;
    }
    else {
        return (pci_ind(device, bar) & 0xFFFFFFF0);
    }
}

void pci_device_install_interrupt_handler(struct pci_device_info_t device, void (*handler)(void)) {
    // TODO: add support for MSI and MSI-X

    // TODO: add support for ACPI

    // set handler for PCI IRQ
    set_irq_handler(device.interrupt_line, (dword_t)handler);
}

byte_t *pci_get_vendor_name(word_t vendor_id) {
    for(dword_t i = 0; pci_vendor_list[i].vendor_name != 0; i++) {
        if(pci_vendor_list[i].vendor_id == vendor_id) {
            return pci_vendor_list[i].vendor_name;
        }
    }

    return "Unknown Vendor";
}

byte_t *pci_get_device_type_string(dword_t type) {
    for(dword_t i = 0; pci_device_type_list[i].description != 0; i++) {
        if(pci_device_type_list[i].type == type) {
            return pci_device_type_list[i].description;
        }
    }

    return "Unknown Device";
}

dword_t pci_is_device_in_list(word_t vendor_id, word_t device_id, struct pci_supported_devices_list_t *device_list) {
    for(dword_t i = 0; device_list[i].vendor_id != 0; i++) {
        if(vendor_id == device_list[i].vendor_id && device_id == device_list[i].device_id) {
            return STATUS_TRUE;
        }
    }

    return STATUS_FALSE;
}

void scan_pci(void) {
    logf("\n\nPCI INFO\nAccess: ");

    if(components->pci.is_memory_access_supported == STATUS_TRUE) {
        logf("MMIO");

        dword_t number_of_segments = (components->pci.mcfg->header.length - sizeof(struct acpi_table_header_t) - 8)/sizeof(struct mcfg_pci_segment_info_t);
        logf("\nNumber of segments: %d", number_of_segments);

        struct pci_device_info_t pci_device;
        for(dword_t i = 0; i < number_of_segments; i++) {
            pci_device.segment = components->pci.mcfg->segments[i].segment;
            pci_device.segment_memory = components->pci.mcfg->segments[i].base;

            for(dword_t bus = components->pci.mcfg->segments[i].first_bus; bus <= components->pci.mcfg->segments[i].last_bus; bus++) {
                pci_device.bus = bus;

                for(dword_t device = 0; device < 32; device++) {
                    pci_device.device = device;
                    pci_device.function = 0;

                    // read informations about device on primary function
                    if(pci_inw(pci_device, 0x00) != 0xFFFF) {
                        scan_pci_device(pci_device);

                        // check if this is multifunctional device
                        if((pci_inb(pci_device, 0x0E) & (1 << 7)) == (1 << 7)) {
                            for(dword_t function = 1; function < 8; function++) {
                                pci_device.function = function;

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
        logf("IO");

        dword_t buses[256];
        clear_memory((dword_t)&buses, 256*sizeof(dword_t));
        dword_t number_of_buses = 1;

        struct pci_device_info_t pci_device;
        pci_device.segment = 0;
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

void scan_pci_device(struct pci_device_info_t device) {
    // read basic device informations
    device.vendor_id = pci_inw(device, 0x00);
    device.device_id = pci_inw(device, 0x02);
    device.interrupt_line = pci_inb(device, 0x3C);
    device.interrupt_pin = pci_inb(device, 0x3D);
    device.msi_register = 0;
    device.msi_x_register = 0;
    byte_t progif = pci_inb(device, 0x09);
    byte_t subclass = pci_inb(device, 0x0A);
    byte_t class = pci_inb(device, 0x0B);
    dword_t type = (pci_ind(device, 0x08) & 0xFFFFFF00);

    // log basic device informations
    logf("\n\nPCI device\n Location: %d:%d:%d:%d\n Type: %s\n Vendor Name: %s\n Vendor ID: 0x%04x\n Device ID: 0x%04x",
            device.segment, device.bus, device.device, device.function,
            pci_get_device_type_string(type),
            pci_get_vendor_name(device.vendor_id),
            device.vendor_id,
            device.device_id);
    if(device.interrupt_pin != 0) {
        logf("\n Interrupt PIN: %c\n Interrupt Line: %d", device.interrupt_pin - 1 + 'A', device.interrupt_line);
    }

    // read device capability list
    if((pci_inw(device, 0x06) & (1 << 4)) == (1 << 4)) {
        logf("\n Capabilities:");

        dword_t pointer = pci_inb(device, 0x34);
        for(dword_t i = 0; i < 255; i++) {
            byte_t capability_id = pci_inb(device, pointer+0x00);
            logf("\n  ID: ");
            switch(capability_id) {
                case(0x05):
                    logf("MSI");
                    device.msi_register = pointer;
                    break;
                case(0x11):
                    logf("MSI-X");
                    device.msi_x_register = pointer;
                    break;
                default:
                    logf("0x%02x", capability_id);
            }

            // move to next capability
            pointer = pci_inb(device, pointer+0x01);
            if(pointer == 0) {
                break;
            }
        }
    }

    // connect device to driver
    for(dword_t i = 0; pci_device_type_list[i].description != 0; i++) {
        // we find type of device
        if(pci_device_type_list[i].type == type) {
            // check if we have driver for it
            if(pci_device_type_list[i].driver_add_new_pci_device != 0) {
                // check if this device is supported by driver
                if(pci_device_type_list[i].supported_devices == 0
                    || pci_is_device_in_list(device.vendor_id, device.device_id, pci_device_type_list[i].supported_devices) == STATUS_TRUE) {
                    logf("\n Driver: ");
                    pci_device_type_list[i].driver_add_new_pci_device(device);
                    return;
                }
            }
        }
    }

    // TODO: initalize PCI-to-PCI bridges
}