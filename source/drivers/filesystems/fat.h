//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct fat16_bpb {
 byte_t jmp[3];
 byte_t oem[8];
 word_t bytes_per_sector;
 byte_t sectors_per_cluster;
 word_t reserved_sectors;
 byte_t fat_tables;
 word_t root_dir_entries;
 word_t total_sectors_word;
 byte_t media_descriptor;
 word_t fat_table_size;
 word_t sectors_per_track;
 word_t number_of_heads;
 dword_t hidden_sectors;
 dword_t total_sectors_dword;
 
 byte_t drive_number;
 byte_t reserved;
 byte_t signature;
 dword_t volume_id;
 byte_t volume_label[11];
 byte_t fat_label[8];
 
 byte_t code[448];
 
 word_t boot_signature;
}__attribute__((packed));
struct fat16_bpb fat16_boot_sector;

struct fat32_bpb {
 byte_t jmp[3];
 byte_t oem[8];
 word_t bytes_per_sector;
 byte_t sectors_per_cluster;
 word_t reserved_sectors;
 byte_t fat_tables;
 word_t root_dir_entries;
 word_t total_sectors_word;
 byte_t media_descriptor;
 word_t fat_table_size_word;
 word_t sectors_per_track;
 word_t number_of_heads;
 dword_t hidden_sectors;
 dword_t total_sectors_dword;
 
 dword_t fat_table_size;
 word_t flags;
 word_t fat_version;
 dword_t root_dir_cluster;
 word_t fsinfo_sector;
 word_t backup_boot_sector;
 byte_t reserved[12];
 byte_t drive_number;
 byte_t old_flags;
 byte_t signature;
 dword_t volume_id;
 byte_t volume_label[11];
 byte_t fat_label[8];
 
 byte_t code[448];
 
 word_t boot_signature;
}__attribute__((packed));
struct fat32_bpb fat32_boot_sector;

dword_t fat_total_sectors=0;
dword_t fat_cluster_length=0;
dword_t fat_cluster_length_in_bytes=0;
dword_t fat_number_of_clusters=0;
dword_t fat_table_sector=0;
dword_t fat_root_dir_sector=0;
dword_t fat_root_dir_length=0;
dword_t fat_data_sector=0;

dword_t loaded_fat_sector=0;
dword_t fat_table_one_sector[256];

dword_t fat_file_length=0;

byte_t is_partition_fat(dword_t first_partition_sector);
void select_fat_partition(dword_t first_partition_sector);
void set_fat_partition_info_in_device_list_entry(void);
void read_fat_partition_info_from_device_list_entry(void);
dword_t get_fat_entry(dword_t entry);
byte_t set_fat_entry(dword_t entry, dword_t value);
byte_t save_fat_table_sector(void);
byte_t read_fat_cluster(dword_t cluster, dword_t memory);
byte_t write_fat_cluster(dword_t cluster, dword_t memory);
dword_t fat_get_size_of_file_on_disk(dword_t cluster);
dword_t read_fat_file(dword_t cluster, dword_t size_in_bytes);
dword_t write_fat_file(dword_t memory, dword_t size_in_bytes, dword_t first_cluster);
dword_t delete_fat_file(dword_t cluster);
dword_t read_fat_folder(dword_t cluster, dword_t size_in_bytes);
dword_t convert_folder_to_fat_folder(dword_t folder_mem);
dword_t rewrite_fat_folder(dword_t cluster, dword_t folder_mem);