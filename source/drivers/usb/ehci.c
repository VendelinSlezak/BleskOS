//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ehci_controller(byte_t controller_number) {
 //disable BIOS ownership
 dword_t pci_ehci_bios_register_offset = ((mmio_ind(usb_controllers[controller_number].base+0x08)>>8) & 0xFF);
 if(pci_ehci_bios_register_offset>=0x40 && (pci_read(usb_controllers[controller_number].bus, usb_controllers[controller_number].device, usb_controllers[controller_number].function, pci_ehci_bios_register_offset) & 0xFF)==0x01) {
  pci_write(usb_controllers[controller_number].bus, usb_controllers[controller_number].device, usb_controllers[controller_number].function, ((mmio_ind(usb_controllers[controller_number].base+0x08)>>8) & 0xFF), (1 << 24)); //set OS ownership
  
  ticks = 0;
  while(ticks<200) {
   asm("nop");
   if((pci_read(usb_controllers[controller_number].bus, usb_controllers[controller_number].device, usb_controllers[controller_number].function, pci_ehci_bios_register_offset) & 0x01010000)==0x01000000) {
    break;
   }
  }
  if(ticks>=200) {
   log("\nEHCI controller is still in BIOS ownership ");
   log_hex(pci_read(usb_controllers[controller_number].bus, usb_controllers[controller_number].device, usb_controllers[controller_number].function, pci_ehci_bios_register_offset));
  }
 }

 //calculate base of operational registers
 usb_controllers[controller_number].base2 = (usb_controllers[controller_number].base+mmio_inb(usb_controllers[controller_number].base+0x00));
 
 //read number of ports
 usb_controllers[controller_number].number_of_ports = (mmio_ind(usb_controllers[controller_number].base+0x04) & 0xF);
 
 //reset controller
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080002);
 wait(50);
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080000);
 
 //set segment
 mmio_outd(usb_controllers[controller_number].base2+0x10, 0x00000000);
 
 //disable interrupts
 mmio_outd(usb_controllers[controller_number].base2+0x08, 0);
 
 //set frame
 mmio_outd(usb_controllers[controller_number].base2+0x0C, 0);
 
 //periodic list
 usb_controllers[controller_number].mem3 = aligned_malloc(1024*4, 0xFFF);
 dword_t *periodic_list = (dword_t *) (usb_controllers[controller_number].mem3);
 for(dword_t i=0; i<1024; i++, periodic_list++) {
  *periodic_list = EHCI_NO_POINTER;
 }
 mmio_outd(usb_controllers[controller_number].base2+0x14, usb_controllers[controller_number].mem3);
 
 //asynchronous list
 usb_controllers[controller_number].mem1 = aligned_malloc(64*5, 0x1F);
 usb_controllers[controller_number].mem2 = aligned_malloc(1030*64, 0x1F);
 mmio_outd(usb_controllers[controller_number].base2+0x18, usb_controllers[controller_number].mem1);
 
 //clear status
 mmio_outd(usb_controllers[controller_number].base2+0x04, 0x3F);
 
 //run controller
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080011);
 
 //set flag
 mmio_outd(usb_controllers[controller_number].base2+0x40, 0x1);
 
 //power all ports
 for(int i=0, port_base=(usb_controllers[controller_number].base2+0x44); i<usb_controllers[controller_number].number_of_ports; i++, port_base+=0x04) {
  mmio_outd(port_base, (mmio_ind(port_base) | 0x1000));
 }
 wait(50);
}

byte_t ehci_controller_detect_status_change(byte_t controller_number) {
 for(int i=0, port_base=(usb_controllers[controller_number].base2+0x44); i<usb_controllers[controller_number].number_of_ports; i++, port_base+=0x4) {
  //is status change bit set?
  if((mmio_ind(port_base) & 0x2)==0x2) {
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE;
}

byte_t ehci_get_port_connection_status(byte_t controller_number, byte_t port_number) {
 return (mmio_ind(usb_controllers[controller_number].base2+0x44+port_number*4) & 0x1);
}

byte_t ehci_get_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 return ((mmio_ind(usb_controllers[controller_number].base2+0x44+port_number*4)>>1) & 0x1);
}

void ehci_clear_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 mmio_outd((usb_controllers[controller_number].base2+0x44+port_number*4), 0x1002);
}

byte_t ehci_enable_device_on_port(byte_t controller_number, byte_t port_number) {
 dword_t port_base = (usb_controllers[controller_number].base2+0x44+port_number*4);

 //reset device and clear status change
 mmio_outd(port_base, 0x1002);
 mmio_outd(port_base, 0x1100);
 wait(50);
 mmio_outd(port_base, 0x1002);
 wait(30);
 
 //pass low/full speed device to companion controller
 if((mmio_ind(port_base) & 0x4)==0x0) {
  usb_controllers[controller_number].ports_state[port_number] = PORT_NO_DEVICE;
  usb_controllers[controller_number].ports_device_speed[port_number] = 0;
  mmio_outd(port_base, 0x3000);
  wait(100);  
  mmio_outd(port_base, 0x3002);  
  return STATUS_ERROR; //device is not enumerated on this controller
 }
 else {
  usb_controllers[controller_number].ports_device_speed[port_number] = USB_HIGH_SPEED;
 }
 
 return STATUS_GOOD;
}

void ehci_set_qh(byte_t controller_number, dword_t qh_number, dword_t first_td, dword_t device_address, dword_t device_speed, dword_t endpoint) {
 dword_t *queue_head = (dword_t *) (usb_controllers[controller_number].mem1+(qh_number*64));
 dword_t attributes = ((device_address) | (endpoint<<8) | (1<<14) | (1<<15)), attributes2 = (0b01<<30);

 if(device_speed==USB_HIGH_SPEED) {
  attributes |= (0b10<<12); //0b10 = high speed
  if(endpoint==0) {
   attributes |= (64<<16); //max size per one transfer
  }
  else {
   attributes |= (512<<16); //max size per one transfer
  }
 }
 else if(device_speed==USB_FULL_SPEED) {
  attributes |= (64<<16); //0b00 = full speed + max size per one transfer
 }
 else if(device_speed==USB_LOW_SPEED) {
  attributes |= ((0b01<<12) | (8<<16)); //0b01 = low speed + max size per one transfer
 }
 if(device_speed==USB_FULL_SPEED || device_speed==USB_LOW_SPEED) {
  attributes2 |= ((ehci_hub_port_number<<23) | (ehci_hub_address<<16)); //describe where is device connected
  if(endpoint==0) { //control transfer to low/full speed device need some extra fields
   attributes |= (1<<27);
   attributes2 |= (0xFF00);
  }
 }

 if(qh_number==0) { //control/bulk transfer
  queue_head[0]=(usb_controllers[controller_number].mem1 | 0x2); //points to itself for fastest transfer
 }
 else { //interrupt transfer
  queue_head[0]=EHCI_NO_POINTER;
  attributes2 |= (0x1C01); //values from specification on Normal Case
 }
 queue_head[1]=attributes;
 queue_head[2]=attributes2;
 queue_head[3]=(usb_controllers[controller_number].mem2+(first_td*64)); //pointer to TD
 queue_head[4]=(usb_controllers[controller_number].mem2+(first_td*64)); //pointer to TD
 for(dword_t i=5; i<16; i++) {
  queue_head[i]=0; //clear rest of queue head
 }
}

void ehci_set_td(byte_t controller_number, dword_t td_number, dword_t next_td_number, dword_t packet_type, dword_t length, dword_t toggle, dword_t memory) {
 dword_t *td = (dword_t *) (usb_controllers[controller_number].mem2+td_number*64);

 if(next_td_number==0) {
  td[0]=EHCI_NO_POINTER;
 }
 else {
  td[0]=(usb_controllers[controller_number].mem2+next_td_number*64); //pointer to next transfer descriptor
 }
 if(next_td_number==0) {
  td[1]=EHCI_NO_POINTER;
 }
 else {
  td[1]=(usb_controllers[controller_number].mem2+next_td_number*64); //pointer to next transfer descriptor
 }
 td[2]=((toggle<<31) | (length<<16) | 0xC00 | (packet_type<<8) | 0x80);
 td[3]=memory;
 if((memory & 0xFFF)!=0) {
  td[4]=((memory & 0xFFFFF000)+0x1000);
 }
 else {
  td[4]=(memory+0x1000);
 }
 td[5]=(td[4]+0x1000);
 td[6]=(td[4]+0x2000);
 td[7]=(td[4]+0x3000);
 for(dword_t i=8; i<16; i++) {
  td[i]=0; //clear rest of transfer descriptor
 }
}

byte_t ehci_control_or_bulk_transfer(byte_t controller_number, byte_t last_descriptor, dword_t time) {
 //start asynchronous transfer
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080031);

 //wait
 dword_t *transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem2+(last_descriptor*64)+8);
 ticks=0;
 while(ticks<time) {
  asm("nop");
  if((*transfer_descriptor & 0x80)!=0x80) { //active bit
   if((*transfer_descriptor & 0x7C)==0x00) { //error bits
    //stop transfer
    mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080011);
    return STATUS_GOOD;
   }
   else {
    //stop transfer
    log("\nEHCI bad transfer ");
    log_hex(*transfer_descriptor);
    mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080011);
    return STATUS_ERROR;
   }
  }
 }
 
 //stop transfer
 log("\nEHCI transfer timeout");
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080011);
 transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem2+8);
 for(dword_t i=0; i<(last_descriptor+1); i++) {
  log("\n");
  log_hex_with_space(*transfer_descriptor);
  transfer_descriptor+=16;
 }
 return STATUS_ERROR;
}

byte_t ehci_control_transfer_without_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed) {
 ehci_set_qh(controller_number, 0, 0, device_address, device_speed, ENDPOINT_0);
 ehci_set_td(controller_number, 0, 1, EHCI_SETUP, 8, TOGGLE_0, (dword_t)usb_setup_packet);
 ehci_set_td(controller_number, 1, 0, EHCI_IN, 0, TOGGLE_1, 0x0);
 return ehci_control_or_bulk_transfer(controller_number, 1, 200);
}

byte_t ehci_control_transfer_with_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed, word_t length) {
 if(length>16384) {
  log("\nEHCI: too big control transfer ");
  log_var(length);
  return STATUS_ERROR;
 }
 
 //create transfer descriptors
 ehci_set_qh(controller_number, 0, 0, device_address, device_speed, ENDPOINT_0);
 ehci_set_td(controller_number, 0, 1, EHCI_SETUP, 8, TOGGLE_0, (dword_t)usb_setup_packet);
 dword_t transfer_descriptor_number = 1, toggle = TOGGLE_1, memory = (dword_t)usb_setup_packet_data;
 while(1) {
  if(length>usb_control_endpoint_size) {
   ehci_set_td(controller_number, transfer_descriptor_number, (transfer_descriptor_number+1), EHCI_IN, usb_control_endpoint_size, toggle, memory);
   transfer_descriptor_number++;
   toggle = ((toggle+1) & 0x1);
   memory += usb_control_endpoint_size;
   length -= usb_control_endpoint_size;
  }
  else {
   ehci_set_td(controller_number, transfer_descriptor_number, (transfer_descriptor_number+1), EHCI_IN, length, toggle, memory);
   transfer_descriptor_number++;
   break;
  }
 }
 ehci_set_td(controller_number, transfer_descriptor_number, 0, EHCI_OUT, 0, TOGGLE_1, 0x0);

 //control transfer
 return ehci_control_or_bulk_transfer(controller_number, transfer_descriptor_number, 200);
}

dword_t ehci_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory) {
 //round interrupt time
 if(interrupt_time<8) {
  interrupt_time = 4;
 }
 else if(interrupt_time<16) {
  interrupt_time = 8;
 }
 else if(interrupt_time<32) {
  interrupt_time = 16;
 }
 else {
  interrupt_time = 32;
 }

 //clear periodic list in case there are pointers to transfer descriptor from previous connected device
 dword_t *periodic_list = (dword_t *) (usb_controllers[controller_number].mem3);
 for(dword_t i=device_number; i<1024; i+=interrupt_time) {
  periodic_list[i] = EHCI_NO_POINTER;
 }

 //create transfer descriptor
 dword_t transfer_descriptor_number = (device_number+1025);
 ehci_set_qh(controller_number, (device_number+1), transfer_descriptor_number, device_address, device_speed, endpoint);
 ehci_set_td(controller_number, transfer_descriptor_number, 0, EHCI_IN, endpoint_size, TOGGLE_0, memory);

 //insert transfer descriptor to frame list
 dword_t queue_head_offset = (usb_controllers[controller_number].mem1+((device_number+1)*64));
 for(dword_t i=device_number; i<1024; i+=interrupt_time) {
  periodic_list[i] = (queue_head_offset | 0x2);
 }

 return (usb_controllers[controller_number].mem2+(transfer_descriptor_number*64)+8);
}

byte_t ehci_bulk_transfer(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t transfer_type, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(device_speed==USB_LOW_SPEED) {
  log("\nUHCI: bulk transfer to low speed device");
  return STATUS_ERROR;
 }
 dword_t bytes_per_transfer_descriptor = 64;
 if(device_speed==USB_HIGH_SPEED) {
  bytes_per_transfer_descriptor = 512;
 }
 if(length>(bytes_per_transfer_descriptor*512)) {
  log("\nEHCI: too big bulk out transfer ");
  log_var(length);
  return STATUS_ERROR;
 }

 ehci_set_qh(controller_number, 0, 0, device_address, device_speed, endpoint);
 dword_t transfer_descriptor_number = 0;
 while(1) {
  if(length>bytes_per_transfer_descriptor) {
   ehci_set_td(controller_number, transfer_descriptor_number, (transfer_descriptor_number+1), transfer_type, bytes_per_transfer_descriptor, toggle, memory);
   transfer_descriptor_number++;
   toggle = ((toggle+1) & 0x1);
   memory += bytes_per_transfer_descriptor;
   length -= bytes_per_transfer_descriptor;
  }
  else {
   ehci_set_td(controller_number, transfer_descriptor_number, 0, transfer_type, length, toggle, memory);
   usb_bulk_toggle = ((toggle+1) & 0x1);
   break;
  }
 }
 return ehci_control_or_bulk_transfer(controller_number, transfer_descriptor_number, time_to_wait);
}

byte_t ehci_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 dword_t bytes_per_transfer_descriptor = 64, max_length_per_transfer = 0;
 if(device_speed==USB_HIGH_SPEED) {
  bytes_per_transfer_descriptor = 512;
 }
 max_length_per_transfer = (bytes_per_transfer_descriptor*512);
 
 if(length>max_length_per_transfer) {
  while(1) {
   if(length>max_length_per_transfer) {
    if(ehci_bulk_transfer(controller_number, device_address, device_speed, EHCI_OUT, endpoint, toggle, memory, max_length_per_transfer, time_to_wait)==STATUS_ERROR) {
     return STATUS_ERROR;
    }
    memory+=max_length_per_transfer;
    length-=max_length_per_transfer;
    toggle=usb_bulk_toggle;
   }
   else {
    return ehci_bulk_transfer(controller_number, device_address, device_speed, EHCI_OUT, endpoint, toggle, memory, length, time_to_wait);
   }
  }
 }
 else {
  return ehci_bulk_transfer(controller_number, device_address, device_speed, EHCI_OUT, endpoint, toggle, memory, length, time_to_wait);
 }
}

byte_t ehci_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 dword_t bytes_per_transfer_descriptor = 64, max_length_per_transfer = 0;
 if(device_speed==USB_HIGH_SPEED) {
  bytes_per_transfer_descriptor = 512;
 }
 max_length_per_transfer = (bytes_per_transfer_descriptor*512);
 
 if(length>max_length_per_transfer) {
  while(1) {
   if(length>max_length_per_transfer) {
    if(ehci_bulk_transfer(controller_number, device_address, device_speed, EHCI_IN, endpoint, toggle, memory, max_length_per_transfer, time_to_wait)==STATUS_ERROR) {
     return STATUS_ERROR;
    }
    memory+=max_length_per_transfer;
    length-=max_length_per_transfer;
    toggle=usb_bulk_toggle;
   }
   else {
    return ehci_bulk_transfer(controller_number, device_address, device_speed, EHCI_IN, endpoint, toggle, memory, length, time_to_wait);
   }
  }
 }
 else {
  return ehci_bulk_transfer(controller_number, device_address, device_speed, EHCI_IN, endpoint, toggle, memory, length, time_to_wait);
 }
}

byte_t ehci_hub_is_there_some_port_connection_status_change(byte_t hub_number) {
 dword_t buffer_mem_pointer = usb_hub_transfer_setup_packets_mem;
 dword_t *buffer = (dword_t *) (usb_hub_transfer_setup_packets_mem);
 dword_t data_mem_pointer = usb_hub_transfer_data_mem;
 dword_t *data = (dword_t *) (usb_hub_transfer_data_mem);
 dword_t number_of_packets = 0;

 //set read HUB PORT STATE SETUP packet
 for(dword_t i=0; i<usb_hubs[hub_number].number_of_ports; i++) {
  buffer[0]=0x000000A3;
  buffer[1]=(0x00040000 | (i+1)); //ports are 1 based
  buffer+=2;
 }
 clear_memory(usb_hub_transfer_data_mem, 4*8);

 //set chain of packets
 ehci_set_qh(usb_hubs[hub_number].controller_number, 0, 0, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, ENDPOINT_0);
 for(dword_t i=0; i<usb_hubs[hub_number].number_of_ports; i++) {
  ehci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, (number_of_packets+1), EHCI_SETUP, 8, TOGGLE_0, buffer_mem_pointer);
  buffer_mem_pointer+=8;
  number_of_packets++;
  ehci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, (number_of_packets+1), EHCI_IN, 4, TOGGLE_1, data_mem_pointer);
  data_mem_pointer+=4;
  number_of_packets++;
  ehci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, (number_of_packets+1), EHCI_OUT, 0, TOGGLE_1, 0x0);
  number_of_packets++;
 }
 number_of_packets--;
 ehci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, 0, EHCI_OUT, 0, TOGGLE_1, 0x0);
 
 //transfer
 if(ehci_control_or_bulk_transfer(usb_hubs[hub_number].controller_number, number_of_packets, 20)==STATUS_ERROR) {
  return USB_HUB_IS_NOT_RESPONDING;
 }

 //return port state
 for(dword_t i=0; i<usb_hubs[hub_number].number_of_ports; i++) {
  if((data[i] & 0x10000)==0x10000) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}