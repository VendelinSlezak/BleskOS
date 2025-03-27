//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct pci_supported_devices_list_t realtek_8139_supported_pci_devices[] = {
    { 0x10EC, 0x8139 },
    { 0x10EC, 0x8138 },
    { 0x1113, 0x1211 },
    { 0x1500, 0x1360 },
    { 0x4033, 0x1360 },
    { 0x1186, 0x1300 },
    { 0x1186, 0x1340 },
    { 0x13D1, 0xAB06 },
    { 0x1259, 0xA117 },
    { 0x1259, 0xA11E },
    { 0x14EA, 0xAB06 },
    { 0x14EA, 0xAB07 },
    { 0x11DB, 0x1234 },
    { 0x1432, 0x9130 },
    { 0x02AC, 0x1012 },
    { 0x018A, 0x0106 },
    { 0x126C, 0x1211 },
    { 0x1743, 0x8139 },
    { 0x021B, 0x8139 },
    { 0x16EC, 0xAB06 },
    { 0, 0 }
};

#define MAX_NUMBER_OF_REALTEK_8139_CARDS 4
struct realtek_8139_info_t {
    struct pci_device_info_t pci;
    word_t base;

    byte_t mac_address[6];

    void *rx_descriptors_memory;
    dword_t rx_descriptor_pointer;
    byte_t *rx_packets_memory;

    void *tx_descriptors_memory;
    dword_t tx_descriptor_pointer;
    byte_t *tx_packets_memory;
};

struct ec_realtek_8139_received_packet_header_t {
    word_t packet_ok: 1;
    word_t frame_alignment_error: 1;
    word_t crc_error: 1;
    word_t long_packet: 1;
    word_t runt_packet: 1;
    word_t invalid_symbol: 1;
    word_t reserved: 7;
    word_t broadcast_address: 1;
    word_t physical_address_match: 1;
    word_t multicast_address: 1;
    word_t packet_size;
}__attribute__((packed));

#define EC_REALTEK_8139_NUMBER_OF_TX_DESCRIPTORS 256
struct ec_realtek_8139_bleskos_tx_descriptor_t {
    word_t packet_size;
    word_t send_to_card;
}__attribute__((packed));

void realtek_8139_add_new_pci_device(struct pci_device_info_t device);
void ec_realtek_8139_initalize(dword_t number_of_card);
byte_t ec_realtek_8139_get_cable_status(dword_t number_of_card);
void ec_realtek_8139_set_transmit_port(dword_t number_of_card, byte_t transmit_port_number, dword_t packet_memory, dword_t packet_size);
byte_t ec_realtek_8139_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_realtek_8139_process_irq(dword_t number_of_card);