//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct pci_supported_devices_list_t intel_e1000_supported_pci_devices[] = {
    {0x8086, 0x1000},
    {0x8086, 0x1001},
    {0x8086, 0x1004},
    {0x8086, 0x1008},
    {0x8086, 0x1009},
    {0x8086, 0x100C},
    {0x8086, 0x100D},
    {0x8086, 0x100E},
    {0x8086, 0x100F},
    {0x8086, 0x1010},
    {0x8086, 0x1011},
    {0x8086, 0x1012},
    {0x8086, 0x1013},
    {0x8086, 0x1014},
    {0x8086, 0x1015},
    {0x8086, 0x1016},
    {0x8086, 0x1017},
    {0x8086, 0x1018},
    {0x8086, 0x1019},
    {0x8086, 0x101A},
    {0x8086, 0x101D},
    {0x8086, 0x101E},
    {0x8086, 0x1026},
    {0x8086, 0x1027},
    {0x8086, 0x1028},
    {0x8086, 0x1075},
    {0x8086, 0x1076},
    {0x8086, 0x1077},
    {0x8086, 0x1078},
    {0x8086, 0x1079},
    {0x8086, 0x107A},
    {0x8086, 0x107B},
    {0x8086, 0x107C},
    {0x8086, 0x108A},
    {0x8086, 0x1099},
    {0x8086, 0x10B5},
    {0x8086, 0x2E6E},
    {0, 0}
};

struct pci_supported_devices_list_t intel_e1000e_supported_pci_devices[] = {
    {0x8086, 0x105E},  // E1000_DEV_ID_82571EB_COPPER
    {0x8086, 0x105F},  // E1000_DEV_ID_82571EB_FIBER
    {0x8086, 0x1060},  // E1000_DEV_ID_82571EB_SERDES
    {0x8086, 0x10A4},  // E1000_DEV_ID_82571EB_QUAD_COPPER
    {0x8086, 0x10D5},  // E1000_DEV_ID_82571PT_QUAD_COPPER
    {0x8086, 0x10A5},  // E1000_DEV_ID_82571EB_QUAD_FIBER
    {0x8086, 0x10BC},  // E1000_DEV_ID_82571EB_QUAD_COPPER_LP
    {0x8086, 0x10D9},  // E1000_DEV_ID_82571EB_SERDES_DUAL
    {0x8086, 0x10DA},  // E1000_DEV_ID_82571EB_SERDES_QUAD
    {0x8086, 0x107D},  // E1000_DEV_ID_82572EI_COPPER
    {0x8086, 0x107E},  // E1000_DEV_ID_82572EI_FIBER
    {0x8086, 0x107F},  // E1000_DEV_ID_82572EI_SERDES
    {0x8086, 0x10B9},  // E1000_DEV_ID_82572EI
    {0x8086, 0x108B},  // E1000_DEV_ID_82573E
    {0x8086, 0x108C},  // E1000_DEV_ID_82573E_IAMT
    {0x8086, 0x109A},  // E1000_DEV_ID_82573L
    {0x8086, 0x10D3},  // E1000_DEV_ID_82574L
    {0x8086, 0x10F6},  // E1000_DEV_ID_82574LA
    {0x8086, 0x150C},  // E1000_DEV_ID_82583V
    {0x8086, 0x1096},  // E1000_DEV_ID_80003ES2LAN_COPPER_DPT
    {0x8086, 0x1098},  // E1000_DEV_ID_80003ES2LAN_SERDES_DPT
    {0x8086, 0x10BA},  // E1000_DEV_ID_80003ES2LAN_COPPER_SPT
    {0x8086, 0x10BB},  // E1000_DEV_ID_80003ES2LAN_SERDES_SPT
    {0x8086, 0x1501},  // E1000_DEV_ID_ICH8_82567V_3
    {0x8086, 0x1049},  // E1000_DEV_ID_ICH8_IGP_M_AMT
    {0x8086, 0x104A},  // E1000_DEV_ID_ICH8_IGP_AMT
    {0x8086, 0x104B},  // E1000_DEV_ID_ICH8_IGP_C
    {0x8086, 0x104C},  // E1000_DEV_ID_ICH8_IFE
    {0x8086, 0x10C4},  // E1000_DEV_ID_ICH8_IFE_GT
    {0x8086, 0x10C5},  // E1000_DEV_ID_ICH8_IFE_G
    {0x8086, 0x104D},  // E1000_DEV_ID_ICH8_IGP_M
    {0x8086, 0x10BD},  // E1000_DEV_ID_ICH9_IGP_AMT
    {0x8086, 0x10E5},  // E1000_DEV_ID_ICH9_BM
    {0x8086, 0x10F5},  // E1000_DEV_ID_ICH9_IGP_M_AMT
    {0x8086, 0x10BF},  // E1000_DEV_ID_ICH9_IGP_M
    {0x8086, 0x10CB},  // E1000_DEV_ID_ICH9_IGP_M_V
    {0x8086, 0x294C},  // E1000_DEV_ID_ICH9_IGP_C
    {0x8086, 0x10C0},  // E1000_DEV_ID_ICH9_IFE
    {0x8086, 0x10C3},  // E1000_DEV_ID_ICH9_IFE_GT
    {0x8086, 0x10C2},  // E1000_DEV_ID_ICH9_IFE_G
    {0x8086, 0x10CC},  // E1000_DEV_ID_ICH10_R_BM_LM
    {0x8086, 0x10CD},  // E1000_DEV_ID_ICH10_R_BM_LF
    {0x8086, 0x10CE},  // E1000_DEV_ID_ICH10_R_BM_V
    {0x8086, 0x10DE},  // E1000_DEV_ID_ICH10_D_BM_LM
    {0x8086, 0x10DF},  // E1000_DEV_ID_ICH10_D_BM_LF
    {0x8086, 0x1525},  // E1000_DEV_ID_ICH10_D_BM_V
    {0x8086, 0x10EA},  // E1000_DEV_ID_PCH_M_HV_LM
    {0x8086, 0x10EB},  // E1000_DEV_ID_PCH_M_HV_LC
    {0x8086, 0x10EF},  // E1000_DEV_ID_PCH_D_HV_DM
    {0x8086, 0x10F0},  // E1000_DEV_ID_PCH_D_HV_DC
    {0x8086, 0x1502},  // E1000_DEV_ID_PCH2_LV_LM
    {0x8086, 0x1503},  // E1000_DEV_ID_PCH2_LV_V
    {0x8086, 0x153A},  // E1000_DEV_ID_PCH_LPT_I217_LM
    {0x8086, 0x153B},  // E1000_DEV_ID_PCH_LPT_I217_V
    {0x8086, 0x155A},  // E1000_DEV_ID_PCH_LPTLP_I218_LM
    {0x8086, 0x1559},  // E1000_DEV_ID_PCH_LPTLP_I218_V
    {0x8086, 0x15A0},  // E1000_DEV_ID_PCH_I218_LM2
    {0x8086, 0x15A1},  // E1000_DEV_ID_PCH_I218_V2
    {0x8086, 0x15A2},  // E1000_DEV_ID_PCH_I218_LM3
    {0x8086, 0x15A3},  // E1000_DEV_ID_PCH_I218_V3
    {0x8086, 0x156F},  // E1000_DEV_ID_PCH_SPT_I219_LM
    {0x8086, 0x1570},  // E1000_DEV_ID_PCH_SPT_I219_V
    {0x8086, 0x15B7},  // E1000_DEV_ID_PCH_SPT_I219_LM2
    {0x8086, 0x15B8},  // E1000_DEV_ID_PCH_SPT_I219_V2
    {0x8086, 0x15B9},  // E1000_DEV_ID_PCH_LBG_I219_LM3
    {0x8086, 0x15D7},  // E1000_DEV_ID_PCH_SPT_I219_LM4
    {0x8086, 0x15D8},  // E1000_DEV_ID_PCH_SPT_I219_V4
    {0x8086, 0x15E3},  // E1000_DEV_ID_PCH_SPT_I219_LM5
    {0x8086, 0x15D6},  // E1000_DEV_ID_PCH_SPT_I219_V5
    {0x8086, 0x15BD},  // E1000_DEV_ID_PCH_CNP_I219_LM6
    {0x8086, 0x15BE},  // E1000_DEV_ID_PCH_CNP_I219_V6
    {0x8086, 0x15BB},  // E1000_DEV_ID_PCH_CNP_I219_LM7
    {0x8086, 0x15BC},  // E1000_DEV_ID_PCH_CNP_I219_V7
    {0x8086, 0x15DF},  // E1000_DEV_ID_PCH_ICP_I219_LM8
    {0x8086, 0x15E0},  // E1000_DEV_ID_PCH_ICP_I219_V8
    {0x8086, 0x15E1},  // E1000_DEV_ID_PCH_ICP_I219_LM9
    {0x8086, 0x15E2},  // E1000_DEV_ID_PCH_ICP_I219_V9
    {0x8086, 0x0D4E},  // E1000_DEV_ID_PCH_CMP_I219_LM10
    {0x8086, 0x0D4F},  // E1000_DEV_ID_PCH_CMP_I219_V10
    {0x8086, 0x0D4C},  // E1000_DEV_ID_PCH_CMP_I219_LM11
    {0x8086, 0x0D4D},  // E1000_DEV_ID_PCH_CMP_I219_V11
    {0x8086, 0x0D53},  // E1000_DEV_ID_PCH_CMP_I219_LM12
    {0x8086, 0x0D55},  // E1000_DEV_ID_PCH_CMP_I219_V12
    {0x8086, 0x15FB},  // E1000_DEV_ID_PCH_TGP_I219_LM13
    {0x8086, 0x15FC},  // E1000_DEV_ID_PCH_TGP_I219_V13
    {0x8086, 0x15F9},  // E1000_DEV_ID_PCH_TGP_I219_LM14
    {0x8086, 0x15FA},  // E1000_DEV_ID_PCH_TGP_I219_V14
    {0x8086, 0x15F4},  // E1000_DEV_ID_PCH_TGP_I219_LM15
    {0x8086, 0x15F5},  // E1000_DEV_ID_PCH_TGP_I219_V15
    {0x8086, 0x0DC5},  // E1000_DEV_ID_PCH_RPL_I219_LM23
    {0x8086, 0x0DC6},  // E1000_DEV_ID_PCH_RPL_I219_V23
    {0x8086, 0x1A1E},  // E1000_DEV_ID_PCH_ADP_I219_LM16
    {0x8086, 0x1A1F},  // E1000_DEV_ID_PCH_ADP_I219_V16
    {0x8086, 0x1A1C},  // E1000_DEV_ID_PCH_ADP_I219_LM17
    {0x8086, 0x1A1D},  // E1000_DEV_ID_PCH_ADP_I219_V17
    {0x8086, 0x0DC7},  // E1000_DEV_ID_PCH_RPL_I219_LM22
    {0x8086, 0x0DC8},  // E1000_DEV_ID_PCH_RPL_I219_V22
    {0x8086, 0x550A},  // E1000_DEV_ID_PCH_MTP_I219_LM18
    {0x8086, 0x550B},  // E1000_DEV_ID_PCH_MTP_I219_V18
    {0x8086, 0x550C},  // E1000_DEV_ID_PCH_ADP_I219_LM19
    {0x8086, 0x550D},  // E1000_DEV_ID_PCH_ADP_I219_V19
    {0x8086, 0x550E},  // E1000_DEV_ID_PCH_LNP_I219_LM20
    {0x8086, 0x550F},  // E1000_DEV_ID_PCH_LNP_I219_V20
    {0x8086, 0x5510},  // E1000_DEV_ID_PCH_LNP_I219_LM21
    {0x8086, 0x5511},  // E1000_DEV_ID_PCH_LNP_I219_V21
    {0x8086, 0x57A0},  // E1000_DEV_ID_PCH_ARL_I219_LM24
    {0x8086, 0x57A1},  // E1000_DEV_ID_PCH_ARL_I219_V24
    {0x8086, 0x57B3},  // E1000_DEV_ID_PCH_PTP_I219_LM25
    {0x8086, 0x57B4},  // E1000_DEV_ID_PCH_PTP_I219_V25
    {0x8086, 0x57B5},  // E1000_DEV_ID_PCH_PTP_I219_LM26
    {0x8086, 0x57B6},  // E1000_DEV_ID_PCH_PTP_I219_V26
    {0x8086, 0x57B7},  // E1000_DEV_ID_PCH_PTP_I219_LM27
    {0x8086, 0x57B8},  // E1000_DEV_ID_PCH_PTP_I219_V27
    {0x8086, 0x57B9},  // E1000_DEV_ID_PCH_NVL_I219_LM29
    {0x8086, 0x57BA},  // E1000_DEV_ID_PCH_NVL_I219_V29
    {0, 0}
};

#define MAX_NUMBER_OF_INTEL_E1000_CARDS 4
struct intel_e1000_info_t {
    struct pci_device_info_t pci;
    dword_t bar_type;
    dword_t base;
    dword_t eeprom_shift;

    byte_t cable_status;

    byte_t mac_address[6];

    void *rx_descriptors_memory;
    dword_t rx_descriptor_pointer;
    byte_t *rx_packets_memory;

    void *tx_descriptors_memory;
    dword_t tx_descriptor_pointer;
    byte_t *tx_packets_memory;
};

#define EC_INTEL_E1000_PORT_DEVICE_CONTROL 0x00
#define EC_INTEL_E1000_PORT_EEPROM_CONTROL 0x10
#define EC_INTEL_E1000_PORT_EEPROM_READ 0x14

#define EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS 256
struct ec_intel_e1000_rx_descriptor_t {
 dword_t lower_packet_address;
 dword_t upper_packet_address;
 word_t packet_size;
 word_t fragment_checksum;

 byte_t descriptor_done: 1;
 byte_t end_of_packet: 1;
 byte_t reserved_bit: 1;
 byte_t vp: 1;
 byte_t udp_checksum_calculated: 1;
 byte_t l4_checksum_calculated: 1;
 byte_t ipv4_checksum_calculated: 1;
 byte_t passed_in_exact_filter: 1;

 byte_t reserved: 5;
 byte_t tcp_udp_checksum_error: 1;
 byte_t ipv4_checksum_error: 1;
 byte_t rx_data_error: 1;
 
 word_t vlan_tag;
}__attribute__((packed));

#define EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS 256
struct ec_intel_e1000_tx_descriptor_t {
 dword_t lower_packet_address;
 dword_t upper_packet_address;
 word_t packet_size;
 byte_t checksum_offset;

 byte_t end_of_packet: 1;
 byte_t insert_fcs: 1;
 byte_t insert_checksum: 1;
 byte_t report_status: 1;
 byte_t reserved_bit_1: 1;
 byte_t descriptor_extension: 1;
 byte_t vlan_packet_enable: 1;
 byte_t reserved_bit_2: 1;

 byte_t status_descriptor_done: 1;
 byte_t reserved: 7;
 
 byte_t checksum_start;
 word_t vlan;
}__attribute__((packed));

void intel_e1000_add_new_pci_device(struct pci_device_info_t device);

dword_t ec_intel_e1000_read(dword_t number_of_card, dword_t port);
void ec_intel_e1000_write(dword_t number_of_card, dword_t port, dword_t value);
byte_t ec_intel_e1000_read_timeout(dword_t number_of_card, dword_t port, dword_t bits, dword_t value, dword_t wait_in_milliseconds);
word_t ec_intel_e1000_read_eeprom(dword_t number_of_card, dword_t address);

void ec_intel_e1000_initalize(dword_t number_of_card);
byte_t ec_intel_e1000_get_cable_status(dword_t number_of_card);
byte_t ec_intel_e1000_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_intel_e1000_process_irq(dword_t number_of_card);