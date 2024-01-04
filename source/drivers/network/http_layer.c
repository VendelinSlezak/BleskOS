//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//currently supported HTTP 1.0 and HTTP 1.1

void send_http_get(byte_t *url, dword_t ip, word_t source_port) {
 dword_t http_get_length = 0;
 for(int i=0, is_without_slash=1; i<2048; i++) {
  if(url[i]=='/') {
   is_without_slash=0;
  }
  if(url[i]==0) {
   if(is_without_slash==1) {
    http_get_length++;
   }
   break;
  }
  http_get_length++;
 }
 http_get_length += 25;
 clear_memory(packet_memory, 1600);
 byte_t *packet8 = (byte_t *) (packet_memory + ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH + 4);
 dword_t *packet32 = (dword_t *) (packet_memory + ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH);
 
 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 receiver_ip_address = ip;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + TCP_LAYER_LENGTH + http_get_length, TCP_PROTOCOL_TYPE);
 create_tcp_layer(packet_memory, source_port, ack_number, seq_number, ((1 << 3) | (1 << 4))); //PSH and ACK flag
 
 //create HTTP GET
 packet32[0] = 0x20544547; //'GET '
 //copy url after website address
 for(int i=0; i<2048; i++) {
  if(url[i]=='/') {
   *packet8='/';
   for(int j=i; j<2048; j++) {
    if(url[j]==0) {
     break;
    }
    *packet8=url[j];
    packet8++;
   }
   break;
  }
  if(url[i]==0) {
   *packet8='/';
   packet8++;
   break;
  }
 }
 *packet8 = ' ';
 packet32 = (dword_t *) (packet8+1);
 packet32[0] = 0x50545448; //'HTTP'
 packet32[1] = 0x312E312F; //'/1.1'
 packet32[2] = 0x00000A0D;
 packet32 = (dword_t *) ((dword_t)packet32+10);
 packet32[0] = 0x74736F48; //'Host'
 packet32[1] = 0x0000203A; //': '
 packet8 = (byte_t *) ((dword_t)packet32+6);
 //copy only wesite address
 for(int i=0; i<2048; i++) {
  if(url[i]==0 || url[i]=='/') {
   break;
  }
  *packet8=url[i];
  packet8++;
 }
 packet32 = (dword_t *) (packet8);
 *packet32 = 0x0A0D0A0D; //end of HTTP
 
 calculate_tcp_checksum(packet_memory, TCP_LAYER_LENGTH + http_get_length);
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH + http_get_length));
}

void process_http_reply(dword_t memory, dword_t end_of_memory) {
 byte_t *packet8 = (byte_t *) (memory);
 word_t *packet16 = (word_t *) (memory);
 dword_t *packet32 = (dword_t *) memory;
 
 http_response_type = 0;
 http_data_start_mem = 0;
 http_content_length = 0;

 if(packet32[0]!=0x50545448) { //HTTP
  return; //not HTTP response
 }
 
 if(!(packet32[1]==0x302E312F || packet32[1]==0x312E312F)) {
  return; //not HTTP 1.0 / 1.1
 }
 
 http_response_type = packet32[2];

 //skip to first item
 while(packet8[0]!=0x0D && packet8[1]!=0xA) {
  if(packet8[0]==0) {
   return;
  }
  packet8++;
 }
 packet8+=2;
 
 //parse items
 while((dword_t)packet8<end_of_memory) {
  if(packet8[0]=='C' && packet8[1]=='o' && packet8[2]=='n' && packet8[3]=='t' && packet8[4]=='e' && packet8[5]=='n' && packet8[6]=='t' && packet8[7]=='-' && packet8[8]=='L' && packet8[9]=='e' && packet8[10]=='n' && packet8[11]=='g' && packet8[12]=='t' && packet8[13]=='h') {
   http_content_length = convert_byte_string_to_number((dword_t)packet8+16);
   if(http_content_length==0) {
    http_content_length=0xFFFFFFFF;
   }
  }
  if(packet8[0]=='L' && packet8[1]=='o' && packet8[2]=='c' && packet8[3]=='a' && packet8[4]=='t' && packet8[5]=='i' && packet8[6]=='o' && packet8[7]=='n' && packet8[8]==':') {   
   packet8 += 9; //skip to url
   if(*packet8==' ') { //skip space at start
    packet8++;
   }
   if(packet8[0]=='/') {
    dword_t http_new_location_pointer = 0;

    //add base address
    for(int i=0; i<2048; i++) {
     http_url_new_location[i]=file_full_url[i];
     if(file_full_url[i]==0) {
      break;
     }
    }

    //skip base address
    if(are_equal_b_string_b_string((byte_t *)((dword_t)http_url_new_location), "http://")==STATUS_TRUE) {
     http_new_location_pointer += 7;
    }
    else if(are_equal_b_string_b_string((byte_t *)((dword_t)http_url_new_location), "https://")==STATUS_TRUE) {
     http_new_location_pointer += 8;
    }
    while(http_new_location_pointer<2048) {
     if(http_url_new_location[http_new_location_pointer]==0 || http_url_new_location[http_new_location_pointer]=='/') {
      break;
     }
     http_new_location_pointer++;
    }

    //add address to end of base address
    for(int i=0; i<2048; i++) {
     if(packet8[i]==0x0D || http_new_location_pointer>=2048) {
      break;
     }
     http_url_new_location[http_new_location_pointer]=packet8[i];
     http_new_location_pointer++;
    }

    http_url_new_location[http_new_location_pointer]=0;
   }
   else {
    //copy this address
    clear_memory((dword_t)http_url_new_location, 2048);
    for(int i=0; i<2048; i++) {
     if(packet8[i]==0x0D) {
      break;
     }
     http_url_new_location[i]=packet8[i];
    }
   }
  }
  if(packet8[0]=='l' && packet8[1]=='o' && packet8[2]=='c' && packet8[3]=='a' && packet8[4]=='t' && packet8[5]=='i' && packet8[6]=='o' && packet8[7]=='n' && packet8[8]==':') {   
   packet8 += 9; //skip to url
   if(*packet8==' ') { //skip space at start
    packet8++;
   }
   if(packet8[0]=='/') {
    dword_t http_new_location_pointer = 0;

    //add base address
    for(int i=0; i<2048; i++) {
     http_url_new_location[i]=file_full_url[i];
     if(file_full_url[i]==0) {
      break;
     }
    }

    //skip base address
    if(are_equal_b_string_b_string((byte_t *)((dword_t)http_url_new_location), "http://")==STATUS_TRUE) {
     http_new_location_pointer += 7;
    }
    else if(are_equal_b_string_b_string((byte_t *)((dword_t)http_url_new_location), "https://")==STATUS_TRUE) {
     http_new_location_pointer += 8;
    }
    while(http_new_location_pointer<2048) {
     if(http_url_new_location[http_new_location_pointer]==0 || http_url_new_location[http_new_location_pointer]=='/') {
      break;
     }
     http_new_location_pointer++;
    }

    //add address to end of base address
    for(int i=0; i<2048; i++) {
     if(packet8[i]==0x0D || http_new_location_pointer>=2048) {
      break;
     }
     http_url_new_location[http_new_location_pointer]=packet8[i];
     http_new_location_pointer++;
    }

    http_url_new_location[http_new_location_pointer]=0;
   }
   else {
    //copy this address
    clear_memory((dword_t)http_url_new_location, 2048);
    for(int i=0; i<2048; i++) {
     if(packet8[i]==0x0D) {
      break;
     }
     http_url_new_location[i]=packet8[i];
    }
   }
  }
  
  //jump to next item
  while(packet16[0]!=0x0A0D) {
   if((dword_t)packet16>end_of_memory) {
    return;
   }
   packet16 = (word_t *) ((dword_t)packet16+1);
  }
  packet16++;
  if(packet16[0]==0x0A0D) { //end of HTTP items
   http_data_start_mem = ((dword_t)packet16+2);
   http_data_in_packet_length = (end_of_memory-http_data_start_mem);
   return;
  }
  packet8 = (byte_t *) ((dword_t)packet16);
 }
}