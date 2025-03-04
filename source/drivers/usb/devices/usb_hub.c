//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_hub_save_informations(byte_t device_address, struct usb_full_interface_info_t interface) {
 //return means that there will be no information saved about USB hub on this device

 //check if this device do not already have hub interface
 if(usb_devices[device_address].hub.is_present == STATUS_TRUE) {
  l("\nUSB ERROR: more hub interfaces at one device");
  return;
 }

 //save informations
 usb_devices[device_address].hub.is_present = STATUS_TRUE;
 usb_devices[device_address].hub.is_initalized = STATUS_FALSE;
 usb_devices[device_address].hub.initalize = usb_hub_initalize;

 //log
 l("\nUSB hub");
}

void usb_hub_initalize(byte_t device_address) {
 //check if we can use control endpoint
 if(usb_devices[device_address].control_transfer.is_running == STATUS_TRUE) {
  return;
 }

 //set device into initalization phase
 usb_devices[device_address].is_interface_in_initalization = STATUS_TRUE;

 //get hub descriptor
 usb_control_transfer_in(device_address, usb_hub_get_descriptor_success, usb_hub_get_descriptor_error, 100, 0xA0, 0x06, 0x2900, 0, 5); //we need only five bytes to get all needed informations
}

void usb_hub_get_descriptor_success(byte_t device_address) {
 //read number of hub ports
 struct usb_hub_descriptor_t *hub_descriptor = (struct usb_hub_descriptor_t *) usb_devices[device_address].control_transfer.transfer_buffer;
 if(hub_descriptor->number_of_ports == 0) {
  usb_hub_stop_initalization(device_address, "zero ports");
  return;
 }
 usb_devices[device_address].hub.number_of_ports = hub_descriptor->number_of_ports;
 usb_devices[device_address].hub.power_on_to_power_good_time = (hub_descriptor->power_on_to_power_good_time*2);
 l("\nHUB ports: "); lv(usb_devices[device_address].hub.number_of_ports);

 //close transfer
 usb_close_control_transfer(device_address);

 //set power for first port - start powering all ports
 usb_control_transfer_without_data(device_address, usb_hub_power_port_success, usb_hub_power_port_error, 50, 0x23, 0x03, USB_HUB_REQUEST_POWER_PORT, 1);
}

void usb_hub_power_port_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //move to next port
 usb_devices[device_address].hub.actual_port++;
 if(usb_devices[device_address].hub.actual_port >= usb_devices[device_address].hub.number_of_ports) {
  //all ports are powered, now we only need to wait to finalize initalization
  usb_devices[device_address].hub.timeout_for_finalizing_initalization = (time_of_system_running + usb_devices[device_address].hub.power_on_to_power_good_time);
  return;
 }

 //set power for actual port
 usb_control_transfer_without_data(device_address, usb_hub_power_port_success, usb_hub_power_port_error, 50, 0x23, 0x03, USB_HUB_REQUEST_POWER_PORT, usb_devices[device_address].hub.actual_port+1);
}

/* checking connections on hub */

void usb_hub_check_connections(void) {
 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  if(usb_devices[i].is_used == STATUS_TRUE
     && usb_devices[i].hub.is_present == STATUS_TRUE
     && usb_devices[i].control_transfer.is_running == STATUS_FALSE
     && usb_devices[i].hub.is_present == STATUS_TRUE
     && usb_devices[i].hub.is_initalized == STATUS_TRUE
     && usb_devices[i].hub.line_of_errors < usb_devices[i].hub.number_of_ports) {
   //check if some port is in initalization phase
   if(usb_devices[i].hub.port_in_initalization_phase != USB_HUB_PORT_PHASE_NONE) {
    if(usb_devices[i].hub.port_in_initalization_phase == USB_HUB_PORT_PHASE_RESET && time_of_system_running >= usb_devices[i].hub.port_in_initalization_timeout) {
     usb_hub_port_stop_reset(i);
    }
    else if(usb_devices[i].hub.port_in_initalization_phase == USB_HUB_PORT_PHASE_AFTER_RESET && time_of_system_running >= usb_devices[i].hub.port_in_initalization_timeout) {
     usb_hub_clear_status_change(i);
    }
   }
   else {
    //start checking of port status
    usb_devices[i].hub.actual_port = 0;
    usb_control_transfer_in(i, usb_hub_get_port_status_success, usb_hub_get_port_status_error, 100, 0xA3, 0x00, 0, 1, 4);
   }
  }
 }
}

void usb_hub_get_port_status_success(byte_t device_address) {
 //this resets line of errors
 usb_devices[device_address].hub.line_of_errors = 0;

 //check port status
 dword_t *port_status_pointer = (dword_t *) (usb_devices[device_address].control_transfer.transfer_buffer);
 dword_t port_status = *port_status_pointer;

 //close transfer
 usb_close_control_transfer(device_address);

 //is status change bit clear?
 if((port_status & (1 << 16))==0x0) {
  //check if this is zero address device that in not so far initalized
  if(usb_devices[0].is_used == STATUS_TRUE
     && usb_devices[0].is_port_in_initalization == STATUS_FALSE
     && usb_devices[0].controller_number == usb_devices[device_address].controller_number
     && usb_devices[0].port_number == usb_devices[device_address].port_number
     && usb_devices[0].hub_address == device_address
     && usb_devices[0].hub_port_number == (usb_devices[device_address].hub.actual_port+1)
     && usb_devices[0].control_transfer.is_running == STATUS_FALSE) {
   //log
   l("\nUSB HUB: Zero device");

   //start initalizing of zero address device
   initalize_zero_address_device();
  }

  //no, there was no change of device
  goto move_to_next_port;
 }

 //was device removed?
 if((port_status & (1 << 0))==0x0) {
  l("\nUSB HUB: Device removed");
  //clear connection status change
  usb_control_transfer_without_data(device_address, usb_hub_clear_status_change_success, usb_hub_clear_status_change_error, 50, 0x23, 0x01, USB_HUB_CLEAR_PORT_CHANGE_STATUS, usb_devices[device_address].hub.actual_port+1);
  return;
 }

 //check if some port is not being initalized or some device is not in zero address mode or there are too much USB devices already connected
 if(usb_devices[0].is_used == STATUS_TRUE || number_of_usb_devices >= MAX_NUMBER_OF_USB_DEVICES) {
  goto move_to_next_port; //we can not initalize two devices at same time, but condition on this port is not cleared, so when other device will be initalized, we will initalize this device
 }

 //clear zero device entry
 clear_memory((dword_t)(&usb_devices[0]), sizeof(struct usb_device_t));

 //there was connected new device, so start initalization of this port
 usb_devices[0].is_used = STATUS_TRUE;
 usb_devices[0].controller_type = usb_devices[device_address].controller_type;
 usb_devices[0].controller_number = usb_devices[device_address].controller_number;
 usb_devices[0].port_number = usb_devices[device_address].port_number;
 usb_devices[0].hub_address = device_address;
 usb_devices[0].hub_port_number = (usb_devices[device_address].hub.actual_port+1);
 usb_devices[0].is_port_in_initalization = STATUS_TRUE;

 //start reset of port
 usb_control_transfer_without_data(device_address, usb_hub_port_start_reset_success, usb_hub_port_start_reset_error, 50, 0x23, 0x03, USB_HUB_REQUEST_PORT_RESET, usb_devices[device_address].hub.actual_port+1);
 return;

 //move to next port
 move_to_next_port:
 usb_devices[device_address].hub.actual_port++;
 if(usb_devices[device_address].hub.actual_port >= usb_devices[device_address].hub.number_of_ports) {
  //all ports were checked
  usb_devices[device_address].hub.actual_port = 0;
  return;
 }
 usb_control_transfer_in(device_address, usb_hub_get_port_status_success, usb_hub_get_port_status_error, 100, 0xA3, 0x00, 0, usb_devices[device_address].hub.actual_port+1, 4);
}

void usb_hub_port_start_reset_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //this resets line of errors
 usb_devices[device_address].hub.line_of_errors = 0;

 //log
 logf("\nUSB hub port %d is in reset", usb_devices[device_address].hub.actual_port);

 //wait for 100 ms
 usb_devices[device_address].hub.port_in_initalization_phase = USB_HUB_PORT_PHASE_RESET;
 usb_devices[device_address].hub.port_in_initalization_timeout = (time_of_system_running+100);
}

void usb_hub_port_stop_reset(byte_t device_address) {
 //stop waiting
 usb_devices[device_address].hub.port_in_initalization_phase = USB_HUB_PORT_PHASE_NONE;
 usb_devices[device_address].hub.port_in_initalization_timeout = 0;

 //stop reset
 usb_control_transfer_without_data(device_address, usb_hub_port_stop_reset_success, usb_hub_port_stop_reset_error, 50, 0x23, 0x01, USB_HUB_CLEAR_PORT_RESET, usb_devices[device_address].hub.actual_port+1);
}

void usb_hub_port_stop_reset_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //set next phase
 usb_devices[device_address].hub.port_in_initalization_phase = USB_HUB_PORT_PHASE_AFTER_RESET;
 usb_devices[device_address].hub.port_in_initalization_timeout = (time_of_system_running+100);
}

void usb_hub_clear_status_change(byte_t device_address) {
 //stop waiting
 usb_devices[device_address].hub.port_in_initalization_phase = USB_HUB_PORT_PHASE_NONE;
 usb_devices[device_address].hub.port_in_initalization_timeout = 0;

 //clear connection status change
 usb_control_transfer_without_data(device_address, usb_hub_clear_status_change_success, usb_hub_clear_status_change_error, 50, 0x23, 0x01, USB_HUB_CLEAR_PORT_CHANGE_STATUS, usb_devices[device_address].hub.actual_port+1);
}

void usb_hub_clear_status_change_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 l("\nUSB hub port status change was cleared");

 //get again port status
 usb_control_transfer_in(device_address, usb_hub_get_port_status_again_success, usb_hub_get_port_status_again_error, 100, 0xA3, 0x00, 0, usb_devices[device_address].hub.actual_port+1, 4);
}

void usb_hub_get_port_status_again_success(byte_t device_address) {
 //check port status
 dword_t *port_status_pointer = (dword_t *) (usb_devices[device_address].control_transfer.transfer_buffer);
 dword_t port_status = *port_status_pointer;

 logf("\nUSB hub port %d 0x%x", usb_devices[device_address].hub.actual_port, port_status);

 //close transfer
 usb_close_control_transfer(device_address);

 //is there anything connected?
 if((port_status & (1 << 0))==0x0) {
  //device was removed from this port
  logf("\nRemoving device from HUB %d port %d", device_address, (usb_devices[device_address].hub.actual_port+1));
  for(dword_t i = 0; i < MAX_NUMBER_OF_USB_DEVICES; i++) {
    if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].hub_address != 0) {
     logf("\nHUB device in list: %d %d %d", i, usb_devices[i].hub_address, usb_devices[i].hub_port_number);
    }
   if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].hub_address == device_address && usb_devices[i].hub_port_number == (usb_devices[device_address].hub.actual_port+1)) {
    usb_remove_device(i);
   }
  }

  //usb device changed
  usb_device_change_event = STATUS_TRUE;

  //continue to check ports
  usb_devices[device_address].hub.actual_port++;
  if(usb_devices[device_address].hub.actual_port >= usb_devices[device_address].hub.number_of_ports) {
   //all ports were checked
   usb_devices[device_address].hub.actual_port = 0;
  }
  else {
   usb_control_transfer_in(device_address, usb_hub_get_port_status_success, usb_hub_get_port_status_error, 100, 0xA3, 0x00, 0, usb_devices[device_address].hub.actual_port+1, 4);
  }

  return;
 }
 else {
  //check if port is enabled
  if((port_status & (1 << 1))==(1 << 1)) {
   l("\nUSB HUB port is enabled");

   //usb device changed
   usb_device_change_event = STATUS_TRUE;

   //get speed of connected device
   byte_t device_speed = ((port_status >> 9) & 0b11);
   if((port_status & (1 << 9))==(1 << 9)) {
    usb_devices[0].device_speed = USB_LOW_SPEED;
    l(" low speed");
   }
   else if((port_status & (1 << 10))==0) {
    usb_devices[0].device_speed = USB_FULL_SPEED;
    l(" full speed");
   }
   else {
    usb_devices[0].device_speed = USB_HIGH_SPEED;
    l(" high speed");
   }
   
   //when port is enabled, it mean, that device is ready for transfers and have address 0, so fill all needed entries
   usb_devices[0].disable_device_on_port = usb_devices[device_address].disable_device_on_port;
   usb_devices[0].get_state_of_transfer = usb_devices[device_address].get_state_of_transfer;

   usb_devices[0].control_transfer_without_data = usb_devices[device_address].control_transfer_without_data;
   usb_devices[0].control_transfer_with_data = usb_devices[device_address].control_transfer_with_data;
   usb_devices[0].close_control_transfer = usb_devices[device_address].close_control_transfer;

   usb_devices[0].interrupt_transfer = usb_devices[device_address].interrupt_transfer;
   usb_devices[0].restart_interrupt_transfer = usb_devices[device_address].restart_interrupt_transfer;
   usb_devices[0].close_interrupt_transfer = usb_devices[device_address].close_interrupt_transfer;

   usb_devices[0].bulk_transfer = usb_devices[device_address].bulk_transfer;
   usb_devices[0].close_bulk_transfer = usb_devices[device_address].close_bulk_transfer;

   //port is now initalized
   usb_devices[0].is_port_in_initalization = STATUS_FALSE;
  }
  else {
   //timeout error
   l("\nERROR: USB hub device was not enabled");

   //initalization of port ended, and device is not in zero address state
   usb_devices[0].is_used = STATUS_FALSE;
  }
 }
}

/* errors */

void usb_hub_stop_initalization(byte_t device_address, byte_t *err_string) {
 logf("\nUSB hub error: %s", err_string);
 usb_devices[device_address].hub.is_present = STATUS_FALSE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
 usb_devices[device_address].hub.port_in_initalization_phase = USB_HUB_PORT_PHASE_NONE;
 usb_close_control_transfer(device_address);
}

void usb_hub_get_descriptor_error(byte_t device_address) {
 usb_hub_stop_initalization(device_address, "get hub descriptor error");
}

void usb_hub_power_port_error(byte_t device_address) {
 //log port
 logf("\nUSB hub: port %d did not accepted power request", usb_devices[device_address].hub.actual_port);

 //go to powering next ports
 usb_hub_power_port_success(device_address);
}

void usb_hub_error_message(byte_t device_address, byte_t *err_string) {
 usb_close_control_transfer(device_address);

 usb_devices[device_address].hub.line_of_errors++;
 logf("\nUSB hub error: %s Port: %d Errors: %d", err_string, usb_devices[device_address].hub.actual_port, usb_devices[device_address].hub.line_of_errors);

 //try to reset port
 if(usb_devices[device_address].hub.line_of_errors < usb_devices[device_address].hub.number_of_ports) {
  //start reset of port
  usb_control_transfer_without_data(device_address, usb_hub_port_start_reset_success, usb_hub_port_start_reset_error, 50, 0x23, 0x03, USB_HUB_REQUEST_PORT_RESET, usb_devices[device_address].hub.actual_port+1);
 }
}

void usb_hub_get_port_status_error(byte_t device_address) {
 usb_hub_error_message(device_address, "get hub port status error");
}

void usb_hub_port_start_reset_error(byte_t device_address) {
 usb_hub_error_message(device_address, "hub port start reset error");
}

void usb_hub_port_stop_reset_error(byte_t device_address) {
 usb_hub_error_message(device_address, "hub port stop reset error");
}

void usb_hub_clear_status_change_error(byte_t device_address) {
 usb_hub_error_message(device_address, "hub port clear status change error");
}

void usb_hub_get_port_status_again_error(byte_t device_address) {
 usb_hub_error_message(device_address, "get hub port status again error");
}