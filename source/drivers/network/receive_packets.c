//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void process_packet(byte_t *packet_memory, dword_t packet_size) {
 //check if packet is long enough to contain ethernet layer
 if(packet_size < sizeof(struct network_packet_ethernet_layer_t)) {
  log("\nsmall packet: ethernet layer");
  return;
 }

 //save packet info to global variables
 memory_of_full_processed_packet = packet_memory;
 length_of_full_processed_packet = packet_size;

 //define variables of packet type
 dword_t packet_type = 0, packet_sender_port = 0, packet_reciever_port = 0;

 //set variable to examine ethernet layer
 struct network_packet_ethernet_layer_t *packet_ethernet_layer = (struct network_packet_ethernet_layer_t *) packet_memory;

 //examine ethernet layer
 if(packet_ethernet_layer->type_of_next_layer==BIG_ENDIAN_WORD(NETWORK_PACKET_IPV4_LAYER)) {
  //check if packet is long enough to contain IPv4 layer
  if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t))) {
   log("\nsmall packet: IPv4 layer");
   return;
  }

  //set variable to examine IPv4 layer
  struct network_packet_ipv4_layer_t *packet_ipv4_layer = (struct network_packet_ipv4_layer_t *) ((dword_t)packet_ethernet_layer+sizeof(struct network_packet_ethernet_layer_t));

  //TODO: check IPv4 layer checksum

  //TODO: check IPv4 layer reported size

  //examine IPv4 layer
  if(packet_ipv4_layer->protocol==NETWORK_PACKET_TCP_LAYER) {
   //check if packet is long enough to contain TCP layer
   if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+sizeof(struct network_packet_tcp_layer_t))) {
    log("\nsmall packet: TCP layer");
    return;
   }

   //set variable to examine TCP layer
   struct network_packet_tcp_layer_t *packet_tcp_layer = (struct network_packet_tcp_layer_t *) ((dword_t)packet_ipv4_layer+sizeof(struct network_packet_ipv4_layer_t));
   full_packet_tcp_layer = packet_tcp_layer;

   //TODO: check TCP layer checksum

   //TODO: check TCP layer reported size

   //set variables of packet type
   packet_type = PWRM_PACKET_TYPE_IPV4_TCP;
   packet_sender_port = BIG_ENDIAN_WORD(packet_tcp_layer->source_port);
   packet_reciever_port = BIG_ENDIAN_WORD(packet_tcp_layer->destination_port);
   packet_memory += (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+((packet_tcp_layer->layer_length >> 4)*4));
   packet_size = (BIG_ENDIAN_WORD(packet_ipv4_layer->total_length) - (sizeof(struct network_packet_ipv4_layer_t)+((packet_tcp_layer->layer_length >> 4)*4)));
  }
  else if(packet_ipv4_layer->protocol==NETWORK_PACKET_UDP_LAYER) {
   //check if packet is long enough to contain UDP layer
   if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+sizeof(struct network_packet_udp_layer_t))) {
    log("\nsmall packet: UDP layer");
    return;
   }

   //set variable to examine UDP layer
   struct network_packet_udp_layer_t *packet_udp_layer = (struct network_packet_udp_layer_t *) ((dword_t)packet_ipv4_layer+sizeof(struct network_packet_ipv4_layer_t));

   //TODO: check UDP layer checksum

   //TODO: check UDP layer reported size

   //set variables of packet type
   packet_type = PWRM_PACKET_TYPE_IPV4_UDP;
   packet_sender_port = BIG_ENDIAN_WORD(packet_udp_layer->source_port);
   packet_reciever_port = BIG_ENDIAN_WORD(packet_udp_layer->destination_port);
   packet_memory += (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+sizeof(struct network_packet_udp_layer_t));
   packet_size = (BIG_ENDIAN_WORD(packet_ipv4_layer->total_length) - (sizeof(struct network_packet_ipv4_layer_t)+sizeof(struct network_packet_udp_layer_t)));
  }
  else { //unsupported layer
   log("\nunknown layer after IPv4");
   return;
  }
 }
 else if(packet_ethernet_layer->type_of_next_layer==BIG_ENDIAN_WORD(NETWORK_PACKET_ARP_LAYER)) {
  //check if packet is long enough to contain ARP layer
  if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_arp_layer_t))) {
   log("\nsmall packet: ARP layer");
   return;
  }

  //set variable to examine ARP layer
  struct network_packet_arp_layer_t *packet_arp_layer = (struct network_packet_arp_layer_t *) ((dword_t)packet_ethernet_layer+sizeof(struct network_packet_ethernet_layer_t));

  //if this is ARP request from internet, respond immediately
  if(packet_arp_layer->opcode == BIG_ENDIAN_WORD(NETWORK_PACKET_ARP_TYPE_REQUEST) && internet.status == INTERNET_STATUS_CONNECTED) {
   //create ARP reply packet
   start_building_network_packet();
   network_packet_add_ethernet_layer(internet.router_mac);
   network_packet_add_arp_layer(NETWORK_PACKET_ARP_TYPE_RESPONSE, internet.our_mac, internet.our_ip, packet_arp_layer->sender_mac, packet_arp_layer->sender_ip);
   finalize_builded_network_packet();

   //send packet to internet
   send_builded_packet_to_internet();

   return;
  }

  //otherwise pass packet
  packet_type = PWRM_PACKET_TYPE_ARP;
  packet_sender_port = 0;
  packet_reciever_port = 0;
  packet_memory += sizeof(struct network_packet_ethernet_layer_t);
  packet_size -= sizeof(struct network_packet_ethernet_layer_t);
 }
 else { //unsupported layer
  // log("\nunknown layer after ethernet "); log_hex_specific_size(BIG_ENDIAN_WORD(packet_ethernet_layer->type_of_next_layer), 4);
  return;
 }

 //check if this packet is waited response
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  if(packets_with_response_interface[i].milliseconds_count!=0
     && packets_with_response_interface[i].response_packet_type==packet_type
     && packets_with_response_interface[i].response_packet_sender_port==packet_sender_port
     && packets_with_response_interface[i].response_packet_reciever_port==packet_reciever_port ) {
   if(packets_with_response_interface[i].process_response(i, packet_memory, packet_size)==PWRM_END_TRANSFER) {
    remove_packet_entry(i);
   }
  }
 }
}