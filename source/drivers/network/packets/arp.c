//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void network_packet_add_arp_layer(byte_t type, byte_t sender_mac[6], byte_t sender_ip[4], byte_t target_mac[6], byte_t target_ip[4]) {
 network_packet_arp_layer = (struct network_packet_arp_layer_t *) ((dword_t)memory_for_building_network_packet + size_of_network_packet);

 //set ARP layer
 network_packet_arp_layer->hardware_type = BIG_ENDIAN_WORD(1); //ethernet
 network_packet_arp_layer->protocol_type = BIG_ENDIAN_WORD(0x0800); // IPv4
 network_packet_arp_layer->hardware_size = 6; //length of MAC address
 network_packet_arp_layer->protocol_size = 4; //length of IP address
 network_packet_arp_layer->opcode = BIG_ENDIAN_WORD(type); //ARP type
 for(dword_t i = 0; i < 6; i++) {
  network_packet_arp_layer->sender_mac[i] = sender_mac[i];
  network_packet_arp_layer->target_mac[i] = target_mac[i];
 }
 for(dword_t i = 0; i < 4; i++) {
  network_packet_arp_layer->sender_ip[i] = sender_ip[i];
  network_packet_arp_layer->target_ip[i] = target_ip[i];
 }

 //set variables
 size_of_network_packet += sizeof(struct network_packet_arp_layer_t);
 network_packet_save_layer(NETWORK_PACKET_ARP_LAYER, sizeof(struct network_packet_arp_layer_t), 0);
}