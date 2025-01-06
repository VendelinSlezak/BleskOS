//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void create_dhcp_discover_layer(dword_t memory) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 dword_t *packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 
 clear_memory((memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH), 255);
 
 packet8[0]=1; //this is request
 packet8[1]=1; //ethernet
 packet8[2]=6; //length of hardware address
 packet8[3]=0;
 packet32[1]=0x12341234; //value of this transaction
 packet16[4]=8; //number of seconds
 packet16[5]=0; //flags
 //mac address
 packet8[28]=mac_address[0];
 packet8[29]=mac_address[1];
 packet8[30]=mac_address[2];
 packet8[31]=mac_address[3];
 packet8[32]=mac_address[4];
 packet8[33]=mac_address[5];
 //DHCP magic value
 packet8[236]=99;
 packet8[237]=130;
 packet8[238]=83;
 packet8[239]=99;
 
 //DHCP discover
 packet8[240]=0x35;
 packet8[241]=0x01;
 packet8[242]=0x01;
 
 //request for 192.168.1.100
 packet8[243]=0x32;
 packet8[244]=0x04;
 packet8[245]=192;
 packet8[246]=168;
 packet8[247]=1;
 packet8[248]=100;
 
 //parameter request list
 packet8[249]=0x37;
 packet8[250]=0x03;
 packet8[251]=1;
 packet8[252]=3;
 packet8[253]=6;
 
 //end
 packet8[254]=0xFF;
}

void create_dhcp_request_layer(dword_t memory) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 dword_t *packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 
 clear_memory((memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH), 256);
 
 packet8[0]=1; //this is request
 packet8[1]=1; //ethernet
 packet8[2]=6; //length of hardware address
 packet8[3]=0;
 packet32[1]=0x12341234; //value of this transaction
 packet16[4]=8; //number of seconds
 packet16[5]=0; //flags
 //mac address
 packet8[28]=mac_address[0];
 packet8[29]=mac_address[1];
 packet8[30]=mac_address[2];
 packet8[31]=mac_address[3];
 packet8[32]=mac_address[4];
 packet8[33]=mac_address[5];
 //DHCP magic value
 packet8[236]=99;
 packet8[237]=130;
 packet8[238]=83;
 packet8[239]=99;
 
 //DHCP request
 packet8[240]=0x35;
 packet8[241]=0x01;
 packet8[242]=0x03;
 
 //our IP
 packet8[243]=0x32;
 packet8[244]=0x04;
 packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+245);
 *packet32 = ip_address;
 
 //server IP
 packet8[249]=0x36;
 packet8[250]=0x04;
 packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+251);
 *packet32 = server_ip;
 
 //end
 packet8[255]=0xFF;
}

void send_dhcp_discover(void) {
 clear_memory(packet_memory, 1600);
 
 set_broadcast_receiver_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 ip_address=0x00000000;
 receiver_ip_address=0xFFFFFFFF;
 server_ip=0x00000000;
 dns_server_ip=0x00000000;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + UDP_LAYER_LENGTH + DHCP_DISCOVER_LAYER_LENGTH, UDP_PROTOCOL_TYPE);
 create_udp_layer(packet_memory, (UDP_LAYER_LENGTH + DHCP_DISCOVER_LAYER_LENGTH), 68, 67);
 create_dhcp_discover_layer(packet_memory);
 calculate_udp_checksum(packet_memory, (UDP_LAYER_LENGTH + DHCP_DISCOVER_LAYER_LENGTH));
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + UDP_LAYER_LENGTH + DHCP_DISCOVER_LAYER_LENGTH));
}

void send_dhcp_request(void) {
 clear_memory(packet_memory, 1600);

 //receiver MAC is already set from DHCP Offer
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 //receiver_ip_address=server_ip; TODO: needed?
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + UDP_LAYER_LENGTH + DHCP_REQUEST_LAYER_LENGTH, UDP_PROTOCOL_TYPE);
 create_udp_layer(packet_memory, (UDP_LAYER_LENGTH + DHCP_REQUEST_LAYER_LENGTH), 68, 67);
 create_dhcp_request_layer(packet_memory);
 calculate_udp_checksum(packet_memory, (UDP_LAYER_LENGTH + DHCP_REQUEST_LAYER_LENGTH));
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + UDP_LAYER_LENGTH + DHCP_REQUEST_LAYER_LENGTH));
}

void process_dhcp_packet(dword_t memory) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+240);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+240);
 dword_t *packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+240);
 dword_t offset = 0;

 //found DHCP packet type entry
 while(packet8[offset]!=0x35) {
  offset += (packet8[offset+1]+2); //next entry
  if(packet8[offset+1]==0x00 || offset>1512) {
   return; //something is wrong with this packet
  }
 }
 
 //DHCP Offer
 if(packet8[offset+2]==0x02) {
  packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+16);
  ip_address = *packet32; //our IP address
  
  //search for server IP and DNS server IP
  server_ip = 0x00000000;
  dns_server_ip = 0x00000000;
  offset = 0;
  while(packet8[offset]!=0x00 && packet8[offset]!=0xFF) {
   //server IP
   if(packet8[offset]==0x03) {
    packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+240+offset+2);
    server_ip = *packet32;
   }
   //DNS server IP
   if(packet8[offset]==0x06) {
    packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+240+offset+2);
    dns_server_ip = *packet32;
   }
   offset += (packet8[offset+1]+2); //next entry
  }
  
  //copy router MAC
  packet8 = (byte_t *) (memory);
  router_mac[0]=packet8[6];
  router_mac[1]=packet8[7];
  router_mac[2]=packet8[8];
  router_mac[3]=packet8[9];
  router_mac[4]=packet8[10];
  router_mac[5]=packet8[11];
  
  return;
 }
 
 //DHCP Acknowledge
 if(packet8[offset+2]==0x05) {
  dhcp_acknowledge=DHCP_ACK;
  return;
 }
 
 //DHCP Not acknowledge
 if(packet8[offset+2]==0x06) {
  dhcp_acknowledge=DHCP_NAK;
  return;
 }
}