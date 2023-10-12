//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define UHCI_NO_DESCRIPTOR 0xFFFFFFFF
#define UHCI_NO_POINTER 0x00000001
#define UHCI_SETUP 0x2D
#define UHCI_IN 0x69
#define UHCI_OUT 0xE1

byte_t uhci_toggle;

void initalize_uhci_controller(byte_t controller_number);
byte_t uhci_controller_detect_status_change(byte_t controller_number);
void uhci_controller_detect_devices(byte_t controller_number);
void uhci_remove_device_if_connected(byte_t controller_number, byte_t port);
void uhci_set_frame_list(byte_t controller_number, byte_t offset, dword_t value, byte_t frequency);
void uhci_set_queue_head(byte_t controller_number, dword_t offset, dword_t first_value, dword_t second_value);
void uhci_set_transfer_descriptor(byte_t controller_number, byte_t device_speed, dword_t descriptor, dword_t next_descriptor, dword_t length, dword_t toggle, dword_t endpoint, dword_t device_address, dword_t packet_type, dword_t buffer);
byte_t uhci_wait_for_transfer(dword_t controller_number, dword_t descriptor, dword_t time);
void uhci_transfer_set_setup(byte_t controller_number, byte_t device_speed);
byte_t uhci_set_address(byte_t controller_number, byte_t device_speed);
void uhci_set_configuration(byte_t controller_number, byte_t device_speed, dword_t configuration_num);
void uhci_set_interface(byte_t controller_number, byte_t device_speed, dword_t interface_num, dword_t alt_interface_num);
void uhci_set_idle(byte_t controller_number, byte_t device_speed, dword_t interface_num, byte_t idle_value);
void uhci_set_protocol(byte_t controller_number, byte_t device_speed, dword_t interface_num, dword_t protocol_type);
void uhci_set_keyboard_led(byte_t controller_number, byte_t device_speed, byte_t address, dword_t interface_num, byte_t endpoint, byte_t led_state);
void uhci_set_interrupt_transfer(byte_t controller_number, byte_t device_speed, byte_t frame_offset, byte_t transfer_offset, byte_t endpoint, byte_t transfer_length, byte_t transfer_time, dword_t memory);
void uhci_send_setup_to_device(byte_t controller_number, byte_t device_speed, word_t max_length, dword_t memory, byte_t control_endpoint_length);
dword_t uhci_read_descriptor(byte_t controller_number, byte_t device_speed, byte_t type_of_transfer, byte_t control_endpoint_length);
byte_t uhci_read_configuration_descriptor(byte_t controller_number, byte_t port, byte_t device_speed, byte_t control_endpoint_length);
void uhci_read_hid_descriptor(byte_t controller_number, byte_t device_speed, byte_t interface, word_t length, byte_t control_endpoint_length);
byte_t uhci_bulk_out(byte_t controller_number, byte_t device_speed, byte_t address, byte_t endpoint, byte_t toggle, dword_t memory, dword_t max_length, dword_t time_to_wait);
byte_t uhci_bulk_in(byte_t controller_number, byte_t device_speed, byte_t address, byte_t endpoint, byte_t toggle, dword_t memory, dword_t max_length, dword_t time_to_wait);
