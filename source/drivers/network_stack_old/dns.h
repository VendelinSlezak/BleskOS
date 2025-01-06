//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_PACKET_DNS_LAYER 0x0053

#define NETWORK_PACKET_DNS_QUESTION_TYPE_IPV4 BIG_ENDIAN_WORD(0x0001)

#define NETWORK_PACKET_DNS_QUESTION_CLASS_INTERNET BIG_ENDIAN_WORD(0x0001)

struct network_packet_dns_layer_t {
 word_t id;
 word_t flags;
 word_t qd_count;
 word_t an_count;
 word_t ns_count;
 word_t ar_count;
}__attribute__((packed));

struct network_packet_dns_layer_answer_t {
 word_t type;
 word_t class;
 dword_t time_to_live;
 word_t data_length;
 byte_t address[];
}__attribute__((packed));

struct dns_info_t {
 word_t packet_id;
 byte_t ip[4];
}__attribute__((packed)) dns_info;

struct network_packet_dns_layer_t *network_packet_dns_layer;

void network_packet_add_dns_layer(word_t id, word_t qtype, byte_t *domain);
byte_t parse_dns_reply(byte_t *dns_layer_pointer, dword_t dns_layer_size);