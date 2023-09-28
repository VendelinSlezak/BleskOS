//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
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
 dword_t *hcca = (dword_t *) usb_controllers[controller_number].mem1;
 for(int i=0; i<64; i++) {
  hcca[i]=0;
 }
 mmio_outd(usb_controllers[controller_number].base+0x18, usb_controllers[controller_number].mem1);
 
 //current control ED
 mmio_outd(usb_controllers[controller_number].base+0x24, 0);
 
 //current bulk ED
 mmio_outd(usb_controllers[controller_number].base+0x2C, 0);
 
 //pointer to control ED
 usb_controllers[controller_number].mem2 = aligned_malloc(256, 0xF);
 mmio_outd(usb_controllers[controller_number].base+0x20, usb_controllers[controller_number].mem2);
 
 //pointer to bulk ED
 mmio_outd(usb_controllers[controller_number].base+0x28, usb_controllers[controller_number].mem2+16);
 
 //start controller
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x80);
 wait(50);
 
 //allocate memory for transfers
 usb_controllers[controller_number].mem3 = aligned_malloc(600*16, 0xF);
 usb_controllers[controller_number].setup_mem = malloc(16);
}

byte_t ohci_controller_detect_status_change(byte_t controller_number) {
 for(int i=0, port_number=0x54; i<usb_controllers[controller_number].number_of_ports; i++, port_number+=0x4) {
  //is status change bit set?
  if((mmio_ind(usb_controllers[controller_number].base+port_number) & 0x10000)==0x10000) {
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE;
}

void ohci_controller_detect_devices(byte_t controller_number) {
 dword_t port_status=0;

 for(int i=0, port_number=0x54; i<usb_controllers[controller_number].number_of_ports; i++, port_number+=0x4) {
  port_status = mmio_ind(usb_controllers[controller_number].base+port_number);
  
  //is some device connected?
  if((port_status & 0x1)==0x1) {
   if(usb_controllers[controller_number].ports_state[i]==PORT_INITALIZED_DEVICE && ((port_status & 0x10000)==0x0)) {
    continue; //we already initalized this device
   }
   usb_controllers[controller_number].ports_state[i]=PORT_DEVICE;
   
   ohci_remove_device_if_connected(controller_number, i);
   
   //reset device
   mmio_outd(usb_controllers[controller_number].base+port_number, 0x10);//actual ED
   wait(50);
   
   //enable device
   mmio_outd(usb_controllers[controller_number].base+port_number, 0x2);
   wait(30);
   
   //find if this is low speed or full speed device
   if((mmio_ind(usb_controllers[controller_number].base+port_number) & 0x200)==0x200) {
    usb_controllers[controller_number].ports_device_speed[i]=USB_LOW_SPEED;
   }
   else {
    usb_controllers[controller_number].ports_device_speed[i]=USB_FULL_SPEED;
   }
   
   //clear bits
   mmio_outd(usb_controllers[controller_number].base+port_number, 0x001F0000);
   
   //get descriptor
   if(usb_controllers[controller_number].ports_device_speed[i]==USB_LOW_SPEED) {
    usb_control_endpoint_length = 8;
    usb_device_type = ohci_read_descriptor(controller_number, i, 8, 8);
   }
   else if(usb_controllers[controller_number].ports_device_speed[i]==USB_FULL_SPEED) {
    usb_control_endpoint_length = 64;
    usb_device_type = ohci_read_descriptor(controller_number, i, 18, 64);
   }
   
   //return if there was reading error
   if(usb_device_type==0xFFFFFFFF) {
    log("\nerror during reading USB descriptor");
    return;
   }

   //get full descriptor from low speed devices
   if(usb_controllers[controller_number].ports_device_speed[i]==USB_LOW_SPEED) {
    ohci_read_descriptor(controller_number, i, 18, usb_control_endpoint_length);
   }
   
   //set address of device
   ohci_set_address(controller_number, i);
   
   //read descriptor
   ohci_read_configuration_descriptor(controller_number, i, usb_control_endpoint_length);
   
   //initalizing succesfull
   usb_controllers[controller_number].ports_state[i]=PORT_INITALIZED_DEVICE;
  }
  else {
   if(usb_controllers[controller_number].ports_state[i]!=PORT_NO_DEVICE) {
    ohci_remove_device_if_connected(controller_number, i);
   }
   usb_controllers[controller_number].ports_state[i]=PORT_NO_DEVICE;
   
   //remove if there was connected device
   if(usb_controllers[controller_number].ports_state[i]!=PORT_NO_DEVICE) {
    ohci_remove_device_if_connected(controller_number, i);
   }
   
   //clear status change
   mmio_outd(usb_controllers[controller_number].base+port_number, 0x001F0000);
   
   //save port status change
   usb_controllers[controller_number].ports_state[i]=PORT_NO_DEVICE;
  }
 }
}

void ohci_remove_device_if_connected(byte_t controller_number, byte_t port) {
 dword_t *hcca = (dword_t *) usb_controllers[controller_number].mem1;

 if(usb_mouse_state==USB_MOUSE_OHCI_PORT && usb_mouse_controller==controller_number && usb_mouse_port==port) {
  for(int i=1; i<32; i+=4) {
   hcca[i]=0;
  }
  usb_mouse_state=0;
  usb_mouse_controller=0;
  usb_mouse_port=0;
  usb_mouse_endpoint=0;
 }
 if(usb_keyboard_state==USB_KEYBOARD_OHCI_PORT && usb_keyboard_controller==controller_number && usb_keyboard_port==port) {
  for(int i=2; i<32; i+=4) {
   hcca[i]=0;
  }
  usb_keyboard_state=0;
  usb_keyboard_controller=0;
  usb_keyboard_port=0;
  usb_keyboard_endpoint=0;
 }
 for(int i=0; i<10; i++) {
  if(usb_mass_storage_devices[i].controller_type==USB_CONTROLLER_OHCI && usb_mass_storage_devices[i].controller_number==controller_number && usb_mass_storage_devices[i].port==port) {
   usb_mass_storage_devices[i].type=USB_MSD_NOT_ATTACHED;
   usb_mass_storage_devices[i].controller_number=0;
   usb_mass_storage_devices[i].controller_type=0;
   usb_mass_storage_devices[i].port=0;
   usb_mass_storage_devices[i].endpoint_in=0;
   usb_mass_storage_devices[i].toggle_in=0;
   usb_mass_storage_devices[i].endpoint_out=0;
   usb_mass_storage_devices[i].toggle_out=0;
   usb_mass_storage_devices[i].size_in_sectors=0;
   usb_mass_storage_devices[i].size_of_sector=0;
   for(int j=0; j<4; j++) {
    usb_mass_storage_devices[i].partitions_type[j]=0;
    usb_mass_storage_devices[i].partitions_first_sector[j]=0;
    usb_mass_storage_devices[i].partitions_num_of_sectors[j]=0;
   }
   return;
  }
 }
}

byte_t ohci_control_transfer(byte_t controller_number, dword_t last_td) {
 dword_t *mem = (dword_t *) (usb_controllers[controller_number].mem3+last_td*16);

 //current ED
 mmio_outd(usb_controllers[controller_number].base+0x24, 0);
 
 //actual ED
 mmio_outd(usb_controllers[controller_number].base+0x20, usb_controllers[controller_number].mem2);
 
 //run transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, (0x90 | (mmio_ind(usb_controllers[controller_number].base+0x04) & 0x04)));
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x02);
 
 ticks=0;
 while(ticks<500) {
  asm("hlt");
  
  if((mem[0] & 0xE0000000)!=0xE0000000) { //active bits
   //stop transfer
   mmio_outd(usb_controllers[controller_number].base+0x04, (0x80 | (mmio_ind(usb_controllers[controller_number].base+0x04) & 0x04)));
   mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
  
   if((mem[0] & 0xF0000000)==0x00000000 || (mem[0] & 0xF0000000)==0x90000000) { //error bits
    return STATUS_GOOD;
   }
   else {
    log("\nOHCI bad transfer ");
    log_hex(mem[0]);
    return STATUS_ERROR;
   }
  }
 }
 
 //stop transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, (0x80 | (mmio_ind(usb_controllers[controller_number].base+0x04) & 0x04)));
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
 
 log("\nOHCI transfer timeout ");
 return STATUS_ERROR;
}

byte_t ohci_bulk_transfer(byte_t controller_number, dword_t last_td, dword_t time) {
 dword_t *mem = (dword_t *) (usb_controllers[controller_number].mem3+last_td*16);

 //current ED
 mmio_outd(usb_controllers[controller_number].base+0x2C, 0);
 
 //actual ED
 mmio_outd(usb_controllers[controller_number].base+0x28, usb_controllers[controller_number].mem2);
 
 //run transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, (0xA0 | (mmio_ind(usb_controllers[controller_number].base+0x04) & 0x04)));
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x04);
 
 ticks=0;
 while(ticks<time) {
  asm("hlt");
  
  if((mem[0] & 0xE0000000)!=0xE0000000) { //active bits
   //stop transfer
   mmio_outd(usb_controllers[controller_number].base+0x04, (0x80 | (mmio_ind(usb_controllers[controller_number].base+0x04) & 0x04)));
   mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
  
   if((mem[0] & 0xF0000000)==0x00000000) { //error bits
    return STATUS_GOOD;
   }
   else {
    log("\nOHCI bad transfer ");
    log_hex(mem[0]);
    return STATUS_ERROR;
   }
  }
 }
 
 //stop transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, (0x80 | (mmio_ind(usb_controllers[controller_number].base+0x04) & 0x04)));
 mmio_outd(usb_controllers[controller_number].base+0x08, 0x00);
 
 log("\nOHCI transfer timeout ");
 return STATUS_ERROR;
}

void ohci_create_ed(byte_t controller_number, byte_t port, dword_t ed_number, dword_t address, dword_t endpoint, dword_t first_td, dword_t num_of_td) {
 dword_t *ed = (dword_t *) (usb_controllers[controller_number].mem2+ed_number*16);
 dword_t attributes = ((address) | (endpoint<<7));
 first_td = (usb_controllers[controller_number].mem3+first_td*16);
 
 if(usb_controllers[controller_number].ports_device_speed[port]==USB_LOW_SPEED) {
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

void ohci_create_td(byte_t controller_number, dword_t td_number, dword_t next_td, dword_t attributes, dword_t toggle, dword_t buffer, dword_t buffer_length) {
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

void ohci_transfer_set_setup(byte_t controller_number, byte_t port) {
 ohci_create_ed(controller_number, port, 0, (port+1), ENDPOINT_0, 0, 2);
 ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
 ohci_create_td(controller_number, 1, 0, OHCI_IN, TOGGLE_1, 0, 0);
 ohci_control_transfer(controller_number, 1); //TODO: error transaction
}

void ohci_set_address(byte_t controller_number, byte_t port) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set address SETUP packet
 buffer[0]=(0x00000500 | ((port+1)<<16)); //address will be always port number
 buffer[1]=0x00000000;
 
 ohci_create_ed(controller_number, port, 0, 0, ENDPOINT_0, 0, 2);
 ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
 ohci_create_td(controller_number, 1, 0, OHCI_IN, TOGGLE_1, 0, 0);
 ohci_control_transfer(controller_number, 1); //TODO: error transaction
}

void ohci_set_configuration(byte_t controller_number, byte_t port, dword_t configuration_num) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set configuration SETUP packet
 buffer[0]=(0x00000900 | (configuration_num<<16));
 buffer[1]=0x00000000;

 ohci_transfer_set_setup(controller_number, port);
}

void ohci_set_interface(byte_t controller_number, byte_t port, dword_t interface_num, dword_t alt_interface_num) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set interface SETUP packet
 buffer[0]=(0x00000B01 | (alt_interface_num<<16));
 buffer[1]=interface_num;

 ohci_transfer_set_setup(controller_number, port);
}

void ohci_set_idle(byte_t controller_number, byte_t port, dword_t interface_num, byte_t idle_value) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set idle SETUP packet
 buffer[0]=(0x00000A21 | (idle_value<<24));
 buffer[1]=interface_num;

 ohci_transfer_set_setup(controller_number, port);
}

void ohci_set_protocol(byte_t controller_number, byte_t port, dword_t interface_num, dword_t protocol_type) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set protocol SETUP packet
 buffer[0]=(0x00000B21 | (protocol_type<<16));
 buffer[1]=interface_num;

 ohci_transfer_set_setup(controller_number, port);
}

void ohci_set_keyboard_led(byte_t controller_number, byte_t port, dword_t interface_num, byte_t endpoint, byte_t led_state) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set report SETUP packet
 buffer[0]=0x02000921;
 buffer[1]=(0x00010000 | interface_num);
 
 //set data
 buffer[2]=led_state;
 
 //transfer
 ohci_create_ed(controller_number, port, 0, 0, ENDPOINT_0, 0, 3);
 ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
 ohci_create_td(controller_number, 1, 2, OHCI_OUT, TOGGLE_1, usb_controllers[controller_number].setup_mem+8, 1);
 ohci_create_td(controller_number, 2, 0, OHCI_IN, TOGGLE_1, 0, 0);
 ohci_control_transfer(controller_number, 2); //TODO: error transaction
}

void ohci_set_interrupt_transfer(byte_t controller_number, byte_t port, byte_t ed_offset, byte_t td_offset, byte_t endpoint, byte_t transfer_length, byte_t transfer_time, dword_t memory) {
 dword_t *hcca = (dword_t *) usb_controllers[controller_number].mem1;
 
 if(transfer_time<8) {
  transfer_time = 4;
 }
 else if(transfer_time<16) {
  transfer_time = 8;
 }
 else if(transfer_time<32) {
  transfer_time = 16;
 }
 else {
  transfer_time = 32;
 }
 
 //stop interrupt transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, (mmio_ind(usb_controllers[controller_number].base+0x04) & 0xFFFFFFFB));
 
 ohci_create_ed(controller_number, port, ed_offset, (port+1), endpoint, td_offset, 1);
 ohci_create_td(controller_number, td_offset, 0, OHCI_IN, TOGGLE_0, memory, transfer_length);
 
 //set pointers
 for(int i=ed_offset; i<32; i+=transfer_time) {
  hcca[i]=(usb_controllers[controller_number].mem2+ed_offset*16);
 }
 
 //run interrupt transfer
 mmio_outd(usb_controllers[controller_number].base+0x04, 0x84);
}

void ohci_send_setup_to_device(byte_t controller_number, byte_t port, word_t max_length, dword_t memory, byte_t control_endpoint_length) {
 if(control_endpoint_length==8) {
  ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
  for(int i=1, toggle=1; i<128; i++, toggle=((toggle+=1) & 0x1)) {
   if(max_length>8) {
    ohci_create_td(controller_number, i, (i+1), OHCI_IN, toggle, memory, 8);
    memory+=8;
    max_length-=8;
   }
   else {
    ohci_create_td(controller_number, i, (i+1), OHCI_IN, toggle, memory, max_length);
    ohci_create_td(controller_number, (i+1), 0, OHCI_OUT, TOGGLE_1, 0, 0);
    ohci_create_ed(controller_number, port, 0, (port+1), ENDPOINT_0, 0, (i+2));
    ohci_control_transfer(controller_number, (i+1));     
    break;
   }
  }
 }
 else if(control_endpoint_length==64) {
  ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
  for(int i=1, toggle=1; i<128; i++, toggle=((toggle+=1) & 0x1)) {
   if(max_length>64) {
    ohci_create_td(controller_number, i, (i+1), OHCI_IN, toggle, memory, 64);
    memory+=64;
    max_length-=64;
   }
   else {
    ohci_create_td(controller_number, i, (i+1), OHCI_IN, toggle, memory, max_length);
    ohci_create_td(controller_number, (i+1), 0, OHCI_OUT, TOGGLE_1, 0, 0);
    ohci_create_ed(controller_number, port, 0, (port+1), ENDPOINT_0, 0, (i+2));
    ohci_control_transfer(controller_number, (i+1));     
    break;
   }
  }
 }
}

dword_t ohci_read_descriptor(byte_t controller_number, byte_t port, byte_t type_of_transfer, byte_t control_endpoint_length) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc(256);
 dword_t *data = (dword_t *) data_mem;
 byte_t status = 0;

 //set read base descriptor SETUP packet
 buffer[0]=0x01000680;
 if(type_of_transfer==8) {
  buffer[1]=0x00080000;
  
  ohci_create_ed(controller_number, port, 0, 0, ENDPOINT_0, 0, 3);
  ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
  ohci_create_td(controller_number, 1, 2, OHCI_IN, TOGGLE_1, data_mem, 8);
  ohci_create_td(controller_number, 2, 0, OHCI_OUT, TOGGLE_1, 0, 0);
  status = ohci_control_transfer(controller_number, 2);
 }
 else if(type_of_transfer==18) {
  buffer[1]=0x00120000;
  
  if(control_endpoint_length==8) {
   ohci_create_ed(controller_number, port, 0, 0, ENDPOINT_0, 0, 5);
   ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
   ohci_create_td(controller_number, 1, 2, OHCI_IN, TOGGLE_1, data_mem, 8);
   ohci_create_td(controller_number, 2, 3, OHCI_IN, TOGGLE_0, data_mem+8, 8);
   ohci_create_td(controller_number, 3, 4, OHCI_IN, TOGGLE_1, data_mem+8+8, 2);
   ohci_create_td(controller_number, 4, 0, OHCI_OUT, TOGGLE_1, 0, 0);
   status = ohci_control_transfer(controller_number, 4);
  }
  else if(control_endpoint_length==64) {
   ohci_create_ed(controller_number, port, 0, 0, ENDPOINT_0, 0, 3);
   ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
   ohci_create_td(controller_number, 1, 2, OHCI_IN, TOGGLE_1, data_mem, 18);
   ohci_create_td(controller_number, 2, 0, OHCI_OUT, TOGGLE_1, 0, 0);
   status = ohci_control_transfer(controller_number, 2);
  }
 }

 if(status==STATUS_ERROR) {
  return 0xFFFFFFFF;
 }
 
 //parse data
 if(type_of_transfer==18) {
  parse_usb_descriptor(data_mem); 
 }
 
 free(data_mem);
 
 return (data[1] & 0x00FFFFFF);
}

void ohci_read_configuration_descriptor(byte_t controller_number, byte_t port, byte_t control_endpoint_length) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = malloc(8);
 dword_t *data = (dword_t *) data_mem;
 dword_t data_length = 0;

 //read length of CONFIGURATION DESCRIPTOR
 buffer[0]=0x02000680; 
 buffer[1]=0x00080000;
 
 ohci_create_ed(controller_number, port, 0, (port+1), ENDPOINT_0, 0, 3);
 ohci_create_td(controller_number, 0, 1, OHCI_SETUP, TOGGLE_0, usb_controllers[controller_number].setup_mem, 8);
 ohci_create_td(controller_number, 1, 2, OHCI_IN, TOGGLE_1, data_mem, 8);
 ohci_create_td(controller_number, 2, 0, OHCI_OUT, TOGGLE_1, 0, 0);
 ohci_control_transfer(controller_number, 2);
  
 data_length = (data[0]>>16);
 if(data_length==0) {
  log("\nerror size of configuration descriptor");
  return;
 }
  
 //read CONFIGURATION DESCRIPTOR
 buffer[0]=0x02000680;
 buffer[1]=(0x00000000 | (data_length<<16));
 data_mem = realloc(data_mem, data_length);
 
 ohci_send_setup_to_device(controller_number, port, data_length, data_mem, control_endpoint_length);
 
 parse_usb_configuration_descriptor(data_mem);
 
 //TODO: now when there are USB mouse and USB keyboard on one device, prefer USB mouse
 if(usb_descriptor_devices[0].type!=USB_DEVICE_MOUSE) {
  if(usb_descriptor_devices[1].type==USB_DEVICE_MOUSE) {
   usb_descriptor_devices[0].type=usb_descriptor_devices[1].type;
   usb_descriptor_devices[0].configuration=usb_descriptor_devices[1].configuration;
   usb_descriptor_devices[0].interface=usb_descriptor_devices[1].interface;
   usb_descriptor_devices[0].alternative_interface=usb_descriptor_devices[1].alternative_interface;
   usb_descriptor_devices[0].endpoint_interrupt=usb_descriptor_devices[1].endpoint_interrupt;
   usb_descriptor_devices[0].endpoint_interrupt_length=usb_descriptor_devices[1].endpoint_interrupt_length;
   usb_descriptor_devices[0].endpoint_interrupt_time=usb_descriptor_devices[1].endpoint_interrupt_time;
   usb_descriptor_devices[0].endpoint_interrupt_out=usb_descriptor_devices[1].endpoint_interrupt_out;
  }
 }
 
 if(usb_descriptor_devices[0].type==USB_DEVICE_KEYBOARD) {
  log("OHCI: USB keyboard ");
  
  if(usb_keyboard_state!=0) {
   free(data_mem);
   log("duplicated");
   return;
  }
  
  ohci_set_configuration(controller_number, port, usb_descriptor_devices[0].configuration);
  ohci_set_interface(controller_number, port, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  ohci_set_protocol(controller_number, port, usb_descriptor_devices[0].interface, USB_BOOT_PROTOCOL);
  ohci_set_idle(controller_number, port, usb_descriptor_devices[0].interface, 0);
  
  usb_keyboard_controller = controller_number;
  log_var(usb_keyboard_controller);
  usb_keyboard_port = port;
  log_var(usb_keyboard_port);
  usb_keyboard_interface = usb_descriptor_devices[0].interface;
  usb_keyboard_endpoint = usb_descriptor_devices[0].endpoint_interrupt;
  log_var(usb_keyboard_endpoint);
  usb_keyboard_endpoint_out = usb_descriptor_devices[0].endpoint_interrupt_out; //if INTERRUPT OUT is not present, this will be zero - CONTROL endpoint
  log_var(usb_keyboard_endpoint_out);
  usb_keyboard_endpoint_length = usb_descriptor_devices[0].endpoint_interrupt_length;
  usb_keyboard_endpoint_time = usb_descriptor_devices[0].endpoint_interrupt_time;
  log("\n");
  
  ohci_set_interrupt_transfer(usb_keyboard_controller, usb_keyboard_port, 2, 35, usb_keyboard_endpoint, usb_descriptor_devices[0].endpoint_interrupt_length, usb_descriptor_devices[0].endpoint_interrupt_time, usb_keyboard_data_memory);
  usb_keyboard_check_transfer_descriptor = usb_controllers[controller_number].mem3+16*35;
  
  usb_keyboard_state = USB_KEYBOARD_OHCI_PORT;
 }
 else if(usb_descriptor_devices[0].type==USB_DEVICE_MOUSE) {
  log("OHCI: USB mouse ");
  
  if(usb_mouse_state!=0) {
   free(data_mem);
   log("duplicated");
   return;
  }
  
  ohci_set_configuration(controller_number, port, usb_descriptor_devices[0].configuration);
  ohci_set_interface(controller_number, port, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  ohci_set_idle(controller_number, port, usb_descriptor_devices[0].interface, 0);
  
  usb_mouse_controller = controller_number;
  log_var(usb_mouse_controller);
  usb_mouse_port = port;
  log_var(usb_mouse_port);
  usb_mouse_endpoint = usb_descriptor_devices[0].endpoint_interrupt;
  log_var(usb_mouse_endpoint);
  usb_mouse_endpoint_length = usb_descriptor_devices[0].endpoint_interrupt_length;
  usb_mouse_endpoint_time = usb_descriptor_devices[0].endpoint_interrupt_time;
  log("\n");
  
  ohci_read_hid_descriptor(controller_number, port, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].hid_descriptor_length, control_endpoint_length);
  ohci_set_protocol(controller_number, port, usb_descriptor_devices[0].interface, USB_HID_PROTOCOL);

  ohci_set_interrupt_transfer(usb_mouse_controller, usb_mouse_port, 1, 34, usb_mouse_endpoint, usb_descriptor_devices[0].endpoint_interrupt_length, usb_descriptor_devices[0].endpoint_interrupt_time, usb_mouse_data_memory);
  usb_mouse_check_transfer_descriptor = usb_controllers[controller_number].mem3+16*34;
  
  usb_mouse_state = USB_MOUSE_OHCI_PORT;
 }
 else if(usb_descriptor_devices[0].type==USB_DEVICE_MASS_STORAGE) {
  log("OHCI: USB mass storage device ");
  
  ohci_set_configuration(controller_number, port, usb_descriptor_devices[0].configuration);
  ohci_set_interface(controller_number, port, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  
  for(int i=0; i<10; i++) {
   if(usb_mass_storage_devices[i].type==USB_MSD_NOT_ATTACHED) {
    usb_mass_storage_devices[i].type = USB_MSD_ATTACHED;
    usb_mass_storage_devices[i].controller_type = USB_CONTROLLER_OHCI;
    usb_mass_storage_devices[i].controller_number = controller_number;
    usb_mass_storage_devices[i].port = port;
    usb_mass_storage_devices[i].endpoint_in = usb_descriptor_devices[0].endpoint_bulk_in;
    log_var(usb_mass_storage_devices[i].endpoint_in);
    usb_mass_storage_devices[i].toggle_in = 0;
    usb_mass_storage_devices[i].endpoint_out = usb_descriptor_devices[0].endpoint_bulk_out;
    log_var(usb_mass_storage_devices[i].endpoint_out);
    usb_mass_storage_devices[i].toggle_out = 0;
    
    usb_mass_storage_initalize(i);
    
    free(data_mem);
    return;
   }
  }
  
  log("more than 10 devices connected\n");
 }

 free(data_mem);
}

void ohci_read_hid_descriptor(byte_t controller_number, byte_t port, byte_t interface, word_t length, byte_t control_endpoint_length) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc((length+1));
 dword_t *data = (dword_t *) data_mem;

 //set read HID DESCRIPTOR SETUP packet
 buffer[0]=0x22000681;
 buffer[1]=((length<<16) | interface);
 
 ohci_send_setup_to_device(controller_number, port, length, data_mem, control_endpoint_length);

 parse_hid_descriptor(data_mem);
 
 free(data_mem);
}

byte_t ohci_bulk_out(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 byte_t status;

 for(int i=40; i<600; i++, ((toggle++) & 0x1)) {
  if(length>64) {
   ohci_create_td(controller_number, i, (i+1), OHCI_OUT, toggle, memory, 64);
   length -= 64;
   memory += 64;
  }
  else {
   ohci_create_td(controller_number, i, 0, OHCI_OUT, toggle, memory, length);
   ohci_toggle = ((toggle+=1) & 0x1);
   ohci_create_ed(controller_number, port, 0, (port+1), endpoint, 40, (i+1));
   status = ohci_bulk_transfer(controller_number, i, time_to_wait);
   
   return status;
  }
 }
 
 return STATUS_ERROR;
}

byte_t ohci_bulk_in(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 byte_t status;

 for(int i=40; i<600; i++, ((toggle++) & 0x1)) {
  if(length>64) {
   ohci_create_td(controller_number, i, (i+1), OHCI_IN, toggle, memory, 64);
   length -= 64;
   memory += 64;
  }
  else {
   ohci_create_td(controller_number, i, 0, OHCI_IN, toggle, memory, length);
   ohci_toggle = ((toggle+=1) & 0x1);
   ohci_create_ed(controller_number, port, 0, (port+1), endpoint, 40, (i+1));
   status = ohci_bulk_transfer(controller_number, i, time_to_wait);
   
   return status;
  }
 }
 
 return STATUS_ERROR;
}
