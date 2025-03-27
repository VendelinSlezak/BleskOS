//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_EHCI_CONTROLLERS 10
struct ehci_info_t {
    struct pci_device_info_t pci;

    dword_t base;
    dword_t operational_registers_base;
    byte_t number_of_ports;

    dword_t *periodic_list;
    struct ehci_empty_queue_head_t *periodic_qh;
    struct ehci_queue_head_t *asychronous_start_qh;
    struct ehci_queue_head_t *asychronous_end_qh;
};

#define EHCI_NUMBER_OF_POINTERS_IN_PERIODIC_LIST 1024
#define EHCI_INVALID_PERIODIC_POINTER (1 << 0)
#define EHCI_PERIODIC_POINTER_TO_QH (0b01 << 1)

#define EHCI_PACKET_SETUP 0b10
#define EHCI_PACKET_IN 0b01
#define EHCI_PACKET_OUT 0b00
#define EHCI_INVALID_QTD_POINTER (1 << 0)
struct ehci_queue_transfer_descriptor_t {
 dword_t next_qtd_pointer;
 dword_t alternate_next_qtd_pointer;

 union {
  byte_t status;
  struct {
   byte_t ping_or_error_handshake: 1;
   byte_t split_transaction_complete: 1;
   byte_t missed_microframe: 1;
   byte_t transaction_error: 1;
   byte_t babble: 1;
   byte_t data_buffer_error: 1;
   byte_t halted: 1;
   byte_t active: 1;
  } status_bits;
 };
 byte_t packet_identification: 2;
 byte_t error_counter: 2;
 byte_t current_page: 3;
 byte_t interrupt_on_complete: 1;
 word_t total_bytes_to_transfer: 15;
 word_t toggle_bit: 1;

 dword_t lower_buffer_pointer_0;
 dword_t lower_buffer_pointer_1;
 dword_t lower_buffer_pointer_2;
 dword_t lower_buffer_pointer_3;
 dword_t lower_buffer_pointer_4;

 dword_t upper_buffer_pointer_0;
 dword_t upper_buffer_pointer_1;
 dword_t upper_buffer_pointer_2;
 dword_t upper_buffer_pointer_3;
 dword_t upper_buffer_pointer_4;
}__attribute__((packed));

#define EHCI_ASYCHRONOUS_QH 0
#define EHCI_INTERRUPT_QH 1
#define EHCI_INVALID_QH_POINTER (1 << 0)
#define EHCI_QH_POINTS_TO_QH (0b01 << 1)
struct ehci_queue_head_t {
 dword_t horizontal_pointer;

 byte_t address: 7;
 byte_t inactivate_on_next_transaction: 1;
 byte_t endpoint: 4;
 byte_t endpoint_speed: 2;
 byte_t toggle_control: 1;
 byte_t head_of_reclamation_list: 1;
 word_t maximum_packet_length: 11;
 word_t low_full_speed_control_endpoint_flag: 1;
 word_t nak_count_reload: 4;

 byte_t interrupt_schedule_mask;
 byte_t split_completion_mask;
 word_t hub_address: 7;
 word_t hub_port_number: 7;
 word_t high_bandwidth_pipe_multiplier: 2;

 dword_t current_qtd_pointer;

 struct ehci_queue_transfer_descriptor_t td;
}__attribute__((packed));

struct ehci_empty_queue_head_t {
 dword_t horizontal_pointer;

 byte_t address: 7;
 byte_t inactivate_on_next_transaction: 1;
 byte_t endpoint: 4;
 byte_t endpoint_speed: 2;
 byte_t toggle_control: 1;
 byte_t head_of_reclamation_list: 1;
 word_t maximum_packet_length: 11;
 word_t low_full_speed_control_endpoint_flag: 1;
 word_t nak_count_reload: 4;

 byte_t interrupt_schedule_mask;
 byte_t split_completion_mask;
 word_t hub_address: 7;
 word_t hub_port_number: 7;
 word_t high_bandwidth_pipe_multiplier: 2;

 dword_t current_qtd_pointer;

 struct ehci_queue_transfer_descriptor_t td;

 byte_t padding[256-16-sizeof(struct ehci_queue_transfer_descriptor_t)];
}__attribute__((packed));

void ehci_add_new_pci_device(struct pci_device_info_t device);

void initalize_ehci_controller(dword_t number_of_controller);

byte_t ehci_acknowledge_interrupt(dword_t number_of_controller);

byte_t ehci_check_port(dword_t number_of_controller, dword_t number_of_port);
void ehci_stop_port_reset(void);
void ehci_check_if_port_is_enabled(void);
void ehci_check_if_port_passed_device(void);
void ehci_disable_device_on_port(dword_t number_of_controller, dword_t number_of_port);

void ehci_set_qh(struct ehci_queue_head_t *queue_head,
                 dword_t type_of_queue_head,
                 byte_t address,
                 byte_t endpoint,
                 word_t endpoint_size);
void ehci_set_td(byte_t *start_of_transfer_structures_memory,
                 dword_t td_number, dword_t next_td_number,
                 byte_t type,
                 byte_t toggle,
                 word_t length_of_transfer,
                 byte_t *buffer,
                 byte_t interrupt_on_complete);
void ehci_insert_asychronous_queue_head(dword_t number_of_controller, struct ehci_queue_head_t *queue_head);
void ehci_remove_asychronous_queue_head(dword_t number_of_controller, struct ehci_queue_head_t *queue_head);
byte_t get_state_of_ehci_transfer(byte_t device_address, void *transfer_descriptor_pointer, dword_t number_of_tds);

void ehci_control_transfer_without_data(byte_t device_address);
void ehci_control_transfer_with_data(byte_t device_address, byte_t transfer_direction, word_t buffer_size);
void ehci_close_control_transfer(byte_t device_address);

void ehci_interrupt_transfer(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void ehci_restart_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void ehci_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);

void ehci_bulk_transfer(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer);
void ehci_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer);