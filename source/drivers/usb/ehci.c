//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ehci_controller(byte_t controller_number) {
 //calculate base of operational registers
 usb_controllers[controller_number].base2 = (usb_controllers[controller_number].base+mmio_inb(usb_controllers[controller_number].base+0x00));
 
 //read number of ports
 usb_controllers[controller_number].number_of_ports = (mmio_ind(usb_controllers[controller_number].base+0x04) & 0xFF);
 
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
 mmio_outd(usb_controllers[controller_number].base2+0x14, 0x00000000);
 
 //asynchronous list
 usb_controllers[controller_number].mem1 = aligned_malloc(64, 0x1F);
 usb_controllers[controller_number].mem2 = aligned_malloc(1024*64, 0x1F);
 usb_controllers[controller_number].setup_mem = malloc(16);
 mmio_outd(usb_controllers[controller_number].base2+0x18, usb_controllers[controller_number].mem1);
 
 //clear status
 mmio_outd(usb_controllers[controller_number].base2+0x04, 0x3F);
 
 //run controller
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080001);
 
 //set flag
 mmio_outd(usb_controllers[controller_number].base2+0x40, 0x1);
 
 //power all ports
 for(int i=0, ehci_port=(usb_controllers[controller_number].base2+0x44); i<usb_controllers[controller_number].number_of_ports; i++, ehci_port+=0x04) {
  mmio_outd(ehci_port, (mmio_ind(ehci_port) | 0x1000));
 }
 wait(50);
}

byte_t ehci_controller_detect_status_change(byte_t controller_number) {
 for(int i=0, port_number=0x44; i<usb_controllers[controller_number].number_of_ports; i++, port_number+=0x4) {
  //is status change bit set?
  if((mmio_ind(usb_controllers[controller_number].base2+port_number) & 0x2)==0x2) {
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE;
}

void ehci_controller_detect_devices(byte_t controller_number) {
 dword_t port_status=0;

 for(int i=0, port_number=0x44; i<usb_controllers[controller_number].number_of_ports; i++, port_number+=0x4) {
  port_status = mmio_ind(usb_controllers[controller_number].base2+port_number);
  
  //is some device connected?
  if((port_status & 0x1)==0x1 && ((port_status & 0x2000)==0x0000)) {
   if(usb_controllers[controller_number].ports_state[i]==PORT_INITALIZED_DEVICE && ((port_status & 0x2)==0x0)) {
    continue; //we already initalized this device
   }
   usb_controllers[controller_number].ports_state[i]=PORT_DEVICE;
   
   ehci_remove_device_if_connected(controller_number, i);
   
   //reset device and clear status change
   mmio_outd(usb_controllers[controller_number].base2+port_number, 0x1100);
   wait(50);
   mmio_outd(usb_controllers[controller_number].base2+port_number, 0x1002);
   wait(30);
   
   //pass low/full speed device to companion controller
   if((mmio_ind(usb_controllers[controller_number].base2+port_number) & 0x4)==0x0) {
    usb_controllers[controller_number].ports_device_speed[i]=USB_LOW_SPEED;
    usb_controllers[controller_number].ports_state[i]=PORT_INITALIZED_DEVICE;
    mmio_outd(usb_controllers[controller_number].base2+port_number, 0x3000);
    wait(100);  
    mmio_outd(usb_controllers[controller_number].base2+port_number, 0x3002);  
    continue;
   }
   else {
    usb_controllers[controller_number].ports_device_speed[i]=USB_HIGH_SPEED;
   }
   
   //read descriptor
   ehci_read_descriptor(controller_number, i);
   
   //set address of device
   ehci_set_address(controller_number, i);
   
   //read configuration descriptor
   ehci_read_configuration_descriptor(controller_number, i);
   
   //initalizing succesfull
   usb_controllers[controller_number].ports_state[i]=PORT_INITALIZED_DEVICE;
  }
  else if(((port_status & 0x2000)==0x0000)) {
   //remove if there was connected device
   if(usb_controllers[controller_number].ports_state[i]!=PORT_NO_DEVICE) {
    ehci_remove_device_if_connected(controller_number, i);
   }
   
   //clear status change
   mmio_outd(usb_controllers[controller_number].base2+port_number, 0x1002);
   
   //save port status change
   usb_controllers[controller_number].ports_state[i]=PORT_NO_DEVICE;
  }
 }
}

void ehci_remove_device_if_connected(byte_t controller_number, byte_t port) {
 for(int i=0; i<10; i++) {
  if(usb_mass_storage_devices[i].controller_type==USB_CONTROLLER_EHCI && usb_mass_storage_devices[i].controller_number==controller_number && usb_mass_storage_devices[i].port==port) {
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

void ehci_create_queue_head(byte_t controller_number, dword_t address, dword_t endpoint, dword_t endpoint_max_length) {
 dword_t *queue_head = (dword_t *) usb_controllers[controller_number].mem1;
 dword_t attributes = ((address) | (endpoint<<8) | (endpoint_max_length<<16) | (0x8000E000));

 queue_head[0]=(usb_controllers[controller_number].mem1 | 0x2); //points to itself for fastest transfer
 queue_head[1]=attributes;
 queue_head[2]=0x40000000;
 queue_head[3]=usb_controllers[controller_number].mem2;
 queue_head[4]=usb_controllers[controller_number].mem2;
 queue_head[5]=0;
 queue_head[6]=0;
 queue_head[7]=0;
}

void ehci_print_qh(byte_t controller_number) {
 dword_t *queue_head = (dword_t *) usb_controllers[controller_number].mem1;
 
 for(int i=0; i<8; i++) {
  log("\n");
  log_hex(queue_head[i]);
 }
}

void ehci_create_transfer_descriptor(byte_t controller_number, dword_t td_number, dword_t next_td_number, dword_t alternate_td_number, dword_t packet_type, dword_t length, dword_t toggle, dword_t memory) {
 dword_t *td = (dword_t *) (usb_controllers[controller_number].mem2+td_number*64);
 
 if(next_td_number==0) {
  td[0]=EHCI_NO_POINTER;
 }
 else {
  td[0]=(usb_controllers[controller_number].mem2+next_td_number*64);
 }
 if(next_td_number==0) {
  td[1]=EHCI_NO_POINTER;
 }
 else {
  td[1]=(usb_controllers[controller_number].mem2+alternate_td_number*64);
 }
 td[2]=((toggle<<31) | (length<<16) | 0xC00 | (packet_type<<8) | 0x80);
 td[3]=memory;
 td[4]=((memory & 0xFFFFF000)+0x1000);
 td[5]=(td[4]+0x1000);
 td[6]=(td[4]+0x2000);
 td[7]=(td[4]+0x3000);
 td[8]=0;
 td[9]=0;
 td[10]=0;
 td[11]=0;
 td[12]=0;
 td[13]=0;
 td[14]=0;
 td[15]=0;
}

void ehci_print_td(byte_t controller_number, dword_t td_number) {
 dword_t *td = (dword_t *) (usb_controllers[controller_number].mem2+td_number*64);
 
 for(int i=0; i<16; i++) {
  log("\n");
  log_hex(td[i]);
 }
}

byte_t ehci_run_transfer(dword_t controller_number, dword_t td_number, dword_t time) {
 dword_t *transfer_descriptor = (dword_t *) (usb_controllers[controller_number].mem2+(td_number*64));

 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080021);
 
 ticks=0;
 while(ticks<time) {
  asm("hlt");
  
  if((transfer_descriptor[2] & 0x80)!=0x80) { //active bit
   if((transfer_descriptor[2] & 0x7E)==0x00) { //error bits
    mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080001);
    return STATUS_GOOD;
   }
   else {
    log("\nEHCI bad transfer ");
    log_hex(transfer_descriptor[2]);
    mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080001);
    return STATUS_ERROR;
   }
  }
 }
 
 log("\nEHCI transfer timeout");
 mmio_outd(usb_controllers[controller_number].base2+0x00, 0x00080001);
 return STATUS_ERROR;
}

void ehci_read_descriptor(byte_t controller_number, byte_t port) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc(256);
 dword_t *data = (dword_t *) data_mem;

 //set read base descriptor SETUP packet
 buffer[0]=0x01000680;
 buffer[1]=0x00120000;
 
 ehci_create_queue_head(controller_number, 0, ENDPOINT_0, 64);
 ehci_create_transfer_descriptor(controller_number, 0, 1, 0, EHCI_SETUP, 8, TOGGLE_0, usb_controllers[controller_number].setup_mem);
 ehci_create_transfer_descriptor(controller_number, 1, 2, 2, EHCI_IN, 18, TOGGLE_1, data_mem);
 ehci_create_transfer_descriptor(controller_number, 2, 0, 0, EHCI_OUT, 0, TOGGLE_1, 0x0);
 ehci_run_transfer(controller_number, 2, 100); //TODO: error transfer
 
 //parse data
 parse_usb_descriptor(data_mem);
 
 free(data_mem);
}

void ehci_set_address(byte_t controller_number, byte_t port) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set address SETUP packet
 buffer[0]=(0x00000500 | ((port+1)<<16)); //address will be always port number
 buffer[1]=0x00000000;
 
 ehci_create_queue_head(controller_number, 0, ENDPOINT_0, 64);
 ehci_create_transfer_descriptor(controller_number, 0, 1, 0, EHCI_SETUP, 8, TOGGLE_0, usb_controllers[controller_number].setup_mem);
 ehci_create_transfer_descriptor(controller_number, 1, 0, 0, EHCI_IN, 0, TOGGLE_1, 0x0);
 ehci_run_transfer(controller_number, 1, 100); //TODO: error transfer
}

void ehci_transfer_set_setup(byte_t controller_number, byte_t port) {
 ehci_create_queue_head(controller_number, (port+1), ENDPOINT_0, 64);
 ehci_create_transfer_descriptor(controller_number, 0, 1, 0, EHCI_SETUP, 8, TOGGLE_0, usb_controllers[controller_number].setup_mem);
 ehci_create_transfer_descriptor(controller_number, 1, 0, 0, EHCI_IN, 0, TOGGLE_1, 0x0);
 ehci_run_transfer(controller_number, 1, 100); //TODO: error transfer
}

void ehci_set_configuration(byte_t controller_number, byte_t port, dword_t configuration_num) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set configuration SETUP packet
 buffer[0]=(0x00000900 | (configuration_num<<16));
 buffer[1]=0x00000000;

 ehci_transfer_set_setup(controller_number, port);
}

void ehci_set_interface(byte_t controller_number, byte_t port, dword_t interface_num, dword_t alt_interface_num) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;

 //set interface SETUP packet
 buffer[0]=(0x00000B01 | (alt_interface_num<<16));
 buffer[1]=interface_num;

 ehci_transfer_set_setup(controller_number, port);
}

void ehci_read_configuration_descriptor(byte_t controller_number, byte_t port) {
 dword_t *buffer = (dword_t *) usb_controllers[controller_number].setup_mem;
 dword_t data_mem = calloc(256);
 dword_t *data = (dword_t *) data_mem;

 //set read descriptor SETUP packet
 buffer[0]=0x02000680;
 buffer[1]=0x00FF0000;
 
 ehci_create_queue_head(controller_number, (port+1), ENDPOINT_0, 64);
 ehci_create_transfer_descriptor(controller_number, 0, 1, 0, EHCI_SETUP, 8, TOGGLE_0, usb_controllers[controller_number].setup_mem);
 ehci_create_transfer_descriptor(controller_number, 1, 2, 5, EHCI_IN, 64, TOGGLE_1, data_mem);
 ehci_create_transfer_descriptor(controller_number, 2, 3, 5, EHCI_IN, 64, TOGGLE_0, data_mem+64);
 ehci_create_transfer_descriptor(controller_number, 3, 4, 5, EHCI_IN, 64, TOGGLE_1, data_mem+64+64);
 ehci_create_transfer_descriptor(controller_number, 4, 5, 5, EHCI_IN, 64, TOGGLE_0, data_mem+64+64+64);
 ehci_create_transfer_descriptor(controller_number, 5, 0, 0, EHCI_OUT, 0, TOGGLE_1, 0x0);
 ehci_run_transfer(controller_number, 5, 100); //TODO: error transfer
 
 parse_usb_configuration_descriptor(data_mem);
 
 if(usb_descriptor_devices[0].type==USB_DEVICE_MASS_STORAGE) {
  log("EHCI: USB mass storage device ");
  
  ehci_set_configuration(controller_number, port, usb_descriptor_devices[0].configuration);
  ehci_set_interface(controller_number, port, usb_descriptor_devices[0].interface, usb_descriptor_devices[0].alternative_interface);
  
  for(int i=0; i<10; i++) {
   if(usb_mass_storage_devices[i].type==USB_MSD_NOT_ATTACHED) {
    usb_mass_storage_devices[i].type = USB_MSD_ATTACHED;
    usb_mass_storage_devices[i].controller_type = USB_CONTROLLER_EHCI;
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

byte_t ehci_bulk_out(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 ehci_create_queue_head(controller_number, (port+1), endpoint, 512);
 for(int i=0; i<1024; i++, ((toggle++) & 0x1)) {
  if(length>512) {
   ehci_create_transfer_descriptor(controller_number, i, i+1, 0, EHCI_OUT, 512, toggle, memory);
   length -= 512;
   memory += 512;
  }
  else {
   ehci_create_transfer_descriptor(controller_number, i, 0, 0, EHCI_OUT, length, toggle, memory);
   ehci_toggle = ((toggle+=1) & 0x1);
   return ehci_run_transfer(controller_number, i, time_to_wait);
  }
 }
 
 return STATUS_ERROR;
}

byte_t ehci_bulk_in(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait) {
 ehci_create_queue_head(controller_number, (port+1), endpoint, 512);
 for(int i=0; i<1024; i++, ((toggle++) & 0x1)) {
  if(length>512) {
   ehci_create_transfer_descriptor(controller_number, i, i+1, 0, EHCI_IN, 512, toggle, memory);
   length -= 512;
   memory += 512;
  }
  else {
   ehci_create_transfer_descriptor(controller_number, i, 0, 0, EHCI_IN, length, toggle, memory);
   ehci_toggle = ((toggle+=1) & 0x1);
   return ehci_run_transfer(controller_number, i, time_to_wait);
  }
 }
 
 return STATUS_ERROR;
}
