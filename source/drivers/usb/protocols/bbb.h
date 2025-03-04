//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

struct usb_command_status_wrapper_t {
 dword_t signature;
 dword_t transfer_id;
 dword_t data_residue;
 byte_t status;
}__attribute__((packed));

struct usb_bbb_t {
 byte_t is_running;

 struct usb_bulk_transfer_info_t in;
 struct usb_bulk_transfer_info_t out;
 void *buffer;

 struct usb_command_block_wrapper_t usb_command_block_wrapper;
 struct usb_command_status_wrapper_t usb_command_status_wrapper;

 void (*command_successfull)(byte_t device_address);
 void (*command_error)(byte_t device_address);
}__attribute__((packed));

void usb_bulk_only_prepare_transfer(struct usb_bbb_t *bulk_only, void (*command_successfull)(byte_t device_address), void (*command_error)(byte_t device_address), byte_t transfer_direction, dword_t command_length, dword_t transfer_length, void *buffer);
void usb_bulk_only_send_command(byte_t device_address, struct usb_bbb_t *bulk_only);
void usb_bulk_only_send_cbw_success(byte_t device_address, void *transfer_info);
void usb_bulk_only_transfer_data_success(byte_t device_address, void *transfer_info);
void usb_bulk_only_read_csw_success(byte_t device_address, void *transfer_info);

void usb_bulk_only_close_transfer(byte_t device_address, struct usb_bbb_t *bulk_only);
void usb_bulk_only_send_cbw_error(byte_t device_address, void *transfer_info);
void usb_bulk_only_transfer_data_error(byte_t device_address, void *transfer_info);
void usb_bulk_only_read_csw_error(byte_t device_address, void *transfer_info);