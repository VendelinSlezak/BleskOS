//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t ext_one_sector[128];

dword_t ext_block_descriptor_first_sector = 0, ext_size_of_block_in_bytes = 0, ext_size_of_block_in_sectors = 0, ext_number_of_block_groups = 0, ext_number_of_blocks_in_group = 0, ext_number_of_inodes_in_group = 0, ext_inode_size_in_bytes = 0;
dword_t ext_version = 0, ext_file_size_in_bytes = 0, ext_block_group_descriptor_size_in_bytes = 0, ext_num_of_groups_per_flex_group = 0;
dword_t ext_max_number_of_blocks_per_one_read = 0, ext_how_many_bytes_to_read = 0, ext_pointer_to_file_in_memory = 0, ext_already_readed_bytes = 0, ext_indirect_block_memory = 0, ext_double_indirect_block_memory = 0, ext_triple_indirect_block_memory = 0;

byte_t is_partition_ext(dword_t first_partition_sector);
void select_ext_partition(dword_t first_partition_sector);
void set_ext_partition_info_in_device_list_entry(void);
void read_ext_partition_info_from_device_list_entry(void);
byte_t ext_read_block(dword_t block, dword_t memory);
byte_t ext_read_blocks(dword_t block, dword_t num_of_blocks, dword_t memory);
dword_t ext_read_inode(dword_t inode);
byte_t ext_read_indirect_block_pointers(dword_t block);
byte_t ext_read_double_indirect_block_pointers(dword_t block);
byte_t ext_read_triple_indirect_block_pointers(dword_t block);
dword_t ext_read_file(dword_t inode, dword_t size_in_bytes);
dword_t ext_read_folder(dword_t folder_inode, dword_t size_in_bytes);