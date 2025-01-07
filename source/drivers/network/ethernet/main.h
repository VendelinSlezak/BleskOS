//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define ETHERNET_CARD_CABLE_DISCONNECTED 0
#define ETHERNET_CARD_CABLE_CONNECTED 1

#define MAX_NUMBER_OF_ETHERNET_CARDS 10
struct ethernet_card_info_t {
 dword_t id;
 byte_t bus;
 byte_t dev;
 byte_t func;
 dword_t irq;

 dword_t bar_type;
 dword_t base;

 void (*initalize)(dword_t number_of_card);
 byte_t (*get_cable_status)(dword_t number_of_card);
 byte_t (*send_packet)(dword_t number_of_card, byte_t *memory_of_packet, dword_t length_of_packet);
 void (*process_irq)(dword_t number_of_card);

 byte_t cable_status;

 byte_t mac_address[6];

 void *rx_descriptors_memory;
 dword_t rx_descriptor_pointer;
 byte_t *rx_packets_memory;

 void *tx_descriptors_memory;
 dword_t tx_descriptor_pointer;
 byte_t *tx_packets_memory;
}__attribute__((packed));
struct ethernet_card_info_t ethernet_cards[MAX_NUMBER_OF_ETHERNET_CARDS];
dword_t number_of_ethernet_cards;

void initalize_ethernet_cards(void);
byte_t ethernet_void_get_cable_status(dword_t number_of_card);
byte_t ethernet_void_send_packet(dword_t number_of_card, byte_t *memory_of_packet, dword_t length_of_packet);
void ethernet_void_process_irq(dword_t number_of_card);
void ethernet_cards_update_cable_status(void);
void select_ethernet_card_as_internet_connection_device(dword_t number_of_card);
byte_t send_packet_to_internet_through_ethernet(byte_t *packet_memory, dword_t packet_size);
void ethernet_cards_process_irq(void);