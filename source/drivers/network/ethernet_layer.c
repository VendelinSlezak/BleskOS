//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void create_ethernet_layer(dword_t memory, word_t type) {
 byte_t *packet8 = (byte_t *) memory;
 word_t *packet16 = (word_t *) memory;
 
 packet8[0] = receiver_mac_address[0];
 packet8[1] = receiver_mac_address[1];
 packet8[2] = receiver_mac_address[2];
 packet8[3] = receiver_mac_address[3];
 packet8[4] = receiver_mac_address[4];
 packet8[5] = receiver_mac_address[5];
 
 packet8[6] = mac_address[0];
 packet8[7] = mac_address[1];
 packet8[8] = mac_address[2];
 packet8[9] = mac_address[3];
 packet8[10] = mac_address[4];
 packet8[11] = mac_address[5];
 
 packet16[6] = type;
}

void set_broadcast_receiver_mac(void) {
 for(int i=0; i<6; i++) {
  receiver_mac_address[i]=0xFF;
 }
}

void set_receiver_mac_as_router_mac(void) {
 for(int i=0; i<6; i++) {
  receiver_mac_address[i]=router_mac_address[i];
 }
}