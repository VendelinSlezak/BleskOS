//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_network_stack_new(void) {
 memory_for_building_network_packet = (byte_t *) malloc(2048);
 create_task(send_packets_in_queue, TASK_TYPE_PERIODIC_INTERRUPT, 1);
}

void start_building_network_packet(void) {
 size_of_network_packet = 0;
 number_of_layers_in_network_packet = 0;
}

void network_packet_save_layer(word_t type_of_layer, dword_t size_of_layer, void (*finalize_layer)(dword_t type_of_next_layer, dword_t size_of_layer)) {
 if(number_of_layers_in_network_packet>=10) {
  return;
 }

 //save layer to array
 layout_of_network_packet[number_of_layers_in_network_packet].type_of_layer = type_of_layer;
 layout_of_network_packet[number_of_layers_in_network_packet].size_of_layer = size_of_layer;
 layout_of_network_packet[number_of_layers_in_network_packet].finalize_layer = finalize_layer;
 number_of_layers_in_network_packet++;
}

void finalize_builded_network_packet(void) {
 //finalize every layer (set type of next layer, size of layer and checksum of layer)
 for(dword_t i=number_of_layers_in_network_packet, type_of_next_layer=0, size_of_layer=0; i>0; i--) {
  //calculate size of layer
  size_of_layer += layout_of_network_packet[(i-1)].size_of_layer;

  //get type of next layer
  if(i<number_of_layers_in_network_packet) {
   type_of_next_layer = layout_of_network_packet[i].type_of_layer;
  }

  //finalize layer
  if(layout_of_network_packet[(i-1)].finalize_layer!=0) {
   layout_of_network_packet[(i-1)].finalize_layer(type_of_next_layer, size_of_layer);
  }
 }
}

void send_network_packet(void) {
 (*network_send_packet)((dword_t)memory_for_building_network_packet, size_of_network_packet);
}

byte_t send_packet_with_response(word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t error_type)) {
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  //we find free entry
  if(packets_with_response_interface[i].milliseconds_count==0) {
   //fill entry
   copy_memory((dword_t)memory_for_building_network_packet, (dword_t)(&packets_with_response_interface[i].packet), size_of_network_packet);
   packets_with_response_interface[i].milliseconds_before_resending = milliseconds_before_resending;
   packets_with_response_interface[i].packet_length = size_of_network_packet;
   packets_with_response_interface[i].max_attempts = max_attempts;
   packets_with_response_interface[i].response_packet_type = response_packet_type;
   packets_with_response_interface[i].response_packet_sender_port = response_packet_sender_port;
   packets_with_response_interface[i].response_packet_reciever_port = response_packet_reciever_port;
   packets_with_response_interface[i].process_response = process_response;
   packets_with_response_interface[i].process_error = process_error;
   packets_with_response_interface[i].milliseconds_count = (time_of_system_running+milliseconds_before_resending);

   //send packet for first time
   send_network_packet();

   return STATUS_GOOD;
  }
 }

 return PWRM_NO_FREE_ENTRY; 
}

void process_packet(byte_t *packet_memory, dword_t packet_size) {
 //check if packet is long enough to contain ethernet layer
 if(packet_size < sizeof(struct network_packet_ethernet_layer_t)) {
  return;
 }

 //define variables of packet type
 dword_t packet_type = 0, packet_sender_port = 0, packet_reciever_port = 0;

 //set variable to examine ethernet layer
 struct network_packet_ethernet_layer_t *packet_ethernet_layer = (struct network_packet_ethernet_layer_t *) packet_memory;

 //examine ethernet layer
 if(packet_ethernet_layer->type_of_next_layer==BIG_ENDIAN_WORD(NETWORK_PACKET_IPV4_LAYER)) {
  //check if packet is long enough to contain IPv4 layer
  if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t))) {
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
    return;
   }

   //set variable to examine TCP layer
   struct network_packet_tcp_layer_t *packet_tcp_layer = (struct network_packet_tcp_layer_t *) ((dword_t)packet_ipv4_layer+sizeof(struct network_packet_ipv4_layer_t));

   //TODO: check TCP layer checksum

   //TODO: check TCP layer reported size

   //set variables of packet type
   packet_type = PWRM_PACKET_TYPE_IPV4_TCP;
   packet_sender_port = BIG_ENDIAN_WORD(packet_tcp_layer->source_port);
   packet_reciever_port = BIG_ENDIAN_WORD(packet_tcp_layer->destination_port);
   packet_memory += (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+(packet_tcp_layer->layer_length*4));
   packet_size -= (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+(packet_tcp_layer->layer_length*4));
  }
  else if(packet_ipv4_layer->protocol==NETWORK_PACKET_UDP_LAYER) {
   //check if packet is long enough to contain UDP layer
   if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+sizeof(struct network_packet_udp_layer_t))) {
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
   packet_size -= (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_ipv4_layer_t)+sizeof(struct network_packet_udp_layer_t));
  }
  else { //unsupported layer
   pstr("not TCP/UDP");
   return;
  }
 }
 else if(packet_ethernet_layer->type_of_next_layer==NETWORK_PACKET_ARP_LAYER) {
  //check if packet is long enough to contain ARP layer
  if(packet_size < (sizeof(struct network_packet_ethernet_layer_t)+sizeof(struct network_packet_arp_layer_t))) {
   return;
  }

  packet_type = PWRM_PACKET_TYPE_ARP;
  packet_sender_port = 0;
  packet_reciever_port = 0;
  packet_memory += sizeof(struct network_packet_ethernet_layer_t);
  packet_size -= sizeof(struct network_packet_ethernet_layer_t);
 }
 else { //unsupported layer
  pstr("not IPV4/ARP");
  return;
 }

 //check if this packet is waited response
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  if(packets_with_response_interface[i].milliseconds_count!=0
     && packets_with_response_interface[i].response_packet_type==packet_type
     && packets_with_response_interface[i].response_packet_sender_port==packet_sender_port
     && packets_with_response_interface[i].response_packet_reciever_port==packet_reciever_port ) {
   packets_with_response_interface[i].process_response(i, packet_memory, packet_size);
  }
 }
}

void remove_packet_entry(byte_t number_of_packet_entry) {
 packets_with_response_interface[number_of_packet_entry].milliseconds_count = 0;
}

void update_packet_entry(byte_t number_of_packet_entry, word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t error_type)) {
 copy_memory((dword_t)memory_for_building_network_packet, (dword_t)(&packets_with_response_interface[number_of_packet_entry].packet), size_of_network_packet);
 packets_with_response_interface[number_of_packet_entry].milliseconds_before_resending = milliseconds_before_resending;
 packets_with_response_interface[number_of_packet_entry].packet_length = size_of_network_packet;
 packets_with_response_interface[number_of_packet_entry].max_attempts = max_attempts;
 packets_with_response_interface[number_of_packet_entry].response_packet_type = response_packet_type;
 packets_with_response_interface[number_of_packet_entry].response_packet_sender_port = response_packet_sender_port;
 packets_with_response_interface[number_of_packet_entry].response_packet_reciever_port = response_packet_reciever_port;
 packets_with_response_interface[number_of_packet_entry].process_response = process_response;
 packets_with_response_interface[number_of_packet_entry].process_error = process_error;
 packets_with_response_interface[number_of_packet_entry].milliseconds_count = time_of_system_running; //send_packets_in_queue() will immediately send packet
}

void send_packets_in_queue(void) {
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  //this packet needs to be sended again
  if(packets_with_response_interface[i].milliseconds_count!=0 && time_of_system_running>=packets_with_response_interface[i].milliseconds_count) {
   //time out - error with sending packet
   if(packets_with_response_interface[i].max_attempts==0) {
    //call error method
    if(packets_with_response_interface[i].process_error!=0) {
     packets_with_response_interface[i].process_error(PWRM_ERROR_TIMEOUT);
    }

    //remove packet
    remove_packet_entry(i);

    continue;
   }

   //send packet
   pstr("packet sended");
   (*network_send_packet)((dword_t)&packets_with_response_interface[i].packet, packets_with_response_interface[i].packet_length);

   //update variables
   packets_with_response_interface[i].max_attempts--;
   packets_with_response_interface[i].milliseconds_count = (time_of_system_running+packets_with_response_interface[i].milliseconds_before_resending);
  }
 }
}