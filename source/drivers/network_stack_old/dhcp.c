//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void network_packet_add_dhcp_layer(byte_t *options) {
 network_packet_dhcp_layer = (struct network_packet_dhcp_layer_t *) ((dword_t)memory_for_building_network_packet + size_of_network_packet);

 //set DHCP layer
 clear_memory((dword_t)network_packet_dhcp_layer, sizeof(struct network_packet_dhcp_layer_t));
 network_packet_dhcp_layer->op = NETWORK_PACKET_DHCP_REQUEST_OP;
 network_packet_dhcp_layer->htype = 1; //ethernet
 network_packet_dhcp_layer->hlen = 6; //MAC address length
 network_packet_dhcp_layer->xid = 0x78563412; //ID of transaction in big endian
 for(dword_t i=0; i<4; i++) {
  network_packet_dhcp_layer->yiaddr[i] = network_packet_ipv4_layer->source_ip_address[i];
 }
 for(dword_t i=0; i<6; i++) {
  network_packet_dhcp_layer->chaddr[i] = mac_address[i]; //TODO: better name than mac_address
 }

 //DHCP magic cookie
 network_packet_dhcp_layer->options[0] = 0x63;
 network_packet_dhcp_layer->options[1] = 0x82;
 network_packet_dhcp_layer->options[2] = 0x53;
 network_packet_dhcp_layer->options[3] = 0x63;

 //copy DHCP options
 dword_t pointer = 4;
 while(*options!=0xFF) {
  network_packet_dhcp_layer->options[pointer] = *options;
  options++;
  pointer++;
 }
 network_packet_dhcp_layer->options[pointer] = 0xFF; //add ending of list

 //set variables
 size_of_network_packet += sizeof(struct network_packet_dhcp_layer_t);
 network_packet_save_layer(NETWORK_PACKET_DHCP_LAYER, sizeof(struct network_packet_dhcp_layer_t), 0);
}

byte_t parse_dhcp_layer(byte_t *dhcp_layer_pointer, dword_t dhcp_layer_size) {
 clear_memory((dword_t)&dhcp_info, sizeof(struct dhcp_info_t));

 if(dhcp_layer_size < 240) {
  return STATUS_ERROR; //DHCP layer is too small
 }

 //getting our IP address that server assigned
 struct network_packet_dhcp_layer_t *dhcp_layer = (struct network_packet_dhcp_layer_t *) (dhcp_layer_pointer);
 for(dword_t i = 0; i < 4; i++) {
  dhcp_info.our_ip_address[i] = dhcp_layer->yiaddr[i];
 }

 //initalization of variables
 byte_t *options = (byte_t *) ((dword_t)dhcp_layer_pointer + 240); //240 is size of DHCP header
 byte_t *end = (byte_t *) ((dword_t)dhcp_layer_pointer + dhcp_layer_size); //this is where DHCP ends

 //processing of DHCP options
 while(options < end) {
  byte_t option_code = options[0];
  
  //ending of options
  if(option_code == 0xFF) {
   break;
  }

  //padding
  if(option_code == 0) {
   options++;
   continue;
  }

  //get basic info about option
  byte_t option_length = options[1];
  byte_t *option_data = options + 2;

  //read option
  switch(option_code) {
   //IP adress of DNS server
   case 6:
    if(option_length >= 4) {
     for (int i = 0; i < 4; i++) {
      dhcp_info.dns_server_ip_address[i] = option_data[i]; //we use only first IP TODO: read more IP?
     }
    }
    break;

   //IP address of server
   case 54:
    if(option_length == 4) {
     for(dword_t i = 0; i < 4; i++) {
      dhcp_info.server_ip_address[i] = option_data[i];
     }
    }
    break;

   //unknown option
   default:
    break;
  }

  //move to next option
  options += (2 + option_length);
 }

 return STATUS_GOOD;
}