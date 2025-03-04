//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void network_packet_add_ethernet_layer(byte_t receiver_mac_address[6]) {
 network_packet_ethernet_layer = (struct network_packet_ethernet_layer_t *) ((dword_t)memory_for_building_network_packet+size_of_network_packet);
 
 //add receiver MAC address
 for(dword_t i=0; i<6; i++) {
  network_packet_ethernet_layer->receiver_mac_address[i] = receiver_mac_address[i];
 }

 //add our MAC address
 for(dword_t i=0; i<6; i++) {
  network_packet_ethernet_layer->sender_mac_address[i] = internet.our_mac[i];
 }

 //type of next layer will be set later
 network_packet_ethernet_layer->type_of_next_layer = 0;

 //set variables
 size_of_network_packet += sizeof(struct network_packet_ethernet_layer_t);
 network_packet_save_layer(NETWORK_PACKET_ETHERNET_LAYER, sizeof(struct network_packet_ethernet_layer_t), network_packet_finalize_ethernet_layer);
}

void network_packet_finalize_ethernet_layer(dword_t type_of_next_layer, dword_t size_of_layer) {
 network_packet_ethernet_layer->type_of_next_layer = BIG_ENDIAN_WORD(type_of_next_layer);
}