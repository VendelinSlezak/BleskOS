//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ethernet_card_realtek_8139(void) {
 //turn card on
 outb(ethernet_cards[ethernet_selected_card].base+0x52, 0x00);
 
 //reset card
 outb(ethernet_cards[ethernet_selected_card].base+0x37, 0x10);
 wait(10);
 outb(ethernet_cards[ethernet_selected_card].base+0x37, 0x00);
 
 //read MAC address
 mac_address[0] = inb(ethernet_cards[ethernet_selected_card].base+0x00);
 mac_address[1] = inb(ethernet_cards[ethernet_selected_card].base+0x01);
 mac_address[2] = inb(ethernet_cards[ethernet_selected_card].base+0x02);
 mac_address[3] = inb(ethernet_cards[ethernet_selected_card].base+0x03);
 mac_address[4] = inb(ethernet_cards[ethernet_selected_card].base+0x04);
 mac_address[5] = inb(ethernet_cards[ethernet_selected_card].base+0x05);

 //enable interrupts
 set_irq_handler(ethernet_cards[ethernet_selected_card].irq, (dword_t)ethernet_card_realtek_8139_irq);
 outw(ethernet_cards[ethernet_selected_card].base+0x3C, 0xFFFF);

 //enable receiving and transmitting - we *must* enable them before configuring them, otherwise some cards will not work correctly
 outb(ethernet_cards[ethernet_selected_card].base+0x37, 0x0C);
 
 //initalize receiving mechanism
 ethernet_cards[ethernet_selected_card].receive_mem = calloc(1024*8+16+1500); //8 KB + 1,5 KB
 ethernet_cards[ethernet_selected_card].receive_buffer_mem = calloc(1600);
 outd(ethernet_cards[ethernet_selected_card].base+0x30, ethernet_cards[ethernet_selected_card].receive_mem); //receive memory start
 outd(ethernet_cards[ethernet_selected_card].base+0x44, (0xF) | (1<<7) | (0x3<<13)); //all packets accepted, WRAP mode, no rx treshold
 ethernet_cards[ethernet_selected_card].receive_packet_pointer = ethernet_cards[ethernet_selected_card].receive_mem;
 
 //initalize transmitting mechanism
 outd(ethernet_cards[ethernet_selected_card].base+0x40, ((0x3 << 24) | (0x7 << 8))); //interframe gap time(0x3 is only allowed value), max packet size is 2048 bytes
 ethernet_cards[ethernet_selected_card].transmit_mem = calloc(1600*4);
 ethernet_cards[ethernet_selected_card].transmit_free_packet = 0x10; //used as port offset
}

byte_t ethernet_card_realtek_8139_get_cable_status(void) {
 if((inw(ethernet_cards[ethernet_selected_card].base+0x64) & 0x4)==0x4) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void ethernet_card_realtek_8139_send_packet(dword_t memory, dword_t length) {
 //copy packet to transmit buffer
 clear_memory((ethernet_cards[ethernet_selected_card].transmit_mem+((ethernet_cards[ethernet_selected_card].transmit_free_packet-0x10)/4*1600)), 1600);
 copy_memory(memory, (ethernet_cards[ethernet_selected_card].transmit_mem+((ethernet_cards[ethernet_selected_card].transmit_free_packet-0x10)/4*1600)), length);

 //transmit packet
 outd(ethernet_cards[ethernet_selected_card].base+ethernet_cards[ethernet_selected_card].transmit_free_packet+0x10, (ethernet_cards[ethernet_selected_card].transmit_mem+((ethernet_cards[ethernet_selected_card].transmit_free_packet-0x10)/4*1600)));
 outd(ethernet_cards[ethernet_selected_card].base+ethernet_cards[ethernet_selected_card].transmit_free_packet, length);
 
 //set port for next packet
 ethernet_cards[ethernet_selected_card].transmit_free_packet += 0x04;
 if(ethernet_cards[ethernet_selected_card].transmit_free_packet==0x20) {
  ethernet_cards[ethernet_selected_card].transmit_free_packet = 0x10;
 }
}

void ethernet_card_realtek_8139_irq(void) {
 word_t *packet16 = (word_t *) (ethernet_cards[ethernet_selected_card].receive_packet_pointer);
 dword_t *packet32 = (dword_t *) (ethernet_cards[ethernet_selected_card].receive_packet_pointer);
 word_t irq_cause = inw(ethernet_cards[ethernet_selected_card].base+0x3E);
 
 //receive packets
 if((irq_cause & 0x1)==0x1) {
  while(*packet32!=0) {
   //load packet
   packet16 = (word_t *) (ethernet_cards[ethernet_selected_card].receive_packet_pointer);
   word_t packet_state = packet16[0];
   word_t packet_length = packet16[1];
   clear_memory(ethernet_cards[ethernet_selected_card].receive_buffer_mem, 1600);
   copy_memory((ethernet_cards[ethernet_selected_card].receive_packet_pointer+4), (ethernet_cards[ethernet_selected_card].receive_buffer_mem), packet_length);
   clear_memory((ethernet_cards[ethernet_selected_card].receive_packet_pointer), packet_length+4);

   //skip packet
   ethernet_cards[ethernet_selected_card].receive_packet_pointer += (packet_length+7);
   ethernet_cards[ethernet_selected_card].receive_packet_pointer &= 0xFFFFFFFC;
   if(ethernet_cards[ethernet_selected_card].receive_packet_pointer>(ethernet_cards[ethernet_selected_card].receive_mem+1024*8)) {
    ethernet_cards[ethernet_selected_card].receive_packet_pointer -= 1024*8;
   }

   //if good packet, process it
   packet32 = (dword_t *) (ethernet_cards[ethernet_selected_card].receive_packet_pointer);
   if((packet_state & 0x1)==0x1) {
    if(*packet32==0) {
     network_process_packet(ethernet_cards[ethernet_selected_card].receive_buffer_mem, 1); //last packet
    }
    else {
     network_process_packet(ethernet_cards[ethernet_selected_card].receive_buffer_mem, 0);
    }
   }

   //update CAPR
   outw(ethernet_cards[ethernet_selected_card].base+0x38, (ethernet_cards[ethernet_selected_card].receive_packet_pointer-ethernet_cards[ethernet_selected_card].receive_mem-0x10));
  }
 }
 if((irq_cause & 0x20)==0x20) {
  //connection change
  read_ethernet_cable_status();
 }
 
 //acknowledge interrupt
 outw(ethernet_cards[ethernet_selected_card].base+0x3E, irq_cause);
}