//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
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
#define STORAGE_CDDA 5
#define STORAGE_BLESKOS_BOOTABLE 6
#define STORAGE_UNKNOWN_FILESYSTEM 7
struct partition {
 byte_t type;
 dword_t first_sector;
 dword_t num_of_sectors;
}__attribute__((packed));
struct partition partitions[8];

#define ATTRIBUTE_DIRECTORY 0x10

#define NO_CONTROLLER 0
#define IDE_CONTROLLER 1
#define AHCI_CONTROLLER 2
struct storage_controller_info {
 byte_t controller_type;
 dword_t base_1;
 dword_t base_2;
}__attribute__((packed));
#define MAX_NUMBER_OF_STORAGE_CONTROLLERS 10
struct storage_controller_info storage_controllers[MAX_NUMBER_OF_STORAGE_CONTROLLERS];
dword_t number_of_storage_controllers;

struct storage_device_info {
 byte_t controller_type;
 byte_t device_port;
 dword_t base_1;
 dword_t base_2;
 dword_t number_of_sectors;
}__attribute__((packed));
struct storage_device_info hard_disk_info;
struct storage_device_info optical_drive_info;

struct ata_identify_command_device_info {
 byte_t padding_1[120];
 dword_t lba28_total_number_of_sectors;
 byte_t padding_2[76];
 qword_t lba48_total_number_of_sectors;
}__attribute__((packed));
struct ata_identify_command_device_info *device_info;

byte_t storage_medium, storage_medium_number, save_value_storage_medium, save_value_storage_medium_number, selected_partition_number;
byte_t one_sector[2048];
byte_t mbr_sector[512];
byte_t ebr_sector[512];
// dword_t optical_disk_table_of_content[128];
byte_t partition_label[12];
byte_t file_extension[10];
dword_t file_work_done_percents, file_show_file_work_progress;
dword_t hard_disk_size = 0;
dword_t optical_disk_size = 0, optical_disk_sector_size = 0;

struct table_of_content_entry {
 word_t unknown;
 word_t number;
 dword_t first_sector;
}__attribute__((packed));
struct table_of_content {
 word_t length;
 byte_t first_track;
 byte_t last_track;
 struct table_of_content_entry track[32];
}__attribute__((packed));
struct table_of_content optical_disk_table_of_content;

void initalize_storage_controllers(void);
byte_t if_storage_medium_exist(byte_t type_of_medium, byte_t medium_number);
void select_storage_medium(byte_t type_of_medium, byte_t medium_number);
byte_t read_storage_medium(dword_t sector, byte_t num_of_sectors, dword_t memory);
byte_t read_audio_cd(dword_t sector, dword_t num_of_sectors, dword_t memory);
byte_t write_storage_medium(dword_t sector, byte_t num_of_sectors, dword_t memory);
byte_t detect_optical_disk(void);
void eject_optical_disk(void);
void spin_down_optical_drive(void);
void reset_optical_drive(void);
byte_t read_optical_disk_toc(void);
void read_partition_info(void);
void select_partition(byte_t partition_number);
byte_t is_filesystem_read_write(byte_t filesystem_type);
dword_t read_file(dword_t file_starting_entry, dword_t file_size);
dword_t create_file(dword_t file_mem, dword_t size_of_file);
dword_t rewrite_file(dword_t file_starting_entry, dword_t file_size, dword_t file_mem, dword_t size_of_file);
dword_t delete_file(dword_t file_starting_entry);
dword_t read_folder(dword_t file_starting_entry, dword_t file_size);
dword_t create_folder(dword_t folder_mem);
dword_t rewrite_folder(dword_t folder_starting_entry, dword_t folder_mem);