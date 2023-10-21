//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EHCI_NO_POINTER 0x1

#define EHCI_SETUP 0x2
#define EHCI_IN 0x1
#define EHCI_OUT 0x0

byte_t ehci_hub_address = 0, ehci_hub_port_number = 0;

void initalize_ehci_controller(byte_t controller_number);
byte_t ehci_controller_detect_status_change(byte_t controller_number);
byte_t ehci_get_port_connection_status(byte_t controller_number, byte_t port_number);
byte_t ehci_get_port_connection_change_status(byte_t controller_number, byte_t port_number);
void ehci_clear_port_connection_change_status(byte_t controller_number, byte_t port_number);
byte_t ehci_enable_device_on_port(byte_t controller_number, byte_t port_number);
void ehci_set_qh(byte_t controller_number, dword_t qh_number, dword_t first_td, dword_t device_address, dword_t device_speed, dword_t endpoint);
void ehci_set_td(byte_t controller_number, dword_t td_number, dword_t next_td_number, dword_t packet_type, dword_t length, dword_t toggle, dword_t memory);
byte_t ehci_control_or_bulk_transfer(byte_t controller_number, byte_t last_descriptor, dword_t time);
byte_t ehci_control_transfer_without_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed);
byte_t ehci_control_transfer_with_data_stage(byte_t controller_number, byte_t device_address, byte_t device_speed, word_t length);
dword_t ehci_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory);
byte_t ehci_bulk_transfer(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t transfer_type, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t ehci_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t ehci_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t ehci_hub_is_there_some_port_connection_status_change(byte_t hub_number);