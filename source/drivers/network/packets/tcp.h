//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_PACKET_TCP_LAYER 0x0006

#define TCP_LAYER_FIN_FLAG (1 << 0)
#define TCP_LAYER_SYN_FLAG (1 << 1)
#define TCP_LAYER_RST_FLAG (1 << 2)
#define TCP_LAYER_PUSH_FLAG (1 << 3)
#define TCP_LAYER_ACK_FLAG (1 << 4)

struct network_packet_tcp_layer_t {
 word_t source_port;
 word_t destination_port;
 dword_t sequence_number;
 dword_t acknowledgment_number;
 byte_t layer_length;
 byte_t control;
 word_t window_size;
 word_t checksum;
 word_t urgent_pointer;
} __attribute__((packed));

struct network_packet_tcp_layer_t *network_packet_tcp_layer;

void network_packet_add_tcp_layer(word_t source_port, word_t destination_port, dword_t sequence_number, dword_t acknowledgment_number, byte_t control);
void network_packet_finalize_tcp_layer(dword_t type_of_next_layer, dword_t size_of_layer);