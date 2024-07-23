//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_uhci_controller(byte_t controller_number) {
 //reset controller
 outw(usb_controllers[controller_number].base+0x0, 0x4);
 wait(20);
 outw(usb_controllers[controller_number].base+0x0, 0x0);
 
 //clear status
 outw(usb_controllers[controller_number].base+0x2, 0x3F);
 
 //disable interrupts
 outw(usb_controllers[controller_number].base+0x4, 0x0000);
 
 //set frame number 0
 outw(usb_controllers[controller_number].base+0x6, 0);
 
 //initalize frame list
 usb_controllers[controller_number].mem1 = aligned_malloc(1024*4, 0xFFF);
 usb_controllers[controller_number].mem4 = malloc(1024*4); //used to save frame list
 dword_t *frame_list = (dword_t *) (usb_controllers[controller_number].mem1);
 for(dword_t i=0; i<1024; i++, frame_list++) {
  *frame_list = UHCI_NO_POINTER;
 }
 outd(usb_controllers[controller_number].base+0x8, usb_controllers[controller_number].mem1);
 
 //allocate memory for queue heads and transfer descriptors
 usb_controllers[controller_number].mem2 = aligned_malloc(8, 0xF);
 usb_controllers[controller_number].mem3 = aligned_malloc(32*520, 0xF);
 
 //set SOF
 outb(usb_controllers[controller_number].base+0xC, 0x40);
 
 //find number of ports
 usb_controllers[controller_number].number_of_ports=0;
 for(int i=0, port_number=0x10; i<16; i++, port_number+=0x2) {
  if(inw(usb_controllers[controller_number].base+port_number)==0xFFFF || (inw(usb_controllers[controller_number].base+port_number) & 0x80)!=0x80) {
   break;
  }
  
  usb_controllers[controller_number].number_of_ports++;
 }
 
 //run controller
 outw(usb_controllers[controller_number].base+0x0, 0x81);
}

byte_t uhci_controller_detect_status_change(byte_t controller_number) {
 for(int i=0, port_number=0x10; i<usb_controllers[controller_number].number_of_ports; i++, port_number+=0x2) {
  //is status change bit set?
  if((inw(usb_controllers[controller_number].base+port_number) & 0x2)==0x2) {
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE;
}

byte_t uhci_get_port_connection_status(byte_t controller_number, byte_t port_number) {
 return (inw(usb_controllers[controller_number].base+0x10+(port_number*2)) & 0x1);
}

byte_t uhci_get_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 return ((inw(usb_controllers[controller_number].base+0x10+(port_number*2))>>1) & 0x1);
}

void uhci_clear_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 outw((usb_controllers[controller_number].base+0x10+(port_number*2)), 0x2);
}

byte_t uhci_enable_device_on_port(byte_t controller_number, byte_t port_number) {
 word_t port_base = (usb_controllers[controller_number].base+0x10+(port_number*2));

 //reset device
 outw(port_base, 0x0201);
 wait(100);
 outw(port_base, 0x0001);
 wait(50);

 //clear status change and enable device, this must not be done in one write
 outw(port_base, 0x3);
 wait(1);
 outw(port_base, 0x5);
 wait(100);
 if((inw(port_base) & 0x4)==0x0) {
  log("\nUHCI: device was not enabled");
  return STATUS_ERROR;
 }
 
 //find if this is low speed or full speed device
 if((inw(port_base) & 0x0100)==0x0100) {
  usb_controllers[controller_number].ports_device_speed[port_number]=USB_LOW_SPEED;
 }
 else {
  usb_controllers[controller_number].ports_device_speed[port_number]=USB_FULL_SPEED;
 }

 return STATUS_GOOD;
}

void uhci_set_transfer_descriptor(byte_t controller_number, byte_t device_speed, dword_t descriptor, dword_t next_descriptor, dword_t length, dword_t toggle, dword_t endpoint, dword_t device_address, dword_t packet_type, dword_t buffer) {
 dword_t *transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem3+(descriptor*32));
 
 if(next_descriptor==0) {
  transfer_descriptor[0]=UHCI_NO_POINTER;
 }
 else {
  transfer_descriptor[0]=((usb_controllers[controller_number].mem3+(next_descriptor*32)) | 0x4); //depth first
 }
 if(device_speed==USB_LOW_SPEED) {
  transfer_descriptor[1]=0x04800000; //low speed transfer
 }
 else {
  transfer_descriptor[1]=0x00800000; //full speed transfer
 }
 transfer_descriptor[2]=(((length-1)<<21) | (toggle<<19) | (endpoint<<15) | (device_address<<8) | (packet_type));
 transfer_descriptor[3]=buffer;
 transfer_descriptor[4]=0;
 transfer_descriptor[5]=0;
 transfer_descriptor[6]=0;
 transfer_descriptor[7]=0;
}

byte_t uhci_control_or_bulk_transfer(byte_t controller_number, byte_t last_descriptor, dword_t time) {
 //save frame list if there are some interrupt transfers
 copy_memory(usb_controllers[controller_number].mem1, usb_controllers[controller_number].mem4, 1024*4);

 //set queue head
 dword_t *queue_head = (dword_t *) (usb_controllers[controller_number].mem2);
 queue_head[0] = UHCI_NO_POINTER;
 queue_head[1] = (usb_controllers[controller_number].mem3);

 //start transfer
 dword_t *frame_list = (dword_t *) (usb_controllers[controller_number].mem1);
 for(dword_t i=0; i<1024; i++, frame_list++) {
  *frame_list = (usb_controllers[controller_number].mem2 | 0x2); //pointer to queue head
 }

 //wait for transfer
 dword_t *transfer_descriptor_status = (dword_t *) (usb_controllers[controller_number].mem3+(last_descriptor*32)+4);
 ticks=0;
 while(ticks<time) {
  asm("nop");
  if((*transfer_descriptor_status & 0x00800000)==0x00000000) { //active bit
   if((*transfer_descriptor_status & 0x007E0000)==0x00000000) { //error bits
    //restore frame list
    copy_memory(usb_controllers[controller_number].mem4, usb_controllers[controller_number].mem1, 1024*4);

    return STATUS_GOOD;
   }
   else {
    //log state of transfer descriptor
    log("\nUHCI bad transfer ");
    log_hex_with_space(transfer_descriptor_status[0]);
    log_hex_with_space(transfer_descriptor_status[1]);

    //restore frame list
    copy_memory(usb_controllers[controller_number].mem4, usb_controllers[controller_number].mem1, 1024*4);

    return STATUS_ERROR;
   }
  }
 }
 
 //log transfer descriptors status in case of timeout
 log("\nUHCI transfer timeout ");
 log_var(last_descriptor);
 dword_t *transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem3+4);
 for(dword_t i=0; i<(last_descriptor+1); i++) {
  log("\n");
  log_hex_with_space(transfer_descriptor[0]);
  log_hex_with_space(transfer_descriptor[1]);
  transfer_descriptor+=8;
 }
 log("\n");

 //restore frame list
 copy_memory(usb_controllers[controller_number].mem4, usb_controllers[controller_number].mem1, 1024*4);

 return STATUS_ERROR;
}

byte_t uhci_control_transfer_without_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed) {
 //create transfer descriptors
 uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, device_address, UHCI_SETUP, (dword_t)usb_setup_packet);
 uhci_set_transfer_descriptor(controller_number, device_speed, 1, 0, 0, TOGGLE_1, ENDPOINT_0, device_address, UHCI_IN, 0x0);

 //control transfer
 return uhci_control_or_bulk_transfer(controller_number, 1, 200);
}

byte_t uhci_control_transfer_with_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed, word_t length) {
 if(length>16384) {
  log("\nUHCI: too big control transfer ");
  log_var(length);
  return STATUS_ERROR;
 }
 
 //create transfer descriptors
 uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, device_address, UHCI_SETUP, (dword_t)usb_setup_packet);
 dword_t transfer_descriptor_number = 1, toggle = TOGGLE_1, memory = (dword_t)usb_setup_packet_data;
 while(1) {
  if(length>usb_control_endpoint_size) {
   uhci_set_transfer_descriptor(controller_number, device_speed, transfer_descriptor_number, (transfer_descriptor_number+1), usb_control_endpoint_size, toggle, ENDPOINT_0, device_address, UHCI_IN, memory);
   transfer_descriptor_number++;
   toggle = ((toggle+1) & 0x1);
   memory += usb_control_endpoint_size;
   length -= usb_control_endpoint_size;
  }
  else {
   uhci_set_transfer_descriptor(controller_number, device_speed, transfer_descriptor_number, (transfer_descriptor_number+1), length, toggle, ENDPOINT_0, device_address, UHCI_IN, memory);
   transfer_descriptor_number++;
   break;
  }
 }
 uhci_set_transfer_descriptor(controller_number, device_speed, transfer_descriptor_number, 0, 0, TOGGLE_1, ENDPOINT_0, device_address, UHCI_OUT, 0x0);

 //control transfer
 return uhci_control_or_bulk_transfer(controller_number, transfer_descriptor_number, 200);
}

dword_t uhci_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory) {
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

 //clear frame list in case there are pointers to transfer descriptor from previous connected device
 dword_t *frame_list = (dword_t *) (usb_controllers[controller_number].mem1);
 for(dword_t i=device_number; i<1024; i+=interrupt_time) {
  frame_list[i] = UHCI_NO_POINTER;
 }

 //create transfer descriptor
 dword_t transfer_descriptor_number = (device_number+515);
 uhci_set_transfer_descriptor(controller_number, device_speed, transfer_descriptor_number, 0, endpoint_size, TOGGLE_0, endpoint, device_address, UHCI_IN, memory);

 //insert transfer descriptor to frame list
 dword_t transfer_descriptor_offset = (usb_controllers[controller_number].mem3+(transfer_descriptor_number*32));
 for(dword_t i=device_number; i<1024; i+=interrupt_time) {
  frame_list[i] = (transfer_descriptor_offset);
 }

 return (transfer_descriptor_offset+4);
}

byte_t uhci_bulk_transfer(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t transfer_type, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(length>16384) {
  log("\nUHCI: too big bulk out transfer ");
  log_var(length);
  return STATUS_ERROR;
 }
 if(device_speed==USB_LOW_SPEED) {
  log("\nUHCI: bulk transfer to low speed device");
  return STATUS_ERROR;
 }

 dword_t transfer_descriptor_number = 0;
 while(1) {
  if(length>64) {
   uhci_set_transfer_descriptor(controller_number, device_speed, transfer_descriptor_number, (transfer_descriptor_number+1), 64, toggle, endpoint, device_address, transfer_type, memory);
   transfer_descriptor_number++;
   toggle = ((toggle+1) & 0x1);
   memory += 64;
   length -= 64;
  }
  else {
   uhci_set_transfer_descriptor(controller_number, device_speed, transfer_descriptor_number, 0, length, toggle, endpoint, device_address, transfer_type, memory);
   usb_bulk_toggle = ((toggle+1) & 0x1);
   break;
  }
 }
 return uhci_control_or_bulk_transfer(controller_number, transfer_descriptor_number, time_to_wait);
}

byte_t uhci_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(length>16384) {
  while(1) {
   if(length>16384) {
    if(uhci_bulk_transfer(controller_number, device_address, device_speed, UHCI_OUT, endpoint, toggle, memory, 16384, time_to_wait)==STATUS_ERROR) {
     return STATUS_ERROR;
    }
    memory+=16384;
    length-=16384;
    toggle=usb_bulk_toggle;
   }
   else {
    return uhci_bulk_transfer(controller_number, device_address, device_speed, UHCI_OUT, endpoint, toggle, memory, length, time_to_wait);
   }
  }
 }
 else {
  return uhci_bulk_transfer(controller_number, device_address, device_speed, UHCI_OUT, endpoint, toggle, memory, length, time_to_wait);
 }
}

byte_t uhci_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(length>16384) {
  while(1) {
   if(length>16384) {
    if(uhci_bulk_transfer(controller_number, device_address, device_speed, UHCI_IN, endpoint, toggle, memory, 16384, time_to_wait)==STATUS_ERROR) {
     return STATUS_ERROR;
    }
    memory+=16384;
    length-=16384;
    toggle=usb_bulk_toggle;
   }
   else {
    return uhci_bulk_transfer(controller_number, device_address, device_speed, UHCI_IN, endpoint, toggle, memory, length, time_to_wait);
   }
  }
 }
 else {
  return uhci_bulk_transfer(controller_number, device_address, device_speed, UHCI_IN, endpoint, toggle, memory, length, time_to_wait);
 }
}

byte_t uhci_hub_is_there_some_port_connection_status_change(byte_t hub_number) {
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
 for(dword_t i=0; i<usb_hubs[hub_number].number_of_ports; i++) {
  uhci_set_transfer_descriptor(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].device_speed, number_of_packets, (number_of_packets+1), 8, TOGGLE_0, ENDPOINT_0, usb_hubs[hub_number].address, UHCI_SETUP, buffer_mem_pointer);
  buffer_mem_pointer+=8;
  number_of_packets++;
  uhci_set_transfer_descriptor(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].device_speed, number_of_packets, (number_of_packets+1), 4, TOGGLE_1, ENDPOINT_0, usb_hubs[hub_number].address, UHCI_IN, data_mem_pointer);
  data_mem_pointer+=4;
  number_of_packets++;
  uhci_set_transfer_descriptor(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].device_speed, number_of_packets, (number_of_packets+1), 0, TOGGLE_1, ENDPOINT_0, usb_hubs[hub_number].address, UHCI_OUT, 0x0);
  number_of_packets++;
 }
 number_of_packets--;
 uhci_set_transfer_descriptor(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].device_speed, number_of_packets, 0, 0, TOGGLE_1, ENDPOINT_0, usb_hubs[hub_number].address, UHCI_OUT, 0x0);
 
 //transfer
 if(uhci_control_or_bulk_transfer(usb_hubs[hub_number].controller_number, number_of_packets, 20)==STATUS_ERROR) {
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