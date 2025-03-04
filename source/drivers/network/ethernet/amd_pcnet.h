//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct ec_amd_pcnet_initalization_block_t {
 word_t mode;
 byte_t reserved1: 4;
 byte_t number_of_rx_descriptors: 4;
 byte_t reserved2: 4;
 byte_t number_of_tx_descriptors: 4;
 byte_t mac_address[6];
 word_t reserved3;
 byte_t logical_address[8];
 dword_t memory_of_rx_descriptors;
 dword_t memory_of_tx_descriptors;
}__attribute__((packed));

#define EC_AMD_PCNET_NUMBER_OF_RX_DESCRIPTORS 256
struct ec_amd_pcnet_rx_descriptor_t {
 dword_t memory;
 word_t buffer_byte_count: 12;
 word_t signature1: 4;
 byte_t reserved1;
 byte_t end_of_packet: 1;
 byte_t start_of_packet: 1;
 byte_t buffer_error: 1;
 byte_t crc_error: 1;
 byte_t owerflow_error: 1;
 byte_t framing_error: 1;
 byte_t error: 1;
 byte_t own_by_card: 1;
 word_t message_byte_count: 12;
 word_t signature2: 4;
 byte_t runt_packet_count;
 byte_t receive_collision_count;
 dword_t reserved2;
}__attribute__((packed));

#define EC_AMD_PCNET_NUMBER_OF_TX_DESCRIPTORS 256
struct ec_amd_pcnet_tx_descriptor_t {
 dword_t memory;
 word_t buffer_byte_count: 12;
 word_t signature: 4;
 byte_t reserved1;
 byte_t end_of_packet: 1;
 byte_t start_of_packet: 1;
 byte_t deffered: 1;
 byte_t one_retry: 1;
 byte_t more_retries: 1;
 byte_t add_fcs: 1;
 byte_t error: 1;
 byte_t own_by_card: 1;
 word_t transmit_retry_count: 4;
 word_t reserved2: 12;
 word_t time_domain_reflectometry: 10;
 word_t retry_error: 1;
 word_t loss_of_carrier: 1;
 word_t late_collision: 1;
 word_t excessive_defferal: 1;
 word_t underflow_error: 1;
 word_t buffer_error: 1;
 dword_t reserved3;
}__attribute__((packed));

dword_t ec_amd_pcnet_read_csr(dword_t number_of_card, dword_t port);
void ec_amd_pcnet_write_csr(dword_t number_of_card, dword_t port, dword_t value);
dword_t ec_amd_pcnet_read_bcr(dword_t number_of_card, dword_t port);
void ec_amd_pcnet_write_bcr(dword_t number_of_card, dword_t port, dword_t value);

void ec_amd_pcnet_initalize(dword_t number_of_card);
byte_t ec_amd_pcnet_get_cable_status(dword_t number_of_card);
byte_t ec_amd_pcnet_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_amd_pcnet_process_irq(dword_t number_of_card);