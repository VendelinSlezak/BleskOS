//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_DEVICE_HUB 0x090000

#define USB_HUB_IS_NOT_RESPONDING 0xFF

//first 32 addresses are for devices on root ports, and every hub can have max 8 ports
#define CALCULATE_ADDRESS_OF_DEVICE_ON_USB_HUB(hub, port) (33+(hub*8)+port)

struct usb_hub {
 byte_t controller_type;
 byte_t controller_number;
 byte_t port;
 byte_t address;
 byte_t device_speed;
 byte_t number_of_ports;
 dword_t ehci_hub_address;
 dword_t ehci_hub_port_number;
 byte_t ports_state[8];
 byte_t ports_device_speed[8];
}__attribute__((packed));
#define MAX_NUMBER_OF_USB_HUBS 10
struct usb_hub usb_hubs[MAX_NUMBER_OF_USB_HUBS];

dword_t usb_hub_transfer_setup_packets_mem = 0, usb_hub_transfer_data_mem = 0;

byte_t usb_read_hub_number_of_port(byte_t controller_number, byte_t device_address, byte_t device_speed);
dword_t usb_hub_read_port_status(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t port);
byte_t usb_hub_is_there_some_port_connection_status_change(byte_t hub_number);
byte_t usb_hub_set_feature(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t index, byte_t value);
byte_t usb_hub_clear_feature(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t index, byte_t value);