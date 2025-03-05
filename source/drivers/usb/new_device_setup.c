//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_zero_address_device(void) {
 //log
 logf("\nUSB: Start of initalization of zero device");
 usb_devices[0].is_port_in_initalization = STATUS_TRUE;

 //at start we will transfer only 8 bytes from device, so we need to set control endpoint size to 8
 usb_devices[0].control_transfer.endpoint_size = 8;

 //read first 8 bytes of USB device descriptor
 usb_control_transfer_in(0, usb_get_device_descriptor_8_bytes_successfull, usb_get_device_descriptor_8_bytes_error, 100, 0x80, 0x06, 0x0100, 0, 8);
}

void usb_get_device_descriptor_8_bytes_successfull(byte_t device_address) {
 //TODO: check if this is valid device descriptor (how?)

 //read size of control endpoint
 struct usb_device_descriptor_t *usb_device_descriptor = (struct usb_device_descriptor_t *) usb_devices[0].control_transfer.transfer_buffer;
 usb_devices[0].control_transfer.endpoint_size = usb_device_descriptor->size_of_control_endpoint;
 if(usb_devices[0].control_transfer.endpoint_size!=8
    && usb_devices[0].control_transfer.endpoint_size!=16
    && usb_devices[0].control_transfer.endpoint_size!=32
    && usb_devices[0].control_transfer.endpoint_size!=64) {
  usb_new_device_setup_transfer_error(0, "Invalid control endpoint size");
  return;
 }
 else {
  logf("\nUSB device control endpoint with size %d", usb_devices[0].control_transfer.endpoint_size);
 }

 //close transfer
 usb_close_control_transfer(0);

 //get free USB device address
 //there will always be some free address, because before moving device into zero address state it is checked in driver of controller, that some address is free
 byte_t usb_device_address = 0;
 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  if(usb_devices[i].is_used == STATUS_FALSE) {
   usb_device_address = i;
   break;
  }
 }
 number_of_usb_devices++;

 //set address of device
 usb_control_transfer_without_data(0, usb_set_address_successfull, usb_set_address_error, 100, 0x00, 0x05, usb_device_address, 0);
}

void usb_set_address_successfull(byte_t device_address) {
 //log device address
 logf("\nUSB device: successfull SET_ADDRESS %d", usb_devices[0].setup_buffer.value);

 //close transfer
 usb_close_control_transfer(0);

 //copy content of zero device to new device address
 device_address = usb_devices[0].setup_buffer.value; //copy device address from setup packet
 copy_memory((dword_t)&usb_devices[0], (dword_t)&usb_devices[device_address], sizeof(struct usb_device_t));

 //clear zero device entry
 clear_memory((dword_t)&usb_devices[0], sizeof(struct usb_device_t));

 //read full USB device descriptor
 usb_control_transfer_in(device_address, usb_get_device_descriptor_successfull, usb_get_device_descriptor_error, 100, 0x80, 0x06, 0x0100, 0, 18);
}

void usb_get_device_descriptor_successfull(byte_t device_address) {
 //TODO: check if this is valid device descriptor (how?)

 //log content of device descriptor
 parse_usb_device_descriptor((dword_t)usb_devices[device_address].control_transfer.transfer_buffer);

 //close transfer
 usb_close_control_transfer(device_address);

 //TODO: support for more configurations on device

 //read first 8 bytes of configuration descriptor
 usb_control_transfer_in(device_address, usb_get_configuration_descriptor_8_bytes_successfull, usb_get_configuration_descriptor_8_bytes_error, 100, 0x80, 0x06, 0x0200, 0, 8);
}

void usb_get_configuration_descriptor_8_bytes_successfull(byte_t device_address) {
 //TODO: check if this is valid configuration descriptor (how?)

 //read length of full descriptor and prepare setup packet for transfer of full descriptor
 struct usb_configuration_descriptor_t *configuration_descriptor = (struct usb_configuration_descriptor_t *) usb_devices[device_address].control_transfer.transfer_buffer;
 word_t length = configuration_descriptor->wTotalLength;
 usb_devices[device_address].setup_buffer.length = length;

 //close transfer
 usb_close_control_transfer(device_address);

 //read full configuration descriptor
 usb_control_transfer_in(device_address, usb_get_configuration_descriptor_successfull, usb_get_configuration_descriptor_error, 100, 0x80, 0x06, 0x0200, 0, length);
}

void usb_get_configuration_descriptor_successfull(byte_t device_address) {
 //load basic variables
 byte_t *configuration_descriptor_pointer = usb_devices[device_address].control_transfer.transfer_buffer;
 dword_t size_of_configuration_descriptor = usb_devices[device_address].setup_buffer.length;

 //check if this is valid configuration descriptor
 struct usb_configuration_descriptor_t *configuration_descriptor = (struct usb_configuration_descriptor_t *) configuration_descriptor_pointer;
 byte_t configuration_descriptor_status = is_configuration_descriptor_valid(configuration_descriptor);
 if(configuration_descriptor_status == STATUS_FALSE) {
  usb_new_device_setup_transfer_error(device_address, "Invalid configuration descriptor");
  return;
 }

 //save configuration number
 usb_devices[device_address].configuration = configuration_descriptor->bConfigurationValue;

 //start parsing interfaces
 configuration_descriptor_pointer += configuration_descriptor_status;
 size_of_configuration_descriptor -= configuration_descriptor_status;
 while(1) {
  struct usb_full_interface_info_t interface = configuration_descriptor_parse_interface(configuration_descriptor_pointer, size_of_configuration_descriptor);
  
  //check if this is end of interfaces
  if(interface.is_parsed == STATUS_FALSE) {
   break;
  }

  //save interface informations
  //task from main.c will later in order call initalization methods to initalize every interface of this device
  if(interface.interface_descriptor->bInterfaceClass == 0x09
     && interface.interface_descriptor->bInterfaceSubClass == 0x00
     && interface.interface_descriptor->bInterfaceProtocol == 0x00) {
   usb_hub_save_informations(device_address, interface);
  }
  else if(interface.interface_descriptor->bInterfaceClass == 0x03
     && interface.interface_descriptor->bInterfaceSubClass == 0x01
     && interface.interface_descriptor->bInterfaceProtocol == 0x01) {
   usb_keyboard_save_informations(device_address, interface);
  }
  else if(interface.interface_descriptor->bInterfaceClass == 0x03
     && interface.interface_descriptor->bInterfaceSubClass == 0x01
     && interface.interface_descriptor->bInterfaceProtocol == 0x02) {
   usb_mouse_save_informations(device_address, interface);
  }
  else if(interface.interface_descriptor->bInterfaceClass == 0x08
     && interface.interface_descriptor->bInterfaceSubClass == 0x06
     && interface.interface_descriptor->bInterfaceProtocol == 0x50) {
   usb_msd_save_informations(device_address, interface);
  }

  //move to next interface
  configuration_descriptor_pointer += interface.length_to_skip;
  size_of_configuration_descriptor -= interface.length_to_skip;
  if(size_of_configuration_descriptor == 0) {
   break;
  }
 }

 //close transfer
 usb_close_control_transfer(device_address);

 //set this to start initalizing of interfaces
 usb_devices[device_address].are_interfaces_initalized = STATUS_FALSE;

 //set this to prevent immediate start of initalizing interfaces, because we still need to set configuration
 usb_devices[device_address].is_interface_in_initalization = STATUS_TRUE;

 //set configuration
 usb_control_transfer_without_data(device_address, usb_set_configuration_success, usb_set_configuration_error, 100, 0x00, 0x09, usb_devices[device_address].configuration, 0);
}

void usb_set_configuration_success(byte_t device_address) {
 //log
 logf("\nUSB device %d in configured state", device_address);

 //close transfer
 usb_close_control_transfer(device_address);

 //device is initalized, so initalization of interfaces can start
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
}

/* error transfers */

void usb_new_device_setup_transfer_error(byte_t device_address, byte_t *err_string) {
 //log
 logf("\nUSB device setup ERROR: %s", err_string);

 //close transfer
 usb_close_control_transfer(device_address);

 //disable device, so it will not affect new device
 usb_devices[device_address].disable_device_on_port(usb_devices[device_address].controller_number, usb_devices[device_address].port_number);

 //initalization of device ended in error, so clear device entry
 clear_memory((dword_t)&usb_devices[device_address], sizeof(struct usb_device_t));

 //decrease number of connected USB devices
 if(device_address != 0) {
  number_of_usb_devices--;
 }
}

void usb_get_device_descriptor_8_bytes_error(byte_t device_address) {
 usb_new_device_setup_transfer_error(device_address, "USB descriptor 8 bytes not received");
}

void usb_set_address_error(byte_t device_address) {
 usb_new_device_setup_transfer_error(device_address, "Set Address");
}

void usb_get_device_descriptor_error(byte_t device_address) {
 usb_new_device_setup_transfer_error(device_address, "USB descriptor not received");
}

void usb_get_configuration_descriptor_8_bytes_error(byte_t device_address) {
 usb_new_device_setup_transfer_error(device_address, "USB configuration descriptor 8 bytes not received");
}

void usb_get_configuration_descriptor_error(byte_t device_address) {
 usb_new_device_setup_transfer_error(device_address, "USB configuration descriptor not received");
}

void usb_set_configuration_error(byte_t device_address) {
 usb_new_device_setup_transfer_error(device_address, "Set Configuration");
}