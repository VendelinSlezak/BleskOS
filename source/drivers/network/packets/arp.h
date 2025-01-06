//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_PACKET_ARP_LAYER 0x0806

#define NETWORK_PACKET_ARP_TYPE_REQUEST 1
#define NETWORK_PACKET_ARP_TYPE_RESPONSE 2

struct network_packet_arp_layer_t {
 word_t hardware_type;
 word_t protocol_type;
 byte_t hardware_size;
 byte_t protocol_size;
 word_t opcode;
 byte_t sender_mac[6];
 byte_t sender_ip[4];
 byte_t target_mac[6];
 byte_t target_ip[4];
 byte_t padding[];
}__attribute__((packed));

struct network_packet_arp_layer_t *network_packet_arp_layer;

void network_packet_add_arp_layer(byte_t type, byte_t sender_mac[6], byte_t sender_ip[4], byte_t target_mac[6], byte_t target_ip[4]);