//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct mbr_partition_entry_t {
 byte_t status: 7;
 byte_t bootable: 1;
 byte_t H_of_first_sector;
 byte_t S_of_first_sector: 6;
 word_t C_of_first_sector: 10;
 byte_t type;
 byte_t H_of_last_sector;
 byte_t S_of_last_sector: 6;
 word_t C_of_last_sector: 10;
 dword_t first_sector;
 dword_t number_of_sectors;
}__attribute__((packed));

struct mbr_sector_t {
 byte_t reserved[0x1BE];
 struct mbr_partition_entry_t partition_entry[4];
 word_t boot_signature;
}__attribute__((packed));

struct ebr_sector_t {
 byte_t reserved[0x1BE];
 struct mbr_partition_entry_t partition_entry[2];
 byte_t reserved2[0x20];
 word_t boot_signature;
}__attribute__((packed));

struct gpt_partition_entry_t {
 byte_t partition_type_guid[16];
 byte_t unique_partition_guid[16];
 qword_t first_sector;
 qword_t last_sector;
 qword_t attributes;
 byte_t name[72];
}__attribute__((packed));

struct gpt_partition_header_t {
 byte_t signature[8];
 dword_t revision;
 dword_t header_size;
 dword_t checksum;
 dword_t reserved;
 qword_t lba_of_this_header;
 qword_t lba_of_alternative_header;
 qword_t first_usable_block;
 qword_t last_usable_block;
 byte_t guid[16];
 qword_t first_lba_of_guid_partition_entry_array;
 dword_t number_of_partition_entries;
 dword_t size_of_partition_entry;
 dword_t crc32_of_partition_entry_array;
}__attribute__((packed));

#define STORAGE_NO_PARTITION 0 //TODO: delete later
#define PARTITION_FREE_SPACE 1
#define PARTITION_FILESYSTEM_FAT 2
#define PARTITION_FILESYSTEM_EXT 3
#define PARTITION_FILESYSTEM_ISO9660 4
#define PARTITION_FILESYSTEM_CDDA 5
#define PARTITION_FILESYSTEM_BLESKOS_BOOTABLE_PARTITION 6
#define PARTITION_UNKNOWN_FILESYSTEM 7
struct partition_info_t {
 byte_t type;
 dword_t first_sector;
 dword_t num_of_sectors;
}__attribute__((packed));
#define MAX_NUMBER_OF_PARTITIONS_IN_PARTITION_INFO_TABLE 32
struct partition_info_t partitions[MAX_NUMBER_OF_PARTITIONS_IN_PARTITION_INFO_TABLE];

dword_t number_of_partitions_in_table;

byte_t partition_label[12]; //TODO: remove later

void add_partition(byte_t type, dword_t first_sector, dword_t number_of_sectors);
void read_partitions_of_medium(void);
void connect_partitions_of_medium(byte_t medium_type, byte_t medium_number);