//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void process_arp_request(dword_t memory) {
 clear_memory(packet_memory, 1600);
 byte_t *packet8 = (byte_t *) (packet_memory);
 word_t *packet16 = (word_t *) (packet_memory);
 dword_t *packet32 = (dword_t *) (packet_memory);
 dword_t *request_packet32 = (dword_t *) (memory);

 if(ip_address==0x00000000) {
  return;
 }

 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, ARP_LAYER_TYPE);
 
 //ARP reply
 packet16[7]=0x0100; //ethernet
 packet16[8]=0x0008; //IP protocol
 packet8[18]=6; //hardware address have 6 bytes
 packet8[19]=4; //protocol address have 4 bytes
 packet16[10]=ARP_REPLY;
 
 //our MAC
 packet8[22]=mac_address[0];
 packet8[23]=mac_address[1];
 packet8[24]=mac_address[2];
 packet8[25]=mac_address[3];
 packet8[26]=mac_address[4];
 packet8[27]=mac_address[5];
 
 //our IP
 packet32[7] = ip_address;
 
 //receiver MAC
 packet8[32]=receiver_mac_address[0];
 packet8[33]=receiver_mac_address[1];
 packet8[34]=receiver_mac_address[2];
 packet8[35]=receiver_mac_address[3];
 packet8[36]=receiver_mac_address[4];
 packet8[37]=receiver_mac_address[5];
 
 //receiver IP
 packet32 = (dword_t *) (packet_memory+38);
 *packet32 = request_packet32[7];
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + ARP_REPLY_LAYER_LENGTH + 18));
}

void send_arp_request(dword_t ip) {
 clear_memory(packet_memory, 1600);
 byte_t *packet8 = (byte_t *) (packet_memory);
 word_t *packet16 = (word_t *) (packet_memory);
 dword_t *packet32 = (dword_t *) (packet_memory);
 
 if(ip_address==0x00000000) {
  return;
 }

 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, ARP_LAYER_TYPE);
 
 //ARP request
 packet16[7]=0x0100; //ethernet
 packet16[8]=0x0008; //IP protocol
 packet8[18]=6; //hardware address have 6 bytes
 packet8[19]=4; //protocol address have 4 bytes
 packet16[10]=ARP_REQUEST;
 
 //our MAC
 packet8[22]=mac_address[0];
 packet8[23]=mac_address[1];
 packet8[24]=mac_address[2];
 packet8[25]=mac_address[3];
 packet8[26]=mac_address[4];
 packet8[27]=mac_address[5];
 
 //our IP
 packet32[7]=ip_address;
 
 //receiver MAC
 packet8[32]=0xFF;
 packet8[33]=0xFF;
 packet8[34]=0xFF;
 packet8[35]=0xFF;
 packet8[36]=0xFF;
 packet8[37]=0xFF;
 
 //receiver IP
 packet32 = (dword_t *) (packet_memory+38);
 *packet32 = ip;
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + ARP_REPLY_LAYER_LENGTH + 18));
}