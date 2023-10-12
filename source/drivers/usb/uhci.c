//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
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
 uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 1); //nothing is here to execute
 outd(usb_controllers[controller_number].base+0x8, usb_controllers[controller_number].mem1);
 
 //allocate memory for queue heads and transfer descriptors
 usb_controllers[controller_number].mem2 = aligned_malloc(2*4, 0xF);
 usb_controllers[controller_number].mem3 = aligned_malloc(32*600, 0xF);
 usb_controllers[controller_number].setup_mem = malloc(16);
 
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

void uhci_controller_detect_devices(byte_t controller_number) {
 word_t port_status=0;

 for(int i=0, port_number=0x10; i<usb_controllers[controller_number].number_of_ports; i++, port_number+=0x2) {
  port_status = inw(usb_controllers[controller_number].base+port_number);
  
  //is some device connected?
  if((port_status & 0x1)==0x1) {
   if(usb_controllers[controller_number].ports_state[i]==PORT_INITALIZED_DEVICE && ((port_status & 0x2)==0x0)) {
    continue; //we already initalized this device
   }
   usb_controllers[controller_number].ports_state[i]=PORT_DEVICE;
   
   uhci_remove_device_if_connected(controller_number, i);
   
   //reset device
   outw(usb_controllers[controller_number].base+port_number, 0x0201);
   wait(100);
   outw(usb_controllers[controller_number].base+port_number, 0x0001);
   wait(50);
   
   //find if this is low speed or full speed device
   if((inw(usb_controllers[controller_number].base+port_number) & 0x0100)==0x0100) {
    usb_controllers[controller_number].ports_device_speed[i]=USB_LOW_SPEED;
   }
   else {
    usb_controllers[controller_number].ports_device_speed[i]=USB_FULL_SPEED;
   }
   
   //clear status change and enable device, this must not be done in one write
   outw(usb_controllers[controller_number].base+port_number, 0x3);
   wait(1);
   outw(usb_controllers[controller_number].base+port_number, 0x5);
   wait(100);
   if((inw(usb_controllers[controller_number].base+port_number) & 0x4)==0x0) {
    log("\nUHCI: device was not enabled");
    continue;
   }
   
   //get descriptor
   if(usb_controllers[controller_number].ports_device_speed[i]==USB_LOW_SPEED) {
    usb_control_endpoint_length = 8;
    usb_device_type = uhci_read_descriptor(controller_number, usb_controllers[controller_number].ports_device_speed[i], 8, 8);
   }
   else if(usb_controllers[controller_number].ports_device_speed[i]==USB_FULL_SPEED) {
    usb_control_endpoint_length = 64;
    usb_device_type = uhci_read_descriptor(controller_number, usb_controllers[controller_number].ports_device_speed[i], 18, 64);
   }
   
   //return if there was reading error
   if(usb_device_type==0xFFFFFFFF) {
    log("\nerror during reading USB descriptor");
    return;
   }

   //get full descriptor from low speed devices
   if(usb_controllers[controller_number].ports_device_speed[i]==USB_LOW_SPEED) {
    if(uhci_read_descriptor(controller_number, usb_controllers[controller_number].ports_device_speed[i], 18, usb_control_endpoint_length)==0xFFFFFFFF) {
     log("\nerror during reading low-speed USB descriptor");
     return;
    }
   }
    
   //set address of device
   if(uhci_set_address(controller_number, usb_controllers[controller_number].ports_device_speed[i])==STATUS_ERROR) {
    log("\nUHCI: error during setting addres");
    return;
   }
   
   //read configuration descriptor
   if(uhci_read_configuration_descriptor(controller_number, i, usb_controllers[controller_number].ports_device_speed[i], usb_control_endpoint_length)==STATUS_ERROR) {
    log("\nUHCI: error during reading configuration descriptor");
    return;
   }
   
   //initalizing succesfull
   usb_controllers[controller_number].ports_state[i]=PORT_INITALIZED_DEVICE;
  }
  else {
   //remove if there was connected device
   if(usb_controllers[controller_number].ports_state[i]!=PORT_NO_DEVICE) {
    uhci_remove_device_if_connected(controller_number, i);
   }
   
   //clear status change
   outw(usb_controllers[controller_number].base+port_number, 0x2);
   
   //save port status change
   usb_controllers[controller_number].ports_state[i]=PORT_NO_DEVICE;
  }
 }
}

void uhci_remove_device_if_connected(byte_t controller_number, byte_t port) {
 if(usb_mouse_state==USB_MOUSE_UHCI_PORT && usb_mouse_controller==controller_number && usb_mouse_port==port) {
  uhci_set_frame_list(controller_number, 1, UHCI_NO_POINTER, 4);
  release_usb_address(usb_mouse_address);
  usb_mouse_state=0;
  usb_mouse_controller=0;
  usb_mouse_port=0;
  usb_mouse_address=0;
  usb_mouse_device_speed=0;
  usb_mouse_endpoint=0;
 }
 if(usb_keyboard_state==USB_KEYBOARD_UHCI_PORT && usb_keyboard_controller==controller_number && usb_keyboard_port==port) {
  uhci_set_frame_list(controller_number, 2, UHCI_NO_POINTER, 4);
  release_usb_address(usb_keyboard_address);
  usb_keyboard_state=0;
  usb_keyboard_controller=0;
  usb_keyboard_port=0;
  usb_keyboard_address=0;
  usb_keyboard_device_speed=0;
  usb_keyboard_endpoint=0;
 }
 for(int i=0; i<10; i++) {
  if(usb_mass_storage_devices[i].controller_type==USB_CONTROLLER_UHCI && usb_mass_storage_devices[i].controller_number==controller_number && usb_mass_storage_devices[i].port==port) {
   usb_mass_storage_devices[i].type=USB_MSD_NOT_ATTACHED;
   usb_mass_storage_devices[i].controller_number=0;
   usb_mass_storage_devices[i].controller_type=0;
   usb_mass_storage_devices[i].port=0;
   usb_mass_storage_devices[i].device_speed=0;
   release_usb_address(usb_mass_storage_devices[i].address);
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

void uhci_set_frame_list(byte_t controller_number, byte_t offset, dword_t value, byte_t frequency) {
 dword_t *frame_list = (dword_t *) usb_controllers[controller_number].mem1;
 for(int i=offset; i<1024; i+=frequency) {
  frame_list[i]=value;
 }
}

void uhci_set_queue_head(byte_t controller_number, dword_t offset, dword_t first_value, dword_t second_value) {
 dword_t *queue_head = (dword_t *) (usb_controllers[controller_number].mem2+offset);
 
 queue_head[0]=first_value;
 queue_head[1]=second_value;
}

void uhci_set_transfer_descriptor(byte_t controller_number, byte_t device_speed, dword_t descriptor, dword_t next_descriptor, dword_t length, dword_t toggle, dword_t endpoint, dword_t device_address, dword_t packet_type, dword_t buffer) {
 dword_t *transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem3+(descriptor*32));
 
 if(next_descriptor==UHCI_NO_DESCRIPTOR) {
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

byte_t uhci_wait_for_transfer(dword_t controller_number, dword_t descriptor, dword_t time) {
 dword_t *transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem3+(descriptor*32));
 
 ticks=0;
 while(ticks<time) {
  asm("hlt");
  
  if((transfer_descriptor[1] & 0x00800000)==0x00000000) { //active bit
   if((transfer_descriptor[1] & 0x007E0000)==0x00000000) { //error bits
    return STATUS_GOOD;
   }
   else {
    log("\nUHCI bad transfer ");
    log_hex_with_space(transfer_descriptor[1]);
    return STATUS_ERROR;
   }
  }
 }
 
 log("\nUHCI transfer timeout");
 transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem3+4);
 for(dword_t i=0; i<(descriptor+1); i++) {
  log("\n");
  log_hex(*transfer_descriptor);
  transfer_descriptor+=8;
 }
 log("\n");
 return STATUS_ERROR;
}

void uhci_transfer_set_setup(byte_t controller_number, byte_t device_speed) {
 uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
 uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, usb_device_address, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
 uhci_set_transfer_descriptor(controller_number, device_speed, 1, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, usb_device_address, UHCI_IN, 0x0);
 uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
 uhci_wait_for_transfer(controller_number, 1, 200); //TODO: error transaction
 uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
}

byte_t uhci_set_address(byte_t controller_number, byte_t device_speed) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //get address for device
 if(get_free_usb_address()==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //set address SETUP packet
 buffer[0]=(0x00000500 | (usb_device_address<<16));
 buffer[1]=0x00000000;

 uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
 uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, 0, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
 uhci_set_transfer_descriptor(controller_number, device_speed, 1, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, 0, UHCI_IN, 0x0);
 uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
 byte_t status = uhci_wait_for_transfer(controller_number, 1, 200); //TODO: error transaction
 uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);

 return status;
}

void uhci_set_configuration(byte_t controller_number, byte_t device_speed, dword_t configuration_num) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set configuration SETUP packet
 buffer[0]=(0x00000900 | (configuration_num<<16));
 buffer[1]=0x00000000;

 uhci_transfer_set_setup(controller_number, device_speed);
}

void uhci_set_interface(byte_t controller_number, byte_t device_speed, dword_t interface_num, dword_t alt_interface_num) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set interface SETUP packet
 buffer[0]=(0x00000B01 | (alt_interface_num<<16));
 buffer[1]=interface_num;

 uhci_transfer_set_setup(controller_number, device_speed);
}

void uhci_set_idle(byte_t controller_number, byte_t device_speed, dword_t interface_num, byte_t idle_value) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set idle SETUP packet
 buffer[0]=(0x00000A21 | (idle_value<<24));
 buffer[1]=interface_num;

 uhci_transfer_set_setup(controller_number, device_speed);
}

void uhci_set_protocol(byte_t controller_number, byte_t device_speed, dword_t interface_num, dword_t protocol_type) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set protocol SETUP packet
 buffer[0]=(0x00000B21 | (protocol_type<<16));
 buffer[1]=interface_num;

 uhci_transfer_set_setup(controller_number, device_speed);
}

void uhci_set_keyboard_led(byte_t controller_number, byte_t device_speed, byte_t address, dword_t interface_num, byte_t endpoint, byte_t led_state) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set report SETUP packet
 buffer[0]=0x02000921;
 buffer[1]=(0x00010000 | interface_num);
 
 //set data
 buffer[2]=led_state;

 //transfer
 uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
 uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, endpoint, usb_device_address, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
 uhci_set_transfer_descriptor(controller_number, device_speed, 1, 2, 1, TOGGLE_1, endpoint, usb_device_address, UHCI_OUT, usb_controllers[controller_number].setup_mem+8);
 uhci_set_transfer_descriptor(controller_number, device_speed, 2, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, endpoint, usb_device_address, UHCI_IN, 0x0);
 uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
 uhci_wait_for_transfer(controller_number, 2, 200); //TODO: error transaction
 uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
}

void uhci_set_interrupt_transfer(byte_t controller_number, byte_t device_speed, byte_t frame_offset, byte_t transfer_offset, byte_t endpoint, byte_t transfer_length, byte_t transfer_time, dword_t memory) {
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
 
 uhci_set_transfer_descriptor(controller_number, device_speed, transfer_offset, UHCI_NO_DESCRIPTOR, transfer_length, TOGGLE_0, endpoint, usb_device_address, UHCI_IN, memory);
 uhci_set_frame_list(controller_number, frame_offset, usb_controllers[controller_number].mem3+(32*transfer_offset), transfer_time);
}

void uhci_send_setup_to_device(byte_t controller_number, byte_t device_speed, word_t max_length, dword_t memory, byte_t control_endpoint_length) {
 if(control_endpoint_length==8) {
  uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
  uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, usb_device_address, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
  for(int i=1, toggle=1; i<128; i++, toggle=((toggle+=1) & 0x1)) {
   if(max_length>8) {
    uhci_set_transfer_descriptor(controller_number, device_speed, i, (i+1), 8, toggle, ENDPOINT_0, usb_device_address, UHCI_IN, memory);
    memory+=8;
    max_length-=8;
   }
   else {
    uhci_set_transfer_descriptor(controller_number, device_speed, i, (i+1), max_length, toggle, ENDPOINT_0, usb_device_address, UHCI_IN, memory);
    uhci_set_transfer_descriptor(controller_number, device_speed, (i+1), UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, usb_device_address, UHCI_OUT, 0x0);
    uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
    uhci_wait_for_transfer(controller_number, (i+1), 200); //TODO: error transaction
    uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);     
    break;
   }
  }
 }
 else if(control_endpoint_length==64) {
  uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
  uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, usb_device_address, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
  for(int i=1, toggle=1; i<128; i++, toggle=((toggle+=1) & 0x1)) {
   if(max_length>64) {
    uhci_set_transfer_descriptor(controller_number, device_speed, i, (i+1), 64, toggle, ENDPOINT_0, usb_device_address, UHCI_IN, memory);
    memory+=64;
    max_length-=64;
   }
   else {
    uhci_set_transfer_descriptor(controller_number, device_speed, i, (i+1), max_length, toggle, ENDPOINT_0, usb_device_address, UHCI_IN, memory);
    uhci_set_transfer_descriptor(controller_number, device_speed, (i+1), UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, usb_device_address, UHCI_OUT, 0x0);
    uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
    uhci_wait_for_transfer(controller_number, (i+1), 200); //TODO: error transaction
    uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);     
    break;
   }
  }
 }
}

dword_t uhci_read_descriptor(byte_t controller_number, byte_t device_speed, byte_t type_of_transfer, byte_t control_endpoint_length) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc(256);
 dword_t *data = (dword_t *) data_mem;
 byte_t status = 0;

 //set read base descriptor SETUP packet
 buffer[0]=0x01000680;
 if(type_of_transfer==8) {
  buffer[1]=0x00080000;
  
  uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
  uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, 0, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
  uhci_set_transfer_descriptor(controller_number, device_speed, 1, 2, 8, TOGGLE_1, ENDPOINT_0, 0, UHCI_IN, data_mem);
  uhci_set_transfer_descriptor(controller_number, device_speed, 2, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, 0, UHCI_OUT, 0x0);
  uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
  status = uhci_wait_for_transfer(controller_number, 2, 200);
  uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
 }
 else if(type_of_transfer==18) {
  buffer[1]=0x00120000;
  
  if(control_endpoint_length==8) {
   uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
   uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, 0, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
   uhci_set_transfer_descriptor(controller_number, device_speed, 1, 2, 8, TOGGLE_1, ENDPOINT_0, 0, UHCI_IN, data_mem);
   uhci_set_transfer_descriptor(controller_number, device_speed, 2, 3, 8, TOGGLE_0, ENDPOINT_0, 0, UHCI_IN, data_mem+8);
   uhci_set_transfer_descriptor(controller_number, device_speed, 3, 4, 2, TOGGLE_1, ENDPOINT_0, 0, UHCI_IN, data_mem+8+8);
   uhci_set_transfer_descriptor(controller_number, device_speed, 4, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, 0, UHCI_OUT, 0x0);
   uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
   status = uhci_wait_for_transfer(controller_number, 4, 200);
   uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
  }
  else if(control_endpoint_length==64) {
   uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
   uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, 0, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
   uhci_set_transfer_descriptor(controller_number, device_speed, 1, 2, 64, TOGGLE_1, ENDPOINT_0, 0, UHCI_IN, data_mem);
   uhci_set_transfer_descriptor(controller_number, device_speed, 2, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, 0, UHCI_OUT, 0x0);
   uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
   status = uhci_wait_for_transfer(controller_number, 2, 200);
   uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
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

byte_t uhci_read_configuration_descriptor(byte_t controller_number, byte_t port, byte_t device_speed, byte_t control_endpoint_length) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc(8);
 dword_t *data = (dword_t *) data_mem;
 dword_t data_length = 0;
 
 //read length of CONFIGURATION descriptor
 buffer[0]=0x02000680;
 buffer[1]=0x00080000;
  
 uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3);
 uhci_set_transfer_descriptor(controller_number, device_speed, 0, 1, 8, TOGGLE_0, ENDPOINT_0, usb_device_address, UHCI_SETUP, usb_controllers[controller_number].setup_mem);
 uhci_set_transfer_descriptor(controller_number, device_speed, 1, 2, 8, TOGGLE_1, ENDPOINT_0, usb_device_address, UHCI_IN, data_mem);
 uhci_set_transfer_descriptor(controller_number, device_speed, 2, UHCI_NO_DESCRIPTOR, 0, TOGGLE_1, ENDPOINT_0, usb_device_address, UHCI_OUT, 0x0);
 uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
 byte_t status = uhci_wait_for_transfer(controller_number, 2, 200);
 uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
 
 if(status==STATUS_ERROR) {
  free(data_mem);
  return STATUS_ERROR;
 }

 data_length = (data[0]>>16);
 if(data_length==0) {
  log("\nerror size of configuration descriptor");
  free(data_mem);
  return STATUS_ERROR;
 }
 
 //read CONFIGURATION DESCRIPTOR
 buffer[0]=0x02000680;
 buffer[1]=(0x00000000 | (data_length<<16));
 data_mem = realloc(data_mem, data_length);
 
 uhci_send_setup_to_device(controller_number, device_speed, data_length, data_mem, control_endpoint_length);
 
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
  log("UHCI: USB keyboard ");
  
  if(usb_keyboard_state!=0) {
   free(data_mem);
   log("duplicated\n");
   return STATUS_GOOD;
  }
  
  uhci_set_configuration(controller_number, device_speed, usb_descriptor_devices[0].configuration);
  uhci_set_interface(controller_number, device_speed, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  uhci_set_idle(controller_number, device_speed, usb_descriptor_devices[0].interface, 0);
  uhci_set_protocol(controller_number, device_speed, usb_descriptor_devices[0].interface, USB_BOOT_PROTOCOL);
  
  usb_keyboard_controller = controller_number;
  log_var_with_space(usb_keyboard_controller);
  usb_keyboard_port = port;
  usb_keyboard_device_speed = device_speed;
  usb_keyboard_address = usb_device_address;
  log_var_with_space(usb_keyboard_port);
  usb_keyboard_interface = usb_descriptor_devices[0].interface;
  usb_keyboard_endpoint = usb_descriptor_devices[0].endpoint_interrupt;
  log_var_with_space(usb_keyboard_endpoint);
  usb_keyboard_endpoint_out = usb_descriptor_devices[0].endpoint_interrupt_out; //if INTERRUPT OUT is not present, this will be zero - CONTROL endpoint
  log_var(usb_keyboard_endpoint_out);
  log("\n");
  
  uhci_set_interrupt_transfer(usb_keyboard_controller, usb_keyboard_device_speed, 2, 42, usb_keyboard_endpoint, usb_descriptor_devices[0].endpoint_interrupt_length, usb_descriptor_devices[0].endpoint_interrupt_time, usb_keyboard_data_memory);
  usb_keyboard_check_transfer_descriptor = usb_controllers[controller_number].mem3+32*42+4;
  
  usb_keyboard_state = USB_KEYBOARD_UHCI_PORT;
 }
 else if(usb_descriptor_devices[0].type==USB_DEVICE_MOUSE) {
  log("UHCI: USB mouse ");
  
  if(usb_mouse_state!=0) {
   free(data_mem);
   log("duplicated\n");
   return STATUS_GOOD;
  }
  
  uhci_set_configuration(controller_number, device_speed, usb_descriptor_devices[0].configuration);
  uhci_set_interface(controller_number, device_speed, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  uhci_set_idle(controller_number, device_speed, usb_descriptor_devices[0].interface, 0);
  
  usb_mouse_controller = controller_number;
  log_var_with_space(usb_mouse_controller);
  usb_mouse_port = port;
  usb_mouse_device_speed = device_speed;
  usb_mouse_address = usb_device_address;
  log_var_with_space(usb_mouse_port);
  usb_mouse_endpoint = usb_descriptor_devices[0].endpoint_interrupt;
  log_var(usb_mouse_endpoint);
  log("\n");
  
  uhci_read_hid_descriptor(controller_number, device_speed, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].hid_descriptor_length, control_endpoint_length);
  uhci_set_protocol(controller_number, device_speed, usb_descriptor_devices[0].interface, USB_HID_PROTOCOL);
  
  uhci_set_interrupt_transfer(usb_mouse_controller, usb_mouse_device_speed, 1, 40, usb_mouse_endpoint, usb_descriptor_devices[0].endpoint_interrupt_length, usb_descriptor_devices[0].endpoint_interrupt_time, usb_mouse_data_memory);
  usb_mouse_check_transfer_descriptor = usb_controllers[controller_number].mem3+32*40+4;
  
  usb_mouse_state = USB_MOUSE_UHCI_PORT;
 }
 else if(usb_descriptor_devices[0].type==USB_DEVICE_MASS_STORAGE) {
  log("UHCI: USB mass storage device ");
  
  uhci_set_configuration(controller_number, device_speed, usb_descriptor_devices[0].configuration);
  uhci_set_interface(controller_number, device_speed, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  
  for(int i=0; i<10; i++) {
   if(usb_mass_storage_devices[i].type==USB_MSD_NOT_ATTACHED) {
    usb_mass_storage_devices[i].type = USB_MSD_ATTACHED;
    usb_mass_storage_devices[i].controller_type = USB_CONTROLLER_UHCI;
    usb_mass_storage_devices[i].controller_number = controller_number;
    usb_mass_storage_devices[i].port = port;
    usb_mass_storage_devices[i].device_speed = device_speed;
    usb_mass_storage_devices[i].address = usb_device_address;
    usb_mass_storage_devices[i].endpoint_in = usb_descriptor_devices[0].endpoint_bulk_in;
    log_var_with_space(usb_mass_storage_devices[i].endpoint_in);
    usb_mass_storage_devices[i].toggle_in = 0;
    usb_mass_storage_devices[i].endpoint_out = usb_descriptor_devices[0].endpoint_bulk_out;
    log_var_with_space(usb_mass_storage_devices[i].endpoint_out);
    usb_mass_storage_devices[i].toggle_out = 0;
    
    usb_mass_storage_initalize(i);
    
    free(data_mem);
    return STATUS_GOOD;
   }
  }
  
  log("more than 10 devices connected\n");
 }

 free(data_mem);
 return STATUS_GOOD;
}

void uhci_read_hid_descriptor(byte_t controller_number, byte_t device_speed, byte_t interface, word_t length, byte_t control_endpoint_length) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc((length+1));
 dword_t *data = (dword_t *) data_mem;

 //set read HID DESCRIPTOR SETUP packet
 buffer[0]=0x22000681;
 buffer[1]=((length<<16) | interface);
 
 uhci_send_setup_to_device(controller_number, device_speed, length, data_mem, control_endpoint_length);

 parse_hid_descriptor(data_mem);
 
 free(data_mem);
}

byte_t uhci_bulk_out(byte_t controller_number, byte_t device_speed, byte_t address, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 byte_t status;

 uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3+50*32);
 for(int i=50; i<600; i++, toggle=((toggle++) & 0x1)) {
  if(length>64) {
   uhci_set_transfer_descriptor(controller_number, device_speed, i, (i+1), 64, toggle, endpoint, address, UHCI_OUT, memory);
   length -= 64;
   memory += 64;
  }
  else {
   uhci_set_transfer_descriptor(controller_number, device_speed, i, UHCI_NO_DESCRIPTOR, length, toggle, endpoint, address, UHCI_OUT, memory);
   uhci_toggle = ((toggle+=1) & 0x1);
   uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
   status = uhci_wait_for_transfer(controller_number, i, time_to_wait);
   uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
   return status;
  }
 }
 
 return STATUS_ERROR;
}

byte_t uhci_bulk_in(byte_t controller_number, byte_t device_speed, byte_t address, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 byte_t status;
 
 uhci_set_queue_head(controller_number, 0, UHCI_NO_POINTER, usb_controllers[controller_number].mem3+50*32);
 for(int i=50; i<600; i++, toggle=((toggle++) & 0x1)) {
  if(length>64) {
   uhci_set_transfer_descriptor(controller_number, device_speed, i, (i+1), 64, toggle, endpoint, address, UHCI_IN, memory);
   length -= 64;
   memory += 64;
  }
  else {
   uhci_set_transfer_descriptor(controller_number, device_speed, i, UHCI_NO_DESCRIPTOR, length, toggle, endpoint, address, UHCI_IN, memory);
   uhci_toggle = ((toggle+=1) & 0x1);
   uhci_set_frame_list(controller_number, 0, (usb_controllers[controller_number].mem2 | 0x2), 4);
   status = uhci_wait_for_transfer(controller_number, i, time_to_wait);
   uhci_set_frame_list(controller_number, 0, UHCI_NO_POINTER, 4);
   return status;
  }
 }
 
 return STATUS_ERROR;
}
