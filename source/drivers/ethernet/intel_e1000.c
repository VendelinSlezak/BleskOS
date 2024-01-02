//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t read_ethernet_card_intel_e1000(dword_t port) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  return mmio_ind(ethernet_cards[ethernet_selected_card].base+port);
 }
 else {
  outd(ethernet_cards[ethernet_selected_card].base+0, port);
  return ind(ethernet_cards[ethernet_selected_card].base+4);
 }
}

void write_ethernet_card_intel_e1000(dword_t port, dword_t value) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  mmio_outd(ethernet_cards[ethernet_selected_card].base+port, value);
 }
 else {
  outd(ethernet_cards[ethernet_selected_card].base+0, port);
  outd(ethernet_cards[ethernet_selected_card].base+4, value);
 }
}

word_t read_eeprom_ethernet_card_intel_e1000(dword_t address, dword_t shift) {
 write_ethernet_card_intel_e1000(0x14, (0x1 | (address<<shift)));
 wait(10);
 return (read_ethernet_card_intel_e1000(0x14) >> 16);
}

word_t read_phy_ethernet_card_intel_e1000(dword_t address) {
 write_ethernet_card_intel_e1000(0x20, (address<<16 | 0b1<<21 | 0b10<<26));
 for(int i=0; i<100000; i++) {
  if((read_ethernet_card_intel_e1000(0x20) & 0x10000000)==0x10000000) {
   break;
  }
 }
 return (read_ethernet_card_intel_e1000(0x20) & 0xFFFF);
}

void initalize_ethernet_card_intel_e1000(void) {
 //stop transfers
 write_ethernet_card_intel_e1000(0x100, 0);
 write_ethernet_card_intel_e1000(0x400, 0);
 
 //reset card
 write_ethernet_card_intel_e1000(0x0, 0x04000000);
 wait(100);
 
 //read MAC address
 write_ethernet_card_intel_e1000(0x10, 0xE); //unlock EEPROM
 write_ethernet_card_intel_e1000(0x14, 0x01);
 wait(10);
 if((read_ethernet_card_intel_e1000(0x14) & 0x12)==0x0) { //no EEPROM
  write_ethernet_card_intel_e1000(0x10, 0x40); //lock EEPROM

  for(int i=0; i<6; i++) {
   mac_address[i]=(read_ethernet_card_intel_e1000(0x5400+i) & 0xFF);
  }
 }
 else if((read_ethernet_card_intel_e1000(0x14) & 0x2)==0x2) { //EEPROM with address shl 2
  mac_address[0] = (read_eeprom_ethernet_card_intel_e1000(0, 2) & 0xFF);
  mac_address[1] = (read_eeprom_ethernet_card_intel_e1000(0, 2) >> 8);
  mac_address[2] = (read_eeprom_ethernet_card_intel_e1000(1, 2) & 0xFF);
  mac_address[3] = (read_eeprom_ethernet_card_intel_e1000(1, 2) >> 8);
  mac_address[4] = (read_eeprom_ethernet_card_intel_e1000(2, 2) & 0xFF);
  mac_address[5] = (read_eeprom_ethernet_card_intel_e1000(2, 2) >> 8);
 }
 else { //EEPROM with address shl 8
  mac_address[0] = (read_eeprom_ethernet_card_intel_e1000(0, 8) & 0xFF);
  mac_address[1] = (read_eeprom_ethernet_card_intel_e1000(0, 8) >> 8);
  mac_address[2] = (read_eeprom_ethernet_card_intel_e1000(1, 8) & 0xFF);
  mac_address[3] = (read_eeprom_ethernet_card_intel_e1000(1, 8) >> 8);
  mac_address[4] = (read_eeprom_ethernet_card_intel_e1000(2, 8) & 0xFF);
  mac_address[5] = (read_eeprom_ethernet_card_intel_e1000(2, 8) >> 8);
 }
 
 //start card
 write_ethernet_card_intel_e1000(0x0, (read_ethernet_card_intel_e1000(0x0) | 0x40));

 //set receive mechanism
 ethernet_cards[ethernet_selected_card].receive_mem = calloc(256*2048);
 ethernet_cards[ethernet_selected_card].receive_buffer_mem = aligned_malloc(256*16, 0xFF);
 dword_t *rx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].receive_buffer_mem;
 for(int i=0, receive_mem=ethernet_cards[ethernet_selected_card].receive_mem; i<256; i++, receive_mem+=2048) {
  rx_pointer[0] = receive_mem;
  rx_pointer[1] = 0;
  rx_pointer[2] = 0;
  rx_pointer[3] = 0;
  rx_pointer += 4;
 }
 write_ethernet_card_intel_e1000(0x2800, ethernet_cards[ethernet_selected_card].receive_buffer_mem); //lower memory
 write_ethernet_card_intel_e1000(0x2804, 0); //upper memory
 write_ethernet_card_intel_e1000(0x2808, 256*16); //length
 write_ethernet_card_intel_e1000(0x2810, 0); //head
 write_ethernet_card_intel_e1000(0x2818, 255); //tail
 write_ethernet_card_intel_e1000(0x2820, 0); //delay timer
 ethernet_cards[ethernet_selected_card].receive_packet_pointer = 0;
 
 //set transmit mechanism
 ethernet_cards[ethernet_selected_card].transmit_mem = calloc(1600*256);
 ethernet_cards[ethernet_selected_card].transmit_buffer_mem = aligned_malloc(256*16, 0xFF);
 dword_t *tx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].transmit_buffer_mem;
 for(int i=0, transmit_mem=ethernet_cards[ethernet_selected_card].transmit_mem; i<256; i++, transmit_mem+=1600) {
  tx_pointer[0] = transmit_mem;
  tx_pointer[1] = 0;
  tx_pointer[2] = 0;
  tx_pointer[3] = 0x1; //status = no packet here
  tx_pointer += 4;
 }
 write_ethernet_card_intel_e1000(0x3800, ethernet_cards[ethernet_selected_card].transmit_buffer_mem); //lower memory
 write_ethernet_card_intel_e1000(0x3804, 0); //upper memory
 write_ethernet_card_intel_e1000(0x3808, 256*16); //length
 write_ethernet_card_intel_e1000(0x3810, 0); //head
 write_ethernet_card_intel_e1000(0x3818, 0); //tail
 write_ethernet_card_intel_e1000(0x3820, 0); //delay timer
 ethernet_cards[ethernet_selected_card].transmit_free_packet = 0;
 
 //enable receiving packets
 write_ethernet_card_intel_e1000(0x100, ((1 << 1) | (1 << 3) | (1 << 15)));
 
 //enable transmitting packets
 if((ethernet_cards[ethernet_selected_card].id>>16)==0x10F5) { //e1000e
  write_ethernet_card_intel_e1000(0x400, ((1 << 1) | (1 << 3) | (1 << 15) | (0x3F << 12) | (0x3 << 28)));
 }
 else { //e1000 and other cards
  write_ethernet_card_intel_e1000(0x400, ((1 << 1) | (1 << 3) | (4 << 15) | (0x40 << 12)));
 }

 //enable interrupts
 set_irq_handler(ethernet_cards[ethernet_selected_card].irq, (dword_t)ethernet_card_intel_e1000_irq);
 write_ethernet_card_intel_e1000(0xD0, 0x1F6DC);
 read_ethernet_card_intel_e1000(0xC0);
}

byte_t ethernet_card_intel_e1000_get_cable_status(void) {
 if((read_ethernet_card_intel_e1000(0x8) & 0x2)==0x2) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void ethernet_card_intel_e1000_send_packet(dword_t memory, dword_t length) {
 dword_t *tx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].transmit_buffer_mem;

 //copy packet
 clear_memory((ethernet_cards[ethernet_selected_card].transmit_mem+ethernet_cards[ethernet_selected_card].transmit_free_packet*1600), 1600);
 copy_memory(memory, (ethernet_cards[ethernet_selected_card].transmit_mem+ethernet_cards[ethernet_selected_card].transmit_free_packet*1600), length);
 
 //fill entry
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+2] = (length | 0x03000000);
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+3] = 0;
 ethernet_cards[ethernet_selected_card].transmit_free_packet++;
 ethernet_cards[ethernet_selected_card].transmit_free_packet &= 0xFF;
 
 //send packet
 write_ethernet_card_intel_e1000(0x3818, ethernet_cards[ethernet_selected_card].transmit_free_packet);
}

void ethernet_card_intel_e1000_irq(void) {
 dword_t irq_status = read_ethernet_card_intel_e1000(0xC0), processed_packets = 0, actual_packet = 0, packet_pointer;
 dword_t *rx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].receive_buffer_mem;
 
 //acknowledge interrupt
 write_ethernet_card_intel_e1000(0xC0, (irq_status & 0xFFFF));
 
 //receive
 if((irq_status & 0x80)==0x80) {
  processed_packets = 0;

  //process all received packets
  while(ethernet_cards[ethernet_selected_card].receive_packet_pointer!=read_ethernet_card_intel_e1000(0x2810)) {
   //update tail
   write_ethernet_card_intel_e1000(0x2818, ethernet_cards[ethernet_selected_card].receive_packet_pointer);

   //process one packet
   packet_pointer = rx_pointer[(ethernet_cards[ethernet_selected_card].receive_packet_pointer)*4];
   ethernet_cards[ethernet_selected_card].receive_packet_pointer++;
   ethernet_cards[ethernet_selected_card].receive_packet_pointer &= 0xFF;
   if(ethernet_cards[ethernet_selected_card].receive_packet_pointer==read_ethernet_card_intel_e1000(0x2810)) {
    network_process_packet(packet_pointer, 1);
   }
   else {
    network_process_packet(packet_pointer, 0);
   }
   
   //prevent from infinite while
   processed_packets++;
   if(processed_packets>2048) {
    return;
   }
  }
 }
 else if((irq_status & 0x4)==0x4) { //link status change
  read_ethernet_cable_status();
 }
}