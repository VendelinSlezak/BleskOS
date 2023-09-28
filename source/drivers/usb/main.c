//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_usb_controllers(void) {
 usb_mouse_state = 0;
 usb_keyboard_state = 0;
 for(int i=0; i<10; i++) {
  usb_mass_storage_devices[i].type = USB_MSD_NOT_ATTACHED;
 }
 
 if(usb_controllers_pointer==0) {
  log("\nno USB controllers\n");
  return;
 }
 
 //allocate memory for USB devices
 usb_mouse_data_memory = calloc(8);
 usb_keyboard_data_memory = calloc(8);
 usb_mass_storage_cbw_memory = malloc(32);
 usb_mass_storage_response_memory = calloc(128);
 usb_mass_storage_csw_memory = malloc(16);
 
 //initalize EHCI USB controllers
 for(int i=0; i<usb_controllers_pointer; i++) {
  for(int j=0; j<16; j++) {
   usb_controllers[i].ports_state[j]=PORT_NO_DEVICE;
   usb_controllers[i].ports_device_speed[j]=0;
  }
 
  if(usb_controllers[i].type==USB_CONTROLLER_EHCI) {
   initalize_ehci_controller(i);
   continue;
  }
 }
 
 //initalize UHCI and OHCI USB controllers
 for(int i=0; i<usb_controllers_pointer; i++) {
  for(int j=0; j<16; j++) {
   usb_controllers[i].ports_state[j]=PORT_NO_DEVICE;
   usb_controllers[i].ports_device_speed[j]=0;
  }
 
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
 
 log("\n");
}

void detect_usb_devices(void) {
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_EHCI) {
   ehci_controller_detect_devices(i);
   continue;
  }
 }
 
 for(int i=0; i<usb_controllers_pointer; i++) {
  if(usb_controllers[i].type==USB_CONTROLLER_UHCI) {
   uhci_controller_detect_devices(i);
   continue;
  }
  if(usb_controllers[i].type==USB_CONTROLLER_OHCI) {
   ohci_controller_detect_devices(i);
   continue;
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
    usb_new_device_detected = 1;
    return;
   }
   continue;
  }
  if(usb_controllers[i].type==USB_CONTROLLER_UHCI) {
   if(uhci_controller_detect_status_change(i)==STATUS_TRUE) {
    usb_new_device_detected = 1;
    return;
   }
   continue;
  }
  if(usb_controllers[i].type==USB_CONTROLLER_OHCI) {
   if(ohci_controller_detect_status_change(i)==STATUS_TRUE) {
    usb_new_device_detected = 1;
    return;
   }
   continue;
  }
 }
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
 usb_control_endpoint_length = (descriptor32[1]>>24);
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
  usb_descriptor_devices[i].endpoint_interrupt_length=0;
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
   log_var(usb_descriptor_devices[device_pointer].configuration);
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
   log_var(usb_descriptor_devices[device_pointer].interface);
   log_var(usb_descriptor_devices[device_pointer].alternative_interface);
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
    log_hex(descriptor8[4]);
    log_hex(descriptor8[6]);
    usb_descriptor_devices[device_pointer].endpoint_interrupt_length=descriptor8[4];
    usb_descriptor_devices[device_pointer].endpoint_interrupt_time=descriptor8[6];
    usb_descriptor_devices[device_pointer].endpoint_interrupt=endpoint;
   }
   else if(endpoint_type==0x6) {
    log("INTERRUPT OUT ");
    log_hex(descriptor8[4]);
    log_hex(descriptor8[6]);
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
    log_hex(descriptor8[6+j*2]);
    log_hex(descriptor8[7+j*2]);
    
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

byte_t usb_bulk_in(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length_of_transfer, dword_t time_to_wait) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_bulk_in(controller_number, port, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_UHCI) {
  return uhci_bulk_in(controller_number, port, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_OHCI) {
  return ohci_bulk_in(controller_number, port, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }

 return STATUS_ERROR;
}

byte_t usb_bulk_out(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length_of_transfer, dword_t time_to_wait) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_bulk_out(controller_number, port, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_UHCI) {
  return uhci_bulk_out(controller_number, port, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_OHCI) {
  return ohci_bulk_out(controller_number, port, endpoint, toggle, memory, length_of_transfer, time_to_wait);
 }

 return STATUS_ERROR;
}

byte_t get_usb_bulk_toggle(byte_t controller_number) {
 if(usb_controllers[controller_number].type==USB_CONTROLLER_EHCI) {
  return ehci_toggle;
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_UHCI) {
  return uhci_toggle;
 }
 else if(usb_mass_storage_devices[controller_number].controller_type==USB_CONTROLLER_OHCI) {
  return ohci_toggle;
 }

 return STATUS_ERROR;
}