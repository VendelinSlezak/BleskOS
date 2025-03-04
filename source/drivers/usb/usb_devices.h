//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_USB_DEVICES 128
struct usb_device_t {
 /* general device informations */
 byte_t is_used;

 byte_t controller_type;
 byte_t controller_number;
 byte_t port_number;
 byte_t hub_address;
 byte_t hub_port_number;
 byte_t device_speed;
 byte_t configuration;

 dword_t timeout;

 byte_t is_port_in_initalization;

 byte_t are_interfaces_initalized;
 byte_t is_interface_in_initalization;

 void (*disable_device_on_port)(dword_t number_of_controller, dword_t number_of_port);

 /* transfers */
 byte_t (*get_state_of_transfer)(byte_t device_address, void *transfer_pointer, dword_t number_of_tds);

 struct {
  byte_t request_type;
  byte_t request;
  word_t value;
  word_t index;
  word_t length;
 } setup_buffer;
 struct {
  byte_t endpoint_size;

  byte_t is_running;

  void *tms_pointer;
  void *first_td_pointer;
  dword_t number_of_tds;
  void *transfer_buffer;

  dword_t timeout;

  void (*transfer_successfull)(byte_t device_address);
  void (*transfer_error)(byte_t device_address);
 } control_transfer;
 void (*control_transfer_without_data)(byte_t device_address);
 void (*control_transfer_with_data)(byte_t device_address, byte_t transfer_direction, word_t length);
 void (*close_control_transfer)(byte_t device_address);

 void (*interrupt_transfer)(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer);
 void (*restart_interrupt_transfer)(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);
 void (*close_interrupt_transfer)(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer);

 void (*bulk_transfer)(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer);
 void (*close_bulk_transfer)(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer);

 /* interfaces */
 struct {
  byte_t is_present;
  byte_t is_initalized;
  void (*initalize)(byte_t device_address);

  dword_t power_on_to_power_good_time;
  dword_t timeout_for_finalizing_initalization;

  byte_t number_of_ports;
  byte_t actual_port;

  byte_t port_in_initalization_phase;
  dword_t port_in_initalization_timeout;

  dword_t line_of_errors;
 } hub;

 struct {
  byte_t is_present;
  byte_t is_initalized;
  void (*initalize)(byte_t device_address);
  word_t hid_descriptor_0x22_length;

  byte_t interface;
  byte_t alternative_interface;

  struct hid_entry_data_t button_1;
  struct hid_entry_data_t button_2;
  struct hid_entry_data_t button_3;
  struct hid_entry_data_t x;
  struct hid_entry_data_t y;
  struct hid_entry_data_t wheel;

  struct usb_interrupt_transfer_info_t interrupt_transfer;
 } mouse;

 struct {
  byte_t is_present;
  byte_t is_initalized;
  void (*initalize)(byte_t device_address);

  byte_t interface;
  byte_t alternative_interface;

  struct usb_interrupt_transfer_info_t interrupt_transfer;
  struct usb_interrupt_transfer_info_t interrupt_led_transfer;

  struct keyboard_leds_t leds;
  struct keyboard_keys_t keys;
  dword_t pressed_key_timeout;

  byte_t led_buffer[1];
 } keyboard;

 struct {
  byte_t is_present;
  byte_t is_initalized;
  void (*initalize)(byte_t device_address);
  byte_t reset_sequence_waiting;
  byte_t reset_sequence_running;

  byte_t interface;

  word_t max_sectors_per_transfer_request;

  byte_t unit_state;
  byte_t partitions_connected;
  dword_t number_of_sectors;
  dword_t size_of_sector;

  dword_t sector;
  dword_t number_of_rest_of_sectors;
  byte_t *memory_of_transfer;
  byte_t transfer_status;

  struct usb_bbb_t bulk_only;
  byte_t test_unit_ready_retries;
  void (*retry_after_successfull_request_sense)(byte_t device_address);
 } msd;
}__attribute__((packed));
struct usb_device_t *usb_devices;
dword_t number_of_usb_devices;