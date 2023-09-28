//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_network_cards(void) {
 connected_to_network = STATUS_FALSE;
 is_ethernet_cable_connected = STATUS_FALSE;
 ethernet_timer_monitoring_of_line_status = STATUS_FALSE;
 ethernet_selected_card = 0xFF;

 if(ethernet_cards_pointer==0) {
  log("\nno ethernet card founded\n");
  return;
 }
 
 log("\n");
 log_var(ethernet_cards_pointer);
 log("ethernet cards founded\n");
 
 //initalize first card we have driver for
 for(int card=0; card<ethernet_cards_pointer; card++) {
  if(ethernet_cards[card].driver!=NETWORK_NO_DRIVER) {
   initalize_network_card(card);

   log("MAC address: ");
   for(int i=0; i<6; i++) {
    log_hex_specific_size(mac_address[i], 2);
    log(" ");
   }
   log("\n");
   return;
  }
 }
}

void initalize_network_card(byte_t card) {
 if(ethernet_cards_pointer==0) {
  return;
 }
 
 ethernet_selected_card = card;

 if(ethernet_cards[card].driver==NETWORK_DRIVER_ETHERNET_INTEL_E1000) {
  log("intel e1000 driver\n");
  initalize_ethernet_card_intel_e1000();
 }
 else if(ethernet_cards[card].driver==NETWORK_DRIVER_ETHERNET_AMD_PCNET) {
  log("AMD PC-net driver\n");
  initalize_ethernet_card_amd_pcnet();
 }
 else if(ethernet_cards[card].driver==NETWORK_DRIVER_ETHERNET_REALTEK_8139) {
  log("realtek 8139 driver\n");
  initalize_ethernet_card_realtek_8139();
 }
 else if(ethernet_cards[card].driver==NETWORK_DRIVER_ETHERNET_REALTEK_8169) {
  log("realtek 8169 driver\n");
  initalize_ethernet_card_realtek_8169();
 }
}

void initalize_network_stack(void) {
 packet_memory = calloc(1600);
}

void read_ethernet_cable_status(void) {
byte_t cable_previous_state = is_ethernet_cable_connected;

 if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_INTEL_E1000) {
  is_ethernet_cable_connected = ethernet_card_intel_e1000_get_cable_status();
 }
 else if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_AMD_PCNET) {
  is_ethernet_cable_connected = ethernet_card_amd_pcnet_get_cable_status();
 }
 else if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_REALTEK_8139) {
  is_ethernet_cable_connected = ethernet_card_realtek_8139_get_cable_status();
 }
 else if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_REALTEK_8169) {
  is_ethernet_cable_connected = ethernet_card_realtek_8169_get_cable_status();
 }

 if(is_ethernet_cable_connected==STATUS_FALSE) {
  connected_to_network = STATUS_FALSE;
 }

 if(cable_previous_state!=is_ethernet_cable_connected) {
  ethernet_link_state_change = 1;
 }
}

void network_send_packet(dword_t memory, dword_t length) {
 if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_INTEL_E1000) {
   ethernet_card_intel_e1000_send_packet(memory, length);
 }
 else if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_AMD_PCNET) {
   ethernet_card_amd_pcnet_send_packet(memory, length);
 }
 else if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_REALTEK_8139) {
   ethernet_card_realtek_8139_send_packet(memory, length);
 }
 else if(ethernet_cards[ethernet_selected_card].driver==NETWORK_DRIVER_ETHERNET_REALTEK_8169) {
   ethernet_card_realtek_8169_send_packet(memory, length);
 }
}

void network_process_packet(dword_t memory, byte_t is_last_packet) {
 byte_t *packet8 = (byte_t *) memory;
 word_t *packet16 = (word_t *) memory;
 dword_t *packet32 = (dword_t *) memory;
 word_t packet_data_length, tcp_header_length;
 dword_t packet_ip;
 
 //receive only packets with our MAC or broadcast packets
 if(!(packet8[0]==mac_address[0] && packet8[1]==mac_address[1] && packet8[2]==mac_address[2] && packet8[3]==mac_address[3] && packet8[4]==mac_address[4] && packet8[5]==mac_address[5])) {
  if(!(packet32[0]==0xFFFFFFFF && packet16[2]==0xFFFF)) {
   return;
  }
 }
 
 //IP
 if(packet16[6]==IP_LAYER_TYPE) {
  //UDP
  if(packet8[ETHERNET_LAYER_LENGTH+9]==UDP_PROTOCOL_TYPE) {
   if(packet16[17]==0x4300 && packet16[18]==0x4400) {
    process_dhcp_packet(memory);
   }
   if(packet16[17]==0x3500) {
    process_dns_reply(memory);
   }
  }
  //TCP
  else if(packet8[ETHERNET_LAYER_LENGTH+9]==TCP_PROTOCOL_TYPE) {
   tcp_header_length = ((packet8[46]>>4)*4);
   packet_data_length = (((packet8[16]<<8) | packet8[17]) - IP_LAYER_LENGTH - tcp_header_length);
   packet32 = (dword_t *) (memory+26); //IP address
   packet_ip = *packet32;
   tcp_packet_port = ((packet16[18]<<8) | (packet16[18]>>8));
   
   if(packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x12) { //SYN and ACK flags
    read_tcp_seq_and_ack_numbers(memory);
    seq_number++;
    send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, seq_number); //acknowledge TCP handshake
    tcp_connection_status = TCP_HANDSHAKE_COMPLETE;
    return;
   }
   if(tcp_connection_status==TCP_HTTP_REQUEST) {
    if(packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x10 || packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x18) { //ACK or PSH and ACK
     if(packet_data_length==0) {
      return; //no data in this transaction
     }
     
     packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+tcp_header_length);
     if(packet32[0]!=0x50545448) { //HTTP
      pstr("not http response");
      return; //not HTTP response
     }
     
     //process http reply
     process_http_reply((memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+tcp_header_length), (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+tcp_header_length+packet_data_length));
     
     //set variabiles for transferred file
     file_memory = calloc(1024*1024); //one MB TODO: more
     file_memory_pointer = file_memory;
     file_length = http_content_length;
     file_transferred_length = 0;

     //save data of file in this packet TODO: http chunks
     copy_memory(http_data_start_mem, file_memory_pointer, http_data_in_packet_length);
     file_memory_pointer += http_data_in_packet_length;
     file_transferred_length += http_data_in_packet_length;
     
     //acknowledge
     read_tcp_seq_and_ack_numbers(memory);
     next_seq_number = (seq_number + packet_data_length);
     if(is_last_packet==1) {
      send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, next_seq_number);
      already_sended_seq = next_seq_number;
     }
     tcp_connection_status = TCP_HTTP_TRANSFERRING_FILE;
     
     //test if this is end of file
     packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+tcp_header_length+packet_data_length-4);
     if(http_content_length==file_transferred_length || http_content_length==0xFFFFFFFF || *packet32==0x0A0D0A0D) {
      if(is_last_packet==0) { //always acknowledge that we received full file
       send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, next_seq_number);
       already_sended_seq = next_seq_number;
      }
      send_tcp_finalize(packet_ip, tcp_packet_port, ack_number, next_seq_number);
      tcp_connection_status = TCP_CONNECTION_CLOSED;

      return;
     }
     
     return;
    }
   }
   if(tcp_connection_status==TCP_HTTP_TRANSFERRING_FILE && file_tcp_port==tcp_packet_port) {
    if(packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x10 || packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x18) { //ACK or PSH and ACK    
     //receive only packet with right sequence number
     read_tcp_seq_and_ack_numbers(memory);
     if(seq_number<next_seq_number) { //this packet is not in order
      //this is retransmitted packet, we should send to server number of our last catched packet
      if(next_seq_number!=already_sended_seq) {
       send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, next_seq_number);
      }
      return;
     }
     if(seq_number!=next_seq_number) { //this packet is not in order
      return;
     }

     tcp_new_packet = 1; //this variabile is used in file_transfer.c methods
     
     //TODO: receive more than 1 MB
     if((file_transferred_length+packet_data_length)>1024*1024) {
      send_tcp_reset(packet_ip, tcp_packet_port, ack_number, seq_number);
      tcp_connection_status = TCP_CONNECTION_CLOSED;
      return;
     }
    
     //save data of file in this packet TODO: http chunks
     copy_memory((memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+tcp_header_length), file_memory_pointer, packet_data_length);
     file_memory_pointer += packet_data_length;
     file_transferred_length += packet_data_length;
     
     //acknowledge
     next_seq_number = (seq_number + packet_data_length);
     if(is_last_packet==1) {
      if(next_seq_number!=already_sended_seq) {
       send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, next_seq_number);
       already_sended_seq = next_seq_number;
      }
     }
     
     //test if this is end of file
     packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+tcp_header_length+packet_data_length-4);
     if(http_content_length==file_transferred_length || *packet32==0x0A0D0A0D) {
      if(is_last_packet==0) { //always acknowledge that we received full file
       if(next_seq_number!=already_sended_seq) {
        send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, next_seq_number);
        already_sended_seq = next_seq_number;
       }
      }
      send_tcp_finalize(packet_ip, tcp_packet_port, ack_number, next_seq_number);
      already_sended_seq = next_seq_number;
      tcp_connection_status = TCP_CONNECTION_CLOSED;
      return;
     }
     
     return;
    }
   }
   if(packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x11 || packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x19) { //FIN and ACK or FIN, PSH and ACK
    if(tcp_connection_status==TCP_HTTP_TRANSFERRING_FILE) { //if server finalize connection
     read_tcp_seq_and_ack_numbers(memory);
     seq_number++;
     send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, seq_number);
     send_tcp_finalize(packet_ip, tcp_packet_port, ack_number, seq_number);
     if(file_tcp_port==tcp_packet_port) {
      tcp_connection_status=TCP_CONNECTION_CLOSED;
     }
     return;
    }
    else {
     read_tcp_seq_and_ack_numbers(memory);
     ack_number++;
     seq_number++;
     send_tcp_acknowledge(packet_ip, tcp_packet_port, ack_number, seq_number);
     if(file_tcp_port==tcp_packet_port) {
      tcp_connection_status=TCP_CONNECTION_CLOSED;
     }
     return;
    }
   }
   if(tcp_connection_status==TCP_CONNECTION_CLOSED) {
    if(packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x10 || packet8[ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+13]==0x18) { //ACK or PSH and ACK
     send_tcp_reset(packet_ip, tcp_packet_port, 0, 0); //if server sends data after closed connection, reset it
    }
   }
  }
 }
 else if(packet16[6]==ARP_LAYER_TYPE) {
  if(packet16[10]==ARP_REQUEST) {
   process_arp_request(memory);
  }
  if(packet16[10]==ARP_REPLY) {
   //TODO:
  }
 }
}

void connect_to_network(void) {
 //Discover
 send_dhcp_discover();
 
 //Offer
 ticks=0;
 while(ticks<500) {
  if(ip_address!=0x00000000 && server_ip!=0x00000000 && dns_server_ip!=0x00000000) {
   break;
  }
 }
 if(ip_address==0x00000000 || server_ip==0x00000000 || dns_server_ip==0x00000000) {
  log("Network: DHCP Offer timeout\n");
  connected_to_network = NETWORK_CONNECTION_FALIED;
  return; //correct DHCP Offer was not received
 }
 
 //Request
 dhcp_acknowledge=0;
 send_dhcp_request();
 
 //Acknowledge
 ticks=0;
 while(ticks<500) {
  if(dhcp_acknowledge==DHCP_ACK) {
   break;
  }
  if(dhcp_acknowledge==DHCP_NAK) {
   log("Network: DHCP Not Acknowledged\n");
   connected_to_network = NETWORK_CONNECTION_FALIED;
   return;
  }
 }
 if(dhcp_acknowledge==0) {
  log("Network: DHCP Acknowledge timeout\n"); //DHCP Acknowledge was not received
  connected_to_network = NETWORK_CONNECTION_FALIED;
  return;
 }
 
 connected_to_network = STATUS_TRUE;
 log("Connected to network\nOur IP: ");
 log_var(ip_address & 0xFF);
 log(".");
 log_var((ip_address >> 8) & 0xFF);
 log(".");
 log_var((ip_address >> 16) & 0xFF);
 log(".");
 log_var((ip_address >> 24));
 log("\n");
 log("Router MAC: ");
 for(int i=0; i<6; i++) {
  log_hex_specific_size(router_mac_address[i], 2);
  log(" ");
 }
 log("\n");
 log("Router IP: ");
 log_var(server_ip & 0xFF);
 log(".");
 log_var((server_ip >> 8) & 0xFF);
 log(".");
 log_var((server_ip >> 16) & 0xFF);
 log(".");
 log_var((server_ip >> 24));
 log("\n");
}

void connect_to_network_with_message(void) {
 show_system_message("Connecting to network...");
 connect_to_network();
 remove_system_message();
}
