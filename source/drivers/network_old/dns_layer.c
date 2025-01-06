//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void send_dns_query(byte_t *url) {
 dword_t dns_variable_length = 0;
 for(int i=0; i<2048; i++) { //URL can not be longer than 2048 chars
  if(url[i]==0 || url[i]=='/') {
   break;
  }
  dns_variable_length++;
 }
 dns_variable_length += 6;
 clear_memory(packet_memory, 1600);
 word_t *packet16 = (word_t *) (packet_memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 byte_t *packet8_label_count = (byte_t *) (packet_memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+DNS_BASIC_LAYER_LENGTH);
 byte_t *packet8 = (byte_t *) (packet_memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+DNS_BASIC_LAYER_LENGTH+1);

 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 receiver_ip_address = dns_server_ip;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + UDP_LAYER_LENGTH + DNS_BASIC_LAYER_LENGTH + dns_variable_length, UDP_PROTOCOL_TYPE);
 create_udp_layer(packet_memory, (UDP_LAYER_LENGTH + DNS_BASIC_LAYER_LENGTH + dns_variable_length), 50000, 53);
 
 //DNS layer
 packet16[0]=0xCDAB; //transaction ID
 packet16[1]=0x0101; //flags - standard query with no recursion
 packet16[2]=0x0100; //one question
 packet16[3]=0x0000;
 packet16[4]=0x0000;
 packet16[5]=0x0000;
 
 //DNS question
 for(int i=0; i<2048; i++) {
  if(url[i]==0 || url[i]=='/') {
   *packet8 = 0x00;
   break;
  }
  if(url[i]=='.') {
   packet8_label_count = (byte_t *) ((dword_t)packet8);
   *packet8_label_count = 0;
   packet8++;
   continue;
  }
  //insert char
  *packet8 = url[i];
  packet8++;
  //increase label count
  packet8_label_count[0]++;
 }
 packet8[1]=0x00;
 packet8[2]=0x01; //type A
 packet8[3]=0x00;
 packet8[4]=0x01; //class IN
 
 calculate_udp_checksum(packet_memory, (UDP_LAYER_LENGTH + DNS_BASIC_LAYER_LENGTH + dns_variable_length));
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + UDP_LAYER_LENGTH + DNS_BASIC_LAYER_LENGTH + dns_variable_length));
}

void process_dns_reply(dword_t memory) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH+DNS_BASIC_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+UDP_LAYER_LENGTH);
 
 //not our transaction ID
 if(packet16[0]!=0xCDAB) {
  return;
 }
 //DNS no name/refused
 if((packet16[1] & 0x8F00)==0x8300 || (packet16[1] & 0x8F00)==0x8500) {
  dns_answer_ip_address = 0xFFFFFFFF;
  return;
 }
 //not response or more than one question or no answer
 if((packet16[1] & 0x8000)!=0x8000 || packet16[2]!=0x0100 || packet16[3]==0) {
  dns_answer_ip_address = 0xFFFFFFFF;
  return;
 }
 
 //skip question
 for(int i=0; i<1024; i++) {
  if(*packet8==0) {
   break;
  }
  packet8 += (*packet8+1); //skip to next label
 }
 packet8 += 5;
 
 //read answer
 for(int i=0; i<100; i++) {
  word_t *packet16 = (word_t *) (packet8);
  if(packet16[1]==0x0100) {
   dword_t *packet32 = (dword_t *) (packet8+12);
   dns_answer_ip_address = *packet32;
   return;
  }

  //skip answer
  packet8 = (byte_t *) (packet8+12+(packet8[10]<<8 | packet8[11]));
 }
}

dword_t get_ip_address_of_url(byte_t *url) {
 dns_answer_ip_address = 0;
 send_dns_query(url);
 ticks=0;
 while(ticks<500 && dns_answer_ip_address==0) { //wait max one second
  asm("hlt");
 }
 if(dns_answer_ip_address==0) {
  //no response, send DNS query again
  send_dns_query(url);
  ticks=0;
  while(ticks<500 && dns_answer_ip_address==0) { //wait max one second
   asm("hlt");
  }
  if(dns_answer_ip_address==0) {
   //no response, send DNS query once again
   send_dns_query(url);
   ticks=0;
   while(ticks<1000 && dns_answer_ip_address==0) { //wait max two seconds
    asm("hlt");
   }
   if(dns_answer_ip_address==0) {
    dns_answer_ip_address = 0xFFFFFFFF; //no response
   }
  }
 }
 
 return dns_answer_ip_address;
}