//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define OHCI_SETUP (0 << 19)
#define OHCI_OUT (1 << 19)
#define OHCI_IN (2 << 19)

dword_t ohci_toggle;

void initalize_ohci_controller(byte_t controller_number);
byte_t ohci_controller_detect_status_change(byte_t controller_number);
void ohci_controller_detect_devices(byte_t controller_number);
void ohci_hub_detect_devices(byte_t hub_number);
void ohci_remove_device_if_connected(byte_t controller_number, byte_t port);
byte_t ohci_control_transfer(byte_t controller_number, dword_t last_td);
byte_t ohci_bulk_transfer(byte_t controller_number, dword_t last_td, dword_t time);
void ohci_create_ed(byte_t controller_number, byte_t device_speed, dword_t ed_number, dword_t address, dword_t endpoint, dword_t first_td, dword_t num_of_td);
void ohci_create_td(byte_t controller_number, dword_t td_number, dword_t next_td, dword_t attributes, dword_t toggle, dword_t buffer, dword_t buffer_length);
void ohci_transfer_set_setup(byte_t controller_number, byte_t device_speed);
byte_t ohci_set_address(byte_t controller_number, byte_t device_speed);
void ohci_set_configuration(byte_t controller_number, byte_t device_speed, dword_t configuration_num);
void ohci_set_interface(byte_t controller_number, byte_t device_speed, dword_t interface_num, dword_t alt_interface_num);
void ohci_set_idle(byte_t controller_number, byte_t device_speed, dword_t interface_num, byte_t idle_value);
void ohci_set_protocol(byte_t controller_number, byte_t device_speed, dword_t interface_num, dword_t protocol_type);
void ohci_set_feature(byte_t controller_number, byte_t device_speed, dword_t destination, dword_t value);
void ohci_clear_feature(byte_t controller_number, byte_t device_speed, dword_t destination, dword_t value);
void ohci_set_keyboard_led(byte_t controller_number, byte_t device_speed, dword_t interface_num, byte_t endpoint, byte_t led_state);
void ohci_set_interrupt_transfer(byte_t controller_number, byte_t device_speed, byte_t ed_offset, byte_t td_offset, byte_t endpoint, byte_t transfer_length, byte_t transfer_time, dword_t memory);
void ohci_send_setup_to_device(byte_t controller_number, byte_t device_speed, word_t max_length, dword_t memory, byte_t control_endpoint_length);
dword_t ohci_read_descriptor(byte_t controller_number, byte_t device_speed, byte_t type_of_transfer, byte_t control_endpoint_length);
byte_t ohci_read_configuration_descriptor(byte_t controller_number, byte_t port, byte_t device_speed, byte_t control_endpoint_length);
void ohci_read_hid_descriptor(byte_t controller_number, byte_t device_speed, byte_t interface, word_t length, byte_t control_endpoint_length);
byte_t ohci_read_hub_descriptor(byte_t controller_number, byte_t device_speed, byte_t control_endpoint_length);
dword_t ohci_read_hub_port_status(byte_t controller_number, byte_t device_speed, byte_t address, byte_t port);
byte_t ohci_hub_is_there_some_port_status_change(byte_t hub_number);
byte_t ohci_bulk_out(byte_t controller_number, byte_t device_speed, byte_t address, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t ohci_bulk_in(byte_t controller_number, byte_t device_speed, byte_t address, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
