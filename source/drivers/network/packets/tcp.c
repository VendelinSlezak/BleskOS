//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Function to add TCP layer
void network_packet_add_tcp_layer(word_t source_port, word_t destination_port, dword_t sequence_number, dword_t acknowledgment_number, byte_t control) {
 network_packet_tcp_layer = (struct network_packet_tcp_layer_t *)((dword_t)memory_for_building_network_packet + size_of_network_packet);

 //set TCP layer
 network_packet_tcp_layer->source_port = BIG_ENDIAN_WORD(source_port);
 network_packet_tcp_layer->destination_port = BIG_ENDIAN_WORD(destination_port);
 network_packet_tcp_layer->sequence_number = BIG_ENDIAN_DWORD(sequence_number);
 network_packet_tcp_layer->acknowledgment_number = BIG_ENDIAN_DWORD(acknowledgment_number);
 network_packet_tcp_layer->layer_length = ((sizeof(struct network_packet_tcp_layer_t)/4)<<4);
 network_packet_tcp_layer->control = control;
 network_packet_tcp_layer->window_size = BIG_ENDIAN_WORD(16684);
 network_packet_tcp_layer->checksum = 0;
 network_packet_tcp_layer->urgent_pointer = 0;

 //set variables
 size_of_network_packet += sizeof(struct network_packet_tcp_layer_t);
 network_packet_save_layer(NETWORK_PACKET_TCP_LAYER, sizeof(struct network_packet_tcp_layer_t), network_packet_finalize_tcp_layer);
}

void network_packet_finalize_tcp_layer(dword_t type_of_next_layer, dword_t size_of_layer) {
 //calculate checksum
 dword_t memory = (dword_t)network_packet_tcp_layer;
 word_t *packet_layer_pointer = (word_t *) memory;
 dword_t checksum = 0;
 for (int i = 0; i < (size_of_layer / 2); i++) {
  checksum += *packet_layer_pointer;
  packet_layer_pointer++;
 }
 if ((size_of_layer & 0x1) == 0x1) {
  checksum += (*packet_layer_pointer & 0xFF);
 }
 checksum += (BIG_ENDIAN_WORD(size_of_layer)
              + (NETWORK_PACKET_TCP_LAYER<<8)
              + (network_packet_ipv4_layer->source_ip_address[0] | network_packet_ipv4_layer->source_ip_address[1]<<8)
              + (network_packet_ipv4_layer->source_ip_address[2] | network_packet_ipv4_layer->source_ip_address[3]<<8)
              + (network_packet_ipv4_layer->destination_ip_address[0] | network_packet_ipv4_layer->destination_ip_address[1]<<8)
              + (network_packet_ipv4_layer->destination_ip_address[2] | network_packet_ipv4_layer->destination_ip_address[3]<<8));
 while ((checksum >> 16) > 0) {
  checksum = (checksum & 0xFFFF) + (checksum >> 16);
 }
 network_packet_tcp_layer->checksum = (~checksum);
}