//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_NO_DEVICE_ATTACHED 0
#define USB_CONTROLLER_UHCI 1
#define USB_CONTROLLER_OHCI 2
#define USB_CONTROLLER_EHCI 3
#define USB_CONTROLLER_xHCI 4

#define PORT_NO_DEVICE 0
#define PORT_DEVICE 1
#define PORT_INITALIZED_DEVICE 2

#define USB_LOW_SPEED 1
#define USB_FULL_SPEED 2
#define USB_HIGH_SPEED 3
#define USB_SUPER_SPEED 4

#define TOGGLE_0 0
#define TOGGLE_1 1
#define ENDPOINT_0 0

#define USB_DEVICE_MASS_STORAGE 0x080650
#define USB_DEVICE_HUB 0x090000

#define USB_BOOT_PROTOCOL 0
#define USB_HID_PROTOCOL 1

#define USB_HID_USAGE_BUTTONS 1
#define USB_HID_USAGE_MOVEMENT_X 2
#define USB_HID_USAGE_MOVEMENT_Y 3
#define USB_HID_USAGE_WHEEL 4

struct usb_controller_informations {
 byte_t bus;
 byte_t device;
 byte_t function;
 byte_t type;
 dword_t base;
 dword_t base2;
 byte_t number_of_ports;
 byte_t ports_state[16];
 byte_t ports_device_speed[16];
 dword_t mem1;
 dword_t mem2;
 dword_t mem3;
 dword_t mem4;
 dword_t setup_mem;
}__attribute__((packed));
struct usb_controller_informations usb_controllers[20];

struct usb_descriptor_device_info {
 dword_t type;
 byte_t configuration;
 byte_t interface;
 byte_t alternative_interface;
 byte_t endpoint_bulk_in;
 byte_t endpoint_bulk_out;
 byte_t endpoint_interrupt;
 byte_t endpoint_interrupt_size;
 byte_t endpoint_interrupt_time;
 byte_t endpoint_interrupt_out;
 byte_t endpoint_isynchronous_in;
 word_t hid_descriptor_length;
}__attribute__((packed));
struct usb_descriptor_device_info usb_descriptor_devices[10];

byte_t usb_addresses[127];

dword_t usb_controllers_pointer = 0, usb_control_endpoint_size = 0, usb_device_address = 0, usb_new_device_detected = 0;
dword_t usb_setup_packet_mem = 0, usb_setup_packet_data_mem = 0;
byte_t usb_bulk_toggle = 0;

void initalize_usb_controllers(void);
void detect_usb_devices(void);
void detect_usb_devices_on_hubs(void);
void detect_usb_devices_on_controller(byte_t controller_number);
void detect_usb_devices_on_hub(byte_t hub_number);
void usb_remove_device(byte_t controller_number, byte_t port_number);
void detect_status_change_of_usb_devices(void);;
byte_t usb_get_port_connection_status(byte_t controller_number, byte_t port_number);
byte_t usb_get_port_connection_change_status(byte_t controller_number, byte_t port_number);
void usb_clear_port_connection_change_status(byte_t controller_number, byte_t port_number);
byte_t usb_enable_device_on_port(byte_t controller_number, byte_t port_number);
void usb_create_setup_data(byte_t request_type, byte_t request, word_t value, word_t index, word_t length);
byte_t usb_control_transfer_without_data(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index);
byte_t usb_control_transfer_with_fixed_data_length(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index, word_t length);
byte_t usb_control_transfer_with_dynamic_data_length(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t request_type, byte_t request, word_t value, word_t index);
dword_t usb_interrupt_transfer(byte_t controller_number, byte_t device_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t endpoint_size, byte_t interrupt_time, dword_t memory);
byte_t usb_bulk_in(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length_of_transfer, dword_t time_to_wait);
byte_t usb_bulk_out(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint, byte_t toggle, dword_t memory, dword_t length_of_transfer, dword_t time_to_wait);
byte_t usb_set_configuration_and_interface(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t configuration, byte_t interface, byte_t alternative_interface);
byte_t usb_reset_endpoint(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t endpoint);
void usb_configure_device_with_zero_address(byte_t controller_number, byte_t device_port, byte_t device_speed);
byte_t get_free_usb_address(void);
void release_usb_address(byte_t address);
void parse_usb_descriptor(dword_t descriptor_mem);
void parse_usb_configuration_descriptor(dword_t descriptor_mem);
void parse_hid_descriptor(dword_t descriptor_mem);