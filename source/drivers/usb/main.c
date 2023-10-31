//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_usb_controllers(void) {
 //initalize values
 for(int i=0; i<usb_controllers_pointer; i++) {
  for(int j=0; j<16; j++) {
   usb_controllers[i].ports_state[j]=PORT_NO_DEVICE;
   usb_controllers[i].ports_device_speed[j]=0;
  }
 }
 for(dword_t i=0; i<127; i++) {
  usb_addresses[i]=0;
 }
 usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;
 usb_keyboard[0].controller_type = USB_NO_DEVICE_ATTACHED;
 for(int i=0; i<10; i++) {
  usb_mass_storage_devices[i].type = USB_NO_DEVICE_ATTACHED;
  usb_mass_storage_devices[i].controller_type = 0;
 }
 for(int i=0; i<10; i++) {
  usb_hubs[i].controller_type = USB_NO_DEVICE_ATTACHED;
  for(dword_t j=0; j<8; j++) {
   usb_hubs[i].ports_state[j] = PORT_NO_DEVICE;
   usb_hubs[i].ports_device_speed[j] = 0;
  }
 }
 
 if(usb_controllers_pointer==0) {
  log("\nno USB controllers\n");
  return;
 }
 
 //allocate memory for USB devices
 usb_setup_packet_mem = malloc(16);
 usb_setup_packet_data_mem = malloc(0xFFFF);
 usb_mouse_data_memory = malloc(8);
 usb_keyboard_data_memory = calloc(8);
 usb_hub_transfer_setup_packets_mem = malloc(8*8);
 usb_hub_transfer_data_mem = malloc(4*8);
 usb_mass_storage_cbw_memory = malloc(32);
 usb_mass_storage_response_memory = calloc(128);
 usb_mass_storage_csw_memory = malloc(16);
 
 //initalize EHCI USB controllers
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_EHCI) {
   initalize_ehci_controller(i);
   continue;
  }
 }
 
 //initalize UHCI and OHCI USB controllers
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_UHCI) {
   initalize_uhci_controller(i);
   continue;
  }
  
  if(usb_controllers[i].type==USB_CONTROLLER_OHCI) {
   initalize_ohci_controller(i);
   continue;
  }
 }
 
 //detect already connected USB devices
 detect_usb_devices();

 //detect devices connected to USB HUBs
 detect_usb_devices_on_hubs();
 
 log("\n");
}

void detect_usb_devices(void) {
 //detect devices connected to EHCI
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_EHCI) {
   detect_usb_devices_on_controller(i);
  }
 }
 
 //detect devices connected to UHCI and OHCI
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_UHCI || usb_controllers[i].type==USB_CONTROLLER_OHCI) {
   detect_usb_devices_on_controller(i);
  }
 }
}

void detect_usb_devices_on_hubs(void) {
 for(dword_t i=0; i<10; i++) {
  if(usb_hubs[i].controller_type!=USB_NO_DEVICE_ATTACHED) {
   detect_usb_devices_on_hub(i);
  }
 }
}

void detect_usb_devices_on_controller(byte_t controller_number) {
 for(int port=0; port<usb_controllers[controller_number].number_of_ports; port++) {
  //is some device connected?
  if(usb_get_port_connection_status(controller_number, port)==STATUS_TRUE) {
   //check if we already initalized this device
   if(usb_controllers[controller_number].ports_state[port]==PORT_DEVICE && (usb_get_port_connection_change_status(controller_number, port)==STATUS_FALSE)) {
    continue;
   }

   //if there was connected some device before, remove it
   usb_remove_device(controller_number, port);

   //there is some device
   usb_controllers[controller_number].ports_state[port] = PORT_DEVICE;

   //reset and enable device
   if(usb_enable_device_on_port(controller_number, port)==STATUS_ERROR) {
    continue; //error, go to next device
   }

   //configure device
   ehci_hub_address = 0;
   ehci_hub_port_number = 0;
   usb_configure_device_with_zero_address(controller_number, port, usb_controllers[controller_number].ports_device_speed[port]);
  }
  else if(usb_controllers[controller_number].ports_state[port]!=PORT_NO_DEVICE) {
   //remove device
   usb_remove_device(controller_number, port);
   
   //clear status change
   usb_clear_port_connection_change_status(controller_number, port);
   
   //change port status
   usb_controllers[controller_number].ports_state[port] = PORT_NO_DEVICE;
  }
 }
}

void detect_usb_devices_on_hub(byte_t hub_number) {
 dword_t port_status = 0;

 //read if there is some connection status change
 ehci_hub_address = usb_hubs[hub_number].ehci_hub_address;
 ehci_hub_port_number = usb_hubs[hub_number].ehci_hub_port_number;
 byte_t hub_change = usb_hub_is_there_some_port_connection_status_change(hub_number);
 if(hub_change==0xFF) {
  log("\nUSB: hub is not responding");
  usb_remove_device(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].port);
  usb_new_device_detected = STATUS_TRUE;
  return;
 }
 else if(hub_change==STATUS_FALSE) {
  return;
 }
 else {
  usb_new_device_detected = STATUS_TRUE;
 }

 for(dword_t port=0; port<usb_hubs[hub_number].number_of_ports; port++) {
  //read port status
  usb_device_address = usb_hubs[hub_number].address;
  usb_control_endpoint_size = 8;
  port_status = usb_hub_read_port_status(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, port);
  if(port_status==0) {
   log("\nUSB hub: port status transfer error");
   continue;
  }

  //is some device connected?
  if((port_status & 0x1)==0x1) {
   if(usb_hubs[hub_number].ports_state[port]==PORT_DEVICE && ((port_status & 0x10000)==0x0)) {
    continue; //we already initalized this device
   }

   //if there was connected some device before, remove it
   usb_remove_device(usb_hubs[hub_number].controller_number, ((hub_number+1)*0x10)+port);

   //there is some device
   usb_hubs[hub_number].ports_state[port] = PORT_DEVICE;

   //reset device
   usb_hub_set_feature(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, (port+1), 0x4);
   wait(100);
   usb_hub_clear_feature(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, (port+1), 0x14);
   wait(50);

   //read actual status
   port_status = usb_hub_read_port_status(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, port);
   
   //test if was port enabled
   if((port_status & 0x2)!=0x2) {
    log("\nUSB: hub device was not enabled");
    continue;
   }

   //clear status change
   usb_hub_clear_feature(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, (port+1), 0x10);

   //find if this is low/full/high speed device
   ehci_hub_address = usb_hubs[hub_number].address;
   ehci_hub_port_number = (port+1);
   if((port_status & 0x0600)==0x0200) {
    usb_hubs[hub_number].ports_device_speed[port] = USB_LOW_SPEED;
   }
   else if((port_status & 0x0600)==0x0000) {
    usb_hubs[hub_number].ports_device_speed[port] = USB_FULL_SPEED;
   }
   else if((port_status & 0x0600)==0x0400) {
    usb_hubs[hub_number].ports_device_speed[port] = USB_HIGH_SPEED;
   }
   else {
    log("\nUSB hub: error device speed");
    continue;
   }

   //configure device
   log("\nUSB hub: enabled device");
   usb_configure_device_with_zero_address(usb_hubs[hub_number].controller_number, (((hub_number+1)*0x10)+port), usb_hubs[hub_number].ports_device_speed[port]);
  }
  else if(usb_hubs[hub_number].ports_state[port]!=PORT_NO_DEVICE) {
   //remove device
   usb_remove_device(usb_hubs[hub_number].controller_number, ((hub_number+1)*0x10)+port);
   
   //clear status change
   usb_hub_clear_feature(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, (port+1), 0x10);

   //save port status change
   usb_hubs[hub_number].ports_state[port] = PORT_NO_DEVICE;
   log("\nUSB hub: device removed");
  }
 }
}

void usb_remove_device(byte_t controller_number, byte_t port_number) {
 if(usb_mouse[0].controller_number==controller_number && usb_mouse[0].port==port_number) {
  usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;
  usb_mouse[0].controller_number = 0;
  usb_mouse[0].port = 0;
  release_usb_address(usb_mouse[0].address);
 }
 if(usb_keyboard[0].controller_number==controller_number && usb_keyboard[0].port==port_number) {
  usb_keyboard[0].controller_type = USB_NO_DEVICE_ATTACHED;
  usb_keyboard[0].controller_number = 0;
  usb_keyboard[0].port = 0;
  release_usb_address(usb_keyboard[0].address);
 }
 for(int i=0; i<10; i++) {
  if(usb_mass_storage_devices[i].type!=USB_NO_DEVICE_ATTACHED && usb_mass_storage_devices[i].controller_number==controller_number && usb_mass_storage_devices[i].port==port_number) {
   usb_mass_storage_devices[i].type=USB_NO_DEVICE_ATTACHED;
   usb_mass_storage_devices[i].controller_type = 0;
   usb_mass_storage_devices[i].controller_number = 0;
   usb_mass_storage_devices[i].port = 0;
   release_usb_address(usb_mass_storage_devices[i].address);
   remove_device_from_device_list(MEDIUM_USB_MSD, i);
  }
 }
 for(int i=0; i<10; i++) {
  if(usb_hubs[i].controller_type!=USB_NO_DEVICE_ATTACHED && usb_hubs[i].controller_number==controller_number && usb_hubs[i].port==port_number) {
   //remove devices connected to hub
   for(dword_t j=0; j<16; j++) {
    usb_remove_device(usb_hubs[i].controller_number, ((i+1)*0x10)+j);
   }

   usb_hubs[i].controller_type = USB_NO_DEVICE_ATTACHED;
   usb_hubs[i].controller_number = 0;
   usb_hubs[i].port = 0;
   release_usb_address(usb_hubs[i].address);
  }
 }
}

void detect_status_change_of_usb_devices(void) { 
 if(usb_new_device_detected==1) { //some device was already detected
  return;
 }

 for(int i=0; i<usb_controllers_pointer; i++) { 
  if(usb_controllers[i].type==USB_CONTROLLER_EHCI) {
   if(ehci_controller_detect_status_change(i)==STATUS_TRUE) {
    usb_new_device_detected = STATUS_TRUE;
    return;
   }
   continue;
  }
  if(usb_controllers[i].type==USB_CONTROLLER_UHCI) {
   if(uhci_controller_detect_status_change(i)==STATUS_TRUE) {
    usb_new_device_detected = STATUS_TRUE;
    return;
   }
   continue;
  }
  if(usb_controllers[i].type==USB_CONTROLLER_OHCI) {
   if(ohci_controller_detect_status_change(i)==STATUS_TRUE) {
    usb_new_device_detected = STATUS_TRUE;
    return;
   }
   continue;
  }
 }
}

byte_t usb_get_port_connection_status(byte_t controller_number, byte_t port_number) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_get_port_connection_status(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_get_port_connection_status(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_get_port_connection_status(controller_number, port_number);
 }
}

byte_t usb_get_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_get_port_connection_change_status(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_get_port_connection_change_status(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_get_port_connection_change_status(controller_number, port_number);
 }
}

void usb_clear_port_connection_change_status(byte_t controller_number, byte_t port_number) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  uhci_clear_port_connection_change_status(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  ohci_clear_port_connection_change_status(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  ehci_clear_port_connection_change_status(controller_number, port_number);
 }
}

byte_t usb_enable_device_on_port(byte_t controller_number, byte_t port_number) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_enable_device_on_port(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_enable_device_on_port(controller_number, port_number);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_enable_device_on_port(controller_number, port_number);
 }
}

void usb_create_setup_data(byte_t request_type, byte_t request, word_t value, word_t index, word_t length) {
 dword_t *usb_setup_data = (dword_t *) (usb_setup_packet_mem);
 usb_setup_data[0] = (request_type | (request<<8) | (value<<16));
 usb_setup_data[1] = (index | (length<<16));
 clear_memory(usb_setup_packet_data_mem, 0xFFFF);
}

byte_t usb_control_transfer_without_data(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index) {
 usb_create_setup_data(request_type, request, value, index, 0);
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_control_transfer_without_data_stage(controller_number, device_address, device_speed);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_control_transfer_without_data_stage(controller_number, device_address, device_speed);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_control_transfer_without_data_stage(controller_number, device_address, device_speed);
 }
}

byte_t usb_control_transfer_with_fixed_data_length(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index, word_t length) {
 usb_create_setup_data(request_type, request, value, index, length);
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_control_transfer_with_data_stage(controller_number, device_address, device_speed, length);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_control_transfer_with_data_stage(controller_number, device_address, device_speed, length);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_control_transfer_with_data_stage(controller_number, device_address, device_speed, length);
 }
}

byte_t usb_control_transfer_with_dynamic_data_length(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index) {
 byte_t status = STATUS_ERROR;

 //read length of whole data
 usb_create_setup_data(request_type, request, value, index, 8);
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  status = uhci_control_transfer_with_data_stage(controller_number, device_address, device_speed, 8);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  status = ohci_control_transfer_with_data_stage(controller_number, device_address, device_speed, 8);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  status = ehci_control_transfer_with_data_stage(controller_number, device_address, device_speed, 8);
 }
 if(status==STATUS_ERROR) {
  log("\nUSB: error with control transfer 8 bytes");
  return STATUS_ERROR;
 }

 //read length of data
 word_t *data_length_pointer = (word_t *) (usb_setup_packet_data_mem+2);
 word_t data_length = (*data_length_pointer);
 if(data_length==0) {
  log("\nUSB: error with control transfer no data length");
  return STATUS_ERROR;
 }

 //read data
 usb_create_setup_data(request_type, request, value, index, data_length);
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_control_transfer_with_data_stage(controller_number, device_address, device_speed, data_length);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_control_transfer_with_data_stage(controller_number, device_address, device_speed, data_length);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_control_transfer_with_data_stage(controller_number, device_address, device_speed, data_length);
 }
 else {
  return STATUS_ERROR;
 }
}

dword_t usb_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
  return uhci_interrupt_transfer(controller_number, device_number, device_address, device_speed, endpoint, endpoint_size, interrupt_time, memory);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
  return ohci_interrupt_transfer(controller_number, device_number, device_address, device_speed, endpoint, endpoint_size, interrupt_time, memory);
 }
 else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_interrupt_transfer(controller_number, device_number, device_address, device_speed, endpoint, endpoint_size, interrupt_time, memory);
 }

 return STATUS_ERROR;
}

byte_t usb_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length_of_transfer, dword_t time_to_wait) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_bulk_in(controller_number, device_address, device_speed, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_UHCI) {
  return uhci_bulk_in(controller_number, device_address, device_speed, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_OHCI) {
  return ohci_bulk_in(controller_number, device_address, device_speed, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }

 return STATUS_ERROR;
}

byte_t usb_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length_of_transfer, dword_t time_to_wait) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_bulk_out(controller_number, device_address, device_speed, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_UHCI) {
  return uhci_bulk_out(controller_number, device_address, device_speed, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_OHCI) {
  return ohci_bulk_out(controller_number, device_address, device_speed, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }

 return STATUS_ERROR;
}

byte_t usb_set_configuration_and_interface(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t configuration, byte_t interface, byte_t alternative_interface) {
 if(usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x00, 0x09, configuration, 0)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 return usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x01, 0x0B, alternative_interface, interface);
}

byte_t usb_reset_endpoint(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint) {
 return usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x02, 0x01, 0, endpoint);
}

void usb_configure_device_with_zero_address(byte_t controller_number, byte_t device_port, byte_t device_speed) {
 //get first 8 bytes from descriptor
 usb_control_endpoint_size = 8;
 if(usb_control_transfer_with_fixed_data_length(controller_number, 0, device_speed, 0x80, 0x06, 0x0100, 0, 8)==STATUS_ERROR) {
  log("\nerror during reading USB descriptor 8 bytes");
  return;
 }

 //get control endpoint size
 byte_t *usb_descriptor_control_endpoint_length_size_pointer = (byte_t *) (usb_setup_packet_data_mem+7);
 usb_control_endpoint_size = (*usb_descriptor_control_endpoint_length_size_pointer);
 if(usb_control_endpoint_size==0) {
  log("\ninvalid control endpoint length");
  return;
 }

 //get address for device
 if(get_free_usb_address()==STATUS_ERROR) {
  return; //no free address, go to next device
 }
  
 //set address of device
 if(usb_control_transfer_without_data(controller_number, 0, device_speed, 0x00, 0x05, usb_device_address, 0)==STATUS_ERROR) {
  release_usb_address(usb_device_address);
  log("\nerror during setting device address");
  return;
 }

 //get full descriptor
 if(usb_control_transfer_with_fixed_data_length(controller_number, usb_device_address, device_speed, 0x80, 0x06, 0x0100, 0, 18)==STATUS_ERROR) {
  release_usb_address(usb_device_address);
  log("\nerror during reading USB descriptor 18 bytes ");
  log_var(usb_control_endpoint_size);
  parse_usb_descriptor(usb_setup_packet_data_mem);
  return;
 }
 else {
  parse_usb_descriptor(usb_setup_packet_data_mem);
 }

 //read configuration descriptor
 if(usb_control_transfer_with_dynamic_data_length(controller_number, usb_device_address, device_speed, 0x80, 0x06, 0x0200, 0)==STATUS_ERROR) {
  release_usb_address(usb_device_address);
  log("\nerror during reading configuration descriptor");
  return;
 }
 parse_usb_configuration_descriptor(usb_setup_packet_data_mem);

 //we support max 2 interfaces on one device
 for(dword_t interface_number=0; interface_number<2; interface_number++) {
  if(usb_descriptor_devices[interface_number].type==0x000000) {
   return;
  }

  log("\n");
  if(usb_controllers[controller_number].type==USB_CONTROLLER_UHCI) {
   log("UHCI");
  }
  else if(usb_controllers[controller_number].type==USB_CONTROLLER_OHCI) {
   log("OHCI");
  }
  else if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
   log("EHCI");
  }
  log(" ");
  
  if(usb_descriptor_devices[interface_number].type==USB_DEVICE_MOUSE) {
   log("USB: USB mouse");

   if(usb_mouse[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
    log(" replacing already attached mouse");
    usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;
    release_usb_address(usb_mouse[0].address);
   }
   usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].configuration, usb_descriptor_devices[interface_number].interface, usb_descriptor_devices[interface_number].alternative_interface);
   usb_mouse_read_hid_descriptor(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].interface, usb_descriptor_devices[interface_number].hid_descriptor_length);
   usb_hid_device_set_protocol(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].interface, USB_HID_PROTOCOL);
   usb_hid_device_set_idle(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].interface);

   usb_mouse[0].controller_number = controller_number;
   usb_mouse[0].port = device_port;
   usb_mouse[0].address = usb_device_address;
   usb_mouse[0].device_speed = device_speed;
   usb_mouse[0].endpoint = usb_descriptor_devices[interface_number].endpoint_interrupt;
   usb_mouse[0].endpoint_size = usb_descriptor_devices[interface_number].endpoint_interrupt_size;
   usb_mouse[0].interrupt_time = usb_descriptor_devices[interface_number].endpoint_interrupt_time;
   usb_mouse[0].transfer_descriptor_check = usb_interrupt_transfer(controller_number, 0, usb_device_address, device_speed, usb_descriptor_devices[interface_number].endpoint_interrupt, usb_descriptor_devices[interface_number].endpoint_interrupt_size, usb_descriptor_devices[interface_number].endpoint_interrupt_time, usb_mouse_data_memory);
   usb_mouse[0].controller_type = usb_controllers[controller_number].type;

   log("\n");
  }
  else if(usb_descriptor_devices[interface_number].type==USB_DEVICE_KEYBOARD) {
   log("USB: USB keyboard");

   if(usb_keyboard[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
    log(" replacing already attached keyboard");
    usb_keyboard[0].controller_type = USB_NO_DEVICE_ATTACHED;
    release_usb_address(usb_keyboard[0].address);
   }
   usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].configuration, usb_descriptor_devices[interface_number].interface, usb_descriptor_devices[interface_number].alternative_interface);
   usb_hid_device_set_protocol(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].interface, USB_BOOT_PROTOCOL);
   usb_hid_device_set_idle(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].interface);

   usb_keyboard[0].controller_number = controller_number;
   usb_keyboard[0].port = device_port;
   usb_keyboard[0].address = usb_device_address;
   usb_keyboard[0].device_speed = device_speed;
   usb_keyboard[0].endpoint = usb_descriptor_devices[interface_number].endpoint_interrupt;
   usb_keyboard[0].endpoint_size = usb_descriptor_devices[interface_number].endpoint_interrupt_size;
   usb_keyboard[0].interrupt_time = usb_descriptor_devices[interface_number].endpoint_interrupt_time;
   usb_keyboard[0].transfer_descriptor_check = usb_interrupt_transfer(controller_number, 1, usb_device_address, device_speed, usb_descriptor_devices[interface_number].endpoint_interrupt, usb_descriptor_devices[interface_number].endpoint_interrupt_size, usb_descriptor_devices[interface_number].endpoint_interrupt_time, usb_keyboard_data_memory);
   usb_keyboard[0].controller_type = usb_controllers[controller_number].type;

   log("\n");
  }
  else if(usb_descriptor_devices[0].type==USB_DEVICE_MASS_STORAGE) {
   log("USB: USB mass storage device");
   
   for(int i=0; i<10; i++) {
    if(usb_mass_storage_devices[i].type==USB_MSD_NOT_ATTACHED) {
     usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].configuration, usb_descriptor_devices[interface_number].interface, usb_descriptor_devices[interface_number].alternative_interface);
    
     usb_mass_storage_devices[i].type = USB_MSD_ATTACHED;
     usb_mass_storage_devices[i].controller_type = usb_controllers[controller_number].type;
     usb_mass_storage_devices[i].controller_number = controller_number;
     usb_mass_storage_devices[i].interface = usb_descriptor_devices[interface_number].interface;
     usb_mass_storage_devices[i].port = device_port;
     usb_mass_storage_devices[i].device_speed = device_speed;
     usb_mass_storage_devices[i].address = usb_device_address;
     usb_mass_storage_devices[i].endpoint_in = usb_descriptor_devices[0].endpoint_bulk_in;
     usb_mass_storage_devices[i].toggle_in = 0;
     usb_mass_storage_devices[i].endpoint_out = usb_descriptor_devices[0].endpoint_bulk_out;
     usb_mass_storage_devices[i].toggle_out = 0;
     usb_mass_storage_devices[i].ehci_hub_address = ehci_hub_address;
     usb_mass_storage_devices[i].ehci_hub_port_number = ehci_hub_port_number;
     
     usb_mass_storage_initalize(i);
     
     //add partitions to device list
     select_storage_medium(MEDIUM_USB_MSD, i);
     read_partition_info();
     for(dword_t j=0; j<8; j++) {
      if(partitions[j].type==STORAGE_NO_PARTITION) {
       break; //we went through all partitions
      }

      //add partition
      if(partitions[j].type!=STORAGE_FREE_SPACE && partitions[j].type!=STORAGE_UNKNOWN_FILESYSTEM) {
       add_device_partition_to_device_list(MEDIUM_USB_MSD, i, partitions[j].type, partitions[j].first_sector);
      }
     }

     log("\n");
     return;
    }
   }
   
   log(" more than 10 devices connected\n");
  }
  else if(usb_descriptor_devices[interface_number].type==USB_DEVICE_HUB) {
   log("USB: USB hub ");

   for(int i=0; i<10; i++) {
    if(usb_hubs[i].controller_type==USB_NO_DEVICE_ATTACHED) {
     usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_descriptor_devices[interface_number].configuration, usb_descriptor_devices[interface_number].interface, usb_descriptor_devices[interface_number].alternative_interface);
    
     usb_hubs[i].number_of_ports = usb_read_hub_number_of_port(controller_number, usb_device_address, device_speed);
     if(usb_hubs[i].number_of_ports==0) {
      log("error reading hub descriptor");
      return; //there was error with transfer
     }
     else if(usb_hubs[i].number_of_ports>8) {
      log("too many hub ports ");
      log_var(usb_hubs[i].number_of_ports);
      usb_hubs[i].number_of_ports = 8;
     }
     else {
      log_var(usb_hubs[i].number_of_ports);
     }

     usb_hubs[i].controller_type = usb_controllers[controller_number].type;
     usb_hubs[i].controller_number = controller_number;
     usb_hubs[i].port = device_port;
     usb_hubs[i].device_speed = device_speed;
     usb_hubs[i].address = usb_device_address;
     usb_hubs[i].ehci_hub_address = ehci_hub_address;
     usb_hubs[i].ehci_hub_port_number = ehci_hub_port_number;

     //set power for all devices
     for(dword_t j=0; j<usb_hubs[i].number_of_ports; j++) {
      usb_hubs[i].ports_state[j]=0;
      usb_hubs[i].ports_device_speed[j]=0;
      usb_hub_set_feature(controller_number, usb_device_address, device_speed, (j+1), 0x8);
     }
     wait(50);

     //read connected devices
     detect_usb_devices_on_hub(i);

     log("\n");
     return;
    }
   }
   
   log("more than 10 devices connected\n");
  }
  else {
   log("USB: device ");
   log_hex_specific_size(usb_descriptor_devices[interface_number].type, 6);
   log("\n");
  }
 }
}

byte_t get_free_usb_address(void) {
 for(dword_t i=0; i<127; i++) {
  if(usb_addresses[i]==0) {
   usb_device_address = (i+1);
   usb_addresses[i] = 1;
   log("\nUSB device: address ");
   log_var(usb_device_address);
   return STATUS_GOOD;
  }
 }

 usb_device_address = 0;
 log("\nUSB error: no free address");
 return STATUS_ERROR; //no free address
}

void release_usb_address(byte_t address) {
 usb_addresses[(address-1)] = 0;
}

void parse_usb_descriptor(dword_t descriptor_mem) {
 dword_t *descriptor32 = (dword_t *) descriptor_mem;
 
 log("\nUSB descriptor");
 log("\nUSB version of this device: ");
 if((descriptor32[0]>>16)==0x0100) {
  log("1.0");
 }
 else if((descriptor32[0]>>16)==0x0110) {
  log("1.1");
 }
 else if((descriptor32[0]>>16)==0x0200) {
  log("2.0");
 }
 else {
  log_hex((descriptor32[0]>>16));
 }
 log("\nDevice type: ");
 if((descriptor32[1] & 0xFFFFFF)==0x000000) {
  log("in configuration descriptor");
 }
 else {
  log_hex((descriptor32[1] & 0xFFFFFF));
 }
 log("\nControl endpoint: ");
 usb_control_endpoint_size = (descriptor32[1]>>24);
 if((descriptor32[1]>>24)==8) {
  log("low speed");
 }
 else if((descriptor32[1]>>24)==64) {
  log("full speed");
 }
 else {
  log_var((descriptor32[1]>>24));
 }
 log("\nVendor ID: ");
 log_hex((descriptor32[2] & 0xFFFF));
 log("\nDevice ID: ");
 log_hex((descriptor32[2]>>16));
 log("\nDevice version: ");
 log_hex((descriptor32[3] & 0xFFFF));
 log("\nConfigurations: ");
 log_var(((descriptor32[4]>>8) & 0xFF));
 
 log("\n"); 
}

void parse_usb_configuration_descriptor(dword_t descriptor_mem) {
 byte_t *descriptor8 = (byte_t *) descriptor_mem;
 dword_t *descriptor32 = (dword_t *) descriptor_mem;
 byte_t configuration=0, endpoint_type=0, endpoint=0;
 
 log("\nUSB configuration descriptor");
 
 for(int i=0; i<10; i++) {
  usb_descriptor_devices[i].type=0;
  usb_descriptor_devices[i].configuration=0;
  usb_descriptor_devices[i].interface=0;
  usb_descriptor_devices[i].alternative_interface=0;
  usb_descriptor_devices[i].endpoint_bulk_in=0;
  usb_descriptor_devices[i].endpoint_bulk_out=0;
  usb_descriptor_devices[i].endpoint_interrupt=0;
  usb_descriptor_devices[i].endpoint_interrupt_size=0;
  usb_descriptor_devices[i].endpoint_interrupt_time=0;
  usb_descriptor_devices[i].endpoint_interrupt_out=0;
  usb_descriptor_devices[i].endpoint_isynchronous_in=0;
 }
 
 for(int i=0, device_pointer=-1; i<20; i++) {
  if(descriptor8[0]==0) {
   log("\n");
   return;
  }
  else if(descriptor8[1]==0x02) { //configuration
   log("\nconfiguration ");
   configuration=descriptor8[5];
   log_var(configuration);
  }
  else if(descriptor8[1]==0x04) { //interface
   log("\ninterface ");
   device_pointer++;
   if(device_pointer>=10) {
    return;
   }
   usb_descriptor_devices[device_pointer].configuration=configuration;
   usb_descriptor_devices[device_pointer].interface=descriptor8[2];
   usb_descriptor_devices[device_pointer].alternative_interface=descriptor8[3];
   usb_descriptor_devices[device_pointer].type = ((descriptor8[5]<<16) | (descriptor8[6]<<8) | (descriptor8[7]));
   log_var_with_space(usb_descriptor_devices[device_pointer].interface);
   log_var_with_space(usb_descriptor_devices[device_pointer].alternative_interface);
   log_hex(usb_descriptor_devices[device_pointer].type);
  }
  else if(descriptor8[1]==0x05 && device_pointer>=0) { //endpoint
   log("\nendpoint ");
   endpoint_type = ((descriptor8[2]>>7) | ((descriptor8[3] & 0x3)<<1));
   endpoint = (descriptor8[2] & 0xF);
   if(endpoint_type==0x4) {
    log("BULK OUT ");
    usb_descriptor_devices[device_pointer].endpoint_bulk_out=endpoint;
   }
   else if(endpoint_type==0x5) {
    log("BULK IN ");
    usb_descriptor_devices[device_pointer].endpoint_bulk_in=endpoint;
   }
   else if(endpoint_type==0x7) {
    log("INTERRUPT IN ");
    log_hex_with_space(descriptor8[4]);
    log_hex_with_space(descriptor8[6]);
    usb_descriptor_devices[device_pointer].endpoint_interrupt_size=descriptor8[4];
    usb_descriptor_devices[device_pointer].endpoint_interrupt_time=descriptor8[6];
    usb_descriptor_devices[device_pointer].endpoint_interrupt=endpoint;
   }
   else if(endpoint_type==0x6) {
    log("INTERRUPT OUT ");
    log_hex_with_space(descriptor8[4]);
    log_hex_with_space(descriptor8[6]);
    usb_descriptor_devices[device_pointer].endpoint_interrupt_out=endpoint;
   }
   else if(endpoint_type==0x0) {
    log("CONTROL OUT ");
   }
   else if(endpoint_type==0x1) {
    log("CONTROL IN ");
   }
   else if(endpoint_type==0x3) {
    log("ISYNCHRONOUS IN ");
    usb_descriptor_devices[device_pointer].endpoint_isynchronous_in=endpoint;
   }
   else if(endpoint_type==0x2) {
    log("ISYNCHRONOUS OUT ");
   }
   log_var(endpoint);
  }
  else if(descriptor8[1]==0x21 && device_pointer>=0) { //HID descriptor
   //TODO:
   log("\nHID descriptor: ");
   for(int j=0; j<descriptor8[5]; j++) {
    log_hex_with_space(descriptor8[6+j*2]);
    log_hex_with_space(descriptor8[7+j*2]);
    
    if(descriptor8[6+j*2]==0x22) {
     usb_descriptor_devices[device_pointer].hid_descriptor_length=descriptor8[7+j*2];
    }
   }
  }
  
  //next item
  descriptor8 += ((dword_t)descriptor8[0]);
 }
 
 log("\n");
}

void parse_hid_descriptor(dword_t descriptor_mem) {
 byte_t *descriptor8 = (byte_t *) descriptor_mem;
 dword_t packet_input_offset = calloc(256);
 byte_t *packet_input_offset_ptr = (byte_t *) packet_input_offset;
 dword_t packet_input_length = calloc(256);
 byte_t *packet_input_length_ptr = (byte_t *) packet_input_length;
 dword_t packet_input_type = calloc(256);
 byte_t *packet_input_type_ptr = (byte_t *) packet_input_type;
 dword_t packet_input_actual_offset = 0;
 dword_t packet_input_item = 0;
 dword_t packet_input_type_item = 0;
 dword_t report_size = 0, report_count = 0;
  
 for(int i=0; i<1000; i++) { 
  if(descriptor8[0]==0) {
   break;
  }
  else if(descriptor8[0]==0x85) { //report ID
   packet_input_actual_offset += 8;
  }
  else if(descriptor8[0]==0x75) { //report size
   report_size = descriptor8[1];
  }
  else if(descriptor8[0]==0x95) { //report count
   report_count = descriptor8[1];
  }
  else if(descriptor8[0]==0x81) { //insert input
   for(int i=0; i<report_count; i++) {
    packet_input_offset_ptr[packet_input_item]=packet_input_actual_offset;
    packet_input_length_ptr[packet_input_item]=report_size;
    packet_input_actual_offset += report_size;
    packet_input_item++;
   }
   if(packet_input_type_item<packet_input_item) {
    packet_input_type_item=packet_input_item;
   }
  }
  else if(descriptor8[0]==0x09) { //usage
   if(descriptor8[1]==0x30) {
    packet_input_type_ptr[packet_input_type_item] = USB_HID_USAGE_MOVEMENT_X;
    packet_input_type_item++;
   }
   else if(descriptor8[1]==0x31) {
    packet_input_type_ptr[packet_input_type_item] = USB_HID_USAGE_MOVEMENT_Y;
    packet_input_type_item++;
   }
   else if(descriptor8[1]==0x38) {
    packet_input_type_ptr[packet_input_type_item] = USB_HID_USAGE_WHEEL;
    packet_input_type_item++;
   }
  }
  else if(descriptor8[0]==0x05) { //usage page
   if(descriptor8[1]==0x09) {
    packet_input_type_ptr[packet_input_type_item] = USB_HID_USAGE_BUTTONS;
    packet_input_type_item++;
   }
  }
  //TODO: parse all buttons separately
  
  //next item
  if(descriptor8[0]==0xFE) {
   descriptor8 += ((dword_t)descriptor8[1]+1);
  }
  else {
   descriptor8 += ((dword_t)(descriptor8[0] & 0x3)+1);
  }
 }
 
 log("\nPARSED HID DESCRIPTOR:");
 for(int i=0; i<256; i++) {
  if(packet_input_length_ptr[i]==0) {
   break;
  }
  log("\n");
  log_var_with_space(packet_input_offset_ptr[i]);
  log_var_with_space(packet_input_length_ptr[i]);
  if(packet_input_type_ptr[i]==USB_HID_USAGE_BUTTONS) {
   log("BUTTONS");
  }
  else if(packet_input_type_ptr[i]==USB_HID_USAGE_MOVEMENT_X) {
   log("X");
  }
  else if(packet_input_type_ptr[i]==USB_HID_USAGE_MOVEMENT_Y) {
   log("Y");
  }
  else if(packet_input_type_ptr[i]==USB_HID_USAGE_WHEEL) {
   log("WHEEL");
  }
 }
 log("\n");
 
 //get useful values from HID output
 //example: data length is 12 bits from bit 10 - offset byte is 1, shift is 2, length is 0xFFF
 usb_mouse_movement_x_data_length = 0;
 usb_mouse_movement_y_data_length = 0;
 usb_mouse_movement_wheel_data_length = 0;
 for(int i=0; i<256; i++) {
  if(packet_input_type_ptr[i]==USB_HID_USAGE_BUTTONS) {
   usb_mouse_buttons_data_offset_byte = (packet_input_offset_ptr[i]/8);
   usb_mouse_buttons_data_offset_shift = (packet_input_offset_ptr[i]%8);
  }
  else if(packet_input_type_ptr[i]==USB_HID_USAGE_MOVEMENT_X) {
   usb_mouse_movement_x_data_offset_byte = (packet_input_offset_ptr[i]/8);
   usb_mouse_movement_x_data_offset_shift = (packet_input_offset_ptr[i]%8);
   usb_mouse_movement_x_data_length = ~(0xFFFFFFFF << packet_input_length_ptr[i]);
  }
  else if(packet_input_type_ptr[i]==USB_HID_USAGE_MOVEMENT_Y) {
   usb_mouse_movement_y_data_offset_byte = (packet_input_offset_ptr[i]/8);
   usb_mouse_movement_y_data_offset_shift = (packet_input_offset_ptr[i]%8);
   usb_mouse_movement_y_data_length = ~(0xFFFFFFFF << packet_input_length_ptr[i]);
  }
  else if(packet_input_type_ptr[i]==USB_HID_USAGE_WHEEL) {
   usb_mouse_movement_wheel_data_offset_byte = (packet_input_offset_ptr[i]/8);
   usb_mouse_movement_wheel_data_offset_shift = (packet_input_offset_ptr[i]%8);
   usb_mouse_movement_wheel_data_length = ~(0xFFFFFFFF << packet_input_length_ptr[i]);
  }
 }
 
 free(packet_input_offset);
 free(packet_input_length);
 free(packet_input_type);
}