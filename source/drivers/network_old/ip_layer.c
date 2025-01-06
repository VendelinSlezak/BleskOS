//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void create_ip_layer(dword_t memory, dword_t length_of_data, byte_t protocol_type) {
 byte_t *packet8 = (byte_t *) (memory+ETHERNET_LAYER_LENGTH);
 word_t *packet16 = (word_t *) (memory+ETHERNET_LAYER_LENGTH);
 dword_t *packet32 = (dword_t *) (memory+ETHERNET_LAYER_LENGTH);
 dword_t checksum;
 
 packet16[0] = 0x0045; //header
 packet16[1] = BIG_ENDIAN_WORD(length_of_data);
 packet32[1] = 0;
 packet8[8] = 0x80; //time to live
 packet8[9] = protocol_type;
 packet16[5] = 0; //checksum
 packet32[3] = ip_address;
 packet32[4] = receiver_ip_address;
 
 //checksum
 checksum = 0;
 for(int i=0; i<10; i++) {
  checksum += packet16[i];
 }
 while((checksum>>16)>0) {
  checksum = ((checksum & 0xFFFF) + (checksum>>16));
 }
 checksum = ~checksum;
 
 packet16[5] = (word_t)checksum; //checksum
}
