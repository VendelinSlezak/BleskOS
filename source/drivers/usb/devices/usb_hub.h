//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_HUB_REQUEST_PORT_RESET 0x04
#define USB_HUB_REQUEST_POWER_PORT 0x08
#define USB_HUB_CLEAR_PORT_CHANGE_STATUS 0x10
#define USB_HUB_CLEAR_PORT_RESET 0x14

#define USB_HUB_PORT_PHASE_NONE 0
#define USB_HUB_PORT_PHASE_RESET 1
#define USB_HUB_PORT_PHASE_AFTER_RESET 2

struct usb_hub_descriptor_t {
 byte_t length;
 byte_t type;
 byte_t number_of_ports;
 word_t hub_chars;
 byte_t power_on_to_power_good_time;
 byte_t maximum_hub_current;
 byte_t removable[];
}__attribute__((packed));

void usb_hub_save_informations(byte_t device_address, struct usb_full_interface_info_t interface);
void usb_hub_initalize(byte_t device_address);
void usb_hub_get_descriptor_success(byte_t device_address);
void usb_hub_power_port_success(byte_t device_address);

void usb_hub_check_connections(void);
void usb_hub_get_port_status_success(byte_t device_address);
void usb_hub_port_start_reset_success(byte_t device_address);
void usb_hub_port_stop_reset(byte_t device_address);
void usb_hub_port_stop_reset_success(byte_t device_address);
void usb_hub_clear_status_change(byte_t device_address);
void usb_hub_clear_status_change_success(byte_t device_address);
void usb_hub_get_port_status_again_success(byte_t device_address);

void usb_hub_stop_initalization(byte_t device_address, byte_t *err_string);
void usb_hub_get_descriptor_error(byte_t device_address);
void usb_hub_power_port_error(byte_t device_address);
void usb_hub_error_message(byte_t device_address, byte_t *err_string);
void usb_hub_get_port_status_error(byte_t device_address);
void usb_hub_port_start_reset_error(byte_t device_address);
void usb_hub_port_stop_reset_error(byte_t device_address);
void usb_hub_clear_status_change_error(byte_t device_address);
void usb_hub_get_port_status_again_error(byte_t device_address);