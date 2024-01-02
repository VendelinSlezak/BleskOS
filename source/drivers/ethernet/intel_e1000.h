//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t ethernet_card_intel_e1000_rx_mem = 0;
dword_t ethernet_card_intel_e1000_tx_mem = 0;
dword_t ethernet_card_intel_e1000_tx_free_entry = 0;

dword_t read_ethernet_card_intel_e1000(dword_t port);
void write_ethernet_card_intel_e1000(dword_t port, dword_t value);
word_t read_eeprom_ethernet_card_intel_e1000(dword_t address, dword_t shift);
word_t read_phy_ethernet_card_intel_e1000(dword_t address);
void initalize_ethernet_card_intel_e1000(void);
byte_t ethernet_card_intel_e1000_get_cable_status(void);
void ethernet_card_intel_e1000_send_packet(dword_t memory, dword_t length);
void ethernet_card_intel_e1000_irq(void);