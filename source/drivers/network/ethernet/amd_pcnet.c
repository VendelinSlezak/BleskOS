//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t ec_amd_pcnet_read_csr(dword_t number_of_card, dword_t port) {
 outd(ethernet_cards[number_of_card].base+0x14, port);
 return ind(ethernet_cards[number_of_card].base+0x10);
}

void ec_amd_pcnet_write_csr(dword_t number_of_card, dword_t port, dword_t value) {
 outd(ethernet_cards[number_of_card].base+0x14, port);
 outd(ethernet_cards[number_of_card].base+0x10, value);
}

dword_t ec_amd_pcnet_read_bcr(dword_t number_of_card, dword_t port) {
 outd(ethernet_cards[number_of_card].base+0x14, port);
 return ind(ethernet_cards[number_of_card].base+0x1C);
}

void ec_amd_pcnet_write_bcr(dword_t number_of_card, dword_t port, dword_t value) {
 outd(ethernet_cards[number_of_card].base+0x14, port);
 outd(ethernet_cards[number_of_card].base+0x1C, value);
}

void ec_amd_pcnet_initalize(dword_t number_of_card) {
 //log device ID of card
 log("\n\nAmd PCNET driver\nDevice ID: ");
 log_hex_specific_size((ethernet_cards[number_of_card].id >> 16), 4);

 //set card methods
 ethernet_cards[number_of_card].get_cable_status = ec_amd_pcnet_get_cable_status;
 ethernet_cards[number_of_card].send_packet = ec_amd_pcnet_send_packet;
 ethernet_cards[number_of_card].process_irq = ec_amd_pcnet_process_irq;

 //card can be in 16 or 32 bit mode, so we need to reset it to put to 16 bit mode and then move to 32 bit mode
 //reset card
 ind(ethernet_cards[number_of_card].base+0x18); //this will reset card if it is in 32 bit mode and move it to 16 bit mode
 inw(ethernet_cards[number_of_card].base+0x14); //this will reset card in 16 bit mode

 //set 32 bit mode
 outd(ethernet_cards[number_of_card].base+0x10, 0);

 //set SWSTYLE to enable 4 GB of memory
 ec_amd_pcnet_write_csr(number_of_card, 58, ((ec_amd_pcnet_read_csr(number_of_card, 58) & 0xFF00) | 0x0002));
 // ec_amd_pcnet_write_bcr(number_of_card, 58, 0x0102); //???

 //read MAC address
 dword_t mac_value = ind(ethernet_cards[number_of_card].base+0x0);
 for(dword_t i=0; i<4; i++) {
  ethernet_cards[number_of_card].mac_address[i] = (mac_value >> (i*8));
 }
 mac_value = ind(ethernet_cards[number_of_card].base+0x4);
 for(dword_t i=0; i<2; i++) {
  ethernet_cards[number_of_card].mac_address[4+i] = (mac_value >> (i*8));
 }

 //log MAC address
 log("\nMAC: ");
 for(dword_t i=0; i<6; i++) {
  log_hex_specific_size_with_space(ethernet_cards[number_of_card].mac_address[i], 2);
 }
 
 //enable ASEL to automatically switch between ethernet interfaces
 ec_amd_pcnet_write_bcr(number_of_card, 2, (ec_amd_pcnet_read_bcr(number_of_card, 2) | 0x0002));
 
 //enable interrupts
 set_irq_handler(ethernet_cards[number_of_card].irq, (dword_t)network_irq);
 ec_amd_pcnet_write_csr(number_of_card, 3, ~((1 << 10) | (1 << 9))); //set interrupts mask for everything except for receiving and transmitting packets

 //allocate memory of receive descriptors
 struct ec_amd_pcnet_rx_descriptor_t *rx_descriptors = (struct ec_amd_pcnet_rx_descriptor_t *) calloc(sizeof(struct ec_amd_pcnet_rx_descriptor_t)*EC_AMD_PCNET_NUMBER_OF_RX_DESCRIPTORS);
 byte_t *rx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_AMD_PCNET_NUMBER_OF_RX_DESCRIPTORS);

 //set variables of receive descriptors
 ethernet_cards[number_of_card].rx_descriptors_memory = (void *) rx_descriptors;
 ethernet_cards[number_of_card].rx_packets_memory = rx_packets_memory;
 ethernet_cards[number_of_card].rx_descriptor_pointer = 0;

 //initalize receive descriptors
 for(dword_t i=0; i<EC_AMD_PCNET_NUMBER_OF_RX_DESCRIPTORS; i++) {
  //set memory of packet
  rx_descriptors[i].memory = (dword_t) rx_packets_memory;
  rx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

  //set max length of one packet
  rx_descriptors[i].buffer_byte_count = MAX_SIZE_OF_ONE_PACKET;

  //set signature
  rx_descriptors[i].signature1 = 0b1111;

  //card owns this descriptor
  rx_descriptors[i].own_by_card = 1;

  //all other fields are zero
 }

 //allocate memory of transmit descriptors
 struct ec_amd_pcnet_tx_descriptor_t *tx_descriptors = (struct ec_amd_pcnet_tx_descriptor_t *) calloc(sizeof(struct ec_amd_pcnet_tx_descriptor_t)*EC_AMD_PCNET_NUMBER_OF_TX_DESCRIPTORS);
 byte_t *tx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_AMD_PCNET_NUMBER_OF_TX_DESCRIPTORS);

 //set variables of transmit descriptors
 ethernet_cards[number_of_card].tx_descriptors_memory = (void *) tx_descriptors;
 ethernet_cards[number_of_card].tx_packets_memory = tx_packets_memory;
 ethernet_cards[number_of_card].tx_descriptor_pointer = 0;

 //initalize transmit descriptors
 for(dword_t i=0; i<EC_AMD_PCNET_NUMBER_OF_TX_DESCRIPTORS; i++) {
  //set memory of packet
  tx_descriptors[i].memory = (dword_t) tx_packets_memory;
  tx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

  //all other fields are zero
 }
 
 //create initalization block
 struct ec_amd_pcnet_initalization_block_t *initalization_block = (struct ec_amd_pcnet_initalization_block_t *) aligned_calloc(sizeof(struct ec_amd_pcnet_initalization_block_t), 0x7);
 initalization_block->mode = 0x8000; //promiscuous mode
 initalization_block->number_of_rx_descriptors = 0b1000; //256 descriptors
 initalization_block->number_of_tx_descriptors = 0b1000; //256 descriptors
 for(dword_t i=0; i<6; i++) {
  initalization_block->mac_address[i] = ethernet_cards[number_of_card].mac_address[i];
 }
 initalization_block->memory_of_rx_descriptors = (dword_t) ethernet_cards[number_of_card].rx_descriptors_memory;
 initalization_block->memory_of_tx_descriptors = (dword_t) ethernet_cards[number_of_card].tx_descriptors_memory;
 
 //send initalization block to card
 ec_amd_pcnet_write_csr(number_of_card, 1, ((dword_t)initalization_block & 0xFFFF));
 ec_amd_pcnet_write_csr(number_of_card, 2, ((dword_t)initalization_block >> 16));
 ec_amd_pcnet_write_csr(number_of_card, 0, (1 << 0));
 dword_t timeout = (time_of_system_running+50); //wait max 50ms
 while((ec_amd_pcnet_read_csr(number_of_card, 0) & (1 << 8))!=(1 << 8)) {
  asm("nop");
  if(time_of_system_running >= timeout) {
   log("\nERROR: card did not received initalization block");
   return;
  }
 }
 log("\nCard successfully received initalization block");

 //enable that LED status will reflect status of link
 //TODO: is there any better way to get status of link?
 ec_amd_pcnet_write_bcr(number_of_card, 4, (1 << 6));

 //start card
 ec_amd_pcnet_write_csr(number_of_card, 0, (1 << 1) | (1 << 6) | (1 << 10)); //start card, enable interrupts, clear init_done bit
}

byte_t ec_amd_pcnet_get_cable_status(dword_t number_of_card) {
 if((ec_amd_pcnet_read_bcr(number_of_card, 4) & (1 << 15))==(1 << 15)) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

byte_t ec_amd_pcnet_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
 //check if descriptors are not full
 struct ec_amd_pcnet_tx_descriptor_t *tx_descriptor = (struct ec_amd_pcnet_tx_descriptor_t *) (((dword_t)ethernet_cards[number_of_card].tx_descriptors_memory)+(sizeof(struct ec_amd_pcnet_tx_descriptor_t)*ethernet_cards[number_of_card].tx_descriptor_pointer));
 if(tx_descriptor->own_by_card == 1) {
  log("\nERROR: Ethernet tx buffer is full");
  return STATUS_ERROR;
 }

 //copy packet to transfer packet memory
 copy_memory((dword_t)packet_memory, (dword_t)&ethernet_cards[number_of_card].tx_packets_memory[MAX_SIZE_OF_ONE_PACKET*ethernet_cards[number_of_card].tx_descriptor_pointer], packet_size);
 
 //add padding to too small packets
 if(packet_size<60) {
  clear_memory((dword_t)&ethernet_cards[number_of_card].tx_packets_memory[MAX_SIZE_OF_ONE_PACKET*ethernet_cards[number_of_card].tx_descriptor_pointer]+packet_size, 60-packet_size);
  packet_size = 60;
 }

 //fill tx descriptor
 tx_descriptor->buffer_byte_count = (0x1000 - packet_size);
 tx_descriptor->signature = 0b1111;
 tx_descriptor->start_of_packet = 1;
 tx_descriptor->end_of_packet = 1;
 tx_descriptor->own_by_card = 1; //this will immediately send packet
 
 //move free descriptor number
 ethernet_cards[number_of_card].tx_descriptor_pointer++;
 if(ethernet_cards[number_of_card].tx_descriptor_pointer >= EC_AMD_PCNET_NUMBER_OF_TX_DESCRIPTORS) {
  ethernet_cards[number_of_card].tx_descriptor_pointer = 0;
 }

 return STATUS_GOOD;
}

void ec_amd_pcnet_process_irq(dword_t number_of_card) {
 //read irq status to know what caused interrupt
 dword_t irq_status = ec_amd_pcnet_read_csr(number_of_card, 0);
 if(irq_status == 0x00000000) {
  return;
 }

 //packet was received
 if((irq_status & (1 << 10))==(1 << 10)) {
  //for cycle will prevent infinite cycle if there is some error
  for(dword_t i=0; i<2048; i++) {
   //set pointer to rx descriptor
   struct ec_amd_pcnet_rx_descriptor_t *rx_descriptor = (struct ec_amd_pcnet_rx_descriptor_t *) (((dword_t)ethernet_cards[number_of_card].rx_descriptors_memory)+(sizeof(struct ec_amd_pcnet_rx_descriptor_t)*ethernet_cards[number_of_card].rx_descriptor_pointer));
   
   //check if we processed everything
   if(rx_descriptor->own_by_card == 1) {
    break;
   }

   //process packet
   if(rx_descriptor->error==0) {
    process_packet((byte_t *)rx_descriptor->memory, rx_descriptor->message_byte_count);
   }

   //reset descriptor
   rx_descriptor->buffer_byte_count = MAX_SIZE_OF_ONE_PACKET;
   rx_descriptor->signature1 = 0b1111;
   rx_descriptor->own_by_card = 1;

   //move to next descriptor
   ethernet_cards[number_of_card].rx_descriptor_pointer++;
   if(ethernet_cards[number_of_card].rx_descriptor_pointer >= EC_AMD_PCNET_NUMBER_OF_RX_DESCRIPTORS) {
    ethernet_cards[number_of_card].rx_descriptor_pointer = 0;
   }
  }
 }

 //acknowledge interrupt
 ec_amd_pcnet_write_csr(number_of_card, 0, irq_status);
}