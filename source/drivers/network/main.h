//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_NO_DRIVER 0
#define NETWORK_DRIVER_ETHERNET_INTEL_E1000 1
#define NETWORK_DRIVER_ETHERNET_AMD_PCNET 2
#define NETWORK_DRIVER_ETHERNET_BROADCOM_NETXTREME 3
#define NETWORK_DRIVER_ETHERNET_REALTEK_8139 4
#define NETWORK_DRIVER_ETHERNET_REALTEK_8169 5

#define NETWORK_DEFAULT_CARD 0

#define NETWORK_NOT_CONNECTED 0
#define NETWORK_CONNECTED 1
#define NETWORK_CONNECTION_FALIED 2

struct ethernet_card {
 dword_t present;
 dword_t id;
 byte_t driver;
 dword_t irq;
 dword_t bar_type;
 dword_t base;
 byte_t bus;
 byte_t dev;
 byte_t func;
 dword_t receive_mem;
 dword_t receive_buffer_mem;
 dword_t receive_packet_pointer;
 dword_t transmit_mem;
 dword_t transmit_buffer_mem;
 dword_t transmit_free_packet;
}__attribute__((packed));
struct ethernet_card ethernet_cards[10];
byte_t ethernet_cards_pointer = 0;
byte_t ethernet_selected_card = 0;
byte_t ethernet_card_working_driver = STATUS_FALSE;

byte_t mac_address[6];
byte_t connected_to_network = STATUS_FALSE;
byte_t is_ethernet_cable_connected = STATUS_FALSE;
byte_t ethernet_timer_monitoring_of_line_status = STATUS_FALSE;
byte_t ethernet_link_state_change = 0;

dword_t packet_memory = 0;

void initalize_network_cards(void);
void initalize_network_card(byte_t card);
void initalize_network_stack(void);
void read_ethernet_cable_status(void);
void network_process_packet(dword_t memory, byte_t is_last_packet);
void connect_to_network(void);
void connect_to_network_with_message(void);

byte_t (*ethernet_card_get_cable_status)(void);
void (*network_send_packet)(dword_t memory, dword_t length);