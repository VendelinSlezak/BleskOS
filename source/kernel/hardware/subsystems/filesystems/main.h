/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/groups/storage/storage.h>

typedef struct {
    uint8_t status: 7;
    uint8_t bootable: 1;
    uint8_t chs_address_of_first_sector[3];
    uint8_t type;
    uint8_t chs_address_of_last_sector[3];
    uint32_t lba_address_of_first_sector;
    uint32_t number_of_sectors;
}__attribute__((packed)) mbr_partition_entry_t;

typedef struct {
    uint8_t reserved[0x1BE];
    mbr_partition_entry_t partition[4];
    uint16_t boot_signature;
}__attribute__((packed)) mbr_sector_t;

typedef struct {
    uint8_t reserved[0x1BE];
    mbr_partition_entry_t partition[2];
    uint8_t reserved2[0x20];
    uint16_t boot_signature;
}__attribute__((packed)) ebr_sector_t;

typedef struct {
    uint8_t partition_type_guid[16];
    uint8_t unique_partition_guid[16];
    uint64_t first_sector;
    uint64_t last_sector;
    uint64_t attributes;
    uint8_t name[72];
}__attribute__((packed)) gpt_partition_entry_t;

typedef struct {
    uint8_t signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t checksum;
    uint32_t reserved;
    uint64_t lba_of_this_header;
    uint64_t lba_of_alternative_header;
    uint64_t first_usable_block;
    uint64_t last_usable_block;
    uint8_t guid[16];
    uint64_t first_lba_of_guid_partition_entry_array;
    uint32_t number_of_partition_entries;
    uint32_t size_of_partition_entry;
    uint32_t crc32_of_partition_entry_array;
}__attribute__((packed)) gpt_partition_header_t;

typedef struct {
    uint32_t start_sector;
    uint32_t number_of_sectors;
    uint8_t type;
    uint8_t name[80];
    void *data;
} partition_info_t;

typedef struct {
    void *device;
    uint32_t type;
    uint32_t number_of_partitions;
    partition_info_t partitions[];
} filesystem_device_info_t;