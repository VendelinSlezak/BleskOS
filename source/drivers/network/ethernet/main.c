//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ethernet_cards(void) {
 //initalize all founded ethernet cards
 for(dword_t i=0; i<number_of_ethernet_cards; i++) {
  //set default values
  ethernet_cards[i].get_cable_status = ethernet_void_get_cable_status;
  ethernet_cards[i].send_packet = ethernet_void_send_packet;
  ethernet_cards[i].process_irq = ethernet_void_process_irq;
  ethernet_cards[i].cable_status = ETHERNET_CARD_CABLE_DISCONNECTED;

  //if we have driver for this card, PCI scan will set its initalization method
  if(ethernet_cards[i].initalize!=0) {
   ethernet_cards[i].initalize(i);
  }
 }

 //get actual state of connection on cards
 ethernet_cards_update_cable_status();
}

byte_t ethernet_void_get_cable_status(dword_t number_of_card) {
 return STATUS_FALSE;
}

byte_t ethernet_void_send_packet(dword_t number_of_card, byte_t *memory_of_packet, dword_t length_of_packet) {
 return STATUS_ERROR;
}

void ethernet_void_process_irq(dword_t number_of_card) {
 return;
}

void ethernet_cards_update_cable_status(void) {
 for(dword_t i=0; i<number_of_ethernet_cards; i++) {
  //update status only on cards with driver
  if(ethernet_cards[i].get_cable_status!=0) {
   ethernet_cards[i].cable_status = ethernet_cards[i].get_cable_status(i);
  }
 }
}

void select_ethernet_card_as_internet_connection_device(dword_t number_of_card) {
 //set variables
 internet.connection_type = INTERNET_CONNECTION_ETHERNET;
 internet.connection_device_number = number_of_card;
 internet.send_packet = send_packet_to_internet_through_ethernet;

 //save mac address
 for(dword_t i=0; i<6; i++) {
  internet.our_mac[i] = ethernet_cards[number_of_card].mac_address[i];
 }
}

byte_t send_packet_to_internet_through_ethernet(byte_t *packet_memory, dword_t packet_size) {
 //this sends packet to ethernet card that is in structure internet as connection device
 return ethernet_cards[internet.connection_device_number].send_packet(internet.connection_device_number, packet_memory, packet_size);
}

void ethernet_cards_process_irq(void) {
 //we do not know from which card interrupt came from, so check status of all cards
 for(dword_t i=0; i<number_of_ethernet_cards; i++) {
  ethernet_cards[i].process_irq(i);
 }
}