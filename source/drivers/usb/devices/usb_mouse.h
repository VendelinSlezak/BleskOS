//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t usb_mouse_packet_received;

void usb_mouse_save_informations(byte_t device_address, struct usb_full_interface_info_t interface);

void usb_mouse_initalize(byte_t device_address);

void usb_mouse_set_hid_protocol_success(byte_t device_address);
void usb_mouse_set_idle_success(byte_t device_address);
void usb_mouse_get_hid_descriptor_success(byte_t device_address);
void usb_mouse_interrupt_transfer_successfull(byte_t device_address);

void usb_mouse_stop_initalization(byte_t device_address, byte_t *err_string);
void usb_mouse_set_hid_protocol_error(byte_t device_address);
void usb_mouse_set_idle_error(byte_t device_address);
void usb_mouse_get_hid_descriptor_error(byte_t device_address);