//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_mouse_save_informations(byte_t device_address, struct usb_full_interface_info_t interface) {
 //return means that there will be no information saved about USB mouse on this device

 //check if this device do not already have mouse interface
 if(usb_devices[device_address].mouse.is_present == STATUS_TRUE) {
  l("\nUSB ERROR: more mouse interfaces at one device");
  return;
 }

 //check if this interface has all needed informations
 if(interface.interrupt_in_endpoint == 0) {
  l("\nUSB ERROR: USB mouse do not have interrupt in endpoint");
  return;
 }
 if(interface.interrupt_in_endpoint->bInterval == 0) {
  l("\nUSB ERROR: USB mouse invalid endpoint interval");
  return;
 }
 if(interface.hid_descriptor == 0) {
  l("\nUSB ERROR: USB mouse do not have any HID descriptor");
  return;
 }
 word_t hid_descriptor_0x22_length = 0;
 for(dword_t i=0; i<interface.hid_descriptor->bNumDescriptors; i++) {
  //check if we founded descriptor 0x22
  if(interface.hid_descriptor->descriptors[i].bDescriptorType == 0x22) {
   hid_descriptor_0x22_length = interface.hid_descriptor->descriptors[i].bDescriptorLength;
   break;
  }
 }
 if(hid_descriptor_0x22_length == 0) {
  l("\nUSB ERROR: USB mouse do not have HID descriptor 0x22");
  return;
 }

 //save informations
 usb_devices[device_address].mouse.is_present = STATUS_TRUE;
 usb_devices[device_address].mouse.is_initalized = STATUS_FALSE;
 usb_devices[device_address].mouse.initalize = usb_mouse_initalize;
 usb_devices[device_address].mouse.hid_descriptor_0x22_length = hid_descriptor_0x22_length;

 usb_devices[device_address].mouse.interface = interface.interface_descriptor->bInterfaceNumber;
 usb_devices[device_address].mouse.alternative_interface = interface.interface_descriptor->bAlternateSetting;

 usb_devices[device_address].mouse.interrupt_transfer.endpoint = interface.interrupt_in_endpoint->endpoint_number;
 usb_devices[device_address].mouse.interrupt_transfer.endpoint_size = interface.interrupt_in_endpoint->wMaxPacketSize;
 usb_devices[device_address].mouse.interrupt_transfer.interval = interface.interrupt_in_endpoint->bInterval;

 //log
 l("\nUSB mouse\n Endpoint: "); lvw(usb_devices[device_address].mouse.interrupt_transfer.endpoint);
 l("\n Size: "); lvw(usb_devices[device_address].mouse.interrupt_transfer.endpoint_size);
 l("\n Interval: "); lv(usb_devices[device_address].mouse.interrupt_transfer.interval); l("ms");
}

void usb_mouse_initalize(byte_t device_address) {
 //check if we can use control endpoint
 if(usb_devices[device_address].control_transfer.is_running == STATUS_TRUE) {
  return;
 }

 //set device into initalization phase
 usb_devices[device_address].is_interface_in_initalization = STATUS_TRUE;

 //set use of HID protocol = 1
 usb_control_transfer_without_data(device_address, usb_mouse_set_hid_protocol_success, usb_mouse_set_hid_protocol_error, 100, 0x21, 0x0B, 1, usb_devices[device_address].mouse.interface);
}

void usb_mouse_set_hid_protocol_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //disable idle - not all devices supports this, but some expect it
 usb_control_transfer_without_data(device_address, usb_mouse_set_idle_success, usb_mouse_set_idle_error, 100, 0x21, 0x0A, 0, usb_devices[device_address].mouse.interface);
}

void usb_mouse_set_idle_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //read HID descriptor
 usb_control_transfer_in(device_address, usb_mouse_get_hid_descriptor_success, usb_mouse_get_hid_descriptor_error, 100, 0x81, 0x06, 0x2200, usb_devices[device_address].mouse.interface, usb_devices[device_address].mouse.hid_descriptor_0x22_length);
}

void usb_mouse_get_hid_descriptor_success(byte_t device_address) {
 //parse HID descriptor
 parse_hid_descriptor(usb_devices[device_address].control_transfer.transfer_buffer, usb_devices[device_address].mouse.hid_descriptor_0x22_length);

 //save parsed variables
 byte_t hid_entry = get_hid_descriptor_entry(HID_USAGE_PAGE_BUTTON, 0, 1);
 if(hid_entry != HID_NO_ENTRY_FOUNDED) {
  copy_memory((dword_t)&parsed_hid[hid_entry].data, (dword_t)&usb_devices[device_address].mouse.button_1, sizeof(struct hid_entry_data_t));
 }
 hid_entry = get_hid_descriptor_entry(HID_USAGE_PAGE_BUTTON, 0, 2);
 if(hid_entry != HID_NO_ENTRY_FOUNDED) {
  copy_memory((dword_t)&parsed_hid[hid_entry].data, (dword_t)&usb_devices[device_address].mouse.button_2, sizeof(struct hid_entry_data_t));
 }
 hid_entry = get_hid_descriptor_entry(HID_USAGE_PAGE_BUTTON, 0, 3);
 if(hid_entry != HID_NO_ENTRY_FOUNDED) {
  copy_memory((dword_t)&parsed_hid[hid_entry].data, (dword_t)&usb_devices[device_address].mouse.button_3, sizeof(struct hid_entry_data_t));
 }
 hid_entry = get_hid_descriptor_entry(HID_USAGE_PAGE_GENERIC_DESKTOP, HID_GENERIC_DESKTOP_USAGE_X, 0);
 if(hid_entry != HID_NO_ENTRY_FOUNDED) {
  copy_memory((dword_t)&parsed_hid[hid_entry].data, (dword_t)&usb_devices[device_address].mouse.x, sizeof(struct hid_entry_data_t));
 }
 hid_entry = get_hid_descriptor_entry(HID_USAGE_PAGE_GENERIC_DESKTOP, HID_GENERIC_DESKTOP_USAGE_Y, 0);
 if(hid_entry != HID_NO_ENTRY_FOUNDED) {
  copy_memory((dword_t)&parsed_hid[hid_entry].data, (dword_t)&usb_devices[device_address].mouse.y, sizeof(struct hid_entry_data_t));
 }
 hid_entry = get_hid_descriptor_entry(HID_USAGE_PAGE_GENERIC_DESKTOP, HID_GENERIC_DESKTOP_USAGE_WHEEL, 0);
 if(hid_entry != HID_NO_ENTRY_FOUNDED) {
  copy_memory((dword_t)&parsed_hid[hid_entry].data, (dword_t)&usb_devices[device_address].mouse.wheel, sizeof(struct hid_entry_data_t));
 }

 //close transfer
 usb_close_control_transfer(device_address);

 //allocate memory for transfers
 usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer = (void *) calloc(usb_devices[device_address].mouse.interrupt_transfer.endpoint_size);

 //start interrupt transfers
 usb_interrupt_transfer_in(device_address, (struct usb_interrupt_transfer_info_t *)&usb_devices[device_address].mouse.interrupt_transfer, usb_mouse_interrupt_transfer_successfull);

 //initalization is successfully done
 usb_devices[device_address].mouse.is_initalized = STATUS_TRUE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
}

void usb_mouse_interrupt_transfer_successfull(byte_t device_address) {
 //buttons
 mouse_buttons = (parse_value_from_hid_packet(usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer, usb_devices[device_address].mouse.button_1)<<0
                  | parse_value_from_hid_packet(usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer, usb_devices[device_address].mouse.button_2)<<1
                  | parse_value_from_hid_packet(usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer, usb_devices[device_address].mouse.button_3)<<2);
 mouse_update_click_button_state();

 //X movement
 mouse_movement_x = parse_value_from_hid_packet(usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer, usb_devices[device_address].mouse.x);
 if(mouse_movement_x>((usb_devices[device_address].mouse.x.mask+1)/2)) {
  mouse_movement_x = (0xFFFFFFFF - (usb_devices[device_address].mouse.x.mask-mouse_movement_x));
 }

 //Y movement
 mouse_movement_y = parse_value_from_hid_packet(usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer, usb_devices[device_address].mouse.y);
 if(mouse_movement_y>((usb_devices[device_address].mouse.y.mask+1)/2)) {
  mouse_movement_y = (0xFFFFFFFF - (usb_devices[device_address].mouse.y.mask-mouse_movement_y));
 }

 //wheel movement
 mouse_wheel_movement = parse_value_from_hid_packet(usb_devices[device_address].mouse.interrupt_transfer.transfer_buffer, usb_devices[device_address].mouse.wheel);
 if(mouse_wheel_movement>((usb_devices[device_address].mouse.wheel.mask+1)/2)) {
  mouse_wheel_movement = (0xFFFFFFFF - (usb_devices[device_address].mouse.wheel.mask-mouse_wheel_movement));
 }

 //inform method wait_for_user_input() from source/drivers/system/user_wait.c that there was received packet from mouse
 usb_mouse_packet_received = STATUS_TRUE;
}

/* errors */

void usb_mouse_stop_initalization(byte_t device_address, byte_t *err_string) {
 l("\nUSB mouse error: "); l(err_string);
 usb_devices[device_address].mouse.is_present = STATUS_FALSE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
 usb_close_control_transfer(device_address);
}

void usb_mouse_set_hid_protocol_error(byte_t device_address) {
 usb_mouse_stop_initalization(device_address, "set HID protocol error");
}

void usb_mouse_set_idle_error(byte_t device_address) {
 //some deviced do not support this feature, so it do not matter if it fails, we will continue initalization
 l("\nUSB mouse: set idle not supported");
 
 //close transfer
 usb_close_control_transfer(device_address);

 //read first 8 bytes of HID descriptor
 usb_control_transfer_in(device_address, usb_mouse_get_hid_descriptor_success, usb_mouse_get_hid_descriptor_error, 100, 0x81, 0x06, 0x2200, usb_devices[device_address].mouse.interface, usb_devices[device_address].mouse.hid_descriptor_0x22_length);
}

void usb_mouse_get_hid_descriptor_error(byte_t device_address) {
 usb_mouse_stop_initalization(device_address, "get HID descriptor error");
}