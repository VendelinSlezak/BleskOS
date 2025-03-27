//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_UHCI_CONTROLLERS 10
struct uhci_info_t {
    struct pci_device_info_t pci;

    word_t base;
    byte_t number_of_ports;

    dword_t *frame_list;
    struct uhci_queue_head_t *queue_head;
};

#define UHCI_INVALID_QH_POINTER (1 << 0)
#define UHCI_QH_POINTS_TO_QH (1 << 1)
struct uhci_queue_head_t {
 dword_t head_pointer;
 dword_t element_pointer;

 //rest is for align and software use
 byte_t interrupt_interval;
 byte_t software_use[23];
}__attribute__((packed));

#define UHCI_NUMBER_OF_FRAMES_IN_FRAME_LIST 1024
#define UHCI_VALID_FRAME (0 << 0)
#define UHCI_INVALID_FRAME (1 << 0)
#define UHCI_FRAME_POINTS_TO_QH (1 << 1)

#define UHCI_PACKET_IN 0x69
#define UHCI_PACKET_OUT 0xE1
#define UHCI_PACKET_SETUP 0x2D
struct uhci_transfer_descriptor_t {
 dword_t terminate: 1;
 dword_t link_points_to_qh: 1;
 dword_t depth_first: 1;
 dword_t reserved1: 1;
 dword_t link_pointer: 28;

 word_t number_of_transferred_bytes: 11;
 word_t reserved2: 5;
 union {
  byte_t status;
  struct {
   byte_t reserved3: 1;
   byte_t bitstuff_error: 1;
   byte_t crc_timeout_error: 1;
   byte_t nak_received: 1;
   byte_t babble_detected: 1;
   byte_t data_buffer_error: 1;
   byte_t stalled: 1;
   byte_t active: 1;
  } status_bits;
 };
 byte_t interrupt_on_complete: 1;
 byte_t isochronous_transfer: 1;
 byte_t low_speed_transfer: 1;
 byte_t error_counter: 2;
 byte_t enable_short_packet_detect: 1;
 byte_t reserved4: 2;

 byte_t packet_identification;
 dword_t address: 7;
 dword_t endpoint: 4;
 dword_t toggle_bit: 1;
 dword_t reserved5: 1;
 dword_t length_of_transfer: 11;

 byte_t *buffer;

 byte_t software_use[16];
}__attribute__((packed));

struct uhci_transfer_memory_structure_t {
 struct uhci_queue_head_t queue_head;
 struct uhci_transfer_descriptor_t transfer_descriptor[];
}__attribute__((packed));

void uhci_add_new_pci_device(struct pci_device_info_t device);

void initalize_uhci_controller(dword_t number_of_controller);

byte_t uhci_acknowledge_interrupt(dword_t number_of_controller);

byte_t uhci_check_port(dword_t number_of_controller, dword_t number_of_port);
void uhci_stop_port_reset(void);
void uhci_enable_port(void);
void uhci_check_if_port_is_enabled(void);
void uhci_disable_device_on_port(dword_t number_of_controller, dword_t number_of_port);

void uhci_set_td(byte_t *start_of_td_memory, dword_t td_number, dword_t next_td_number, byte_t speed, byte_t type, byte_t address, byte_t endpoint, byte_t toggle, word_t length_of_transfer, byte_t *buffer, byte_t interrupt_on_complete, byte_t is_isochronous);
void uhci_insert_queue_head(dword_t number_of_controller, struct uhci_queue_head_t *queue_head, dword_t interval);
void uhci_remove_queue_head(dword_t number_of_controller, struct uhci_queue_head_t *queue_head);
byte_t get_state_of_uhci_transfer(byte_t device_address, void *transfer_descriptor_pointer, dword_t number_of_tds);

void uhci_control_transfer_without_data(byte_t device_address);
void uhci_control_transfer_with_data(byte_t device_address, byte_t transfer_direction, word_t buffer_size);
void uhci_close_control_transfer(byte_t device_address);

void uhci_interrupt_transfer(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void uhci_restart_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void uhci_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);

void uhci_bulk_transfer(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer);
void uhci_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer);

/* deep debugger */
struct ddbg_value_parsing_info_t ddbg_uhci_reg_usbcmd[] = {
 {1, "Run", 'd'},
 {1, "Host Controller Reset", 'd'},
 {1, "Global Reset", 'd'},
 {1, "Global Suspend", 'd'},
 {1, "Global Resume", 'd'},
 {1, "Software Debug", 'd'},
 {1, "Configured", 'd'},
 {1, "Max packet size", 'd'},
 {8, 0, 'x'},
 {0, 0, 0}
};
struct ddbg_value_parsing_info_t ddbg_uhci_reg_usbsts[] = {
 {1, "Interrupt", 'd'},
 {1, "Error Interrupt", 'd'},
 {1, "Resume Detected", 'd'},
 {1, "System Error", 'd'},
 {1, "Process Error", 'd'},
 {1, "Halted", 'd'},
 {10, 0, 'x'},
 {0, 0, 0}
};
struct ddbg_value_parsing_info_t ddbg_uhci_reg_usbintr[] = {
 {1, "Timeout CRC", 'd'},
 {1, "Resume", 'd'},
 {1, "Complete", 'd'},
 {1, "Short Packet", 'd'},
 {12, 0, 'x'},
 {0, 0, 0}
};
struct ddbg_value_parsing_info_t ddbg_uhci_reg_port[] = {
 {1, "Device Connected", 'd'},
 {1, "Connection Status Change", 'd'},
 {1, "Port Enabled", 'd'},
 {1, "Enable Status Change", 'd'},
 {2, "Line Status", 'x'},
 {1, "Resume Detected", 'd'},
 {1, "Reserved For Port Detection", 'd'},
 {1, "Low Speed Device", 'd'},
 {1, "Port Reset", 'd'},
 {2, 0, 'x'},
 {1, "Port Suspended", 'd'},
 {3, 0, 'x'},
 {0, 0, 0}
};
void ddbg_show_devregs_uhci(dword_t number_of_controller);