//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void create_udp_layer(dword_t memory, word_t length_of_data, word_t source_port, word_t destination_port) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 word_t big_endain_value;
 
 big_endain_value = BIG_ENDIAN(source_port);
 packet16[0] = big_endain_value;
 big_endain_value = BIG_ENDIAN(destination_port);
 packet16[1] = big_endain_value;
 big_endain_value = BIG_ENDIAN(length_of_data);
 packet16[2] = big_endain_value;
 packet16[3] = 0; //checksum
}

void calculate_udp_checksum(dword_t memory, word_t length_of_data) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH+IP_LAYER_LENGTH);
 dword_t checksum;
 
 packet16[3] = 0; //checksum
 
 //checksum
 checksum = 0;
 for(int i=0; i<(length_of_data/2); i++) {
  checksum += packet16[i];
 }
 if((length_of_data & 0x1)==0x1) {
  checksum += (packet8[length_of_data-1]);
 }
 checksum += ((word_t)(length_of_data>>8)+(word_t)(length_of_data<<8)+0x1100);
 checksum += (ip_address & 0xFFFF);
 checksum += (ip_address >> 16);
 checksum += (receiver_ip_address & 0xFFFF);
 checksum += (receiver_ip_address >> 16);
 while((checksum>>16)>0) {
  checksum = ((checksum & 0xFFFF) + (checksum>>16));
 }
 checksum = ~checksum;
 
 packet16[3] = (word_t)checksum; //checksum
}