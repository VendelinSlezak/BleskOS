//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_PACKET_IPV4_LAYER 0x0800

struct network_packet_ipv4_layer_t {
 byte_t version: 4;
 byte_t ihl: 4;
 byte_t type_of_service;
 word_t total_length;
 word_t identification;
 word_t flags_fragment;
 byte_t time_to_live;
 byte_t protocol;
 word_t checksum;
 byte_t source_ip_address[4];
 byte_t destination_ip_address[4];
}__attribute__((packed));

struct network_packet_ipv4_layer_t *network_packet_ipv4_layer;

void network_packet_add_ipv4_layer(byte_t source_ip_address[4], byte_t destination_ip_address[4]);
void network_packet_finalize_ipv4_layer(dword_t type_of_next_layer, dword_t size_of_layer);