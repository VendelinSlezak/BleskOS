//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
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
 usb_controllers_pointer = 0;
 number_of_graphic_cards = 0;
 number_of_sound_cards = 0;
 number_of_storage_controllers = 0;
 number_of_ethernet_cards = 0;

 //this array is used in System board
 pci_devices_array_mem = calloc(12*1000);
 pci_num_of_devices = 0;
 
 log("\n\nPCI devices:");

 for(int bus=0; bus<256; bus++) {
  for(int device=0; device<32; device++) {
   scan_pci_device(bus, device, 0);
   
   //multifunctional device
   if( (pci_read(bus, device, 0, 0x0C) & 0x00800000)==0x00800000) {
    for(int function=1; function<8; function++) {
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

  if(number_of_graphic_cards>=MAX_NUMBER_OF_GRAPHIC_CARDS) {
   return;
  }

  graphic_cards_info[number_of_graphic_cards].vendor_id = vendor_id;
  graphic_cards_info[number_of_graphic_cards].device_id = device_id;

  if(vendor_id==VENDOR_INTEL) {
   pci_enable_io_busmastering(bus, device, function);
   graphic_cards_info[number_of_graphic_cards].mmio_base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
   graphic_cards_info[number_of_graphic_cards].linear_frame_buffer = (byte_t *) (pci_read_mmio_bar(bus, device, function, PCI_BAR2));
  }

  number_of_graphic_cards++;
 }
 
 //IDE controller
 if((type_of_device & 0xFFFF00)==0x010100 && number_of_storage_controllers<MAX_NUMBER_OF_STORAGE_CONTROLLERS) { 
  pci_device_ide_controller:
  log("\nIDE controller");
  pci_enable_io_busmastering(bus, device, function);
  pci_disable_interrupts(bus, device, function);

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
  log("\nUHCI");

  if(usb_controllers_pointer>=MAX_NUMBER_OF_USB_CONTROLLERS) {
   return;
  }

  usb_controllers[usb_controllers_pointer].type=USB_CONTROLLER_UHCI;
  usb_controllers[usb_controllers_pointer].base=pci_read_io_bar(bus, device, function, PCI_BAR4);
  usb_controllers_pointer++;

  //disable BIOS legacy support
  pci_write(bus, device, function, 0xC0, 0x8F00);

  return;
 }
 
 //Open Host Controller Interface
 if(type_of_device==0x0C0310) {
  log("\nOHCI");

  if(usb_controllers_pointer>=MAX_NUMBER_OF_USB_CONTROLLERS) {
   return;
  }

  usb_controllers[usb_controllers_pointer].type=USB_CONTROLLER_OHCI;
  usb_controllers[usb_controllers_pointer].base=pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  usb_controllers_pointer++;

  //disable BIOS legacy support
  if(mmio_ind(usb_controllers[usb_controllers_pointer].base+0x0)==0x110) {
   mmio_outd(usb_controllers[usb_controllers_pointer].base+0x100, 0x00);
  }

  return;
 }
 
 //Enhanced Host Controller Interface
 if(type_of_device==0x0C0320) {
  log("\nEHCI");

  if(usb_controllers_pointer>=MAX_NUMBER_OF_USB_CONTROLLERS) {
   return;
  }

  usb_controllers[usb_controllers_pointer].bus=bus;
  usb_controllers[usb_controllers_pointer].device=device;
  usb_controllers[usb_controllers_pointer].function=function;
  usb_controllers[usb_controllers_pointer].type=USB_CONTROLLER_EHCI;
  usb_controllers[usb_controllers_pointer].base=pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  usb_controllers_pointer++;

  return;
 }
 
 //eXtensible Host Controller Interface
 if(type_of_device==0x0C0330) {
  log("\nxHCI");
  
  return;
 }
 
 log("\nunknown device ");
 log_hex(type_of_device);
 pci_disable_interrupts(bus, device, function); //disable interrupts from every device that we do not know
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