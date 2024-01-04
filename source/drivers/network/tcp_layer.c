//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void create_tcp_layer(dword_t memory, word_t source_port, dword_t sequence_number, dword_t acknowledge_number, byte_t control) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 dword_t *packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 word_t big_endain_value;
 
 big_endain_value = BIG_ENDIAN(source_port);
 packet16[0] = big_endain_value; //source port
 packet16[1] = BIG_ENDIAN(80); //destination port
 packet32[1] = (((sequence_number & 0xFF)<<24) | (((sequence_number>>8) & 0xFF)<<16) | (((sequence_number>>16) & 0xFF)<<8) | (sequence_number>>24)); //sequence number
 packet32[2] = (((acknowledge_number & 0xFF)<<24) | (((acknowledge_number>>8) & 0xFF)<<16) | (((acknowledge_number>>16) & 0xFF)<<8) | (acknowledge_number>>24)); //acknowledge number
 packet8[12]=0x60; //TCP layer length
 packet8[13]=control; //TCP packet type
 packet16[7]=0xFFFF; //max window length 64 KB
 packet16[8]=0; //checksum
 packet16[9]=0; //urgent pointer
 packet32[5]=0xB4050402; //Maximum Segment Size 1460 bytes
}

void calculate_tcp_checksum(dword_t memory, word_t length_of_data) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 dword_t checksum;
 
 packet16[8] = 0; //checksum
 
 //checksum
 checksum = 0;
 for(int i=0; i<(length_of_data/2); i++) {
  checksum += packet16[i];
 }
 if((length_of_data & 0x1)==0x1) {
  checksum += (packet8[length_of_data-1]);
 }
 checksum += ((word_t)(length_of_data>>8)+(word_t)(length_of_data<<8)+0x0600);
 checksum += (ip_address & 0xFFFF);
 checksum += (ip_address >> 16);
 checksum += (receiver_ip_address & 0xFFFF);
 checksum += (receiver_ip_address >> 16);
 while((checksum>>16)>0) {
  checksum = ((checksum & 0xFFFF) + (checksum>>16));
 }
 checksum = ~checksum;
 
 packet16[8] = (word_t)checksum; //checksum
}

void read_tcp_seq_and_ack_numbers(dword_t memory) {
 dword_t *packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH+4);
 
 seq_number = (((packet32[0] & 0xFF)<<24) | (((packet32[0]>>8) & 0xFF)<<16) | (((packet32[0]>>16) & 0xFF)<<8) | (packet32[0]>>24));
 ack_number = (((packet32[1] & 0xFF)<<24) | (((packet32[1]>>8) & 0xFF)<<16) | (((packet32[1]>>16) & 0xFF)<<8) | (packet32[1]>>24));
}

void send_tcp_handshake(dword_t ip, word_t source_port) {
 clear_memory(packet_memory, 1600);
 already_sended_seq = 0;

 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 receiver_ip_address = ip;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + TCP_LAYER_LENGTH, TCP_PROTOCOL_TYPE);
 create_tcp_layer(packet_memory, source_port, 0, 0, (1 << 1)); //SYN flag
 calculate_tcp_checksum(packet_memory, TCP_LAYER_LENGTH);
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH));
}

void send_tcp_acknowledge(dword_t ip, word_t source_port, dword_t sequence_number, dword_t acknowledge_number) {
 clear_memory(packet_memory, 1600);
 
 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 receiver_ip_address = ip;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + TCP_LAYER_LENGTH, TCP_PROTOCOL_TYPE);
 create_tcp_layer(packet_memory, source_port, sequence_number, acknowledge_number, (1 << 4)); //ACK flag
 calculate_tcp_checksum(packet_memory, TCP_LAYER_LENGTH);
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH));
}

void send_tcp_finalize(dword_t ip, word_t source_port, dword_t sequence_number, dword_t acknowledge_number) {
 clear_memory(packet_memory, 1600);

 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 receiver_ip_address = ip;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + TCP_LAYER_LENGTH, TCP_PROTOCOL_TYPE);
 create_tcp_layer(packet_memory, source_port, sequence_number, acknowledge_number, (1 << 0)); //FIN and ACK flag
 calculate_tcp_checksum(packet_memory, TCP_LAYER_LENGTH);
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH));
}

void send_tcp_reset(dword_t ip, word_t source_port, dword_t sequence_number, dword_t acknowledge_number) {
 clear_memory(packet_memory, 1600);
 
 set_receiver_mac_as_router_mac();
 create_ethernet_layer(packet_memory, IP_LAYER_TYPE);
 receiver_ip_address = ip;
 create_ip_layer(packet_memory, IP_LAYER_LENGTH + TCP_LAYER_LENGTH, TCP_PROTOCOL_TYPE);
 create_tcp_layer(packet_memory, source_port, sequence_number, acknowledge_number, (1 << 2)); //RST flag
 calculate_tcp_checksum(packet_memory, TCP_LAYER_LENGTH);
 
 (*network_send_packet)(packet_memory, (ETHERNET_LAYER_LENGTH + IP_LAYER_LENGTH + TCP_LAYER_LENGTH));
}
