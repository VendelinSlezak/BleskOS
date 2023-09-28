//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_MSD_NOT_ATTACHED 0
#define USB_MSD_ATTACHED 1
#define USB_MSD_INITALIZED 2

#define USB_CBW_READ 0x80
#define USB_CBW_WRITE 0x00

#define USB_MSD_REQUEST_SENSE 0x03
#define USB_MSD_INQUIRY 0x12
#define USB_MSD_CAPACITY 0x25
#define USB_MSD_READ_10 0x28
#define USB_MSD_WRITE_10 0x2A

struct usb_mass_storage_device {
 byte_t type;
 byte_t controller_number;
 byte_t controller_type;
 byte_t port;
 byte_t endpoint_in;
 byte_t toggle_in;
 byte_t endpoint_out;
 byte_t toggle_out;
 dword_t size_in_sectors;
 dword_t size_of_sector;
 byte_t partitions_type[4];
 dword_t partitions_first_sector[4];
 dword_t partitions_num_of_sectors[4];
};
struct usb_mass_storage_device usb_mass_storage_devices[10];

dword_t usb_mass_storage_cbw_memory, usb_mass_storage_response_memory, usb_mass_storage_csw_memory;

void usb_mass_storage_initalize(byte_t device_number);
void usb_msd_create_cbw(byte_t transfer_type, byte_t command_length, dword_t transfer_length);
byte_t usb_msd_transfer_command(byte_t device_number, byte_t direction, dword_t memory, dword_t length_of_transfer);
byte_t usb_msd_send_inquiry_command(byte_t device_number);
byte_t usb_msd_send_request_sense_command(byte_t device_number);
byte_t usb_msd_send_capacity_command(byte_t device_number);
byte_t usb_msd_read(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory);
byte_t usb_msd_write(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory);
