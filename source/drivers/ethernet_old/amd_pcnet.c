//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ethernet_card_amd_pcnet(void) {
 //reset card
 ind(ethernet_cards[ethernet_selected_card].base+0x18);
 inw(ethernet_cards[ethernet_selected_card].base+0x14);
 
 //set 32 bit mode
 outd(ethernet_cards[ethernet_selected_card].base+0x10, 0);
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 58);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, ((ind(ethernet_cards[ethernet_selected_card].base+0x10) & 0xFF00) | 0x0002));
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 58);
 outd(ethernet_cards[ethernet_selected_card].base+0x1C, 0x0102);
 
 //read MAC address
 mac_address[0]=((ind(ethernet_cards[ethernet_selected_card].base+0x0)>>0) & 0xFF);
 mac_address[1]=((ind(ethernet_cards[ethernet_selected_card].base+0x0)>>8) & 0xFF);
 mac_address[2]=((ind(ethernet_cards[ethernet_selected_card].base+0x0)>>16) & 0xFF);
 mac_address[3]=((ind(ethernet_cards[ethernet_selected_card].base+0x0)>>24) & 0xFF);
 mac_address[4]=((ind(ethernet_cards[ethernet_selected_card].base+0x4)>>0) & 0xFF);
 mac_address[5]=((ind(ethernet_cards[ethernet_selected_card].base+0x4)>>8) & 0xFF);
 
 //enable ASEL
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 0x2);
 outd(ethernet_cards[ethernet_selected_card].base+0x1C, (ind(ethernet_cards[ethernet_selected_card].base+0x1C) | 0x2));
 
 //enable interrupts
 set_irq_handler(ethernet_cards[ethernet_selected_card].irq, (dword_t)ethernet_card_amd_pcnet_irq);
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 3);
 ind(ethernet_cards[ethernet_selected_card].base+0x10);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, 0x0100); //interrupts for receiving packets
 
 //create initalization block
 dword_t amd_pcnet_init_block_mem = malloc(28);
 mmio_outw(amd_pcnet_init_block_mem+0, 0x8000); //promiscuous mode
 mmio_outb(amd_pcnet_init_block_mem+2, (8 << 4)); //256 receive buffers
 mmio_outb(amd_pcnet_init_block_mem+3, (8 << 4)); //256 transmit buffers
 mmio_outb(amd_pcnet_init_block_mem+4, mac_address[0]); //mac address 0
 mmio_outb(amd_pcnet_init_block_mem+5, mac_address[1]); //mac address 1
 mmio_outb(amd_pcnet_init_block_mem+6, mac_address[2]); //mac address 2
 mmio_outb(amd_pcnet_init_block_mem+7, mac_address[3]); //mac address 3
 mmio_outb(amd_pcnet_init_block_mem+8, mac_address[4]); //mac address 4
 mmio_outb(amd_pcnet_init_block_mem+9, mac_address[5]); //mac address 5
 mmio_outw(amd_pcnet_init_block_mem+10, 0);
 mmio_outd(amd_pcnet_init_block_mem+12, 0);
 mmio_outd(amd_pcnet_init_block_mem+16, 0);
 
 //initalize receive buffers
 ethernet_cards[ethernet_selected_card].receive_mem = calloc(256*1520);
 ethernet_cards[ethernet_selected_card].receive_buffer_mem = malloc(16*256);
 ethernet_cards[ethernet_selected_card].receive_packet_pointer = 0;
 dword_t *rx_pointer = (dword_t *) (ethernet_cards[ethernet_selected_card].receive_buffer_mem);
 for(int i=0, receive_mem = ethernet_cards[ethernet_selected_card].receive_mem; i<256; i++, receive_mem += 1520) {
  rx_pointer[0] = receive_mem;
  rx_pointer[1] = (0x80000000 | (0xF << 12) | (1520));
  rx_pointer[2] = 0;
  rx_pointer[3] = 0;
  rx_pointer += 4;
 }
 mmio_outd(amd_pcnet_init_block_mem+20, ethernet_cards[ethernet_selected_card].receive_buffer_mem); //receive buffers memory

 //initalize transmit buffers
 ethernet_cards[ethernet_selected_card].transmit_mem = calloc(1600*256);
 ethernet_cards[ethernet_selected_card].transmit_buffer_mem = calloc(256*16);
 ethernet_cards[ethernet_selected_card].transmit_free_packet = 0;
 dword_t *tx_pointer = (dword_t *) (ethernet_cards[ethernet_selected_card].transmit_buffer_mem);
 for(int i=0, transmit_mem = ethernet_cards[ethernet_selected_card].transmit_mem; i<256; i++, transmit_mem += 1600) {
  tx_pointer[0] = transmit_mem;
  tx_pointer += 4;
 }
 mmio_outd(amd_pcnet_init_block_mem+24, ethernet_cards[ethernet_selected_card].transmit_buffer_mem); //transmit buffers memory
 
 //send initalization block to card
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 1);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, (amd_pcnet_init_block_mem & 0xFFFF));
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 2);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, (amd_pcnet_init_block_mem >> 16));
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 0);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, 0x41);
 wait(50);

 //start card
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 0);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, 0x142);

 //enable monitoring of link status
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 4);
 outd(ethernet_cards[ethernet_selected_card].base+0x1C, (ind(ethernet_cards[ethernet_selected_card].base+0x1C) | (1<<6)));
 ethernet_timer_monitoring_of_line_status = STATUS_TRUE;
}

byte_t ethernet_card_amd_pcnet_get_cable_status(void) {
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 4);
 if((ind(ethernet_cards[ethernet_selected_card].base+0x1C) & (1<<15))==(1<<15)) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void ethernet_card_amd_pcnet_send_packet(dword_t memory, dword_t length) {
 dword_t *tx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].transmit_buffer_mem;
 
 //copy packet
 clear_memory((ethernet_cards[ethernet_selected_card].transmit_mem+ethernet_cards[ethernet_selected_card].transmit_free_packet*1600), 1600);
 copy_memory(memory, (ethernet_cards[ethernet_selected_card].transmit_mem+ethernet_cards[ethernet_selected_card].transmit_free_packet*1600), length);

 //fill entry
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+2] = 0;
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+3] = 0;
 tx_pointer[ethernet_cards[ethernet_selected_card].transmit_free_packet*4+1] = (0x8300F000 | (0x1000 - length)); //filling this part sends packet immediately
 ethernet_cards[ethernet_selected_card].transmit_free_packet++;
 ethernet_cards[ethernet_selected_card].transmit_free_packet &= 0xFF;
}

void ethernet_card_amd_pcnet_irq(void) {
 dword_t *rx_pointer = (dword_t *) ethernet_cards[ethernet_selected_card].receive_buffer_mem;
 dword_t packet_pointer;
 
 //some packets were received
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 0);
 dword_t irq_status = ind(ethernet_cards[ethernet_selected_card].base+0x10);
 if((irq_status & 0x400)==0x400) {
  //process all received packets
  while((rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4+1] & 0x80000000)==0x00000000) {
   packet_pointer = rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4];
   rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4+1] = (0x80000000 | (0xF << 12) | (1520));
   ethernet_cards[ethernet_selected_card].receive_packet_pointer++;
   ethernet_cards[ethernet_selected_card].receive_packet_pointer &= 0xFF;
   
   if((rx_pointer[ethernet_cards[ethernet_selected_card].receive_packet_pointer*4+1] & 0x80000000)==0x80000000) {
    network_process_packet(packet_pointer, 1); //last packet
   }
   else {
    network_process_packet(packet_pointer, 0);
   }
  }
 }
 
 //acknowledge interrupt
 outd(ethernet_cards[ethernet_selected_card].base+0x14, 0);
 outd(ethernet_cards[ethernet_selected_card].base+0x10, irq_status);
}