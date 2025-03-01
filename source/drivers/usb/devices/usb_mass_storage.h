//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_MSD_WITHOUT_UNIT 0
#define USB_MSD_WITH_UNIT 1

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

struct usb_read_capacity_t {
 dword_t big_endian_number_of_sectors_on_disk;
 dword_t big_endian_size_of_sector_on_disk;
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

void usb_msd_save_informations(byte_t device_address, struct usb_full_interface_info_t interface);
void usb_msd_initalize(byte_t device_address);
void usb_msd_inquiry_success(byte_t device_address);

void usb_msd_monitor_unit_state(void);
void usb_msd_send_tur(byte_t device_address);
void usb_msd_tur_success(byte_t device_address);
void usb_msd_read_capacity_success(byte_t device_address);

byte_t usb_msd_read(byte_t device_address, dword_t sector, dword_t number_of_sectors, byte_t *memory);
byte_t usb_msd_write(byte_t device_address, dword_t sector, dword_t number_of_sectors, byte_t *memory);
byte_t usb_msd_transfer(byte_t device_address, byte_t direction, dword_t sector, dword_t number_of_sectors, byte_t *memory);
void usb_msd_read_10(byte_t device_address, dword_t sector, word_t number_of_sectors, byte_t *memory);
void usb_msd_read_10_success(byte_t device_address);
void usb_msd_write_10(byte_t device_address, dword_t sector, word_t number_of_sectors, byte_t *memory);
void usb_msd_write_10_success(byte_t device_address);
void usb_msd_transfer_not_successfull(byte_t device_address);

void usb_msd_send_request_sense(byte_t device_address);
void usb_msd_request_sense_success(byte_t device_address);

void usb_msd_inquiry_error(byte_t device_address);
void usb_msd_tur_error(byte_t device_address);
void usb_msd_read_capacity_error(byte_t device_address);
void usb_msd_read_10_error(byte_t device_address);
void usb_msd_write_10_error(byte_t device_address);
void usb_msd_request_sense_error(byte_t device_address);

void usb_msd_call_reset_sequence(byte_t device_address);
void usb_msd_reset_sequence_device_reset_success(byte_t device_address);
void usb_msd_reset_sequence_reset_endpoint_in_success(byte_t device_address);
void usb_msd_reset_sequence_reset_endpoint_out_success(byte_t device_address);
void usb_msd_reset_sequence_error(byte_t device_address);