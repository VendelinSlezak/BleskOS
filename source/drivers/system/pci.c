//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
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
 ide_controllers_info_mem = calloc(10*16);
 ide_controllers_pointer = 0;
 ide_0x1F0_controller_present = STATUS_FALSE;
 ide_0x170_controller_present = STATUS_FALSE;
 ahci_controllers_pointer = 0;
 ahci_ports_pointer = 0;
 ethernet_cards_pointer = 0;
 usb_controllers_pointer = 0;
 ac97_present = DEVICE_NOT_PRESENT;
 hda_sound_card_pointer = 0;

 //this array is used in System board
 pci_devices_array_mem = calloc(12*1000);
 pci_num_of_devices = 0;
 
 log("PCI devices:\n");

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
 dword_t device_id, type_of_device, class, subclass, progif, mmio_port_base;
 word_t io_port_base;

 //read base informations about device
 device_id = pci_read(bus, device, function, 0);
 if(device_id==0xFFFFFFFF) {
  return; //no device
 }
 type_of_device = (pci_read(bus, device, function, 0x08) >> 8);
 class = (type_of_device >> 16);
 subclass = ((type_of_device >> 8) & 0xFF);
 progif = (type_of_device & 0xFF);

 //save device to array
 if(pci_num_of_devices<1000) {
  dword_t *pci_devices_array = (dword_t *) (pci_devices_array_mem+pci_num_of_devices*12);
  pci_devices_array[0] = (bus | (device<<8) | (function<<16));
  pci_devices_array[1] = type_of_device;
  pci_devices_array[2] = device_id;
  pci_num_of_devices++;
 }
 
 //IDE controller
 if((type_of_device & 0xFFFF00)==0x010100) {
  log("IDE controller\n");
 
  pci_device_ide_controller:
  pci_enable_io_busmastering(bus, device, function);
  pci_disable_interrupts(bus, device, function);

  //first controller
  word_t *ide_controllers_info = (word_t *) (ide_controllers_info_mem+ide_controllers_pointer*16);
  io_port_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
  if(ide_controllers_pointer>=10) {
   return;
  }
  if(io_port_base==0 || io_port_base==0x1F0) {
   if(ide_0x1F0_controller_present==STATUS_FALSE && inb(0x1F7)!=0xFF) {
    ide_0x1F0_controller_present = STATUS_TRUE;
    ide_controllers_info[0] = 0x1F0;
    ide_controllers_info[1] = 0x3F4;
    ide_controllers_pointer++;
   }
  }
  else {
   ide_controllers_info[0] = io_port_base;
   ide_controllers_info[1] = pci_read_io_bar(bus, device, function, PCI_BAR1);
   ide_controllers_pointer++;
  }

  //second controller
  ide_controllers_info = (word_t *) (ide_controllers_info_mem+ide_controllers_pointer*16);
  io_port_base = pci_read_io_bar(bus, device, function, PCI_BAR2);
  if(ide_controllers_pointer>=10) {
   return;
  }
  if(io_port_base==0 || io_port_base==0x170) {
   if(ide_0x170_controller_present==STATUS_FALSE && inb(0x177)!=0xFF) {
    ide_0x170_controller_present = STATUS_TRUE;
    ide_controllers_info[0] = 0x170;
    ide_controllers_info[1] = 0x374;
    ide_controllers_pointer++;
   }
  }
  else {
   ide_controllers_info[0] = io_port_base;
   ide_controllers_info[1] = pci_read_io_bar(bus, device, function, PCI_BAR3);
   ide_controllers_pointer++;
  }
  
  return;
 }
 
 //SATA controller
 if(type_of_device==0x010601) {
  log("AHCI controller\n");
  
  ahci_base = pci_read_mmio_bar(bus, device, function, PCI_BAR5);
  pci_enable_mmio_busmastering(bus, device, function);
  
  //test presence of AHCI interface
  if((mmio_ind(ahci_base + 0x04) & 0x80000000)==0x00000000) {
   goto pci_device_ide_controller; //IDE interface
  }
  
  if(ahci_controllers_pointer<10) {
   ahci_controllers[ahci_controllers_pointer]=ahci_base;
   ahci_controllers_pointer++;
  }
  return;
 }
 
 //Ethernet card
 if(type_of_device==0x020000) {
  log("Ethernet card ");
  log_hex_with_space(device_id);
  
  if(ethernet_cards_pointer>=10) {
   return;
  }
  
  //read base values
  ethernet_cards[ethernet_cards_pointer].present = DEVICE_PRESENT;
  ethernet_cards[ethernet_cards_pointer].id = device_id;
  ethernet_cards[ethernet_cards_pointer].irq = (pci_read(bus, device, function, 0x3C) & 0xFF);
  ethernet_cards[ethernet_cards_pointer].driver = NETWORK_NO_DRIVER;
  ethernet_cards[ethernet_cards_pointer].bus = bus;
  ethernet_cards[ethernet_cards_pointer].dev = device;
  ethernet_cards[ethernet_cards_pointer].func = function;
  
  //Intel E1000
  if((device_id & 0xFFFF)==VENDOR_INTEL) {
   ethernet_cards[ethernet_cards_pointer].driver = NETWORK_DRIVER_ETHERNET_INTEL_E1000;
   ethernet_cards[ethernet_cards_pointer].bar_type = pci_read_bar_type(bus, device, function, PCI_BAR0);
   if(ethernet_cards[ethernet_cards_pointer].bar_type==PCI_MMIO_BAR) {
    ethernet_cards[ethernet_cards_pointer].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
    pci_enable_mmio_busmastering(bus, device, function);
   }
   else {
    ethernet_cards[ethernet_cards_pointer].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
    pci_enable_io_busmastering(bus, device, function);
   }
   
   ethernet_cards_pointer++;
   log("Intel E1000\n");
   return;
  }
  
  //AMD PC-net
  if(((device_id & 0xFFFF)==VENDOR_AMD_1 || (device_id & 0xFFFF)==VENDOR_AMD_2)) {
   ethernet_cards[ethernet_cards_pointer].driver = NETWORK_DRIVER_ETHERNET_AMD_PCNET;
   pci_enable_io_busmastering(bus, device, function);
   ethernet_cards[ethernet_cards_pointer].bar_type = PCI_IO_BAR;
   ethernet_cards[ethernet_cards_pointer].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
   
   ethernet_cards_pointer++;
   log("AMD PC-net\n");
   return;
  }
  
  //Broadcom NetXtreme
  if((device_id & 0xFFFF)==VENDOR_BROADCOM) {
   ethernet_cards[ethernet_cards_pointer].driver = NETWORK_DRIVER_ETHERNET_BROADCOM_NETXTREME;
   pci_enable_mmio_busmastering(bus, device, function);
   ethernet_cards[ethernet_cards_pointer].bar_type = PCI_MMIO_BAR;
   ethernet_cards[ethernet_cards_pointer].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
   
   ethernet_cards_pointer++;
   log("Broadcom NetXtreme\n");
   return;
  }
  
  //Realtek 8139/8169
  if((device_id & 0xFFFF)==VENDOR_REALTEK) {
   device_id >>= 16;
   
   if(device_id==0x8139) {
    ethernet_cards[ethernet_cards_pointer].driver = NETWORK_DRIVER_ETHERNET_REALTEK_8139;
    pci_enable_io_busmastering(bus, device, function);
    ethernet_cards[ethernet_cards_pointer].bar_type = PCI_IO_BAR;
    ethernet_cards[ethernet_cards_pointer].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
   
    ethernet_cards_pointer++;
    log("Realtek 8139\n");
    return;
   }
   
   if(device_id==0x8136 || device_id==0x8161 || device_id==0x8168 || device_id==0x8169) {
    ethernet_cards[ethernet_cards_pointer].driver = NETWORK_DRIVER_ETHERNET_REALTEK_8169;
    ethernet_cards[ethernet_cards_pointer].bar_type = pci_read_bar_type(bus, device, function, PCI_BAR0);
    if(ethernet_cards[ethernet_cards_pointer].bar_type==PCI_MMIO_BAR) {
     ethernet_cards[ethernet_cards_pointer].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
     pci_enable_mmio_busmastering(bus, device, function);
    }
    else {
     ethernet_cards[ethernet_cards_pointer].base = pci_read_io_bar(bus, device, function, PCI_BAR0);
     pci_enable_io_busmastering(bus, device, function);
    }
   
    ethernet_cards_pointer++;
    log("Realtek 8169\n");
    return;
   }
  }
  
  ethernet_cards_pointer++;
  log("\n");
  return;
 }
 
 if(class==0x03) {
  log("graphic card\n");
 }
 
 //AC97 sound card
 if(type_of_device==0x040100) {
  log("sound card AC97\n");
  ac97_present = DEVICE_PRESENT;
  ac97_nam_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
  ac97_nabm_base = pci_read_io_bar(bus, device, function, PCI_BAR1);
  pci_enable_io_busmastering(bus, device, function);
  return;
 }
 
 //HD Audio sound card
 if(type_of_device==0x040300) {
  log("sound card HDA\n");
  if(hda_sound_card_pointer>=10) {
   return;
  }
  hda_sound_cards[hda_sound_card_pointer].present = DEVICE_PRESENT;
  hda_sound_cards[hda_sound_card_pointer].base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  hda_sound_cards[hda_sound_card_pointer].communication = HDA_UNINITALIZED;
  pci_enable_mmio_busmastering(bus, device, function);
  hda_sound_card_pointer++;
  return;
 }
 
 //Universal Host Controller Interface
 if(type_of_device==0x0C0300) {
  log("UHCI\n");
  if(usb_controllers_pointer>=20) {
   return;
  }
  pci_write(bus, device, function, 0xC0, 0x8F00); //disable BIOS legacy support
  usb_controllers[usb_controllers_pointer].type=USB_CONTROLLER_UHCI;
  usb_controllers[usb_controllers_pointer].base=pci_read_io_bar(bus, device, function, PCI_BAR4);
  usb_controllers_pointer++;
  return;
 }
 
 //Open Host Controller Interface
 if(type_of_device==0x0C0310) {
  log("OHCI\n");
  if(usb_controllers_pointer>=20) {
   return;
  }
  usb_controllers[usb_controllers_pointer].type=USB_CONTROLLER_OHCI;
  usb_controllers[usb_controllers_pointer].base=pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  //disable legacy support
  if(mmio_ind(usb_controllers[usb_controllers_pointer].base+0x0)==0x110) {
   mmio_outd(usb_controllers[usb_controllers_pointer].base+0x100, 0x00);
  }
  usb_controllers_pointer++;
  return;
 }
 
 //Enhanced Host Controller Interface
 if(type_of_device==0x0C0320) {
  log("EHCI\n");
  if(usb_controllers_pointer>=20) {
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
  log("xHCI\n");
  return;
 }
 
 log("unknown device ");
 log_hex(type_of_device);
 log("\n");
 pci_disable_interrupts(bus, device, function); //disable interrupts from every device that we do not know
}

byte_t *get_pci_vendor_string(dword_t vendor_id) {
 extern dword_t pci_vendor_id_string_array[256];

 for(dword_t i=0; i<256; i+=2) {
  if(pci_vendor_id_string_array[i]==0) {
   return ""; //this vendor id is not in list
  }
  else if(pci_vendor_id_string_array[i]==vendor_id) {
   return (byte_t *)(pci_vendor_id_string_array[i+1]);
  }
 }
}