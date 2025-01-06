//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define UHCI_SETUP 0x2D
#define UHCI_IN 0x69
#define UHCI_OUT 0xE1

#define UHCI_NO_POINTER 0x1

void initalize_uhci_controller(byte_t controller_number);
byte_t uhci_controller_detect_status_change(byte_t controller_number);
byte_t uhci_get_port_connection_status(byte_t controller_number, byte_t port_number);
byte_t uhci_get_port_connection_change_status(byte_t controller_number, byte_t port_number);
void uhci_clear_port_connection_change_status(byte_t controller_number, byte_t port_number);
byte_t uhci_enable_device_on_port(byte_t controller_number, byte_t port_number);
void uhci_set_transfer_descriptor(byte_t controller_number, byte_t device_speed, dword_t descriptor, dword_t next_descriptor, dword_t length, dword_t toggle, dword_t endpoint, dword_t device_address, dword_t packet_type, dword_t buffer);
byte_t uhci_control_or_bulk_transfer(byte_t controller_number, byte_t last_descriptor, dword_t time);
byte_t uhci_control_transfer_without_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed);
byte_t uhci_control_transfer_with_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed, word_t length);
dword_t uhci_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory);
byte_t uhci_bulk_transfer(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t transfer_type, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t uhci_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t uhci_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t uhci_hub_is_there_some_port_connection_status_change(byte_t hub_number);