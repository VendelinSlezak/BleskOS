//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void network_packet_add_ipv4_layer(byte_t source_ip_address[4], byte_t destination_ip_address[4]) {
 network_packet_ipv4_layer = (struct network_packet_ipv4_layer_t *) ((dword_t)memory_for_building_network_packet+size_of_network_packet);
 
 //set IPv4 layer
 network_packet_ipv4_layer->version = 5; //size in dwords
 network_packet_ipv4_layer->ihl = 4; //version IPv4
 network_packet_ipv4_layer->type_of_service = 0;
 network_packet_ipv4_layer->total_length = 0;
 network_packet_ipv4_layer->identification = 0;
 network_packet_ipv4_layer->flags_fragment = 0;
 network_packet_ipv4_layer->time_to_live = 0x80;
 network_packet_ipv4_layer->protocol = 0;
 network_packet_ipv4_layer->checksum = 0;
 for(dword_t i=0; i<4; i++) {
  network_packet_ipv4_layer->source_ip_address[i] = source_ip_address[i];
 }
 for(dword_t i=0; i<4; i++) {
  network_packet_ipv4_layer->destination_ip_address[i] = destination_ip_address[i];
 }

 //set variables
 size_of_network_packet += sizeof(struct network_packet_ipv4_layer_t);
 network_packet_save_layer(NETWORK_PACKET_IPV4_LAYER, sizeof(struct network_packet_ipv4_layer_t), network_packet_finalize_ipv4_layer);
}

void network_packet_finalize_ipv4_layer(dword_t type_of_next_layer, dword_t size_of_layer) {
 //set type of next layer
 network_packet_ipv4_layer->protocol = type_of_next_layer;

 //set size of layer
 network_packet_ipv4_layer->total_length = BIG_ENDIAN_WORD(size_of_layer);

 //calculate checksum
 word_t *packet_layer_pointer = (word_t *) ((dword_t)network_packet_ipv4_layer);
 dword_t checksum = 0;
 for(int i=0; i<10; i++) {
  checksum += *packet_layer_pointer;
  packet_layer_pointer++;
 }
 while((checksum>>16)>0) {
  checksum = ((checksum & 0xFFFF) + (checksum>>16));
 }
 network_packet_ipv4_layer->checksum = (~checksum);
}