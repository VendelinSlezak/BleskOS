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

void pci_new_scan(void) {
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
                        pci_new_scan_device(pci_device);

                        // check if this is multifunctional device
                        if((pci_inb(pci_device, 0x0E) & (1 << 7)) == (1 << 7)) {
                            for(dword_t function = 1; function < 8; function++) {
                                pci_device.function = function;

                                // read informations about device on other functions
                                if(pci_inw(pci_device, 0x00) != 0xFFFF) {
                                    pci_new_scan_device(pci_device);
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
                    pci_new_scan_device(pci_device);

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
                                pci_new_scan_device(pci_device);

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

void pci_new_scan_device(struct pci_device_info_t device) {
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
                if(pci_device_type_list[i].supported_devices == 0) {
                    logf("\n Driver: ");
                    pci_device_type_list[i].driver_add_new_pci_device(device);
                    return;
                }
                else {
                    for(dword_t j = 0; pci_device_type_list[i].supported_devices[j].vendor_id != 0; j++) {
                        if(device.vendor_id == pci_device_type_list[i].supported_devices[j].vendor_id
                           && device.device_id == pci_device_type_list[i].supported_devices[j].device_id) {
                            logf("\n Driver: ");
                            pci_device_type_list[i].driver_add_new_pci_device(device);
                            return;
                        }
                    }
                }
            }
        }
    }

    // TODO: initalize PCI-to-PCI bridges
}

word_t pci_get_io(struct pci_device_info_t device, dword_t bar) {
    return (pci_inw(device, bar) & 0xFFFC);
}

dword_t pci_get_mmio(struct pci_device_info_t device, dword_t bar) {
    return (pci_ind(device, bar) & 0xFFFFFFF0);
}

void pci_device_install_interrupt_handler(struct pci_device_info_t device, void (*handler)(void)) {
    // TODO: add support for MSI and MSI-X

    // TODO: add support for ACPI

    // set handler for PCI IRQ
    set_irq_handler(device.interrupt_line, (dword_t)handler);
}

dword_t pci_read(dword_t segment_memory, dword_t bus, dword_t device, dword_t function, dword_t offset) {
 outd(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset)));
 return ind(0xCFC);
}

void pci_write(dword_t bus, dword_t device, dword_t function, dword_t offset, dword_t value) {
 outd(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset)));
 outd(0xCFC, value);
}

void pci_writeb(dword_t bus, dword_t device, dword_t function, dword_t offset, dword_t value) {
 outd(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC)));
 outb(0xCFF, value);
}

dword_t pci_read_bar_type(dword_t bus, dword_t device, dword_t function, dword_t bar) {
 outd(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (bar)));
 return (word_t) (ind(0xCFC) & 0x1);
}

word_t pci_read_io_bar(dword_t bus, dword_t device, dword_t function, dword_t bar) {
 outd(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (bar)));
 return (word_t) (ind(0xCFC) & 0xFFFC);
}

dword_t pci_read_mmio_bar(dword_t bus, dword_t device, dword_t function, dword_t bar) {
 outd(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (bar)));
 return (ind(0xCFC) & 0xFFFFFFF0);
}

void pci_enable_io_busmastering(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, ((pci_read(0, bus, device, function, 0x04) & ~(1<<10)) | (1<<2) | (1<<0))); //enable interrupts, enable bus mastering, enable IO space
}

void pci_enable_mmio_busmastering(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, ((pci_read(0, bus, device, function, 0x04) & ~(1<<10)) | (1<<2) | (1<<1))); //enable interrupts, enable bus mastering, enable MMIO space
}

void pci_disable_interrupts(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, (pci_read(0, bus, device, function, 0x04) | (1<<10))); //disable interrupts
}

void scan_pci(void) {
 //initalize values that are used to determine presence of devices
 ide_0x1F0_controller_present = STATUS_FALSE;
 ide_0x170_controller_present = STATUS_FALSE;
 number_of_sound_cards = 0;
 number_of_storage_controllers = 0;
 number_of_ethernet_cards = 0;
 number_of_uhci_controllers = 0;
 number_of_ehci_controllers = 0;
 
 for(dword_t bus=0; bus<256; bus++) {
  for(dword_t device=0; device<32; device++) {
   scan_pci_device(bus, device, 0);
   
   //multifunctional device
   if((pci_read(0, bus, device, 0, 0x0C) & 0x00800000)==0x00800000) {
    for(dword_t function=1; function<8; function++) {
     scan_pci_device(bus, device, function);
    }
   }
  }
 }
}

void scan_pci_device(dword_t bus, dword_t device, dword_t function) {
 dword_t full_device_id, vendor_id, device_id, type_of_device, class, subclass, progif, device_irq, mmio_port_base;
 word_t io_port_base;

 //read base informations about device
 vendor_id = (pci_read(0, bus, device, function, 0) & 0xFFFF);
 device_id = (pci_read(0, bus, device, function, 0) >> 16);
 full_device_id = pci_read(0, bus, device, function, 0);
 if(full_device_id==0xFFFFFFFF) {
  return; //no device
 }
 type_of_device = (pci_read(0, bus, device, function, 0x08) >> 8);
 class = (type_of_device >> 16);
 subclass = ((type_of_device >> 8) & 0xFF);
 progif = (type_of_device & 0xFF);
 device_irq = (pci_read(0, bus, device, function, 0x3C) & 0xFF);
 
 //IDE controller
 if((type_of_device & 0xFFFF00)==0x010100 && number_of_storage_controllers<MAX_NUMBER_OF_STORAGE_CONTROLLERS) { 
  pci_device_ide_controller:
  pci_enable_io_busmastering(bus, device, function);
  pci_disable_interrupts(bus, device, function);

  storage_controllers[number_of_storage_controllers].bus = bus;
  storage_controllers[number_of_storage_controllers].device = device;
  storage_controllers[number_of_storage_controllers].function = function;

  //first controller
  storage_controllers[number_of_storage_controllers].controller_type = IDE_CONTROLLER;
  io_port_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
  if(io_port_base==0 || ide_is_bus_floating(io_port_base)==STATUS_FALSE) {
   if(io_port_base==0 || io_port_base==0x1F0) {
    if(ide_0x1F0_controller_present==STATUS_FALSE) {
     ide_0x1F0_controller_present = STATUS_TRUE;
     storage_controllers[number_of_storage_controllers].base_1 = 0x1F0;
     storage_controllers[number_of_storage_controllers].base_2 = 0x3F4;
     number_of_storage_controllers++;
    }
   }
   else {
    storage_controllers[number_of_storage_controllers].base_1 = io_port_base;
    storage_controllers[number_of_storage_controllers].base_2 = pci_read_io_bar(bus, device, function, PCI_BAR1);
    number_of_storage_controllers++;
   }
  }
  if(number_of_storage_controllers>=MAX_NUMBER_OF_STORAGE_CONTROLLERS) {
   return;
  }

  //second controller
  storage_controllers[number_of_storage_controllers].controller_type = IDE_CONTROLLER;
  io_port_base = pci_read_io_bar(bus, device, function, PCI_BAR2);
  if(io_port_base==0 || ide_is_bus_floating(io_port_base)==STATUS_FALSE) {
   if(io_port_base==0 || io_port_base==0x170) {
    if(ide_0x170_controller_present==STATUS_FALSE) {
     ide_0x170_controller_present = STATUS_TRUE;
     storage_controllers[number_of_storage_controllers].base_1 = 0x170;
     storage_controllers[number_of_storage_controllers].base_2 = 0x374;
     number_of_storage_controllers++;
    }
   }
   else {
    storage_controllers[number_of_storage_controllers].base_1 = io_port_base;
    storage_controllers[number_of_storage_controllers].base_2 = pci_read_io_bar(bus, device, function, PCI_BAR3);
    number_of_storage_controllers++;
   }
  }
  
  return;
 }
 
 //SATA controller
 if(type_of_device==0x010601 && number_of_storage_controllers<MAX_NUMBER_OF_STORAGE_CONTROLLERS) {
  pci_enable_mmio_busmastering(bus, device, function);
  pci_disable_interrupts(bus, device, function);
  
  //test presence of AHCI interface
  if((mmio_ind(pci_read_mmio_bar(bus, device, function, PCI_BAR5) + 0x04) & 0x80000000)==0x00000000) {
   goto pci_device_ide_controller; //IDE interface
  }
  
  //save device
  storage_controllers[number_of_storage_controllers].controller_type = AHCI_CONTROLLER;
  storage_controllers[number_of_storage_controllers].base_1 = pci_read_mmio_bar(bus, device, function, PCI_BAR5);
  number_of_storage_controllers++;
  return;
 }
 
 //Ethernet card
 if(type_of_device==0x020000) {
  if(number_of_ethernet_cards>=MAX_NUMBER_OF_ETHERNET_CARDS) {
   return;
  }
  
  //read basic values
  ethernet_cards[number_of_ethernet_cards].id = full_device_id;
  ethernet_cards[number_of_ethernet_cards].irq = (pci_read(0, bus, device, function, 0x3C) & 0xFF);
  ethernet_cards[number_of_ethernet_cards].bus = bus;
  ethernet_cards[number_of_ethernet_cards].dev = device;
  ethernet_cards[number_of_ethernet_cards].func = function;
  ethernet_cards[number_of_ethernet_cards].initalize = 0; //by default card has no driver
  
  //Intel E1000
  if(vendor_id==VENDOR_INTEL) {
   //connect to driver for Intel e1000
   ethernet_cards[number_of_ethernet_cards].initalize = ec_intel_e1000_initalize;

   ethernet_cards[number_of_ethernet_cards].bar_type = pci_read_bar_type(bus, device, function, PCI_BAR0);
   if(ethernet_cards[number_of_ethernet_cards].bar_type==PCI_MMIO_BAR) {
    ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
    pci_enable_mmio_busmastering(bus, device, function);
   }
   else {
    ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
    pci_enable_io_busmastering(bus, device, function);
   }
  }
  
  //AMD PC-net
  else if(vendor_id==VENDOR_AMD_1 || vendor_id==VENDOR_AMD_2) {
   //connect to driver for Amd PCNET
   ethernet_cards[number_of_ethernet_cards].initalize = ec_amd_pcnet_initalize;

   pci_enable_io_busmastering(bus, device, function);
   ethernet_cards[number_of_ethernet_cards].bar_type = PCI_IO_BAR;
   ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
  }
  
  //Broadcom NetXtreme
  else if(vendor_id==VENDOR_BROADCOM) {
   //connect to driver for Broadcom NetXtreme
   ethernet_cards[number_of_ethernet_cards].initalize = ec_broadcom_netxtreme_initalize;

   pci_enable_mmio_busmastering(bus, device, function);
   ethernet_cards[number_of_ethernet_cards].bar_type = PCI_MMIO_BAR;
   ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  }

  //Qualcomm Atheros
  else if(vendor_id==VENDOR_QUALCOMM_ATHEROS_1 || vendor_id==VENDOR_QUALCOMM_ATHEROS_2) {
   //connect to driver for Qualcomm Atheros
   ethernet_cards[number_of_ethernet_cards].initalize = ec_atheros_initalize;

   pci_enable_io_busmastering(bus, device, function);
   pci_enable_mmio_busmastering(bus, device, function);
   ethernet_cards[number_of_ethernet_cards].bar_type = PCI_MMIO_BAR;
   ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  }
  
  //Realtek
  else if(vendor_id==VENDOR_REALTEK) {
   if(device_id==0x8139) {
    //connect to driver for Realtek 8139
    ethernet_cards[number_of_ethernet_cards].initalize = ec_realtek_8139_initalize;

    pci_enable_io_busmastering(bus, device, function);
    ethernet_cards[number_of_ethernet_cards].bar_type = PCI_IO_BAR;
    ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
   }
   else if(device_id==0x8136 || device_id==0x8161 || device_id==0x8168 || device_id==0x8169) {
    //connect to driver for Realtek 8169
    ethernet_cards[number_of_ethernet_cards].initalize = ec_realtek_8169_initalize;

    ethernet_cards[number_of_ethernet_cards].bar_type = pci_read_bar_type(bus, device, function, PCI_BAR0);
    if(ethernet_cards[number_of_ethernet_cards].bar_type==PCI_MMIO_BAR) {
     ethernet_cards[number_of_ethernet_cards].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
     pci_enable_mmio_busmastering(bus, device, function);
    }
    else {
     ethernet_cards[number_of_ethernet_cards].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
     pci_enable_io_busmastering(bus, device, function);
    }
   }
  }
  
  number_of_ethernet_cards++;
  return;
 }

 //Ethernet card
 if(type_of_device==0x028000) {
  return;
 }
 
 //AC97 sound card
 if(type_of_device==0x040100 && number_of_sound_cards<MAX_NUMBER_OF_SOUND_CARDS) {
  if(number_of_sound_cards>=MAX_NUMBER_OF_SOUND_CARDS) {
   return;
  }

  pci_enable_io_busmastering(bus, device, function);

  sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_AC97;
  sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
  sound_cards_info[number_of_sound_cards].device_id = device_id;
  sound_cards_info[number_of_sound_cards].io_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
  sound_cards_info[number_of_sound_cards].io_base_2 = pci_read_io_bar(bus, device, function, PCI_BAR1);
  number_of_sound_cards++;

  return;
 }
 
 //HD Audio sound card
 if(type_of_device==0x040300 && number_of_sound_cards<MAX_NUMBER_OF_SOUND_CARDS) {
  if(number_of_sound_cards>=MAX_NUMBER_OF_SOUND_CARDS) {
   return;
  }

  pci_enable_mmio_busmastering(bus, device, function);

  sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_HDA;
  sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
  sound_cards_info[number_of_sound_cards].device_id = device_id;
  sound_cards_info[number_of_sound_cards].mmio_base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  number_of_sound_cards++;

  return;
 }
 
 //Universal Host Controller Interface
 if(type_of_device==0x0C0300) {
  if(number_of_uhci_controllers >= MAX_NUMBER_OF_UHCI_CONTROLLERS) {
   return;
  }

  pci_enable_io_busmastering(bus, device, function);

  //save UHCI controller info
  uhci_controllers[number_of_uhci_controllers].bus = bus;
  uhci_controllers[number_of_uhci_controllers].device = device;
  uhci_controllers[number_of_uhci_controllers].function = function;
  uhci_controllers[number_of_uhci_controllers].irq = device_irq;
  uhci_controllers[number_of_uhci_controllers].base = pci_read_io_bar(bus, device, function, PCI_BAR4);

  //disable BIOS legacy support and enable normal IRQ to be called
  pci_write(bus, device, function, 0xC0, (1 << 13));

  number_of_uhci_controllers++;

  return;
 }
 
 //Open Host Controller Interface
 if(type_of_device==0x0C0310) {
  if(number_of_ohci_controllers >= MAX_NUMBER_OF_UHCI_CONTROLLERS) {
   return;
  }

  pci_enable_mmio_busmastering(bus, device, function);

  //save OHCI controller info
  ohci_controllers[number_of_ohci_controllers].bus = bus;
  ohci_controllers[number_of_ohci_controllers].device = device;
  ohci_controllers[number_of_ohci_controllers].function = function;
  ohci_controllers[number_of_ohci_controllers].irq = device_irq;
  ohci_controllers[number_of_ohci_controllers].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);

  //disable BIOS legacy support
  if((mmio_ind(ohci_controllers[number_of_ohci_controllers].base+0x0) & (1 << 8)) == (1 << 8)) {
   mmio_outd(ohci_controllers[number_of_ohci_controllers].base+0x100, 0x00);
  }

  number_of_ohci_controllers++;

  return;
 }
 
 //Enhanced Host Controller Interface
 if(type_of_device==0x0C0320) {
  pci_enable_mmio_busmastering(bus, device, function);

  //save EHCI controller info
  ehci_controllers[number_of_ehci_controllers].bus = bus;
  ehci_controllers[number_of_ehci_controllers].device = device;
  ehci_controllers[number_of_ehci_controllers].function = function;
  ehci_controllers[number_of_ehci_controllers].irq = device_irq;
  ehci_controllers[number_of_ehci_controllers].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);

  //disable BIOS ownership
  dword_t pci_ehci_bios_register_offset = ((mmio_ind(ehci_controllers[number_of_ehci_controllers].base+0x08)>>8) & 0xFF);
  if(pci_ehci_bios_register_offset >= 0x40 && (pci_read(0, bus, device, function, pci_ehci_bios_register_offset) & 0xFF)==0x01) {
   pci_write(bus, device, function, pci_ehci_bios_register_offset, (1 << 24)); //set OS ownership
  }

  number_of_ehci_controllers++;

  return;
 }
 
 //eXtensible Host Controller Interface
 if(type_of_device==0x0C0330) {
  pci_enable_mmio_busmastering(bus, device, function);

  //save xHCI controller
  xhci_controllers[number_of_xhci_controllers].bus = bus;
  xhci_controllers[number_of_xhci_controllers].device = device;
  xhci_controllers[number_of_xhci_controllers].function = function;
  xhci_controllers[number_of_xhci_controllers].irq = device_irq;
  xhci_controllers[number_of_xhci_controllers].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);

  //disable BIOS ownership
  dword_t xhci_bios_register_offset = ((mmio_ind(xhci_controllers[number_of_xhci_controllers].base+0x10)>>16)*4);
  logf("%x %x", xhci_bios_register_offset, mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset));
  if(xhci_bios_register_offset != 0 && (mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset) & 0xFF)==0x01) {
   mmio_outd(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset, (mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset) & ~(1 << 16)) | (1 << 24)); //set OS ownership
  }

  number_of_xhci_controllers++;
  
  return;
 }

 //Host bridge
 if(type_of_device==0x060000) {
  return;
 }

 //ISA bridge
 if(type_of_device==0x060100) {
  return;
 }

 //PCI-to-PCI bridge
 if((type_of_device & 0xFFFF00)==0x060400) {
  return;
 }

 //PCI-to-PCI bridge
 if((type_of_device & 0xFFFF00)==0x060900) {
  return;
 }

 //Other bridge
 if(type_of_device==0x068000) {
  return;
 }
}

byte_t *get_pci_vendor_string(dword_t vendor_id) {
 extern dword_t pci_vendor_id_string_array[256];

 for(dword_t i=0; i<256; i+=2) {
  if(pci_vendor_id_string_array[i]==0) {
   break;
  }
  else if(pci_vendor_id_string_array[i]==vendor_id) {
   return (byte_t *)(pci_vendor_id_string_array[i+1]);
  }
 }

 return ""; //this vendor id is not in list
}