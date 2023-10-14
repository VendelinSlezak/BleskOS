//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_HUB_NOT_ATTACHED 0
#define USB_HUB_ATTACHED 1

struct usb_hub {
 byte_t entry_state;
 byte_t controller_number;
 byte_t controller_type;
 byte_t port;
 byte_t address;
 byte_t device_speed;
 byte_t number_of_ports;
 byte_t interrupt_endpoint;
 byte_t interrupt_endpoint_length;
 byte_t toggle;
 byte_t ports_state[8];
 byte_t ports_device_speed[8];
};
struct usb_hub usb_hub_devices[10];

dword_t usb_hub_interrupt_endpoint_memory = 0;