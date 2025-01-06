//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

 struct ec_broadcom_netxtreme_status_block_t {
  dword_t status_word;
  byte_t status_tag;
  byte_t reserved[3];
  word_t receive_return_ring_1_index;
  word_t receive_standard_producer_ring_consumer_index;
  word_t receive_return_ring_3_index;
  word_t receive_return_ring_2_index;
  word_t receive_return_ring_0_index;
  word_t send_bd_ring_1_consumer_index;
  word_t receive_jumbo_producer_ring_consumer_index;
  word_t send_bd_ring_2_consumer_index;
 }__attribute__((packed));

struct ec_broadcom_netxtreme_ring_control_block_t {
 dword_t lower_memory;
 dword_t upper_memory;
 word_t reserved: 1;
 word_t disabled: 1;
 word_t max_receive_frame_size: 14;
 word_t length;
 dword_t nic_ring_address;
}__attribute__((packed));

struct ec_broadcom_netxtreme_rx_descriptor_t {
 dword_t lower_memory;
 dword_t upper_memory;
 word_t length_of_packet;
 word_t index;
 struct {
  word_t reserved1: 2;
  word_t packet_end: 1;
  word_t rss_hash_valid: 1;
  word_t reserved2: 2;
  word_t vlan_tag: 1;
  word_t rss_hash_type: 3;
  word_t frame_has_error: 1;
  word_t reserved3: 1;
  word_t ip_checksum: 1;
  word_t tcp_udp_checksum: 1;
  word_t tcp_udp_is_tcp: 1;
  word_t ip_version: 1;
 } flags;
 word_t type;
 word_t tcp_udp_checksum;
 word_t ip_checksum;
 word_t vlan_tag;
 struct {
  word_t bad_ethernet_crc: 1;
  word_t collision_detected: 1;
  word_t link_lost: 1;
  word_t physical_layer_error: 1;
  word_t odd_number_of_nibbles: 1;
  word_t mac_aborted: 1;
  word_t small_packet: 1;
  word_t packet_truncated: 1;
  word_t giant_packet: 1;
  word_t reserved: 7;
 } error_flags;
 dword_t rss_hash;
 dword_t opaque;
}__attribute__((packed));

struct ec_broadcom_netxtreme_tx_descriptor_t {
 dword_t lower_memory;
 dword_t upper_memory;
 struct {
  word_t tcp_udp_checksum_insert: 1;
  word_t ipv4_checksum_insert: 1;
  word_t packet_end: 1;
  word_t jumbo_packet: 1;
  word_t hdr_length: 1;
  word_t snap: 1;
  word_t vlan_tag_insert: 1;
  word_t immediate_consumer_index_update: 1;
  word_t cpu_pre_dma: 1;
  word_t cpu_post_dma: 1;
  word_t reserved2: 5;
  word_t dont_generate_crc: 1;
 } flags;
 word_t length_of_packet;
 word_t vlan_tag;
 word_t reserved;
}__attribute__((packed));

dword_t ec_broadcom_netxtreme_read(dword_t number_of_card, dword_t port);
void ec_broadcom_netxtreme_write(dword_t number_of_card, dword_t port, dword_t value);
void ec_broadcom_netxtreme_set_bits(dword_t number_of_card, dword_t port, dword_t bits);
void ec_broadcom_netxtreme_clear_bits(dword_t number_of_card, dword_t port, dword_t bits);
word_t ec_broadcom_netxtreme_read_phy(dword_t number_of_card, byte_t phy_register);
void ec_broadcom_netxtreme_write_phy(dword_t number_of_card, byte_t phy_register, word_t value);

void ec_broadcom_netxtreme_initalize(dword_t number_of_card);
byte_t ec_broadcom_netxtreme_get_cable_status(dword_t number_of_card);
byte_t ec_broadcom_netxtreme_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_broadcom_netxtreme_process_irq(dword_t number_of_card);