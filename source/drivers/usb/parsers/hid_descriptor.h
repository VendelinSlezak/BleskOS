//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

struct hid_entry_data_t {
 byte_t byte_offset;
 byte_t shift;
 dword_t mask;
}__attribute__((packed));

#define MAX_NUMBER_OF_PARSED_HID_ENTRIES 64
struct parsed_hid_entry_t {
 byte_t offset_in_bits;
 byte_t size_in_bits;
 byte_t usage_page;
 byte_t usage;
 byte_t usage_value;

 struct hid_entry_data_t data;
}__attribute__((packed));

#define HID_USAGE_PAGE_GENERIC_DESKTOP 0x01
#define HID_USAGE_PAGE_BUTTON 0x09

#define HID_GENERIC_DESKTOP_USAGE_X 0x30
#define HID_GENERIC_DESKTOP_USAGE_Y 0x31
#define HID_GENERIC_DESKTOP_USAGE_WHEEL 0x38

#define HID_NO_ENTRY_FOUNDED MAX_NUMBER_OF_PARSED_HID_ENTRIES

struct parsed_hid_entry_t parsed_hid[MAX_NUMBER_OF_PARSED_HID_ENTRIES];
dword_t number_of_hid_parsed_entries = 0;

void parse_hid_descriptor(struct hid_entry_t *hid_descriptor, dword_t hid_descriptor_length);
byte_t get_hid_descriptor_entry(byte_t usage_page, byte_t usage, byte_t usage_value);
dword_t parse_value_from_hid_packet(dword_t *packet, struct hid_entry_data_t hid_entry);