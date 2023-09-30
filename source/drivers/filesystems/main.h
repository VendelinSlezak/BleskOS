//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define ROOT_DIRECTORY 0xFFFFFFFF

#define MEDIUM_HDD 1
#define MEDIUM_CDROM 2
#define MEDIUM_USB_MSD 3

#define DEFAULT_MEDIUM 0

#define STORAGE_NO_PARTITION 0
#define STORAGE_FREE_SPACE 1
#define STORAGE_FAT 2
#define STORAGE_EXT 3
#define STORAGE_ISO9660 4
#define STORAGE_UNKNOWN_FILESYSTEM 5
struct partition {
 byte_t type;
 dword_t first_sector;
 dword_t num_of_sectors;
};
struct partition partitions[8];

byte_t storage_medium, storage_medium_number, save_value_storage_medium, save_value_storage_medium_number, selected_partition_number;
byte_t one_sector[2048];
byte_t mbr_sector[512];
byte_t ebr_sector[512];
byte_t partition_label[12];
byte_t file_extension[10];
dword_t file_work_done_percents, file_show_file_work_progress;
dword_t hard_disk_size;

byte_t if_storage_medium_exist(byte_t type_of_medium, byte_t medium_number);
void select_storage_medium(byte_t type_of_medium, byte_t medium_number);
byte_t read_storage_medium(dword_t sector, byte_t num_of_sectors, dword_t memory);
byte_t write_storage_medium(dword_t sector, byte_t num_of_sectors, dword_t memory);
byte_t detect_optical_disk(void);
void eject_optical_disk(void);
void read_partition_info(void);
void read_global_partition_table(dword_t sector);
void select_partition(byte_t partition_number);
dword_t compare_file_extension(dword_t folder_mem, dword_t entry_number, dword_t extension_memory, dword_t extension_length);
void get_file_extension(dword_t folder_mem, dword_t entry_number);
byte_t is_loaded_file_extension(byte_t *extension);
dword_t get_file_attribute(dword_t folder_mem, dword_t entry_number);
dword_t get_file_starting_entry(dword_t folder_mem, dword_t entry_number);
dword_t get_file_size(dword_t folder_mem, dword_t entry_number);
void set_file_entry_size(dword_t folder_mem, dword_t entry_number, dword_t size);
dword_t read_file(dword_t file_starting_entry, dword_t file_size);
dword_t create_file(dword_t file_mem, dword_t size_of_file);
dword_t rewrite_file(dword_t file_starting_entry, dword_t file_size, dword_t file_mem, dword_t size_of_file);
dword_t delete_file(dword_t file_starting_entry);
dword_t read_folder(dword_t file_starting_entry, dword_t file_size);
dword_t create_folder(dword_t folder_mem);
dword_t rewrite_folder(dword_t folder_starting_entry, dword_t folder_mem);
