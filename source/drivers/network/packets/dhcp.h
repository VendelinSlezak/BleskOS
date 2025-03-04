//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_PACKET_DHCP_LAYER 0x0801

#define NETWORK_PACKET_DHCP_REQUEST_OP 1
#define NETWORK_PACKET_DHCP_REPLY_OP 1

struct network_packet_dhcp_layer_t {
 byte_t op;
 byte_t htype;
 byte_t hlen;
 byte_t hops;
 dword_t xid; 
 word_t secs;
 word_t flags; 
 byte_t ciaddr[4];
 byte_t yiaddr[4];
 byte_t siaddr[4];
 byte_t giaddr[4];
 byte_t chaddr[16];
 byte_t sname[64];
 byte_t file[128]; 
 byte_t options[312];
}__attribute__((packed));

struct dhcp_info_t {
 byte_t our_ip_address[4];
 byte_t server_ip_address[4]; 
 byte_t dns_server_ip_address[4];
}__attribute__((packed)) dhcp_info;

struct network_packet_dhcp_layer_t *network_packet_dhcp_layer;

void network_packet_add_dhcp_layer(byte_t *options);
byte_t parse_dhcp_layer(byte_t *dhcp_layer_pointer, dword_t dhcp_layer_size);