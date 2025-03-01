//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t ec_broadcom_netxtreme_read(dword_t number_of_card, dword_t port) {
 return mmio_ind(ethernet_cards[number_of_card].base+port);
}

void ec_broadcom_netxtreme_write(dword_t number_of_card, dword_t port, dword_t value) {
 mmio_outd(ethernet_cards[number_of_card].base+port, value);
}

void ec_broadcom_netxtreme_set_bits(dword_t number_of_card, dword_t port, dword_t bits) {
 mmio_outd(ethernet_cards[number_of_card].base+port, (mmio_ind(ethernet_cards[number_of_card].base+port) | bits));
}

void ec_broadcom_netxtreme_clear_bits(dword_t number_of_card, dword_t port, dword_t bits) {
 mmio_outd(ethernet_cards[number_of_card].base+port, (mmio_ind(ethernet_cards[number_of_card].base+port) & ~bits));
}

word_t ec_broadcom_netxtreme_read_phy(dword_t number_of_card, byte_t phy_register) {
 ec_broadcom_netxtreme_write(number_of_card, 0x44C, ( (1 << 29) | (0b10 << 26) | (0b1 << 21) | (phy_register << 16) | (0x0000 << 0) ));
 wait(10);
 log("\n0x404: "); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x404));
 log("\n0x6810: "); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x6810));
 log("\n0x684C: "); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x684C));
 return (ec_broadcom_netxtreme_read(number_of_card, 0x44C) & 0xFFFF);
}

void ec_broadcom_netxtreme_write_phy(dword_t number_of_card, byte_t phy_register, word_t value) {
 ec_broadcom_netxtreme_write(number_of_card, 0x44C, ( (1 << 29) | (0b01 << 26) | (0b1 << 21) | (phy_register << 16) | (value << 0) ));
 wait(10);
 log("\n0x404: "); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x404));
 log("\n0x6810: "); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x6810));
 log("\n0x684C: "); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x684C));
}

void ec_broadcom_netxtreme_initalize(dword_t number_of_card) {
 //log device ID of card
 log("\n\nBroadcom NetXtreme driver\nDevice ID: ");
 log_hex_specific_size((ethernet_cards[number_of_card].id >> 16), 4);

 //set card methods
 ethernet_cards[number_of_card].get_cable_status = ec_broadcom_netxtreme_get_cable_status;
 ethernet_cards[number_of_card].send_packet = ec_broadcom_netxtreme_send_packet;
 ethernet_cards[number_of_card].process_irq = ec_broadcom_netxtreme_process_irq;

 //initalize interrupts
 // set_irq_handler(ethernet_cards[number_of_card].irq, (dword_t)network_irq);
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x6808, (1 << 2));
 // wait(100);

 //PCI 0x68
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x6808, (1 << 3));
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x5000, (1 << 7));
 // ec_broadcom_netxtreme_clear_bits(number_of_card, 0x5008, (1 << 12));
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x5028, 0x1);
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x408, (1 << 12) | (1 << 22) | (1 << 23));
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x450, (1 << 0));
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x6800, (1 << 26));
 // ec_broadcom_netxtreme_write_phy(number_of_card, 0x1B, 0x0000);

//  // Clear existing interrupt bits for link changes
// ec_broadcom_netxtreme_clear_bits(number_of_card, 0x5008, (1 << 12));

// // Enable link change interrupt
// ec_broadcom_netxtreme_set_bits(number_of_card, 0x5000, (1 << 7));

// // Enable status check for link state changes
// ec_broadcom_netxtreme_set_bits(number_of_card, 0x408, (1 << 12));

// // Enable global interrupt enabling
// ec_broadcom_netxtreme_set_bits(number_of_card, 0x6800, (1 << 26));

// // Optionally configure PHY if necessary
// ec_broadcom_netxtreme_write_phy(number_of_card, 0x1B, 0x0000);

 //generate interrupt
 // ec_broadcom_netxtreme_set_bits(number_of_card, 0x6808, (1 << 2));

 //Interrupt Mailbox 0 register “Interrupt Mailbox 0 (High Priority Mailbox) Register
 // (offset: 0x200-207)” on page 319 for host standard
 // and “Interrupt Mailbox 0 Register (offset: 0x5800)” on
 // page 513 for indirect mode.
 // Receive Coalescing Ticks register “Receive Coalescing Ticks Register (offset: 0x3C08)” on
 // page 444.
 // Send Coalescing Ticks register “Send Coalescing Ticks Register (offset: 0x3C0C)” on
 // page 445.
 // Receive Max Coalesced BD Count register “Receive Max Coalesced BD Count Register (offset:
 // 0x3C10)” on page 446.
 // Send Max Coalesced BD Count register “Send Max Coalesced BD Count Register (offset:
 // 0x3C14)” on page 446. 

 //0x400 promiscuos + MII/GMII

 //read MAC address from ports
 // for(dword_t i=0; i<6; i++) {
 //  ethernet_cards[number_of_card].mac_address[i] = (ec_intel_e1000_read(number_of_card, 0x5400+i) & 0xFF);
 // }

 //log MAC
 log("\nMAC: "); log_hex_with_space(ec_broadcom_netxtreme_read(number_of_card, 0x410)); log_hex(ec_broadcom_netxtreme_read(number_of_card, 0x414));
}

byte_t ec_broadcom_netxtreme_get_cable_status(dword_t number_of_card) {
 if((ec_broadcom_netxtreme_read(number_of_card, 0x460) & (1 << 3))==(1 << 3)) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

byte_t ec_broadcom_netxtreme_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
 return STATUS_ERROR;
}

void ec_broadcom_netxtreme_process_irq(dword_t number_of_card) {
 l("Broadcom NetXtreme irq");

 l("\nCard: ");
 lhw(pci_read(0x02, 0x0E, 0x00, 0x4));
 l("\nBridge 1: ");
 lhw(pci_read(0x00, 0x1C, 0x00, 0x4));
 l("\nBridge 2: ");
 lhw(pci_read(0x00, 0x1E, 0x00, 0x4));
 l("\nISA Bridge: ");
 lhw(pci_read(0x00, 0x1F, 0x00, 0x4));
 l("\nHost Bridge: ");
 lhw(pci_read(0x00, 0x02, 0x00, 0x4));

 ec_broadcom_netxtreme_set_bits(number_of_card, 0x6808, (1 << 1));
}