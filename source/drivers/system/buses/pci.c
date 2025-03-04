//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t pci_read(dword_t bus, dword_t device, dword_t function, dword_t offset) {
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
 pci_write(bus, device, function, 0x04, ((pci_read(bus, device, function, 0x04) & ~(1<<10)) | (1<<2) | (1<<0))); //enable interrupts, enable bus mastering, enable IO space
}

void pci_enable_mmio_busmastering(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, ((pci_read(bus, device, function, 0x04) & ~(1<<10)) | (1<<2) | (1<<1))); //enable interrupts, enable bus mastering, enable MMIO space
}

void pci_disable_interrupts(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, (pci_read(bus, device, function, 0x04) | (1<<10))); //disable interrupts
}

void scan_pci(void) {
 //initalize values that are used to determine presence of devices
 ide_0x1F0_controller_present = STATUS_FALSE;
 ide_0x170_controller_present = STATUS_FALSE;
 number_of_graphic_cards = 0;
 number_of_sound_cards = 0;
 number_of_storage_controllers = 0;
 number_of_ethernet_cards = 0;
 number_of_uhci_controllers = 0;
 number_of_ehci_controllers = 0;

 //this array is used in System board
 pci_devices_array_mem = (dword_t) calloc(12*1000);
 pci_num_of_devices = 0;
 
 log("\n\nPCI devices:");

 for(dword_t bus=0; bus<256; bus++) {
  for(dword_t device=0; device<32; device++) {
   scan_pci_device(bus, device, 0);
   
   //multifunctional device
   if((pci_read(bus, device, 0, 0x0C) & 0x00800000)==0x00800000) {
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
 vendor_id = (pci_read(bus, device, function, 0) & 0xFFFF);
 device_id = (pci_read(bus, device, function, 0) >> 16);
 full_device_id = pci_read(bus, device, function, 0);
 if(full_device_id==0xFFFFFFFF) {
  return; //no device
 }
 type_of_device = (pci_read(bus, device, function, 0x08) >> 8);
 class = (type_of_device >> 16);
 subclass = ((type_of_device >> 8) & 0xFF);
 progif = (type_of_device & 0xFF);
 device_irq = (pci_read(bus, device, function, 0x3C) & 0xFF);

 //save device to array
 if(pci_num_of_devices<1000) {
  dword_t *pci_devices_array = (dword_t *) (pci_devices_array_mem+pci_num_of_devices*12);
  pci_devices_array[0] = (bus | (device<<8) | (function<<16));
  pci_devices_array[1] = type_of_device;
  pci_devices_array[2] = full_device_id;
  pci_num_of_devices++;
 }

 //Graphic card
 if(type_of_device==0x030000 && number_of_graphic_cards<MAX_NUMBER_OF_GRAPHIC_CARDS) {
  log("\nGraphic card");

  if(number_of_graphic_cards >= MAX_NUMBER_OF_GRAPHIC_CARDS) {
   return;
  }

  graphic_cards_info[number_of_graphic_cards].vendor_id = vendor_id;
  graphic_cards_info[number_of_graphic_cards].device_id = device_id;
  graphic_cards_info[number_of_graphic_cards].initalize = 0;

  if(vendor_id == VENDOR_INTEL) {
   pci_enable_io_busmastering(bus, device, function);
   pci_enable_mmio_busmastering(bus, device, function);
   graphic_cards_info[number_of_graphic_cards].mmio_base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
   graphic_cards_info[number_of_graphic_cards].linear_frame_buffer = (byte_t *) (pci_read_mmio_bar(bus, device, function, PCI_BAR2));
   graphic_cards_info[number_of_graphic_cards].initalize = initalize_intel_graphic_card;
  }
  else if(vendor_id == 0x15AD && device_id == 0x0405) {
   pci_enable_io_busmastering(bus, device, function);
   pci_enable_mmio_busmastering(bus, device, function);
   graphic_cards_info[number_of_graphic_cards].io_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
   graphic_cards_info[number_of_graphic_cards].initalize = initalize_vmware_graphic_card;
  }

  number_of_graphic_cards++;
  return;
 }
 
 //IDE controller
 if((type_of_device & 0xFFFF00)==0x010100 && number_of_storage_controllers<MAX_NUMBER_OF_STORAGE_CONTROLLERS) { 
  log("\nIDE controller");

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
  log("\nAHCI controller");
  pci_enable_mmio_busmastering(bus, device, function);
  pci_disable_interrupts(bus, device, function);
  
  //test presence of AHCI interface
  if((mmio_ind(pci_read_mmio_bar(bus, device, function, PCI_BAR5) + 0x04) & 0x80000000)==0x00000000) {
   log(" with IDE interface");
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
  log("\nEthernet card ");
  log_hex_with_space(full_device_id);
  
  if(number_of_ethernet_cards>=MAX_NUMBER_OF_ETHERNET_CARDS) {
   return;
  }
  
  //read basic values
  ethernet_cards[number_of_ethernet_cards].id = full_device_id;
  ethernet_cards[number_of_ethernet_cards].irq = (pci_read(bus, device, function, 0x3C) & 0xFF);
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
  log("\nWireless card "); log_hex_with_space(full_device_id);

  return;
 }
 
 //AC97 sound card
 if(type_of_device==0x040100 && number_of_sound_cards<MAX_NUMBER_OF_SOUND_CARDS) {
  log("\nsound card AC97");

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
  log("\nsound card HDA");

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
  log("\nUHCI ");

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
  log("\nOHCI");

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
  log("\nEHCI ");

  pci_enable_mmio_busmastering(bus, device, function);

  //save EHCI controller info
  ehci_controllers[number_of_ehci_controllers].bus = bus;
  ehci_controllers[number_of_ehci_controllers].device = device;
  ehci_controllers[number_of_ehci_controllers].function = function;
  ehci_controllers[number_of_ehci_controllers].irq = device_irq;
  ehci_controllers[number_of_ehci_controllers].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);

  //disable BIOS ownership
  dword_t pci_ehci_bios_register_offset = ((mmio_ind(ehci_controllers[number_of_ehci_controllers].base+0x08)>>8) & 0xFF);
  if(pci_ehci_bios_register_offset >= 0x40 && (pci_read(bus, device, function, pci_ehci_bios_register_offset) & 0xFF)==0x01) {
   l("(releasing BIOS ownership)");
   pci_write(bus, device, function, pci_ehci_bios_register_offset, (1 << 24)); //set OS ownership
  }

  number_of_ehci_controllers++;

  return;
 }
 
 //eXtensible Host Controller Interface
 if(type_of_device==0x0C0330) {
  log("\nxHCI");

  pci_enable_mmio_busmastering(bus, device, function);

  //save xHCI controller
  xhci_controllers[number_of_xhci_controllers].bus = bus;
  xhci_controllers[number_of_xhci_controllers].device = device;
  xhci_controllers[number_of_xhci_controllers].function = function;
  xhci_controllers[number_of_xhci_controllers].irq = device_irq;
  xhci_controllers[number_of_xhci_controllers].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);

  //disable BIOS ownership
  dword_t xhci_bios_register_offset = ((mmio_ind(xhci_controllers[number_of_xhci_controllers].base+0x10)>>16)*4);
  lhw(xhci_bios_register_offset);
  lhw(mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset));
  if(xhci_bios_register_offset != 0 && (mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset) & 0xFF)==0x01) {
   l("(releasing BIOS ownership)");
   mmio_outd(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset, (mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset) & ~(1 << 16)) | (1 << 24)); //set OS ownership
  }

  number_of_xhci_controllers++;
  
  return;
 }

 //Host bridge
 if(type_of_device==0x060000) {
  log("\nHost bridge");
  
  return;
 }

 //ISA bridge
 if(type_of_device==0x060100) {
  log("\nISA bridge ");
  
  return;
 }

 //PCI-to-PCI bridge
 if((type_of_device & 0xFFFF00)==0x060400) {
  log("\nPCI-to-PCI bridge 0x04");
  
  return;
 }

 //PCI-to-PCI bridge
 if((type_of_device & 0xFFFF00)==0x060900) {
  log("\nPCI-to-PCI bridge 0x09");
  
  return;
 }

 //Other bridge
 if(type_of_device==0x068000) {
  log("\nOther bridge");
  
  return;
 }
 
 log("\nunknown device ");
 log_hex(type_of_device);
 // pci_disable_interrupts(bus, device, function); //disable interrupts from every device that we do not know
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