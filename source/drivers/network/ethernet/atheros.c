//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

word_t ec_atheros_read_phy(dword_t number_of_card, dword_t reg) {
 //send command
 mmio_outd(ethernet_cards[number_of_card].base+0x1414, (reg << 16) | (1 << 21) | (1 << 22) | (1 << 23) | (0 << 24)); //register, read, send preamble, start operation, clock 25 MHz

 //wait to process command
 for(dword_t i=0; i<1000; i++) {
  if((mmio_ind(ethernet_cards[number_of_card].base+0x1414) & (1 << 27))==0x0) {
   //return value in register
   return (mmio_ind(ethernet_cards[number_of_card].base+0x1414) & 0xFFFF);
  }
 }

 //timeout error
 return STATUS_ERROR;
}

void ec_atheros_write_phy(dword_t number_of_card, dword_t reg, word_t value) {
 //send command
 mmio_outd(ethernet_cards[number_of_card].base+0x1414, (value) | (reg << 16) | (0 << 21) | (1 << 22) | (1 << 23) | (0 << 24)); //value, register, write, send preamble, start operation, clock 25 MHz

 //wait to process command
 for(dword_t i=0; i<1000; i++) {
  if((mmio_ind(ethernet_cards[number_of_card].base+0x1414) & (1 << 27))==0x0) {
   return; //successful write
  }
 }

 //timeout error
 log("\nwrite PHY error");
 return;
}

void ec_atheros_initalize(dword_t number_of_card) {
 //log device ID of card
 log("\n\nQualcomm Atheros driver\nDevice ID: ");
 log_hex_specific_size((ethernet_cards[number_of_card].id >> 16), 4);

 //set card methods
 ethernet_cards[number_of_card].get_cable_status = ec_atheros_get_cable_status;
 ethernet_cards[number_of_card].send_packet = ec_atheros_send_packet;
 ethernet_cards[number_of_card].process_irq = ec_atheros_process_irq;

 //read MAC address
 //TODO: is this reading reversed?
 dword_t mac_value = mmio_ind(ethernet_cards[number_of_card].base+0x1488);
 for(dword_t i=0; i<4; i++) {
  ethernet_cards[number_of_card].mac_address[i] = (mac_value >> (i*8));
 }
 mac_value = mmio_ind(ethernet_cards[number_of_card].base+0x148C);
 for(dword_t i=0; i<2; i++) {
  ethernet_cards[number_of_card].mac_address[4+i] = (mac_value >> (i*8));
 }

 //log MAC address
 log("\nMAC: ");
 for(dword_t i=0; i<6; i++) {
  log_hex_specific_size_with_space(ethernet_cards[number_of_card].mac_address[i], 2);
 }

 //enable interrupts
 set_irq_handler(ethernet_cards[number_of_card].irq, (dword_t)network_irq);

 #define ATHEROS_ISR 0x1600
 #define ATHEROS_IMR 0x1604
 #define ATHEROS_MSIX_MASK 0x0090

 #define ATHEROS_MII_GIGA_PSSR 0x11
 #define ATHEROS_MII_IER 0x12
 #define ATHEROS_MII_ISR 0x13

 //set interrupts for PHY
 // log("\nIRQ PHY ISR mask: "); log_hex_specific_size(ec_atheros_read_phy(number_of_card, ATHEROS_MII_ISR), 4);
 // word_t irq_phy_mask = ec_atheros_read_phy(number_of_card, ATHEROS_MII_IER);
 // log("\nIRQ PHY mask: "); log_hex_specific_size(irq_phy_mask, 4);
 // ec_atheros_write_phy(number_of_card, ATHEROS_MII_IER, irq_phy_mask | 0x0400 | 0x0800); //link up, link down
 // log("\nIRQ PHY mask after: "); log_hex_specific_size(ec_atheros_read_phy(number_of_card, ATHEROS_MII_IER), 4);

 //set interrupts
 log("\nMSI MASK: "); log_hex(mmio_ind(ethernet_cards[number_of_card].base+ATHEROS_MSIX_MASK));
 log("\nISR: "); log_hex(mmio_ind(ethernet_cards[number_of_card].base+ATHEROS_ISR));
 log("\nIMR: "); log_hex(mmio_ind(ethernet_cards[number_of_card].base+ATHEROS_IMR));
 //mov     dword[esi + ATHEROS_MSI_RETRANS_TIMER], 0

 mmio_outd(ethernet_cards[number_of_card].base + ATHEROS_ISR, ~(1 << 31));
 mmio_outd(ethernet_cards[number_of_card].base + ATHEROS_ISR, 0);
 mmio_outd(ethernet_cards[number_of_card].base + ATHEROS_IMR, (1 << 12)); //PHY interrupt enable
 mmio_ind(ethernet_cards[number_of_card].base + 0x0); //write flush

 log("\nISR: "); log_hex(mmio_ind(ethernet_cards[number_of_card].base+ATHEROS_ISR));
 log("\nIMR: "); log_hex(mmio_ind(ethernet_cards[number_of_card].base+ATHEROS_IMR));
}

byte_t ec_atheros_get_cable_status(dword_t number_of_card) {
 ec_atheros_read_phy(number_of_card, 0x01); //this register should be readed twice TODO: find out why?
 if((ec_atheros_read_phy(number_of_card, 0x01) & (1 << 2))==(1 << 2)) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

byte_t ec_atheros_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
 return STATUS_ERROR;
}

void ec_atheros_process_irq(dword_t number_of_card) {
 //read interrupt status
 dword_t irq_status = mmio_ind(ethernet_cards[number_of_card].base+ATHEROS_ISR);

 pstr("irq");
 phex(irq_status);

 //acknowledge interrupt
 mmio_outd(ethernet_cards[number_of_card].base+ATHEROS_ISR, irq_status | (1 << 31));
}