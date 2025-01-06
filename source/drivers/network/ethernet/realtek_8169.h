//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS 256
struct ec_realtek_8169_rx_descriptor_t {
  word_t packet_size;
  word_t reserved: 14;
  word_t end_of_ring: 1;
  word_t owned_by_card: 1;
  dword_t vlan;
  dword_t lower_memory;
  dword_t upper_memory;
 }__attribute__((packed));

#define EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS 256
struct ec_realtek_8169_tx_descriptor_t {
 word_t packet_size;
 word_t reserved: 12;
 word_t last_segment_descriptor: 1;
 word_t first_segment_descriptor: 1;
 word_t end_of_ring: 1;
 word_t owned_by_card: 1;
 dword_t vlan;
 dword_t lower_memory;
 dword_t upper_memory;
}__attribute__((packed));

void ec_realtek_8169_outb(dword_t number_of_card, dword_t port, byte_t value);
void ec_realtek_8169_outw(dword_t number_of_card, dword_t port, word_t value);
void ec_realtek_8169_outd(dword_t number_of_card, dword_t port, dword_t value);
byte_t ec_realtek_8169_inb(dword_t number_of_card, dword_t port);
word_t ec_realtek_8169_inw(dword_t number_of_card, dword_t port);
dword_t ec_realtek_8169_ind(dword_t number_of_card, dword_t port);

void ec_realtek_8169_initalize(dword_t number_of_card);
byte_t ec_realtek_8169_get_cable_status(dword_t number_of_card);
byte_t ec_realtek_8169_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_realtek_8169_process_irq(dword_t number_of_card);