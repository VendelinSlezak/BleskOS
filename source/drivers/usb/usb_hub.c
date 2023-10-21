//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t usb_read_hub_number_of_port(byte_t controller_number, byte_t device_address, byte_t device_speed) {
 byte_t status = usb_control_transfer_with_fixed_data_length(controller_number, device_address, device_speed, 0xA0, 0x06, 0x2900, 0, 8);
 if(status==STATUS_ERROR) {
  return 0xFF;
 }
 else {
  byte_t *number_of_ports = (byte_t *) (usb_setup_packet_data_mem+2);
  return *number_of_ports;
 }
}

dword_t usb_hub_read_port_status(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t port) {
 usb_control_transfer_with_fixed_data_length(controller_number, device_address, device_speed, 0xA3, 0x00, 0x0000, (port+1), 4);
 dword_t *data = (dword_t *) (usb_setup_packet_data_mem);
 return *data;
}

byte_t usb_hub_is_there_some_port_connection_status_change(byte_t hub_number) {
 if(usb_hubs[hub_number].controller_type==USB_CONTROLLER_EHCI) {
  return ehci_hub_is_there_some_port_connection_status_change(hub_number);
 }
 else if(usb_hubs[hub_number].controller_type==USB_CONTROLLER_UHCI) {
  return uhci_hub_is_there_some_port_connection_status_change(hub_number);
 }
 else if(usb_hubs[hub_number].controller_type==USB_CONTROLLER_OHCI) {
  return ohci_hub_is_there_some_port_connection_status_change(hub_number);
 }

 return STATUS_FALSE;
}

byte_t usb_hub_set_feature(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t index, byte_t value) {
 return usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x23, 0x03, value, index);
}

byte_t usb_hub_clear_feature(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t index, byte_t value) {
 return usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x23, 0x01, value, index);
}