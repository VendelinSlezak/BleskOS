//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define FAT12 0
#define FAT16 1
#define FAT32 2
struct fat_specific_info_t {
 byte_t type; //FAT12/16/32
 dword_t total_number_of_sectors;

 dword_t fat_table_first_sector;
 dword_t fat_table_size_in_sectors;
 byte_t number_of_fat_tables;

 dword_t data_first_sector;
 word_t cluster_size_in_sectors;
 word_t cluster_size_in_bytes;
 dword_t number_of_clusters;

 dword_t loaded_fat_table_sector; //offset from start of table
 byte_t fat_table_sector[512];

 dword_t root_directory_location; //sector if FAT12/16 and first cluster if FAT32
 dword_t root_directory_size_in_sectors; //only FAT12/16

 dword_t file_ending_entry_value;

 dword_t first_free_cluster;
}__attribute__((packed));

struct fat16_bpb_t {
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

struct fat32_bpb_t {
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

#define FAT_NO_ATTRIBUTE 0
#define FAT_ATTRIBUTE_FOLDER 0x10
#define FAT_ATTRIBUTE_ARCHIVE 0x20
#define FAT_ATTRIBUTE_LONG_FILE_NAME 0x0F
struct fat_folder_entry_t {
 byte_t name[8];
 byte_t extension[3];
 byte_t attributes;
 word_t reserved;

 word_t second_of_creation: 5;
 word_t minute_of_creation: 6;
 word_t hour_of_creation: 5;
 word_t day_of_creation: 5;
 word_t month_of_creation: 4;
 word_t year_of_creation: 7;

 word_t day_of_last_access: 5;
 word_t month_of_last_access: 4;
 word_t year_of_last_access: 7;

 word_t high_word_of_cluster;

 word_t second_of_modification: 5;
 word_t minute_of_modification: 6;
 word_t hour_of_modification: 5;
 word_t day_of_modification: 5;
 word_t month_of_modification: 4;
 word_t year_of_modification: 7;

 word_t low_word_of_cluster;

 dword_t size_of_file_in_bytes;
}__attribute__((packed));

struct fat_folder_lfn_entry_t {
 byte_t order;
 word_t name_first_5_chars[5];
 byte_t attribute;
 byte_t entry_type;
 byte_t checksum;
 word_t name_second_6_chars[6];
 word_t reserved;
 word_t name_last_2_chars[2];
}__attribute__((packed));

//TODO:
dword_t size_of_loaded_fat_file_in_memory;
dword_t number_of_files_in_fat_folder;

byte_t is_partition_fat(dword_t first_partition_sector);
void filesystem_fat_read_specific_info(struct connected_partition_info_t *connected_partition_info);
byte_t load_fat_table_sector(dword_t sector);
byte_t save_loaded_fat_table_sector(void);
byte_t save_and_load_fat_table_sector(dword_t sector);
dword_t read_fat_table_entry(dword_t entry);
byte_t write_fat_table_entry(dword_t entry, dword_t value);
byte_t *read_fat_file(dword_t cluster);
#define FAT_FIND_FREE_CLUSTER 0
dword_t create_fat_file(dword_t first_cluster, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t delete_fat_file(dword_t cluster);
dword_t rewrite_fat_file(dword_t first_cluster, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t *read_fat_folder(dword_t cluster);
byte_t rewrite_fat_folder(dword_t folder_location, dword_t previous_folder_location, struct file_descriptor_t *vfs_folder, dword_t number_of_files_in_vfs_folder);
dword_t create_fat_folder(dword_t previous_folder_location);