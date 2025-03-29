//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_UHCI 0
#define USB_OHCI 1
#define USB_EHCI 2
#define USB_XHCI 3

#define USB_LOW_SPEED 0
#define USB_FULL_SPEED 1
#define USB_HIGH_SPEED 2
#define USB_SUPER_SPEED 3

#define USB_TOGGLE_0 0
#define USB_TOGGLE_1 1

#define CONTROLLER_PORT_NO_CHANGE 0
#define CONTROLLER_PORT_REMOVED_DEVICE 1
#define CONTROLLER_PORT_ZERO_ADDRESS_DEVICE 2

#define USB_TRANSFER_IN 0
#define USB_TRANSFER_OUT 1

#define USB_TRANSFER_NOT_TRANSFERRED 0
#define USB_TRANSFER_DONE 1
#define USB_TRANSFER_ERROR 2

#define USB_CONTROL_TRANSFER 1
#define USB_BULK_TRANSFER 2
#define USB_INTERRUPT_TRANSFER 3
#define USB_ISOCHRONOUS_TRANSFER 4

struct usb_interrupt_transfer_info_t {
 byte_t is_running;

 byte_t endpoint;
 byte_t endpoint_size;
 byte_t interval;

 void *tms_pointer;
 void *td_pointer;
 void *transfer_buffer;

 void (*transfer_successfull)(byte_t device_address);

 dword_t line_of_errors;
}__attribute__((packed));

struct usb_bulk_transfer_info_t {
 byte_t is_running;

 byte_t endpoint;
 word_t endpoint_size;
 byte_t endpoint_toggle;

 dword_t size_of_untransferred_data;
 void *tms_pointer;
 void *td_pointer;
 dword_t number_of_tds;

 void *transfer_buffer_pointer;

 void (*transfer_successfull)(byte_t device_address, void *transfer_info);
 void (*transfer_error)(byte_t device_address, void *transfer_info);
 void *transfer_info;
 dword_t timeout;
}__attribute__((packed));

dword_t usb_device_change_event;

void initalize_usb_controllers(void);
void check_usb_ports(void);
void usb_process_port_status(byte_t port_status, byte_t type_of_controller, byte_t controller_number, byte_t port_number);
void check_usb_interface_initalization(void);
void usb_remove_device(byte_t device_address);

void usb_irq(void);
void usb_check_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);
void usb_check_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer, byte_t transfer_type);

void usb_device_set_control_transfer_variables(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout);
void usb_device_set_setup_buffer(byte_t device_address, byte_t request_type, byte_t request, word_t value, word_t index, word_t length);
void usb_control_transfer_without_data(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout, byte_t request_type, byte_t request, word_t value, word_t index);
void usb_control_transfer_in(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout, byte_t request_type, byte_t request, word_t value, word_t index, word_t length);
void usb_control_transfer_out(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout, byte_t request_type, byte_t request, word_t value, word_t index, word_t length, byte_t *buffer);
void usb_close_control_transfer(byte_t device_address);

void usb_interrupt_transfer_in(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer, void (*transfer_successfull)(byte_t device_address));
void usb_interrupt_transfer_out(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer, byte_t *buffer, void (*transfer_successfull)(byte_t device_address));
void usb_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);

void usb_bulk_transfer_out(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer, void (*transfer_successfull)(byte_t device_address, void *transfer_info), void (*transfer_error)(byte_t device_address, void *transfer_info), void *transfer_info, dword_t timeout, byte_t *buffer, dword_t length);
void usb_bulk_transfer_in(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer, void (*transfer_successfull)(byte_t device_address, void *transfer_info), void (*transfer_error)(byte_t device_address, void *transfer_info), void *transfer_info, dword_t timeout, byte_t *buffer, dword_t length);
void usb_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer);