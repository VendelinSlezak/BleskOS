//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void dhcp_connect_to_router(void) {
 //create DHCP discover packet
 start_building_network_packet();
 network_packet_add_ethernet_layer((byte_t [6]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
 network_packet_add_ipv4_layer((byte_t [4]){0x00, 0x00, 0x00, 0x00}, (byte_t [4]){0xFF, 0xFF, 0xFF, 0xFF});
 network_packet_add_udp_layer(68, 67);
 network_packet_add_dhcp_layer((byte_t []){0x35, 0x01, 0x01, //DHCP discover
                                           0x37, 0x03, 1, 3, 6, //Parameter Request List
                                           0xFF //End
                                          });
 finalize_builded_network_packet();

 //send DHCP discover packet and dhcp_offer_received() or dhcp_discover_error() will be called based on response
 send_builded_packet_with_response_to_internet(2000, 4, PWRM_PACKET_TYPE_IPV4_UDP, 67, 68, dhcp_offer_received, dhcp_discover_error);
}

byte_t dhcp_offer_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length) {
 //parse received DHCP offer packet
 if(parse_dhcp_layer(packet_memory, packet_length)==STATUS_ERROR) {
  //TODO: save error about connecting to internet?
  return PWRM_END_TRANSFER;
 }

 //save MAC address of router
 struct network_packet_ethernet_layer_t *ethernet_layer = (struct network_packet_ethernet_layer_t *) memory_of_full_processed_packet;
 for(dword_t i=0; i<6; i++) {
  internet.router_mac[i] = ethernet_layer->sender_mac_address[i];
 }

 //create DHCP response packet based on info from DHCP offer packet
 start_building_network_packet();
 network_packet_add_ethernet_layer(internet.router_mac);
 network_packet_add_ipv4_layer((byte_t [4]){0x00, 0x00, 0x00, 0x00}, (byte_t [4]){0xFF, 0xFF, 0xFF, 0xFF});
 network_packet_add_udp_layer(68, 67);
 network_packet_add_dhcp_layer((byte_t []){0x35, 0x01, 0x03, //DHCP request
                                           0x32, 0x04, dhcp_info.our_ip_address[0], dhcp_info.our_ip_address[1], dhcp_info.our_ip_address[2], dhcp_info.our_ip_address[3], //our IP
                                           0x36, 0x04, dhcp_info.server_ip_address[0], dhcp_info.server_ip_address[1], dhcp_info.server_ip_address[2], dhcp_info.server_ip_address[3], //server IP
                                           0xFF // End
                                          });
 finalize_builded_network_packet();

 //update entry to send DHCP response packet and dhcp_acknowledge_received() or dhcp_response_error() will be called based on response
 update_packet_entry(number_of_packet_entry, 2000, 4, PWRM_PACKET_TYPE_IPV4_UDP, 67, 68, dhcp_acknowledge_received, dhcp_response_error);
 return PWRM_CONTINUE_TRANSFER;
}

byte_t dhcp_acknowledge_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length) {
 //update internet structure based on data from DHCP acknowledge
 if(parse_dhcp_layer(packet_memory, packet_length)==STATUS_ERROR) {
  //TODO: save error about connecting to internet?
  return PWRM_END_TRANSFER;
 }

 //set variables
 for(dword_t i=0; i<4; i++) {
  internet.our_ip[i] = dhcp_info.our_ip_address[i];
  internet.router_ip[i] = dhcp_info.server_ip_address[i];
  internet.dns_server_ip[i] = dhcp_info.dns_server_ip_address[i];
 }
 internet.status = INTERNET_STATUS_CONNECTED;

 //log connection
 logf("\n\nNetwork: Connection to router was succesfull");
 logf("\nOur IP: %d.%d.%d.%d", internet.our_ip[0], internet.our_ip[1], internet.our_ip[2], internet.our_ip[3]);
 logf("\nRouter IP: %d.%d.%d.%d", internet.router_ip[0], internet.router_ip[1], internet.router_ip[2], internet.router_ip[3]);
 logf("\nDNS server IP: %d.%d.%d.%d", internet.dns_server_ip[0], internet.dns_server_ip[1], internet.dns_server_ip[2], internet.dns_server_ip[3]);

 //this transfer ends here
 return PWRM_END_TRANSFER;
}

void dhcp_discover_error(byte_t number_of_packet_entry, byte_t error) {
 logf("\nERROR: DHCP offer was not received");

 internet.status = INTERNET_STATUS_CONNECTION_ERROR;
}

void dhcp_response_error(byte_t number_of_packet_entry, byte_t error) {
 logf("\nERROR: DHCP acknowledgement was not received");

 internet.status = INTERNET_STATUS_CONNECTION_ERROR;
}