//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EHCI_NO_POINTER 0x00000001
#define EHCI_SETUP 0x2
#define EHCI_IN 0x1
#define EHCI_OUT 0x0

byte_t ehci_toggle;

void initalize_ehci_controller(byte_t controller_number);
byte_t ehci_controller_detect_status_change(byte_t controller_number);
void ehci_controller_detect_devices(byte_t controller_number);
void ehci_remove_device_if_connected(byte_t controller_number, byte_t port);
void ehci_create_queue_head(byte_t controller_number, dword_t address, dword_t endpoint, dword_t endpoint_max_length);
void ehci_create_transfer_descriptor(byte_t controller_number, dword_t td_number, dword_t next_td_number, dword_t alternate_td_number, dword_t packet_type, dword_t length, dword_t toggle, dword_t memory);
byte_t ehci_run_transfer(dword_t controller_number, dword_t td_number, dword_t time);
void ehci_read_descriptor(byte_t controller_number, byte_t port);
void ehci_set_address(byte_t controller_number, byte_t port);
void ehci_transfer_set_setup(byte_t controller_number, byte_t port);
void ehci_set_configuration(byte_t controller_number, byte_t port, dword_t configuration_num);
void ehci_set_interface(byte_t controller_number, byte_t port, dword_t interface_num, dword_t alt_interface_num);
void ehci_read_configuration_descriptor(byte_t controller_number, byte_t port);
byte_t ehci_bulk_out(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
byte_t ehci_bulk_in(byte_t controller_number, byte_t port, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length, dword_t time_to_wait);
