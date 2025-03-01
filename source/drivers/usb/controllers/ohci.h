//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define OHCI_NUMBER_OF_POINTERS_IN_INTERRUPT_TABLE 32
struct ohci_hcca_t {
 dword_t interrupt_table[OHCI_NUMBER_OF_POINTERS_IN_INTERRUPT_TABLE];
 word_t frame_number;
 word_t padding1; 
 dword_t done_head;
 byte_t padding2[116]; //padding to 256 bytes
}__attribute__((packed));

struct ohci_endpoint_descriptor_t {
 dword_t address: 7;
 dword_t endpoint: 4;
 dword_t direction: 2;
 dword_t low_speed_transfer: 1;
 dword_t skip: 1;
 dword_t isochronous_transfer_descriptors: 1;
 dword_t maximum_packet_length: 11;
 dword_t reserved1: 5;

 dword_t tail_of_td;

 dword_t halted: 1;
 dword_t toggle_carry: 1;
 dword_t reserved2: 2;
 dword_t head_of_td: 28;

 dword_t next_ed_pointer;
}__attribute__((packed));

#define OHCI_PACKET_SETUP 0b00
#define OHCI_PACKET_OUT 0b01
#define OHCI_PACKET_IN 0b10
struct ohci_transfer_descriptor_t {
 dword_t reserved: 18;
 dword_t buffer_rounding: 1;
 dword_t packet_identification: 2;
 dword_t delay_interrupt: 3;
 dword_t toggle: 2;
 dword_t error_counter: 2;
 dword_t status: 4;

 dword_t current_buffer_pointer;
 dword_t next_td_pointer;
 dword_t buffer_end;
}__attribute__((packed));

#define MAX_NUMBER_OF_OHCI_CONTROLLERS 10
struct ohci_controller_info_t {
 byte_t bus;
 byte_t device;
 byte_t function;

 dword_t base;
 byte_t irq;
 byte_t number_of_ports;

 struct ohci_hcca_t *hcca;
 struct ohci_endpoint_descriptor_t *interrupt_endpoint_descriptor;
}__attribute__((packed));
struct ohci_controller_info_t ohci_controllers[MAX_NUMBER_OF_OHCI_CONTROLLERS];
dword_t number_of_ohci_controllers;

void initalize_ohci_controller(byte_t controller_number);

byte_t ohci_acknowledge_interrupt(dword_t number_of_controller);

byte_t ohci_check_port(dword_t number_of_controller, dword_t number_of_port);
void ohci_enable_port(void);
void ohci_check_if_port_is_enabled(void);
void ohci_disable_device_on_port(dword_t number_of_controller, dword_t number_of_port);

void ohci_set_ed(struct ohci_endpoint_descriptor_t *ed,
                 byte_t address,
                 byte_t endpoint,
                 word_t endpoint_size,
                 dword_t number_of_tds);
void ohci_set_td(byte_t *tms,
                 dword_t td_number, dword_t next_td_number,
                 byte_t type,
                 byte_t interrupt_on_complete,
                 byte_t toggle,
                 byte_t *buffer,
                 word_t length_of_buffer);
void ohci_insert_endpoint_descriptor(dword_t number_of_controller, dword_t type_of_transfer, struct ohci_endpoint_descriptor_t *ed);
void ohci_remove_endpoint_descriptor(dword_t number_of_controller, dword_t type_of_transfer, struct ohci_endpoint_descriptor_t *ed);
byte_t get_state_of_ohci_transfer(byte_t device_address, void *transfer_descriptor_pointer, dword_t number_of_tds);

void ohci_control_transfer_without_data(byte_t device_address);
void ohci_control_transfer_with_data(byte_t device_address, byte_t transfer_direction, word_t buffer_size);
void ohci_close_control_transfer(byte_t device_address);

void ohci_interrupt_transfer(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void ohci_restart_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void ohci_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);

void ohci_bulk_transfer(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer);
void ohci_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer);