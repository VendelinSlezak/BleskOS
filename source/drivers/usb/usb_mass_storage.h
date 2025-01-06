//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_DEVICE_MASS_STORAGE 0x080650

#define USB_MSD_ENTRY_NOTHING_ATTACHED 0
#define USB_MSD_ENTRY_DEVICE_ATTACHED 1
#define USB_MSD_ENTRY_DEVICE_INITALIZED 2

struct usb_mass_storage_device_t {
 byte_t entry;

 byte_t controller_number;
 byte_t controller_type;
 byte_t interface;
 byte_t port;
 byte_t address;
 byte_t device_speed;

 dword_t ehci_hub_address;
 dword_t ehci_hub_port_number;

 byte_t endpoint_in;
 byte_t toggle_in;
 byte_t endpoint_out;
 byte_t toggle_out;

 dword_t size_in_sectors;
 dword_t size_of_sector;
}__attribute__((packed));
#define MAX_NUMBER_OF_USB_MSD_DEVICES 10
struct usb_mass_storage_device_t usb_mass_storage_devices[MAX_NUMBER_OF_USB_MSD_DEVICES];

#define USB_CBW_DIRECTION_READ 1
#define USB_CBW_DIRECTION_WRITE 0
struct usb_command_block_wrapper_t {
 dword_t signature;
 dword_t transfer_id;
 dword_t transfer_length;
 byte_t reserved: 7;
 byte_t direction: 1;
 byte_t lun;
 byte_t command_length;
 union {
  struct {
   byte_t command;
   byte_t product_data;
   byte_t page_code;
   word_t big_endian_transfer_length;
   byte_t control;
   byte_t zeroes[10];
  }__attribute__((packed)) inquiry;
  struct {
   byte_t command;
   byte_t reserved[3];
   byte_t transfer_length;
   byte_t control;
   byte_t zeroes[10];
  }__attribute__((packed)) request_sense;
  struct {
   byte_t command;
   byte_t reserved1;
   dword_t lba;
   byte_t reserved2[3];
   byte_t control;
   byte_t zeroes[6];
  }__attribute__((packed)) read_capacity;
  struct {
   byte_t command;
   byte_t reserved1;
   dword_t big_endian_lba;
   byte_t reserved2;
   word_t big_endian_number_of_sectors;
   byte_t control;
   byte_t zeroes[6];
  }__attribute__((packed)) read_write_command;
 };
}__attribute__((packed));
struct usb_command_block_wrapper_t *usb_command_block_wrapper;

struct usb_command_status_wrapper_t {
 dword_t signature;
 dword_t transfer_id;
 dword_t data_residue;
 byte_t status;
}__attribute__((packed));
struct usb_command_status_wrapper_t *usb_command_status_wrapper;

struct usb_msd_inquiry_t {
 byte_t peripheral_device_type: 5;
 byte_t peripheral_qualifier: 3;
 byte_t reserved1: 7;
 byte_t rmb: 1;
 byte_t version;
 byte_t response_data_format: 4;
 byte_t hi_sup: 1;
 byte_t norm_aca: 1;
 byte_t obsolete1: 2;
 byte_t additional_length;
 byte_t reserved2;
 byte_t reserved3;
 byte_t reserved4;
 byte_t vendor_id[8];
 byte_t product_id[16];
 byte_t product_revision_level[4];
}__attribute__((packed));

struct usb_msd_request_sense_t {
 byte_t response_code: 7;
 byte_t valid: 1;
 byte_t segment_number;
 byte_t sense_key: 4;
 byte_t reserved: 1;
 byte_t ili: 1;
 byte_t eom: 1;
 byte_t filemark: 1;
 byte_t information[4];
 byte_t additional_sense_length;
 byte_t command_specific_info[4];
 byte_t additional_sense_code;
 byte_t additional_sense_qualifier;
 byte_t field_replaceable_unit_code;
 byte_t sense_key_specific[3];
}__attribute__((packed));

void usb_mass_storage_initalize(byte_t device_number);

void usb_msd_recover_from_error(byte_t device_number);

void usb_msd_prepare_cbw_for_command(byte_t command_length, dword_t transfer_length);
byte_t usb_msd_transfer_command(byte_t device_number, byte_t direction, dword_t memory, dword_t length_of_transfer);

byte_t usb_msd_read_inquiry(byte_t device_number);
byte_t usb_msd_test_unit_ready(byte_t device_number);
byte_t usb_msd_send_request_sense(byte_t device_number);
byte_t usb_msd_read_capacity(byte_t device_number);

byte_t usb_msd_read(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory);
byte_t usb_msd_write(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory);