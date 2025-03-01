//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

struct usb_configuration_descriptor_header_t {
 byte_t bLength;
 byte_t bDescriptorType;
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
 byte_t endpoint_type: 2;
 byte_t bmAttributes: 6;
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

struct usb_full_interface_info_t {
 byte_t is_parsed;
 dword_t length_to_skip;

 struct usb_interface_descriptor_t *interface_descriptor;
 struct usb_endpoint_descriptor_t *bulk_in_endpoint;
 struct usb_endpoint_descriptor_t *bulk_out_endpoint;
 struct usb_endpoint_descriptor_t *isochronous_in_endpoint;
 struct usb_endpoint_descriptor_t *isochronous_out_endpoint;
 struct usb_endpoint_descriptor_t *interrupt_in_endpoint;
 struct usb_endpoint_descriptor_t *interrupt_out_endpoint;

 struct usb_hid_descriptor_t *hid_descriptor;
}__attribute__((packed));

byte_t is_configuration_descriptor_valid(struct usb_configuration_descriptor_t *configuration_descriptor);
struct usb_full_interface_info_t configuration_descriptor_parse_interface(byte_t *configuration_descriptor_pointer, dword_t length_of_rest_of_descriptor);