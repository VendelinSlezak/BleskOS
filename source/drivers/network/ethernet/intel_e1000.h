//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EC_INTEL_E1000_PORT_DEVICE_CONTROL 0x00
#define EC_INTEL_E1000_PORT_EEPROM_CONTROL 0x10
#define EC_INTEL_E1000_PORT_EEPROM_READ 0x14

#define EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS 256
struct ec_intel_e1000_rx_descriptor_t {
 dword_t lower_packet_address;
 dword_t upper_packet_address;
 word_t packet_size;
 word_t fragment_checksum;

 byte_t descriptor_done: 1;
 byte_t end_of_packet: 1;
 byte_t reserved_bit: 1;
 byte_t vp: 1;
 byte_t udp_checksum_calculated: 1;
 byte_t l4_checksum_calculated: 1;
 byte_t ipv4_checksum_calculated: 1;
 byte_t passed_in_exact_filter: 1;

 byte_t reserved: 5;
 byte_t tcp_udp_checksum_error: 1;
 byte_t ipv4_checksum_error: 1;
 byte_t rx_data_error: 1;
 
 word_t vlan_tag;
}__attribute__((packed));

#define EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS 256
struct ec_intel_e1000_tx_descriptor_t {
 dword_t lower_packet_address;
 dword_t upper_packet_address;
 word_t packet_size;
 byte_t checksum_offset;

 byte_t end_of_packet: 1;
 byte_t insert_fcs: 1;
 byte_t insert_checksum: 1;
 byte_t report_status: 1;
 byte_t reserved_bit_1: 1;
 byte_t descriptor_extension: 1;
 byte_t vlan_packet_enable: 1;
 byte_t reserved_bit_2: 1;

 byte_t status_descriptor_done: 1;
 byte_t reserved: 7;
 
 byte_t checksum_start;
 word_t vlan;
}__attribute__((packed));

dword_t ec_intel_e1000_read(dword_t number_of_card, dword_t port);
void ec_intel_e1000_write(dword_t number_of_card, dword_t port, dword_t value);
byte_t ec_intel_e1000_read_timeout(dword_t number_of_card, dword_t port, dword_t bits, dword_t value, dword_t wait_in_milliseconds);
word_t ec_intel_e1000_read_eeprom(dword_t number_of_card, dword_t address, dword_t shift);

void ec_intel_e1000_initalize(dword_t number_of_card);
byte_t ec_intel_e1000_get_cable_status(dword_t number_of_card);
byte_t ec_intel_e1000_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_intel_e1000_process_irq(dword_t number_of_card);