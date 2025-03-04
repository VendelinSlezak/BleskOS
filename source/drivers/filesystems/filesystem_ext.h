//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EXT_SIGNATURE 0xEF53
#define EXT_REQUIRED_FEATURE_DIR_TYPE_FIELD 0x2
struct ext_superblock_t {
 dword_t total_inodes;
 dword_t total_blocks;
 dword_t reserved_blocks;
 dword_t unallocated_blocks;
 dword_t unallocated_inodes;
 dword_t superblock_block_number;
 dword_t log_block_size;
 dword_t log_fragment_size;
 dword_t blocks_per_group;
 dword_t fragments_per_group;
 dword_t inodes_per_group;
 dword_t last_mount_time;
 dword_t last_written_time;
 word_t mount_count;
 word_t max_mount_count;
 word_t ext_signature;
 word_t file_system_state;
 word_t error_handling;
 word_t minor_version;
 dword_t last_check_time;
 dword_t check_interval;
 dword_t os_id;
 dword_t major_version;
 word_t user_id;
 word_t group_id;
 dword_t first_non_reserved_inode;
 word_t inode_size;
 word_t block_group;
 dword_t optional_features;
 dword_t required_features;
 dword_t read_only_features;
 byte_t file_system_id[16];
 byte_t volume_name[16];
 byte_t last_mounted_path[64];
 dword_t compression_algorithms;
 byte_t prealloc_blocks;
 byte_t prealloc_dir_blocks;
 word_t unused;
 byte_t journal_id[16];
 dword_t journal_inode;
 dword_t journal_device;
 dword_t orphan_inode_list_head;
 byte_t unused_padding[788];
}__attribute__((packed));

struct block_group_descriptor_t {
 dword_t block_usage_bitmap;
 dword_t inode_usage_bitmap;
 dword_t inode_table_first_block;
 word_t unallocated_blocks;
 word_t unallocated_inodes;
 word_t directories;
 byte_t unused[14];
}__attribute__((packed));

struct ext2_inode_t {
 word_t type_and_permissions;
 word_t user_id;
 dword_t size_lower;
 dword_t access_time;
 dword_t creation_time;
 dword_t modification_time;
 dword_t deletion_time;
 word_t group_id;
 word_t link_count;
 dword_t sector_count;
 dword_t flags;
 dword_t os_specific_1;
 dword_t direct_block_ptr[12];
 dword_t single_indirect_block_ptr;
 dword_t double_indirect_block_ptr;
 dword_t triple_indirect_block_ptr;
 dword_t generation_number;
 dword_t file_acl;
 dword_t dir_acl_or_upper_size;
 dword_t fragment_block;
 byte_t os_specific_2[12];
}__attribute__((packed));

struct ext_folder_entry_t {
 dword_t inode;
 word_t size_of_entry;
 byte_t name_length;
 byte_t type_indicator;
 byte_t name[];
}__attribute__((packed));

struct ext_specific_info_t {
 dword_t required_features;
 dword_t number_of_block_groups;
 dword_t number_of_inodes_in_group;

 dword_t block_size_in_bytes;
 word_t block_size_in_sectors;
 word_t inode_size_in_bytes;

 dword_t first_partition_sector;
 dword_t block_group_descriptor_first_sector;

 dword_t loaded_block_group_descriptor_sector;
 byte_t block_group_descriptors_sector[512];

 dword_t loaded_inode_table_sector;
 byte_t inode_table_sectors[1024];

 struct ext2_inode_t inode;
}__attribute__((packed));

dword_t size_of_loaded_ext_file_in_memory;
dword_t number_of_files_in_ext_folder;

byte_t is_partition_ext(dword_t first_partition_sector);
void filesystem_ext_read_specific_info(struct connected_partition_info_t *connected_partition_info);
byte_t read_ext_inode(dword_t inode);
byte_t *read_ext_file(dword_t file_location);
byte_t ext_add_single_indirect_pointer_blocks(struct byte_stream_descriptor_t *descriptor_of_file_sectors, dword_t block);
byte_t ext_add_double_indirect_pointer_blocks(struct byte_stream_descriptor_t *descriptor_of_file_sectors, dword_t block);
byte_t ext_add_triple_indirect_pointer_blocks(struct byte_stream_descriptor_t *descriptor_of_file_sectors, dword_t block);
byte_t *read_ext_folder(dword_t folder_location);