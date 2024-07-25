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

struct usb_controllers_t {
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
#define MAX_NUMBER_OF_USB_CONTROLLERS 20
struct usb_controllers_t usb_controllers[MAX_NUMBER_OF_USB_CONTROLLERS];

struct usb_device_interfaces_t {
 dword_t type;

 byte_t configuration;
 byte_t interface;
 byte_t alternative_interface;

 byte_t endpoint_bulk_in;

 byte_t endpoint_bulk_out;

 byte_t endpoint_interrupt;
 word_t endpoint_interrupt_size;
 byte_t endpoint_interrupt_time;

 word_t hid_descriptor_0x22_length;
}__attribute__((packed));
#define MAX_NUMBER_OF_INTERFACES_IN_ONE_USB_DEVICE 10
struct usb_device_interfaces_t usb_device_interfaces[MAX_NUMBER_OF_INTERFACES_IN_ONE_USB_DEVICE];

struct usb_setup_packet_t {
  byte_t request_type;
  byte_t request;
  word_t value;
  word_t index;
  word_t length;
 }__attribute__((packed));

struct usb_device_descriptor_t {
 byte_t length;
 byte_t descriptor_type;
 word_t usb_specification_release_number;
 byte_t device_class;
 byte_t device_subclass;
 byte_t device_protocol;
 byte_t size_of_control_endpoint;
 word_t vendor_id;
 word_t product_id;
 word_t device_release_number;
 byte_t index_of_manufacturer_string;
 byte_t index_of_product_string;
 byte_t index_of_serial_number_string;
 byte_t number_of_configurations;
}__attribute__((packed));

struct usb_header_of_descriptor_t {
 byte_t bLength;
 byte_t bDescriptorType;
}__attribute__((packed));

#define USB_CONFIGURATION_DESCRIPTOR_TYPE 0x02
struct usb_configuration_descriptor_t {
 byte_t bLength;
 byte_t bDescriptorType;
 word_t wTotalLength;
 byte_t bNumInterfaces;
 byte_t bConfigurationValue;
 byte_t iConfiguration;
 byte_t bmAttributes;
 byte_t bMaxPower;
}__attribute__((packed));

#define USB_INTERFACE_DESCRIPTOR_TYPE 0x04
struct usb_interface_descriptor_t {
 byte_t bLength;
 byte_t bDescriptorType;
 byte_t bInterfaceNumber;
 byte_t bAlternateSetting;
 byte_t bNumEndpoints;
 byte_t bInterfaceClass;
 byte_t bInterfaceSubClass;
 byte_t bInterfaceProtocol;
 byte_t iInterface;
}__attribute__((packed));

#define USB_ENDPOINT_DESCRIPTOR_TYPE 0x05
#define USB_ENDPOINT_DIRECTION_OUT 0b0
#define USB_ENDPOINT_DIRECTION_IN 0b1
#define USB_ENDPOINT_TYPE_CONTROL 0b00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS 0b01
#define USB_ENDPOINT_TYPE_BULK 0b10
#define USB_ENDPOINT_TYPE_INTERRUPT 0b11
struct usb_endpoint_descriptor_t {
 byte_t bLength;
 byte_t bDescriptorType;
 byte_t endpoint_number: 4;
 byte_t reserved: 3;
 byte_t endpoint_direction: 1;
 byte_t bmAttributes;
 word_t wMaxPacketSize;
 byte_t bInterval;
}__attribute__((packed));

#define USB_HID_DESCRIPTOR_TYPE 0x21
struct usb_hid_descriptor_t {
 byte_t bLength;
 byte_t bDescriptorType;
 word_t bcdHID;
 byte_t bCountryCode;
 byte_t bNumDescriptors;
 struct {
  byte_t bDescriptorType;
  byte_t bDescriptorLength;
 }__attribute__((packed)) descriptors[];
}__attribute__((packed));

#define HID_LONG_ENTRY_SIGNATURE 0xFE
#define HID_SHORT_ENTRY_TYPE_MAIN 0b00
#define HID_SHORT_ENTRY_TYPE_GLOBAL 0b01
#define HID_SHORT_ENTRY_TYPE_LOCAL 0b10
#define HID_SHORT_ENTRY_TYPE_RESERVED 0b11
struct hid_entry_t {
 union {
  struct {
   byte_t size: 2;
   byte_t type: 2;
   byte_t tag: 4;
   byte_t data[];
  }__attribute__((packed)) short_item;
  struct {
   byte_t signature;
   byte_t size_in_bytes;
   byte_t tag;
   byte_t data[];
  }__attribute__((packed)) long_item;
 };
}__attribute__((packed));

#define USB_HID_USAGE_BUTTON 0x09
#define USB_HID_USAGE_MOVEMENT_X 0x30
#define USB_HID_USAGE_MOVEMENT_Y 0x31
#define USB_HID_USAGE_WHEEL 0x38
#define USB_HID_USAGE_ID 0xFF
struct hid_parsed_entry_t {
 byte_t offset_in_bits;
 byte_t length_in_bits;
 byte_t usage;
 dword_t usage_value;
}__attribute__((packed));

struct usb_unidentified_connected_device_t {
 byte_t controller_number;
 byte_t port_number;
 byte_t address;
}__attribute__((packed));
struct usb_unidentified_connected_device_t usb_unidentified_connected_devices[127];
byte_t usb_addresses[32];

dword_t usb_controllers_pointer = 0, usb_control_endpoint_size = 0, usb_device_address = 0, usb_new_device_detected = 0;
byte_t usb_bulk_toggle = 0;

struct usb_setup_packet_t *usb_setup_packet;
byte_t *usb_setup_packet_data;
#define USB_SETUP_PACKET_MAX_SIZE_OF_DATA 0xFFFF

#define MAX_NUMBER_OF_HID_PARSED_ENTRIES 256
struct hid_parsed_entry_t *hid_parsed_entries;
dword_t hid_entry_offset_byte, hid_entry_offset_shift, hid_entry_offset_length;

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
byte_t get_hid_descriptor_entry(byte_t usage, byte_t usage_value);