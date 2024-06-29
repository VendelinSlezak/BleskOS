//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct iso9660_specific_info_t {
 dword_t root_directory_sector;
 dword_t root_directory_size_in_bytes;
}__attribute__((packed));

#define ISO9660_VOLUME_DESCRIPTOR_BOOT_RECORD 0
#define ISO9660_PRIMARY_VOLUME_DESCRIPTOR 1
#define ISO9660_VOLUME_DESCRIPTOR_SET_TERMINATOR 255
struct iso9660_volume_descriptor_t {
 byte_t type;
 byte_t identifier[5];
 byte_t version;
 byte_t data[2041];
}__attribute__((packed));

struct iso9660_directory_entry_t {
 byte_t length_of_directory_record;
 byte_t extended_attribute_record_length;
 dword_t location_of_extent;
 dword_t location_of_extent_big_endian;
 dword_t data_length;
 dword_t data_length_big_endian;
 byte_t year_of_creation;
 byte_t month_of_creation;
 byte_t day_of_creation;
 byte_t hour_of_creation;
 byte_t minute_of_creation;
 byte_t second_of_creation;
 byte_t offset_from_gmt;
 byte_t file_flags;
 byte_t file_unit_size;
 byte_t interleave_gap_size;
 byte_t volume_sequence_number[4];
 byte_t length_of_file_identifier;
 byte_t file_identifier[];
}__attribute__((packed));

struct iso9660_primary_volume_descriptor_t {
 byte_t type;
 byte_t identifier[5];
 byte_t version;
 byte_t unused;
 byte_t system_identifier[32];
 byte_t volume_identifier[32];
 byte_t unused2[8];
 byte_t volume_space_size[8];
 byte_t unused3[32];
 byte_t volume_set_size[4];
 byte_t volume_sequence_number[4];
 word_t logical_block_size;
 word_t logical_block_size_big_endian;
 byte_t path_table_size[8];
 byte_t location_of_type_l_path_table[4];
 byte_t location_of_optional_type_l_path_table[4];
 byte_t location_of_type_m_path_table[4];
 byte_t location_of_optional_type_m_path_table[4];
 struct iso9660_directory_entry_t root_directory_entry;
 word_t reserved1;
 byte_t volume_set_identifier[128];
 byte_t publisher_identifier[128];
 byte_t data_preparer_identifier[128];
 byte_t application_identifier[128];
 byte_t copyright_file_identifier[37];
 byte_t abstract_file_identifier[37];
 byte_t bibliographic_file_identifier[37];
 byte_t volume_creation_date_and_time[17];
 byte_t volume_modification_date_and_time[17];
 byte_t volume_expiration_date_and_time[17];
 byte_t volume_effective_date_and_time[17];
 byte_t file_structure_version;
 byte_t unused4;
 byte_t application_used[512];
 byte_t reserved2[653];
}__attribute__((packed));

dword_t number_of_files_in_iso9660_folder;

byte_t is_partition_iso9660(dword_t first_partition_sector);
void filesystem_iso9660_read_specific_info(struct connected_partition_info_t *connected_partition_info);
byte_t *read_iso9660_file(dword_t file_location, dword_t file_size_in_bytes);
byte_t *read_iso9660_folder(dword_t folder_location, dword_t folder_size_in_bytes);