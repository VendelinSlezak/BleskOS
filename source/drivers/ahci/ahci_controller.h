//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define AHCI_NO_DEVICE_ATTACHED 0x00000000
#define AHCI_SATA_DEVICE 0x00000101
#define AHCI_SATAPI_DEVICE 0xEB140101

#define AHCI_ATA 0
#define AHCI_ATAPI 1
#define AHCI_READ 0
#define AHCI_WRITE 1

struct ahci_command_list_entry_t {
 byte_t command_fis_length_in_dwords: 5;
 byte_t atapi: 1;
 byte_t write: 1;
 byte_t prefetchable: 1;
 byte_t reset: 1;
 byte_t bist: 1;
 byte_t clear: 1;
 byte_t reserved1: 1;
 byte_t port_multiplier: 4;
 word_t number_of_command_table_entries;
 dword_t byte_count;
 dword_t command_table_low_memory;
 dword_t command_table_high_memory;
 byte_t reserved2[16];
}__attribute__((packed));

struct ahci_command_and_prd_table_t {
 byte_t fis_type;
 byte_t flags;
 byte_t command;
 byte_t features;
 byte_t lba_0;
 byte_t lba_1;
 byte_t lba_2;
 byte_t device_head;
 byte_t lba_3;
 byte_t lba_4;
 byte_t lba_5;
 byte_t features_expanded;
 byte_t sector_count_low;
 byte_t sector_count_high;
 byte_t reserved1;
 byte_t control;
 byte_t reserved2[0x30];

 byte_t atapi_command[0x10];
 byte_t reserved3[0x30];

 dword_t data_base_low_memory;
 dword_t data_base_high_memory;
 dword_t reserved4;
 dword_t data_byte_count;
}__attribute__((packed));

void initalize_ahci_controller(byte_t number_of_controller);
byte_t ahci_wait(dword_t base_address, dword_t mask, dword_t desired_result, dword_t time);
byte_t *ahci_enable_port(dword_t port_base_address);
byte_t ahci_send_command(dword_t port_base_address, dword_t command_list_memory, byte_t command_type, byte_t command_direction, byte_t atapi_command[0x10], byte_t command, word_t sector_count, dword_t lba, dword_t memory, dword_t byte_count);
byte_t ahci_send_ata_command(dword_t port_base_address, dword_t command_list_memory, byte_t command_direction, byte_t command, word_t sector_count, dword_t lba, dword_t memory, dword_t byte_count);
byte_t ahci_send_atapi_command(dword_t port_base_address, dword_t command_list_memory, dword_t memory, word_t byte_count, byte_t atapi_command[0x10]);
byte_t ahci_send_identify_command(dword_t port_base_address, dword_t command_list_memory);