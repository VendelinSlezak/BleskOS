//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_usb_controllers(void) {
 //all USB addresses are available
 for(dword_t i=0; i<32; i++) {
  usb_addresses[i]=0;
 }

 //no USB device is connected
 for(int i=0; i<usb_controllers_pointer; i++) {
  for(int j=0; j<16; j++) {
   usb_controllers[i].ports_state[j]=PORT_NO_DEVICE;
   usb_controllers[i].ports_device_speed[j]=0;
  }
 }

 usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;

 usb_keyboard[0].controller_type = USB_NO_DEVICE_ATTACHED;

 for(int i=0; i<MAX_NUMBER_OF_USB_MSD_DEVICES; i++) {
  usb_mass_storage_devices[i].entry = USB_MSD_ENTRY_NOTHING_ATTACHED;
  usb_mass_storage_devices[i].controller_type = 0;
 }

 for(int i=0; i<MAX_NUMBER_OF_USB_HUBS; i++) {
  usb_hubs[i].controller_type = USB_NO_DEVICE_ATTACHED;
  for(dword_t j=0; j<8; j++) {
   usb_hubs[i].ports_state[j] = PORT_NO_DEVICE;
   usb_hubs[i].ports_device_speed[j] = 0;
  }
 }
 
 for(dword_t i=0; i<127; i++) {
  usb_unidentified_connected_devices[i].controller_number = 0xFF;
 }

 //return if there are no USB controllers
 if(usb_controllers_pointer==0) {
  log("\nNo USB controllers");
  return;
 }
 
 //allocate memory for USB devices
 usb_setup_packet = (struct usb_setup_packet_t *) (malloc(16));
 usb_setup_packet_data = (byte_t *) (malloc(USB_SETUP_PACKET_MAX_SIZE_OF_DATA));

 usb_mouse_data_memory = malloc(8);

 usb_keyboard_data_memory = calloc(8);

 usb_hub_transfer_setup_packets_mem = malloc(8*8);
 usb_hub_transfer_data_mem = malloc(4*8);

 usb_command_block_wrapper = (struct usb_command_block_wrapper_t *) (malloc(sizeof(struct usb_command_block_wrapper_t)));
 usb_command_status_wrapper = (struct usb_command_status_wrapper_t *) (malloc(sizeof(struct usb_command_status_wrapper_t)));

 hid_parsed_entries = (struct hid_parsed_entry_t *) (calloc(sizeof(struct hid_parsed_entry_t)*MAX_NUMBER_OF_HID_PARSED_ENTRIES));
 
 //initalize EHCI USB controllers
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_EHCI) {
   initalize_ehci_controller(i);
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
  }
 }
 
 //detect connected USB devices
 detect_usb_devices();

 //detect devices connected to USB HUBs
 detect_usb_devices_on_hubs();

 //add task to continue checking for changes in USB HUBs connections
 create_task(detect_usb_devices_on_hubs, TASK_TYPE_USER_INPUT, 250);
}

void detect_usb_devices(void) {
 //detect devices connected to EHCI
 //we need to check EHCI before UHCI and OHCI, because we will pass connected low/full speed devices to them
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
 for(dword_t i=0; i<MAX_NUMBER_OF_USB_HUBS; i++) {
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

   //set location of this device for EHCI transfers
   if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
    ehci_hub_address = 0; //device is not connected to HUB
    ehci_hub_port_number = 0;
   }

   //configure device
   log("\n");
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

 //set location of this device for EHCI transfers
 if(usb_hubs[hub_number].controller_type==USB_CONTROLLER_EHCI) {
  ehci_hub_address = usb_hubs[hub_number].ehci_hub_address;
  ehci_hub_port_number = usb_hubs[hub_number].ehci_hub_port_number;
 }

 //read if there is some connection status change
 byte_t hub_change = usb_hub_is_there_some_port_connection_status_change(hub_number);
 if(hub_change==USB_HUB_IS_NOT_RESPONDING) {
  log("\nUSB: hub is not responding");

  //remove HUB if it is not responding
  usb_remove_device(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].port);

  //we removed HUB so there was change of device
  usb_new_device_detected = STATUS_TRUE;

  return;
 }
 else if(hub_change==STATUS_FALSE) { //no connection change
  return;
 }
 else { //some connection change
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
   usb_remove_device(usb_hubs[hub_number].controller_number, CALCULATE_ADDRESS_OF_DEVICE_ON_USB_HUB(hub_number, port));

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

   //set location of this device for EHCI transfers
   if(usb_hubs[hub_number].controller_type==USB_CONTROLLER_EHCI) {
    ehci_hub_address = usb_hubs[hub_number].address;
    ehci_hub_port_number = (port+1);
   }

   //find if this is low/full/high speed device
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
   log("\nUSB hub: enabled device\n");
   usb_configure_device_with_zero_address(usb_hubs[hub_number].controller_number, CALCULATE_ADDRESS_OF_DEVICE_ON_USB_HUB(hub_number, port), usb_hubs[hub_number].ports_device_speed[port]);
  }
  else if(usb_hubs[hub_number].ports_state[port]!=PORT_NO_DEVICE) {
   //remove device
   usb_remove_device(usb_hubs[hub_number].controller_number, CALCULATE_ADDRESS_OF_DEVICE_ON_USB_HUB(hub_number, port));
   
   //clear status change
   usb_hub_clear_feature(usb_hubs[hub_number].controller_number, usb_hubs[hub_number].address, usb_hubs[hub_number].device_speed, (port+1), 0x10);

   //save port status change
   usb_hubs[hub_number].ports_state[port] = PORT_NO_DEVICE;
   log("\nUSB hub: device removed");
  }
 }
}

void usb_remove_device(byte_t controller_number, byte_t port_number) {
 //remove USB mouse
 if(usb_mouse[0].controller_number==controller_number && usb_mouse[0].port==port_number) {
  usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;
  usb_mouse[0].controller_number = 0;
  usb_mouse[0].port = 0;
  release_usb_address(usb_mouse[0].address);
 }

 //remove USB keyboard
 if(usb_keyboard[0].controller_number==controller_number && usb_keyboard[0].port==port_number) {
  usb_keyboard[0].controller_type = USB_NO_DEVICE_ATTACHED;
  usb_keyboard[0].controller_number = 0;
  usb_keyboard[0].port = 0;
  release_usb_address(usb_keyboard[0].address);
 }

 //remove USB mass storage device
 for(int i=0; i<MAX_NUMBER_OF_USB_MSD_DEVICES; i++) {
  if(usb_mass_storage_devices[i].entry!=USB_MSD_ENTRY_NOTHING_ATTACHED && usb_mass_storage_devices[i].controller_number==controller_number && usb_mass_storage_devices[i].port==port_number) {
   usb_mass_storage_devices[i].entry = USB_MSD_ENTRY_NOTHING_ATTACHED;
   usb_mass_storage_devices[i].controller_type = 0;
   usb_mass_storage_devices[i].controller_number = 0;
   usb_mass_storage_devices[i].port = 0;
   release_usb_address(usb_mass_storage_devices[i].address);

   //remove connected partitions from partition list
   remove_partitions_of_medium_from_list(MEDIUM_USB_MSD, i);
  }
 }

 //remove USB hub
 for(int i=0; i<MAX_NUMBER_OF_USB_HUBS; i++) {
  if(usb_hubs[i].controller_type!=USB_NO_DEVICE_ATTACHED && usb_hubs[i].controller_number==controller_number && usb_hubs[i].port==port_number) {
   //remove devices connected to hub
   for(dword_t j=0; j<16; j++) {
    usb_remove_device(usb_hubs[i].controller_number, CALCULATE_ADDRESS_OF_DEVICE_ON_USB_HUB(i, j));
   }

   usb_hubs[i].controller_type = USB_NO_DEVICE_ATTACHED;
   usb_hubs[i].controller_number = 0;
   usb_hubs[i].port = 0;
   release_usb_address(usb_hubs[i].address);
  }
 }

 //remove unidentified device
 for(dword_t i=0; i<127; i++) {
  if(usb_unidentified_connected_devices[i].controller_number==controller_number && usb_unidentified_connected_devices[i].port_number==port_number) {
   usb_unidentified_connected_devices[i].controller_number = 0xFF;
   release_usb_address(usb_unidentified_connected_devices[i].address);
  }
 }
}

void detect_status_change_of_usb_devices(void) { 
 //return if some device change was already detected
 if(usb_new_device_detected==1) {
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
 else {
  return STATUS_ERROR;
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
 else {
  return STATUS_ERROR;
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
 else {
  return STATUS_ERROR;
 }
}

void usb_create_setup_data(byte_t request_type, byte_t request, word_t value, word_t index, word_t length) {
 //set values of packet
 usb_setup_packet->request_type = request_type;
 usb_setup_packet->request = request;
 usb_setup_packet->value = value;
 usb_setup_packet->index = index;
 usb_setup_packet->length = length;

 //clear space for data that will be transferred
 clear_memory((dword_t)usb_setup_packet_data, USB_SETUP_PACKET_MAX_SIZE_OF_DATA);
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
 else {
  return STATUS_ERROR;
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
 else {
  return STATUS_ERROR;
 }
}

byte_t usb_control_transfer_with_dynamic_data_length(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index) {
 byte_t status = STATUS_ERROR;

 //read first 8 bytes to get length of data
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
 else {
  return STATUS_ERROR;
 }
 if(status==STATUS_ERROR) {
  log("\nUSB: error with control transfer 8 bytes");
  return STATUS_ERROR;
 }

 //read length of data
 word_t *data_length_pointer = (word_t *) ((dword_t)usb_setup_packet_data+2);
 word_t data_length = (*data_length_pointer); //save value
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
 else {
  return STATUS_ERROR;
 }
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
 else {
  return STATUS_ERROR;
 }
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
 else {
  return STATUS_ERROR;
 }
}

byte_t usb_set_configuration_and_interface(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t configuration, byte_t interface, byte_t alternative_interface) {
 //set configuration
 if(usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x00, 0x09, configuration, 0)==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //set interface
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
 byte_t *usb_descriptor_control_endpoint_length_size_pointer = (byte_t *) ((dword_t)usb_setup_packet_data+7);
 usb_control_endpoint_size = (*usb_descriptor_control_endpoint_length_size_pointer);
 if(usb_control_endpoint_size==0) {
  log("\ninvalid control endpoint length");
  return;
 }

 //get address for device
 usb_device_address = get_free_usb_address();
 if(usb_device_address==STATUS_ERROR) {
  return; //no free address, go to next device
 }
 log("\nUSB device: address ");
 log_var(usb_device_address);
  
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
  parse_usb_descriptor((dword_t)usb_setup_packet_data);
  return;
 }
 else {
  parse_usb_descriptor((dword_t)usb_setup_packet_data);
 }

 //read configuration descriptor
 if(usb_control_transfer_with_dynamic_data_length(controller_number, usb_device_address, device_speed, 0x80, 0x06, 0x0200, 0)==STATUS_ERROR) {
  release_usb_address(usb_device_address);

  log("\nerror during reading configuration descriptor");
  return;
 }

 //parse configuration descriptor
 parse_usb_configuration_descriptor((dword_t)usb_setup_packet_data);

 //we support max 2 interfaces on one device
 for(dword_t interface_number=0; interface_number<2; interface_number++) {
  if(usb_device_interfaces[interface_number].type==0x000000) {
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
  
  if(usb_device_interfaces[interface_number].type==USB_DEVICE_MOUSE) {
   log("USB: USB mouse");

   //if there was already connected USB mouse, replace it with new mouse
   if(usb_mouse[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
    log(" replacing already attached mouse");
    usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;
    release_usb_address(usb_mouse[0].address);
   }

   //configure USB mouse
   usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].configuration, usb_device_interfaces[interface_number].interface, usb_device_interfaces[interface_number].alternative_interface);
   usb_hid_device_set_protocol(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].interface, USB_HID_PROTOCOL);
   usb_hid_device_set_idle(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].interface);

   //parse USB mouse HID
   usb_mouse_read_hid_descriptor(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].interface, usb_device_interfaces[interface_number].hid_descriptor_0x22_length);
   if(get_hid_descriptor_entry(USB_HID_USAGE_BUTTON, 0x01)==STATUS_GOOD) {
    usb_mouse_button_1_data_offset_byte = hid_entry_offset_byte;
    usb_mouse_button_1_data_offset_shift = hid_entry_offset_shift;
    usb_mouse_button_1_data_length = 1;
   }
   else {
    usb_mouse_button_1_data_length = 0;
   }
   if(get_hid_descriptor_entry(USB_HID_USAGE_BUTTON, 0x02)==STATUS_GOOD) {
    usb_mouse_button_2_data_offset_byte = hid_entry_offset_byte;
    usb_mouse_button_2_data_offset_shift = hid_entry_offset_shift;
    usb_mouse_button_2_data_length = 1;
   }
   else {
    usb_mouse_button_2_data_length = 0;
   }
   if(get_hid_descriptor_entry(USB_HID_USAGE_BUTTON, 0x03)==STATUS_GOOD) {
    usb_mouse_button_3_data_offset_byte = hid_entry_offset_byte;
    usb_mouse_button_3_data_offset_shift = hid_entry_offset_shift;
    usb_mouse_button_3_data_length = 1;
   }
   else {
    usb_mouse_button_3_data_length = 0;
   }
   if(get_hid_descriptor_entry(USB_HID_USAGE_MOVEMENT_X, 0x00)==STATUS_GOOD) {
    usb_mouse_movement_x_data_offset_byte = hid_entry_offset_byte;
    usb_mouse_movement_x_data_offset_shift = hid_entry_offset_shift;
    usb_mouse_movement_x_data_length = hid_entry_offset_length;
   }
   else {
    usb_mouse_movement_x_data_length = 0;
   }
   if(get_hid_descriptor_entry(USB_HID_USAGE_MOVEMENT_Y, 0x00)==STATUS_GOOD) {
    usb_mouse_movement_y_data_offset_byte = hid_entry_offset_byte;
    usb_mouse_movement_y_data_offset_shift = hid_entry_offset_shift;
    usb_mouse_movement_y_data_length = hid_entry_offset_length;
   }
   else {
    usb_mouse_movement_y_data_length = 0;
   }
   if(get_hid_descriptor_entry(USB_HID_USAGE_WHEEL, 0x00)==STATUS_GOOD) {
    usb_mouse_movement_wheel_data_offset_byte = hid_entry_offset_byte;
    usb_mouse_movement_wheel_data_offset_shift = hid_entry_offset_shift;
    usb_mouse_movement_wheel_data_length = hid_entry_offset_length;
   }
   else {
    usb_mouse_movement_wheel_data_length = 0;
   }

   //save USB mouse variables
   usb_mouse[0].controller_number = controller_number;
   usb_mouse[0].port = device_port;
   usb_mouse[0].address = usb_device_address;
   usb_mouse[0].device_speed = device_speed;
   usb_mouse[0].endpoint = usb_device_interfaces[interface_number].endpoint_interrupt;
   usb_mouse[0].endpoint_size = usb_device_interfaces[interface_number].endpoint_interrupt_size;
   usb_mouse[0].interrupt_time = usb_device_interfaces[interface_number].endpoint_interrupt_time;
   usb_mouse[0].transfer_descriptor_check = usb_interrupt_transfer(controller_number, 0, usb_device_address, device_speed, usb_device_interfaces[interface_number].endpoint_interrupt, usb_device_interfaces[interface_number].endpoint_interrupt_size, usb_device_interfaces[interface_number].endpoint_interrupt_time, usb_mouse_data_memory);
   usb_mouse[0].controller_type = usb_controllers[controller_number].type;
  }
  else if(usb_device_interfaces[interface_number].type==USB_DEVICE_KEYBOARD) {
   log("USB: USB keyboard");

   //if there was already connected USB keyboard, replace it with new keyboard
   if(usb_keyboard[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
    log(" replacing already attached keyboard");
    usb_keyboard[0].controller_type = USB_NO_DEVICE_ATTACHED;
    release_usb_address(usb_keyboard[0].address);
   }

   //configure USB keyboard
   usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].configuration, usb_device_interfaces[interface_number].interface, usb_device_interfaces[interface_number].alternative_interface);
   usb_hid_device_set_protocol(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].interface, USB_BOOT_PROTOCOL);
   usb_hid_device_set_idle(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].interface);

   //save USB keyboard variables
   usb_keyboard[0].controller_number = controller_number;
   usb_keyboard[0].port = device_port;
   usb_keyboard[0].address = usb_device_address;
   usb_keyboard[0].device_speed = device_speed;
   usb_keyboard[0].endpoint = usb_device_interfaces[interface_number].endpoint_interrupt;
   usb_keyboard[0].endpoint_size = usb_device_interfaces[interface_number].endpoint_interrupt_size;
   usb_keyboard[0].interrupt_time = usb_device_interfaces[interface_number].endpoint_interrupt_time;
   usb_keyboard[0].transfer_descriptor_check = usb_interrupt_transfer(controller_number, 1, usb_device_address, device_speed, usb_device_interfaces[interface_number].endpoint_interrupt, usb_device_interfaces[interface_number].endpoint_interrupt_size, usb_device_interfaces[interface_number].endpoint_interrupt_time, usb_keyboard_data_memory);
   usb_keyboard[0].controller_type = usb_controllers[controller_number].type;
  }
  else if(usb_device_interfaces[interface_number].type==USB_DEVICE_MASS_STORAGE) {
   log("USB: USB mass storage device");
   
   //find free entry for new USB mass storage device
   for(int i=0; i<MAX_NUMBER_OF_USB_MSD_DEVICES; i++) {
    //free entry
    if(usb_mass_storage_devices[i].entry==USB_MSD_ENTRY_NOTHING_ATTACHED) {
     //configure USB mass storage device
     usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].configuration, usb_device_interfaces[interface_number].interface, usb_device_interfaces[interface_number].alternative_interface);
    
     //save USB mass storage device variables
     usb_mass_storage_devices[i].entry = USB_MSD_ENTRY_DEVICE_ATTACHED;
     usb_mass_storage_devices[i].controller_type = usb_controllers[controller_number].type;
     usb_mass_storage_devices[i].controller_number = controller_number;
     usb_mass_storage_devices[i].interface = usb_device_interfaces[interface_number].interface;
     usb_mass_storage_devices[i].port = device_port;
     usb_mass_storage_devices[i].device_speed = device_speed;
     usb_mass_storage_devices[i].address = usb_device_address;
     usb_mass_storage_devices[i].endpoint_in = usb_device_interfaces[interface_number].endpoint_bulk_in;
     usb_mass_storage_devices[i].toggle_in = 0;
     usb_mass_storage_devices[i].endpoint_out = usb_device_interfaces[interface_number].endpoint_bulk_out;
     usb_mass_storage_devices[i].toggle_out = 0;
     usb_mass_storage_devices[i].ehci_hub_address = ehci_hub_address;
     usb_mass_storage_devices[i].ehci_hub_port_number = ehci_hub_port_number;
     
     //initalize USB mass storage device
     usb_mass_storage_initalize(i);
     
     //connect all partitions of USB mass storage device
     if(usb_mass_storage_devices[i].entry==USB_MSD_ENTRY_DEVICE_INITALIZED) {
      connect_partitions_of_medium(MEDIUM_USB_MSD, i);
     }

     break;
    }
   }
  }
  else if(usb_device_interfaces[interface_number].type==USB_DEVICE_HUB) {
   log("USB: USB hub ");

   //find free entry for new USB hub
   for(int i=0; i<MAX_NUMBER_OF_USB_HUBS; i++) {
    //free entry
    if(usb_hubs[i].controller_type==USB_NO_DEVICE_ATTACHED) {
     //configure USB hub
     usb_set_configuration_and_interface(controller_number, usb_device_address, device_speed, usb_device_interfaces[interface_number].configuration, usb_device_interfaces[interface_number].interface, usb_device_interfaces[interface_number].alternative_interface);

     //read number of ports of USB hub
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

     //save USB hub variables
     usb_hubs[i].controller_type = usb_controllers[controller_number].type;
     usb_hubs[i].controller_number = controller_number;
     usb_hubs[i].port = device_port;
     usb_hubs[i].device_speed = device_speed;
     usb_hubs[i].address = usb_device_address;
     usb_hubs[i].ehci_hub_address = ehci_hub_address;
     usb_hubs[i].ehci_hub_port_number = ehci_hub_port_number;

     //set power for all ports on USB hub
     for(dword_t j=0; j<usb_hubs[i].number_of_ports; j++) {
      usb_hubs[i].ports_state[j]=0;
      usb_hubs[i].ports_device_speed[j]=0;
      usb_hub_set_feature(controller_number, usb_device_address, device_speed, (j+1), 0x8);
     }
     wait(50);

     //read connected devices on USB hub
     detect_usb_devices_on_hub(i);

     break;
    }
   }
  }
  else {
   log("USB: device ");
   log_hex_specific_size(usb_device_interfaces[interface_number].type, 6);

   //save device to array
   for(dword_t i=0; i<127; i++) {
    if(usb_unidentified_connected_devices[i].controller_number==0xFF) {
     usb_unidentified_connected_devices[i].controller_number = controller_number;
     usb_unidentified_connected_devices[i].port_number = device_port;
     usb_unidentified_connected_devices[i].address = usb_device_address;
     break;
    }
   }
  }
 }
}

byte_t get_free_usb_address(void) {
 //find unused address
 for(dword_t i=0; i<32; i++) {
  if(usb_addresses[i]==0) {
   //set address as used
   usb_addresses[i] = 1;

   //zero address is reserved, so we need to add 1
   return (i+1);
  }
 }

 //all addresses are used
 log("\nUSB error: no free address");
 return STATUS_ERROR;
}

void release_usb_address(byte_t address) {
 //set address as unused, because zero address is reserved, we need to subtract 1
 usb_addresses[(address-1)] = 0;
}

void parse_usb_descriptor(dword_t descriptor_mem) {
 struct usb_device_descriptor_t *usb_device_descriptor = (struct usb_device_descriptor_t *) (descriptor_mem);
 
 log("\nUSB descriptor");
 log("\n USB version of this device: ");
 if(usb_device_descriptor->usb_specification_release_number==0x0100) {
  log("1.0");
 }
 else if(usb_device_descriptor->usb_specification_release_number==0x0110) {
  log("1.1");
 }
 else if(usb_device_descriptor->usb_specification_release_number==0x0200) {
  log("2.0");
 }
 else {
  log_hex_specific_size(usb_device_descriptor->usb_specification_release_number, 4);
 }
 log("\n Device type: ");
 if(usb_device_descriptor->device_class==0x00 && usb_device_descriptor->device_subclass==0x00 && usb_device_descriptor->device_protocol==0x00) {
  log("in configuration descriptor");
 }
 else {
  log_hex_specific_size_with_space(usb_device_descriptor->device_class, 2);
  log_hex_specific_size_with_space(usb_device_descriptor->device_subclass, 2);
  log_hex_specific_size(usb_device_descriptor->device_protocol, 2);
 }
 log("\n Control endpoint: ");
 usb_control_endpoint_size = usb_device_descriptor->size_of_control_endpoint;
 if(usb_control_endpoint_size==8) {
  log("low speed");
 }
 else if(usb_control_endpoint_size==64) {
  log("full speed");
 }
 else {
  log_var(usb_control_endpoint_size);
 }
 log("\n Vendor ID: ");
 log_hex(usb_device_descriptor->vendor_id);
 log("\n Product ID: ");
 log_hex(usb_device_descriptor->product_id);
 log("\n Device version: ");
 log_hex(usb_device_descriptor->device_release_number);
 log("\n Number of configurations: ");
 log_var(usb_device_descriptor->number_of_configurations);
}

void parse_usb_configuration_descriptor(dword_t descriptor_mem) {
 struct usb_header_of_descriptor_t *usb_header_of_descriptor = (struct usb_header_of_descriptor_t *) (descriptor_mem);
 struct usb_configuration_descriptor_t *usb_configuration_descriptor;
 struct usb_interface_descriptor_t *usb_interface_descriptor;
 struct usb_endpoint_descriptor_t *usb_endpoint_descriptor;
 struct usb_hid_descriptor_t *usb_hid_descriptor;
 int configuration = -1;

 log("\nUSB configuration descriptor");

 //clear array where will be output
 clear_memory((dword_t)(&usb_device_interfaces), sizeof(struct usb_device_interfaces_t)*MAX_NUMBER_OF_INTERFACES_IN_ONE_USB_DEVICE);

 //parse all entries
 int interface_pointer = -1;
 while((dword_t)usb_header_of_descriptor<(descriptor_mem+0xFFFF)) {
  //end
  if(usb_header_of_descriptor->bLength==0) {
   break;
  }

  //read type of descriptor
  log("\n descriptor type: ");
  if(usb_header_of_descriptor->bDescriptorType==USB_CONFIGURATION_DESCRIPTOR_TYPE) {
   usb_configuration_descriptor = (struct usb_configuration_descriptor_t *) usb_header_of_descriptor;

   log("configuration ");
   log_var(usb_configuration_descriptor->bConfigurationValue);

   configuration = usb_configuration_descriptor->bConfigurationValue;
  }
  else if(usb_header_of_descriptor->bDescriptorType==USB_INTERFACE_DESCRIPTOR_TYPE) {
   usb_interface_descriptor = (struct usb_interface_descriptor_t *) usb_header_of_descriptor;

   log("interface ");
   log_var_with_space(usb_interface_descriptor->bInterfaceNumber);
   log_var_with_space(usb_interface_descriptor->bAlternateSetting);
   log_hex_specific_size_with_space(usb_interface_descriptor->bInterfaceClass, 2);
   log_hex_specific_size_with_space(usb_interface_descriptor->bInterfaceSubClass, 2);
   log_hex_specific_size_with_space(usb_interface_descriptor->bInterfaceProtocol, 2);
   
   if(configuration==-1) {
    log("\n error: interface before configuration");
    return;
   }

   interface_pointer++; //we will add other values to this interface entry, so we need to increase it here
   usb_device_interfaces[interface_pointer].configuration = configuration;
   usb_device_interfaces[interface_pointer].interface = usb_interface_descriptor->bInterfaceNumber;
   usb_device_interfaces[interface_pointer].alternative_interface = usb_interface_descriptor->bAlternateSetting;
   usb_device_interfaces[interface_pointer].type = (  usb_interface_descriptor->bInterfaceClass<<16
                                                    | usb_interface_descriptor->bInterfaceSubClass<<8
                                                    | usb_interface_descriptor->bInterfaceProtocol);
  }
  else if(interface_pointer!=-1 && usb_header_of_descriptor->bDescriptorType==USB_ENDPOINT_DESCRIPTOR_TYPE) {
   usb_endpoint_descriptor = (struct usb_endpoint_descriptor_t *) usb_header_of_descriptor;
   byte_t endpoint_type = (usb_endpoint_descriptor->bmAttributes & 0b11);

   log("endpoint ");
   if(endpoint_type==USB_ENDPOINT_TYPE_CONTROL) {
    log("CONTROL");
   }
   else {
    if(usb_endpoint_descriptor->endpoint_direction==USB_ENDPOINT_DIRECTION_OUT) {
     if(endpoint_type==USB_ENDPOINT_TYPE_ISOCHRONOUS) {
      log("ISOCHRONOUS OUT");
     }
     else if(endpoint_type==USB_ENDPOINT_TYPE_BULK) {
      log("BULK OUT");

      usb_device_interfaces[interface_pointer].endpoint_bulk_out = usb_endpoint_descriptor->endpoint_number;
     }
     else { //USB_ENDPOINT_TYPE_INTERRUPT
      log("INTERRUPT OUT");
     }
    }
    else { //USB_ENDPOINT_DIRECTION_IN
     if(endpoint_type==USB_ENDPOINT_TYPE_ISOCHRONOUS) {
      log("ISOCHRONOUS IN");
     }
     else if(endpoint_type==USB_ENDPOINT_TYPE_BULK) {
      log("BULK IN");

      usb_device_interfaces[interface_pointer].endpoint_bulk_in = usb_endpoint_descriptor->endpoint_number;
     }
     else { //USB_ENDPOINT_TYPE_INTERRUPT
      log("INTERRUPT IN");

      usb_device_interfaces[interface_pointer].endpoint_interrupt = usb_endpoint_descriptor->endpoint_number;
      usb_device_interfaces[interface_pointer].endpoint_interrupt_size = usb_endpoint_descriptor->wMaxPacketSize;
      usb_device_interfaces[interface_pointer].endpoint_interrupt_time = usb_endpoint_descriptor->bInterval;
     }
    }
   }
   log(" ");
   log_var(usb_endpoint_descriptor->endpoint_number);
  }
  else if(interface_pointer!=-1 && usb_header_of_descriptor->bDescriptorType==USB_HID_DESCRIPTOR_TYPE) {
   usb_hid_descriptor = (struct usb_hid_descriptor_t *) usb_header_of_descriptor;

   for(dword_t i=0; i<usb_hid_descriptor->bNumDescriptors; i++) {
    log("HID descriptor ");
    log_hex_specific_size_with_space(usb_hid_descriptor->descriptors[i].bDescriptorType, 2);

    if(usb_hid_descriptor->descriptors[i].bDescriptorType==0x22) {
     usb_device_interfaces[interface_pointer].hid_descriptor_0x22_length = usb_hid_descriptor->descriptors[i].bDescriptorLength;
    }
   }
  }
  else {
   log_hex_specific_size_with_space(usb_header_of_descriptor->bDescriptorType, 2);
  }

  //move to next descriptor
  usb_header_of_descriptor = (struct usb_header_of_descriptor_t *) ((dword_t)usb_header_of_descriptor+usb_header_of_descriptor->bLength);
 }
}

void parse_hid_descriptor(dword_t descriptor_mem) {
 struct hid_entry_t *hid = (struct hid_entry_t *) (descriptor_mem);
 dword_t number_of_parsed_entries = 0, number_of_entry_to_insert_usage = 0;
 dword_t offset_in_bits = 0;
 dword_t report_size = 0, report_count = 0;

 clear_memory((dword_t)hid_parsed_entries, (sizeof(struct hid_parsed_entry_t)*MAX_NUMBER_OF_HID_PARSED_ENTRIES));

 log("\nPARSED HID DESCRIPTOR");

 //build buffer format
 while((dword_t)hid<(descriptor_mem+0xFFFF)) {
  //end
  if(hid->long_item.signature==0) {
   break;
  }

  //parse entry
  if(hid->long_item.signature==HID_LONG_ENTRY_SIGNATURE) { //long entry
   //TODO: support for long entry
   log("\nlong entry");
  }
  else { //short entry
   if(hid->short_item.type==HID_SHORT_ENTRY_TYPE_MAIN) {
    if(hid->short_item.tag==0x8) { //insert input
     //add offset and size of entries
     for(int i=0; i<report_count; i++) {
      hid_parsed_entries[number_of_parsed_entries].offset_in_bits = offset_in_bits;
      hid_parsed_entries[number_of_parsed_entries].length_in_bits = report_size;

      offset_in_bits += report_size;

      number_of_parsed_entries++;
      if(number_of_parsed_entries==MAX_NUMBER_OF_HID_PARSED_ENTRIES) {
       return;
      }
     }

     //move pointer
     if(number_of_entry_to_insert_usage<number_of_parsed_entries) {
      number_of_entry_to_insert_usage = number_of_parsed_entries;
     }
    }
   }
   else if(hid->short_item.type==HID_SHORT_ENTRY_TYPE_GLOBAL) {
    if(hid->short_item.tag==0x0) { //usage page
     if(hid->short_item.data[0]==USB_HID_USAGE_BUTTON) { //button
      hid_parsed_entries[number_of_entry_to_insert_usage].usage = USB_HID_USAGE_BUTTON;
     }
    }
    else if(hid->short_item.tag==0x7) { //report size
     report_size = hid->short_item.data[0];
    }
    else if(hid->short_item.tag==0x8) { //report ID
     //add entry
     hid_parsed_entries[number_of_parsed_entries].offset_in_bits = offset_in_bits;
     hid_parsed_entries[number_of_parsed_entries].length_in_bits = 8;
     hid_parsed_entries[number_of_parsed_entries].usage = USB_HID_USAGE_ID;
     hid_parsed_entries[number_of_parsed_entries].usage_value = hid->short_item.data[0];
     offset_in_bits += 8;

     number_of_parsed_entries++;
     if(number_of_parsed_entries==MAX_NUMBER_OF_HID_PARSED_ENTRIES) {
      return;
     }

     //move pointer
     if(number_of_entry_to_insert_usage<number_of_parsed_entries) {
      number_of_entry_to_insert_usage = number_of_parsed_entries;
     }
    }
    else if(hid->short_item.tag==0x9) { //report count
     report_count = hid->short_item.data[0];
    }
   }
   else if(hid->short_item.type==HID_SHORT_ENTRY_TYPE_LOCAL) {
    if(hid->short_item.tag==0x0) { //usage
     if(hid->short_item.data[0]==USB_HID_USAGE_MOVEMENT_X || hid->short_item.data[0]==USB_HID_USAGE_MOVEMENT_Y || hid->short_item.data[0]==USB_HID_USAGE_WHEEL) { //TODO: more
      hid_parsed_entries[number_of_entry_to_insert_usage].usage = hid->short_item.data[0];
      number_of_entry_to_insert_usage++;
     }
    }
    else if(hid->short_item.tag==0x1) { //usage min
     hid_parsed_entries[number_of_entry_to_insert_usage].usage_value = hid->short_item.data[0];
    }
    else if(hid->short_item.tag==0x2) { //usage max
     byte_t usage = hid_parsed_entries[number_of_entry_to_insert_usage].usage; //this usage will be same for all entries
     for(dword_t j=hid_parsed_entries[number_of_entry_to_insert_usage].usage_value; j<(hid->short_item.data[0]+1); j++) { //values between usage min and usage max
      //fill entry
      hid_parsed_entries[number_of_entry_to_insert_usage].usage = usage;
      hid_parsed_entries[number_of_entry_to_insert_usage].usage_value = j;
      number_of_entry_to_insert_usage++;
     }
    }
   }
   else { //HID_SHORT_ENTRY_TYPE_RESERVED

   }
  }

  //move to next entry
  if(hid->long_item.signature==HID_LONG_ENTRY_SIGNATURE) { //long entry
   hid = (struct hid_entry_t *) ((dword_t)hid+3+hid->long_item.size_in_bytes);
  }
  else { //short entry
   hid = (struct hid_entry_t *) ((dword_t)hid+1+hid->short_item.size);
  }
 }

 for(dword_t i=0; i<number_of_parsed_entries; i++) {
  log("\n");
  log_var_with_space(hid_parsed_entries[i].offset_in_bits);
  log_var_with_space(hid_parsed_entries[i].length_in_bits);
  log_hex_specific_size_with_space(hid_parsed_entries[i].usage, 2);
  log_var(hid_parsed_entries[i].usage_value);
 }
}

byte_t get_hid_descriptor_entry(byte_t usage, byte_t usage_value) {
 for(dword_t i=0; i<MAX_NUMBER_OF_HID_PARSED_ENTRIES; i++) {
  if(hid_parsed_entries[i].length_in_bits==0) {
   break;
  }

  if(hid_parsed_entries[i].usage==usage && hid_parsed_entries[i].usage_value==usage_value) {
   //example: entry has size 12 bits from bit 10 - offset byte is 1, shift is 2, length is 0xFFF
   hid_entry_offset_byte = (hid_parsed_entries[i].offset_in_bits/8);
   hid_entry_offset_shift = (hid_parsed_entries[i].offset_in_bits%8);
   hid_entry_offset_length = ~(0xFFFFFFFF << hid_parsed_entries[i].length_in_bits);
   return STATUS_GOOD;
  }
 }

 return STATUS_ERROR;
}