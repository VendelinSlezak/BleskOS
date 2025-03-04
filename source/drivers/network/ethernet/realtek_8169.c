//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void ec_realtek_8169_outb(dword_t number_of_card, dword_t port, byte_t value) {
 if(ethernet_cards[number_of_card].bar_type==PCI_IO_BAR) {
  outb(ethernet_cards[number_of_card].base+port, value);
 }
 else if(ethernet_cards[number_of_card].bar_type==PCI_MMIO_BAR) {
  mmio_outb(ethernet_cards[number_of_card].base+port, value);
 }
}

void ec_realtek_8169_outw(dword_t number_of_card, dword_t port, word_t value) {
 if(ethernet_cards[number_of_card].bar_type==PCI_IO_BAR) {
  outw(ethernet_cards[number_of_card].base+port, value);
 }
 else if(ethernet_cards[number_of_card].bar_type==PCI_MMIO_BAR) {
  mmio_outw(ethernet_cards[number_of_card].base+port, value);
 }
}

void ec_realtek_8169_outd(dword_t number_of_card, dword_t port, dword_t value) {
 if(ethernet_cards[number_of_card].bar_type==PCI_IO_BAR) {
  outd(ethernet_cards[number_of_card].base+port, value);
 }
 else if(ethernet_cards[number_of_card].bar_type==PCI_MMIO_BAR) {
  mmio_outd(ethernet_cards[number_of_card].base+port, value);
 }
}

byte_t ec_realtek_8169_inb(dword_t number_of_card, dword_t port) {
 if(ethernet_cards[number_of_card].bar_type==PCI_IO_BAR) {
  return inb(ethernet_cards[number_of_card].base+port);
 }
 else if(ethernet_cards[number_of_card].bar_type==PCI_MMIO_BAR) {
  return mmio_inb(ethernet_cards[number_of_card].base+port);
 }

 return STATUS_ERROR;
}

word_t ec_realtek_8169_inw(dword_t number_of_card, dword_t port) {
 if(ethernet_cards[number_of_card].bar_type==PCI_IO_BAR) {
  return inw(ethernet_cards[number_of_card].base+port);
 }
 else if(ethernet_cards[number_of_card].bar_type==PCI_MMIO_BAR) {
  return mmio_inw(ethernet_cards[number_of_card].base+port);
 }

 return STATUS_ERROR;
}

dword_t ec_realtek_8169_ind(dword_t number_of_card, dword_t port) {
 if(ethernet_cards[number_of_card].bar_type==PCI_IO_BAR) {
  return ind(ethernet_cards[number_of_card].base+port);
 }
 else if(ethernet_cards[number_of_card].bar_type==PCI_MMIO_BAR) {
  return mmio_ind(ethernet_cards[number_of_card].base+port);
 }

 return STATUS_ERROR;
}

void ec_realtek_8169_initalize(dword_t number_of_card) {
 //log device ID of card
 log("\n\nRealtek 8169 driver\nDevice ID: ");
 log_hex_specific_size((ethernet_cards[number_of_card].id >> 16), 4);

 //set card methods
 ethernet_cards[number_of_card].get_cable_status = ec_realtek_8169_get_cable_status;
 ethernet_cards[number_of_card].send_packet = ec_realtek_8169_send_packet;
 ethernet_cards[number_of_card].process_irq = ec_realtek_8169_process_irq;

 //enable PCI multiple read/write
 ec_realtek_8169_outw(number_of_card, 0xE0, (1<<3));

 //reset card
 ec_realtek_8169_outb(number_of_card, 0x37, (1 << 4));
 volatile dword_t timeout = (time_of_system_running+50);
 while((ec_realtek_8169_inb(number_of_card, 0x37) & (1 << 4))==(1 << 4)) {
  asm("nop");
  if(time_of_system_running >= timeout) {
   log("\nERROR: Reset was not successful");
   return;
  }
 }

 //read MAC address
 for(dword_t i=0; i<6; i++) {
  ethernet_cards[number_of_card].mac_address[i] = ec_realtek_8169_inb(number_of_card, i);
 }

 //log MAC address
 log("\nMAC: ");
 for(dword_t i=0; i<6; i++) {
  log_hex_specific_size_with_space(ethernet_cards[number_of_card].mac_address[i], 2);
 }

 //unlock registers
 ec_realtek_8169_outb(number_of_card, 0x50, 0xC0);

 //enable interrupts
 set_irq_handler(ethernet_cards[number_of_card].irq, (dword_t)network_irq);
 ec_realtek_8169_outw(number_of_card, 0x3C, (1 << 0) | (1 << 4) | (1 << 5)); //packet received, rx buffer is full, link status change

 //allocate memory of receive descriptors
 struct ec_realtek_8169_rx_descriptor_t *rx_descriptors = (struct ec_realtek_8169_rx_descriptor_t *) aligned_calloc(sizeof(struct ec_realtek_8169_rx_descriptor_t)*EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS, 0xFF);
 byte_t *rx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS);

 //set variables of receive descriptors
 ethernet_cards[number_of_card].rx_descriptors_memory = (void *) rx_descriptors;
 ethernet_cards[number_of_card].rx_packets_memory = rx_packets_memory;
 ethernet_cards[number_of_card].rx_descriptor_pointer = 0;

 //initalize receive descriptors
 for(dword_t i=0; i<EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS; i++) {
  //set memory of packet
  rx_descriptors[i].lower_memory = (dword_t) rx_packets_memory;
  rx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

  //set max length of one packet
  rx_descriptors[i].packet_size = MAX_SIZE_OF_ONE_PACKET;

  //card owns this descriptor
  rx_descriptors[i].owned_by_card = 1;

  //all other fields are zero
 }
 rx_descriptors[EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS-1].end_of_ring = 1; //set this bit at last descriptor

 //initialize receiving mechanism
 ec_realtek_8169_outd(number_of_card, 0x44, (0x1F | (0x7 << 8) | (0x7 << 13))); //accept all packets, max DMS burst, no rx treshold
 ec_realtek_8169_outw(number_of_card, 0xDA, 0x1FFF); //max packet size
 ec_realtek_8169_outd(number_of_card, 0xE4, (dword_t)ethernet_cards[number_of_card].rx_descriptors_memory); //lower address of rx descriptors
 ec_realtek_8169_outd(number_of_card, 0xE8, 0); //upper address of rx descriptors

 //allocate memory of transmit descriptors
 struct ec_realtek_8169_tx_descriptor_t *tx_descriptors = (struct ec_realtek_8169_tx_descriptor_t *) aligned_calloc(sizeof(struct ec_realtek_8169_tx_descriptor_t)*EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS, 0xFF);
 byte_t *tx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS);

 //set variables of transmit descriptors
 ethernet_cards[number_of_card].tx_descriptors_memory = (void *) tx_descriptors;
 ethernet_cards[number_of_card].tx_packets_memory = tx_packets_memory;
 ethernet_cards[number_of_card].tx_descriptor_pointer = 0;

 //initalize transmit descriptors
 for(dword_t i=0; i<EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS; i++) {
  //set memory of packet
  tx_descriptors[i].lower_memory = (dword_t) tx_packets_memory;
  tx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

  //all other fields are zero
 }
 tx_descriptors[EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS-1].end_of_ring = 1; //set this bit at last descriptor

 //initialize transmitting mechanism
 ec_realtek_8169_outb(number_of_card, 0x37, 0x04); //enable transmitting packets, this must be set before configuring transmitting
 ec_realtek_8169_outb(number_of_card, 0xEC, 0x3F); //no early transmit allowed
 ec_realtek_8169_outd(number_of_card, 0x40, (0x7 << 8) | (0x3 << 24)); //unlimited dma burst, interframe gap
 ec_realtek_8169_outd(number_of_card, 0x20, (dword_t)ethernet_cards[number_of_card].tx_descriptors_memory); //lower address of normal priority tx descriptors
 ec_realtek_8169_outd(number_of_card, 0x24, 0); //upper address of normal priority tx descriptors
 ec_realtek_8169_outd(number_of_card, 0x28, 0); //lower address of high priority tx descriptors
 ec_realtek_8169_outd(number_of_card, 0x2C, 0); //upper address of high priority tx descriptors

 //enable transmitting and receiving packets
 ec_realtek_8169_outb(number_of_card, 0x37, 0x0C);

 //lock registers
 ec_realtek_8169_outb(number_of_card, 0x50, 0x00);
}

byte_t ec_realtek_8169_get_cable_status(dword_t number_of_card) {
 if((ec_realtek_8169_inb(number_of_card, 0x6C) & 0x2)==0x2) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

byte_t ec_realtek_8169_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
 //check if descriptors are not full
 struct ec_realtek_8169_tx_descriptor_t *tx_descriptor = (struct ec_realtek_8169_tx_descriptor_t *) (((dword_t)ethernet_cards[number_of_card].tx_descriptors_memory)+(sizeof(struct ec_realtek_8169_tx_descriptor_t)*ethernet_cards[number_of_card].tx_descriptor_pointer));
 if(tx_descriptor->owned_by_card == 1) {
  log("\nERROR: Ethernet tx buffer is full");
  return STATUS_ERROR;
 }

 //copy packet to transfer packet memory
 copy_memory((dword_t)packet_memory, (dword_t)&ethernet_cards[number_of_card].tx_packets_memory[MAX_SIZE_OF_ONE_PACKET*ethernet_cards[number_of_card].tx_descriptor_pointer], packet_size);
 
 //card automatically add padding to too small packets

 //fill tx descriptor
 tx_descriptor->packet_size = packet_size;
 tx_descriptor->first_segment_descriptor = 1;
 tx_descriptor->last_segment_descriptor = 1;
 tx_descriptor->owned_by_card = 1;

 //tell card to send packet from normal priority descriptors
 ec_realtek_8169_outb(number_of_card, 0x38, (1 << 6));

 //TODO: why is packet sent, but bit "TX descriptor unavailable" is also always set in interrupt?
 
 //move free descriptor number
 ethernet_cards[number_of_card].tx_descriptor_pointer++;
 if(ethernet_cards[number_of_card].tx_descriptor_pointer >= EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS) {
  ethernet_cards[number_of_card].tx_descriptor_pointer = 0;
 }

 return STATUS_GOOD;
}

void ec_realtek_8169_process_irq(dword_t number_of_card) {
 //read irq status to know what caused interrupt
 word_t irq_status = ec_realtek_8169_inw(number_of_card, 0x3E);
 if(irq_status == 0x00000000) {
  return;
 }

 //packet received
 if((irq_status & 0x11)!=0x00) {
  //for cycle will prevent infinite cycle if there is some error
  for(dword_t i=0; i<2048; i++) {
   //set pointer to rx descriptor
   struct ec_realtek_8169_rx_descriptor_t *rx_descriptor = (struct ec_realtek_8169_rx_descriptor_t *) (((dword_t)ethernet_cards[number_of_card].rx_descriptors_memory)+(sizeof(struct ec_realtek_8169_rx_descriptor_t)*ethernet_cards[number_of_card].rx_descriptor_pointer));
   
   //check if we processed everything
   if(rx_descriptor->owned_by_card == 1) {
    break;
   }

   //process packet
   process_packet((byte_t *)rx_descriptor->lower_memory, rx_descriptor->packet_size);

   //reset descriptor
   rx_descriptor->packet_size = MAX_SIZE_OF_ONE_PACKET;
   rx_descriptor->vlan = 0;
   rx_descriptor->owned_by_card = 1;

   //move to next descriptor
   ethernet_cards[number_of_card].rx_descriptor_pointer++;
   if(ethernet_cards[number_of_card].rx_descriptor_pointer >= EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS) {
    ethernet_cards[number_of_card].rx_descriptor_pointer = 0;
   }
  }
 }

 //link status change
 if((irq_status & 0x20)==0x20) {
  ethernet_cards_update_cable_status();
 }
 
 //acknowledge interrupt
 ec_realtek_8169_outw(number_of_card, 0x3E, irq_status);
}