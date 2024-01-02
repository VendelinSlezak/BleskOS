//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void ethernet_card_realtek_8169_outb(dword_t port, byte_t value) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_IO_BAR) {
  outb(ethernet_cards[ethernet_selected_card].base+port, value);
 }
 else if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  mmio_outb(ethernet_cards[ethernet_selected_card].base+port, value);
 }
}

void ethernet_card_realtek_8169_outw(dword_t port, word_t value) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_IO_BAR) {
  outw(ethernet_cards[ethernet_selected_card].base+port, value);
 }
 else if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  mmio_outw(ethernet_cards[ethernet_selected_card].base+port, value);
 }
}

void ethernet_card_realtek_8169_outd(dword_t port, dword_t value) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_IO_BAR) {
  outd(ethernet_cards[ethernet_selected_card].base+port, value);
 }
 else if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  mmio_outd(ethernet_cards[ethernet_selected_card].base+port, value);
 }
}

byte_t ethernet_card_realtek_8169_inb(dword_t port) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_IO_BAR) {
  return inb(ethernet_cards[ethernet_selected_card].base+port);
 }
 else if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  return mmio_inb(ethernet_cards[ethernet_selected_card].base+port);
 }
}

word_t ethernet_card_realtek_8169_inw(dword_t port) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_IO_BAR) {
  return inw(ethernet_cards[ethernet_selected_card].base+port);
 }
 else if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  return mmio_inw(ethernet_cards[ethernet_selected_card].base+port);
 }
}

dword_t ethernet_card_realtek_8169_ind(dword_t port) {
 if(ethernet_cards[ethernet_selected_card].bar_type==PCI_IO_BAR) {
  return ind(ethernet_cards[ethernet_selected_card].base+port);
 }
 else if(ethernet_cards[ethernet_selected_card].bar_type==PCI_MMIO_BAR) {
  return mmio_ind(ethernet_cards[ethernet_selected_card].base+port);
 }
}

void initalize_ethernet_card_realtek_8169(void) {
 //enable PCI multiple read/write
 ethernet_card_realtek_8169_outw(0xE0, (1<<3));

 //reset card
 ethernet_card_realtek_8169_outb(0x37, 0x10);
 wait(50);
 
 //read MAC address
 mac_address[0] = ethernet_card_realtek_8169_inb(0x00);
 mac_address[1] = ethernet_card_realtek_8169_inb(0x01);
 mac_address[2] = ethernet_card_realtek_8169_inb(0x02);
 mac_address[3] = ethernet_card_realtek_8169_inb(0x03);
 mac_address[4] = ethernet_card_realtek_8169_inb(0x04);
 mac_address[5] = ethernet_card_realtek_8169_inb(0x05);
 
 //unlock registers
 ethernet_card_realtek_8169_outb(0x50, 0xC0);
 
 //enable interrupts
 set_irq_handler(ethernet_cards[ethernet_selected_card].irq, (dword_t)ethernet_card_realtek_8169_irq);
 ethernet_card_realtek_8169_outw(0x3C, 0xFFFF);
 
 //initalize receiving descriptors
 ethernet_cards[ethernet_selected_card].receive_mem = calloc(256*2048);
 ethernet_cards[ethernet_selected_card].receive_buffer_mem = aligned_malloc(256*16, 0xFF);
 dword_t *rx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].receive_buffer_mem;
 for(int i=0, receive_mem=ethernet_cards[ethernet_selected_card].receive_mem; i<256; i++, receive_mem+=2048) {
  rx_pointer[0] = 0x80003FFF; //card own this descriptor and maximal length of packet
  rx_pointer[1] = 0;
  rx_pointer[2] = receive_mem;
  rx_pointer[3] = 0;
  rx_pointer += 4;
 }
 rx_pointer -= 4;
 rx_pointer[0] = 0xC0003FFF; //set last descriptor bit
 ethernet_cards[ethernet_selected_card].receive_packet_pointer = 0;
 
 //initalize receiving mechanism
 ethernet_card_realtek_8169_outd(0x44, (0x1F | (0x7 << 8) | (0x7 << 13)));
 ethernet_card_realtek_8169_outw(0xDA, 0x1FFF); //max packet size
 ethernet_card_realtek_8169_outd(0xE4, ethernet_cards[ethernet_selected_card].receive_buffer_mem); //lower bytes of rx descriptor
 ethernet_card_realtek_8169_outd(0xE8, 0); //upper bytes of rx descriptor

 //initalize transmitting descriptors
 ethernet_cards[ethernet_selected_card].transmit_mem = calloc(1600*256);
 ethernet_cards[ethernet_selected_card].transmit_buffer_mem = aligned_malloc(256*16, 0xFF);
 ethernet_cards[ethernet_selected_card].transmit_free_packet = 0;
 dword_t *tx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].transmit_buffer_mem;
 for(int i=0, transmit_mem=ethernet_cards[ethernet_selected_card].transmit_mem; i<256; i++, transmit_mem+=1600) {
  tx_pointer[0] = 0;
  tx_pointer[1] = 0;
  tx_pointer[2] = transmit_mem;
  tx_pointer[3] = 0;
  tx_pointer += 4;
 }

 //initalize transmitting mechanism
 ethernet_card_realtek_8169_outb(0x37, 0x04); //enable transmitting packets, must be set before configuring transmitting
 ethernet_card_realtek_8169_outb(0xEC, 0x3F); //no early transmit
 ethernet_card_realtek_8169_outd(0x40, ((0x3 << 24) | (0x7 << 8)));
 ethernet_card_realtek_8169_outd(0x20, ethernet_cards[ethernet_selected_card].transmit_buffer_mem); //lower bytes of normal priority tx decriptors
 ethernet_card_realtek_8169_outd(0x24, 0); //upper bytes of normal priority tx decriptors
 ethernet_card_realtek_8169_outd(0x28, 0); //lower bytes of high priority tx decriptors
 ethernet_card_realtek_8169_outd(0x2C, 0); //upper bytes of high priority tx decriptors

 //start receiving and transmitting
 ethernet_card_realtek_8169_outb(0x37, 0x0C);
 
 //lock registers
 ethernet_card_realtek_8169_outb(0x50, 0x00);
}

byte_t ethernet_card_realtek_8169_get_cable_status(void) {
 if((ethernet_card_realtek_8169_inb(0x6C) & 0x2)==0x2) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void ethernet_card_realtek_8169_send_packet(dword_t memory, dword_t length) {
 dword_t *tx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].transmit_buffer_mem;

 //copy packet
 clear_memory((ethernet_cards[ethernet_selected_card].transmit_mem+ethernet_cards[ethernet_selected_card].transmit_free_packet*1600), 1600);
 copy_memory(memory, (ethernet_cards[ethernet_selected_card].transmit_mem+ethernet_cards[ethernet_selected_card].transmit_free_packet*1600), length);

 //set command
 dword_t command = (length | 0xB0000000);
 if(ethernet_cards[ethernet_selected_card].transmit_free_packet==255) {
  command |= 0x40000000; //EOR
 }

 //fill entry
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+1] = 0;
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+3] = 0;
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+0] = command;
 ethernet_cards[ethernet_selected_card].transmit_free_packet++;
 ethernet_cards[ethernet_selected_card].transmit_free_packet &= 0xFF;
 
 //send packet
 ethernet_card_realtek_8169_outb(0x38, 0x40);
}

void ethernet_card_realtek_8169_irq(void) {
 dword_t *rx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].receive_buffer_mem;
 word_t irq_cause = ethernet_card_realtek_8169_inw(0x3E);
 dword_t packet_pointer;

 //receive packets
 if((irq_cause & 0x1)==0x1) {
  //process all received packets
  while((rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4] & 0x80000000)==0x00000000) {
   packet_pointer = rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4+2];
   rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4] &= 0x40000000; //last descriptor bit is untouched
   rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4] |= 0x80003FFF; //card own this descriptor
  
   ethernet_cards[ethernet_selected_card].receive_packet_pointer++;
   ethernet_cards[ethernet_selected_card].receive_packet_pointer &= 0xFF;
   
   if((rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4] & 0x80000000)==0x80000000) {
    network_process_packet(packet_pointer, 1); //last packet
   }
   else {
    network_process_packet(packet_pointer, 0);
   }
  }
 }
 else if((irq_cause & 0x20)==0x20) {
  //connection change
  read_ethernet_cable_status();
 }
 
 //acknowledge interrupt
 ethernet_card_realtek_8169_outw(0x3E, irq_cause);
}