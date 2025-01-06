//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ohci_controller(byte_t controller_number) {
 //reset
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x01);
 wait(10);
 
 //global reset
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x00);
 wait(50);
 
 //suspended state
 mmio_outd(usb_controllers[controller_number].base+0x04, 0xC0);
 
 //disable interrupts
 mmio_outd(usb_controllers[controller_number].base+0x14, 0x80000000);
 
 //FM interval
 mmio_outd(usb_controllers[controller_number].base+0x34, 0xA7782EDF);
 
 //periodic start
 mmio_outd(usb_controllers[controller_number].base+0x40, 0x00002A2F);
 
 //power all ports
 mmio_outd(usb_controllers[controller_number].base+0x48, ((mmio_ind(usb_controllers[controller_number].base+0x48) & 0xFFFFE000) | 0x200));
 mmio_outd(usb_controllers[controller_number].base+0x4C, 0);
 mmio_outd(usb_controllers[controller_number].base+0x50, 0x10000); //set global power
 
 //read number of ports
 usb_controllers[controller_number].number_of_ports = (mmio_ind(usb_controllers[controller_number].base+0x48) & 0xF);
 
 //initalize HCCA
 usb_controllers[controller_number].mem1 = aligned_malloc(256, 0xFF);
 clear_memory(usb_controllers[controller_number].mem1, 256);
 mmio_outd(usb_controllers[controller_number].base+0x18, usb_controllers[controller_number].mem1);
 
 //current control ED
 mmio_outd(usb_controllers[controller_number].base+0x24, 0);
 
 //current bulk ED
 mmio_outd(usb_controllers[controller_number].base+0x2C, 0);
 
 //pointer to control ED
 usb_controllers[controller_number].mem2 = aligned_malloc(5*16, 0xF);
 mmio_outd(usb_controllers[controller_number].base+0x20, usb_controllers[controller_number].mem2);
 
 //pointer to bulk ED
 mmio_outd(usb_controllers[controller_number].base+0x28, usb_controllers[controller_number].mem2+16);
 
 //start controller
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x84);
 wait(50);
 
 //allocate memory for transfers
 usb_controllers[controller_number].mem3 = aligned_malloc(1030*16, 0xF);
}

byte_t ohci_controller_detect_status_change(byte_t controller_number) {
 for(int i=0, port_offset=0x54; i<usb_controllers[controller_number].number_of_ports; i++, port_offset+=0x4) {
  //is status change bit set?
  if((mmio_ind(usb_controllers[controller_number].base+port_offset) & 0x10000)==0x10000) {
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE;
}

byte_t ohci_get_port_connection_status(byte_t controller_number, byte_t port_number) {
 return (mmio_ind(usb_controllers[controller_number].base+0x54+(port_number*4)) & 0x1);
}

byte_t ohci_get_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 return ((mmio_ind(usb_controllers[controller_number].base+0x54+(port_number*4))>>16) & 0x1);
}

void ohci_clear_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 mmio_outd((usb_controllers[controller_number].base+0x54+(port_number*4)), 0x001F0000);
}

byte_t ohci_enable_device_on_port(byte_t controller_number, byte_t port_number) {
 dword_t port_offset = (usb_controllers[controller_number].base+0x54+(port_number*4));

 //reset device
 mmio_outd(port_offset, 0x10);
 wait(50);
 
 //enable device
 mmio_outd(port_offset, 0x2);
 wait(30);
 if((mmio_ind(port_offset) & 0x2)==0x0) {
  log("\nOHCI: device was not enabled");
  return STATUS_ERROR; //device was not enabled
 }
 
 //find if this is low speed or full speed device
 if((mmio_ind(port_offset) & 0x200)==0x200) {
  usb_controllers[controller_number].ports_device_speed[port_number]=USB_LOW_SPEED;
 }
 else {
  usb_controllers[controller_number].ports_device_speed[port_number]=USB_FULL_SPEED;
 }
 
 //clear bits
 mmio_outd(port_offset, 0x001F0000);

 return STATUS_GOOD;
}

void ohci_set_ed(byte_t controller_number, byte_t device_speed, dword_t ed_number, dword_t address, dword_t endpoint, dword_t first_td, dword_t num_of_td) {
 dword_t *ed = (dword_t *) (usb_controllers[controller_number].mem2+ed_number*16);
 dword_t attributes = ((address) | (endpoint<<7));
 first_td = (usb_controllers[controller_number].mem3+first_td*16);
 
 if(device_speed==USB_LOW_SPEED) {
  attributes |= ((8<<16) | (1 << 13));
 }
 else {
  attributes |= (64<<16);
 }
 ed[0]=attributes;
 ed[1]=(first_td+(num_of_td*0x10));
 ed[2]=first_td;
 ed[3]=0;
}

void ohci_set_td(byte_t controller_number, dword_t td_number, dword_t next_td, dword_t attributes, dword_t toggle, dword_t buffer, dword_t buffer_length) {
 dword_t *td = (dword_t *) (usb_controllers[controller_number].mem3+td_number*16);
 
 td[0]=(attributes | 0xE0000000 | (1<<25) | (toggle<<24) | (1<<18));
 if(next_td==0) {
  td[2]=0;
 }
 else {
  td[2]=(usb_controllers[controller_number].mem3+next_td*16);
 }
 
 if(buffer_length>0) {
  td[1]=buffer;
  td[3]=(buffer+buffer_length-1);
 }
 else {
  td[1]=0;
  td[3]=0;
 }
}

byte_t ohci_control_transfer(byte_t controller_number, dword_t last_td, dword_t time) {
 dword_t *mem = (dword_t *) (usb_controllers[controller_number].mem3+last_td*16);

 //current ED
 mmio_outd(usb_controllers[controller_number].base+0x24, 0);
 
 //actual ED
 mmio_outd(usb_controllers[controller_number].base+0x20, usb_controllers[controller_number].mem2);
 
 //run transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x94);
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x02);
 
 ticks=0;
 while(ticks<time) {
  asm("nop");
  if((mem[0] & 0xE0000000)!=0xE0000000) { //active bits
   //stop transfer
   mmio_outd(usb_controllers[controller_number].base+0x04, 0x84);
   mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
  
   if((mem[0] & 0xF0000000)==0x00000000 || (mem[0] & 0xF0000000)==0x90000000) { //error bits
    return STATUS_GOOD;
   }
   else {
    log("\nOHCI bad transfer ");
    log_hex_with_space(mem[0]);
    return STATUS_ERROR;
   }
  }
 }
 
 //stop transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x84);
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
 
 log("\nOHCI control transfer timeout ");
 mem = (dword_t *) (usb_controllers[controller_number].mem3);
 for(dword_t i=0; i<(last_td+1)*4; i++) {
  log("\n");
  log_hex(*mem);
  mem++;
 }
 log("\n");
 return STATUS_ERROR;
}

byte_t ohci_bulk_transfer(byte_t controller_number, dword_t last_td, dword_t time) {
 dword_t *mem = (dword_t *) (usb_controllers[controller_number].mem3+last_td*16);

 //current ED
 mmio_outd(usb_controllers[controller_number].base+0x2C, 0);
 
 //actual ED
 mmio_outd(usb_controllers[controller_number].base+0x28, usb_controllers[controller_number].mem2);
 
 //run transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, 0xA4);
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x04);
 
 ticks=0;
 while(ticks<time) {
  asm("nop");
  if((mem[0] & 0xE0000000)!=0xE0000000) { //active bits
   //stop transfer
   mmio_outd(usb_controllers[controller_number].base+0x04, 0x84);
   mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
  
   if((mem[0] & 0xF0000000)==0x00000000) { //error bits
    return STATUS_GOOD;
   }
   else {
    log("\nOHCI bad transfer ");
    log_hex_with_space(mem[0]);
    return STATUS_ERROR;
   }
  }
 }
 
 //stop transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x84);
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
 
 log("\nOHCI bulk transfer timeout ");
 return STATUS_ERROR;
}

byte_t ohci_control_transfer_without_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed) {
 ohci_set_ed(controller_number, device_speed, 0, device_address, ENDPOINT_0, 0, 2);
 ohci_set_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, (dword_t)usb_setup_packet, 8);
 ohci_set_td(controller_number, 1, 0, OHCI_IN, TOGGLE_1, 0, 0);
 return ohci_control_transfer(controller_number, 1, 200);
}

byte_t ohci_control_transfer_with_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed, word_t length) {
 if(length>16384) {
  log("\nOHCI: too big control transfer ");
  log_var(length);
  return STATUS_ERROR;
 }
 
 //create transfer descriptors
 ohci_set_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, (dword_t)usb_setup_packet, 8);
 dword_t transfer_descriptor_number = 1, toggle = TOGGLE_1, memory = (dword_t)usb_setup_packet_data;
 while(1) {
  if(length>usb_control_endpoint_size) {
   ohci_set_td(controller_number, transfer_descriptor_number, (transfer_descriptor_number+1), OHCI_IN, toggle, memory, usb_control_endpoint_size);
   transfer_descriptor_number++;
   toggle = ((toggle+1) & 0x1);
   memory += usb_control_endpoint_size;
   length -= usb_control_endpoint_size;
  }
  else {
   ohci_set_td(controller_number, transfer_descriptor_number, (transfer_descriptor_number+1), OHCI_IN, toggle, memory, length);
   transfer_descriptor_number++;
   break;
  }
 }
 ohci_set_td(controller_number, transfer_descriptor_number, 0, OHCI_OUT, TOGGLE_1, 0, 0);
 ohci_set_ed(controller_number, device_speed, 0, device_address, ENDPOINT_0, 0, (transfer_descriptor_number+1));

 //control transfer
 return ohci_control_transfer(controller_number, transfer_descriptor_number, 200);
}

dword_t ohci_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory) {
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

 //clear HCCA in case there are pointers from previous connected device
 dword_t *hcca = (dword_t *) (usb_controllers[controller_number].mem1);
 for(int i=device_number; i<32; i+=interrupt_time) {
  hcca[i]=0x00000000;
 }

 //create transfer descriptor
 dword_t transfer_descriptor_number = (device_number+1025);
 device_number++;
 ohci_set_ed(controller_number, device_speed, device_number, device_address, endpoint, transfer_descriptor_number, 1);
 ohci_set_td(controller_number, transfer_descriptor_number, 0, OHCI_IN, TOGGLE_0, memory, endpoint_size);

 //insert transfer to HCCA
 for(int i=(device_number-1); i<32; i+=interrupt_time) {
  hcca[i]=(usb_controllers[controller_number].mem2+(device_number*16));
 }

 return (usb_controllers[controller_number].mem2+(device_number*16));
}

byte_t ohci_send_bulk_transfer(byte_t controller_number, byte_t device_address, byte_t device_speed, dword_t transfer_type, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(length>65536) {
  log("\nOHCI: too big bulk out transfer ");
  log_var(length);
  return STATUS_ERROR;
 }
 if(device_speed==USB_LOW_SPEED) {
  log("\nOHCI: bulk transfer to low speed device");
  return STATUS_ERROR;
 }

 dword_t transfer_descriptor_number = 0;
 while(1) {
  if(length>64) {
   ohci_set_td(controller_number, transfer_descriptor_number, (transfer_descriptor_number+1), transfer_type, toggle, memory, 64);
   transfer_descriptor_number++;
   toggle = ((toggle+1) & 0x1);
   memory += 64;
   length -= 64;
  }
  else {
   ohci_set_td(controller_number, transfer_descriptor_number, 0, transfer_type, toggle, memory, length);
   usb_bulk_toggle = ((toggle+1) & 0x1);
   ohci_set_ed(controller_number, device_speed, 0, device_address, endpoint, 0, (transfer_descriptor_number+1));
   break;
  }
 }
 return ohci_bulk_transfer(controller_number, transfer_descriptor_number, time_to_wait);
}

byte_t ohci_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(length>65536) {
  while(1) {
   if(length>65536) {
    if(ohci_send_bulk_transfer(controller_number, device_address, device_speed, OHCI_OUT, endpoint, toggle, memory, 65536, time_to_wait)==STATUS_ERROR) {
     return STATUS_ERROR;
    }
    memory+=65536;
    length-=65536;
    toggle=usb_bulk_toggle;
   }
   else {
    return ohci_send_bulk_transfer(controller_number, device_address, device_speed, OHCI_OUT, endpoint, toggle, memory, length, time_to_wait);
   }
  }
 }
 else {
  return ohci_send_bulk_transfer(controller_number, device_address, device_speed, OHCI_OUT, endpoint, toggle, memory, length, time_to_wait);
 }
}

byte_t ohci_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 if(length>65536) {
  while(1) {
   if(length>65536) {
    if(ohci_send_bulk_transfer(controller_number, device_address, device_speed, OHCI_IN, endpoint, toggle, memory, 65536, time_to_wait)==STATUS_ERROR) {
     return STATUS_ERROR;
    }
    memory+=65536;
    length-=65536;
    toggle=usb_bulk_toggle;
   }
   else {
    return ohci_send_bulk_transfer(controller_number, device_address, device_speed, OHCI_IN, endpoint, toggle, memory, length, time_to_wait);
   }
  }
 }
 else {
  return ohci_send_bulk_transfer(controller_number, device_address, device_speed, OHCI_IN, endpoint, toggle, memory, length, time_to_wait);
 }
}

byte_t ohci_hub_is_there_some_port_connection_status_change(byte_t hub_number) {
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
  ohci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, (number_of_packets+1), OHCI_SETUP, TOGGLE_0, buffer_mem_pointer, 8);
  buffer_mem_pointer+=8;
  number_of_packets++;
  ohci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, (number_of_packets+1), OHCI_IN, TOGGLE_1, data_mem_pointer, 4);
  data_mem_pointer+=4;
  number_of_packets++;
  ohci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, (number_of_packets+1), OHCI_OUT, TOGGLE_1, 0x0, 0);
  number_of_packets++;
 }
 number_of_packets--;
 ohci_set_td(usb_hubs[hub_number].controller_number, number_of_packets, 0, OHCI_OUT, TOGGLE_1, 0x0, 0);
 
 //transfer
 ohci_set_ed(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].device_speed, 0, usb_hubs[hub_number].address, ENDPOINT_0, 0, (number_of_packets+1));
 if(ohci_control_transfer(usb_hubs[hub_number].controller_number, number_of_packets, 20)==STATUS_ERROR) {
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