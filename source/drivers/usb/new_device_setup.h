//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_zero_address_device(void);

void usb_get_device_descriptor_8_bytes_successfull(byte_t device_address);
void usb_get_device_descriptor_successfull(byte_t device_address);
void usb_set_address_successfull(byte_t device_address);
void usb_get_configuration_descriptor_8_bytes_successfull(byte_t device_address);
void usb_get_configuration_descriptor_successfull(byte_t device_address);
void usb_set_configuration_success(byte_t device_address);

void usb_new_device_setup_transfer_error(byte_t device_address, byte_t *err_string);
void usb_get_device_descriptor_8_bytes_error(byte_t device_address);
void usb_get_device_descriptor_error(byte_t device_address);
void usb_set_address_error(byte_t device_address);
void usb_get_configuration_descriptor_8_bytes_error(byte_t device_address);
void usb_get_configuration_descriptor_error(byte_t device_address);
void usb_set_configuration_error(byte_t device_address);