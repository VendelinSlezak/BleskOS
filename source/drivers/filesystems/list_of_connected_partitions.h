//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct connected_partition_info_t {
 byte_t medium_type;
 byte_t medium_number;
 byte_t filesystem;
 dword_t first_sector;
 byte_t partition_label[11];
 byte_t *filesystem_specific_info_pointer;
}__attribute__((packed));

#define MAX_NUMBER_OF_CONNECTED_PARTITIONS 10
struct connected_partition_info_t *connected_partitions;

#define NO_PARTITION_SELECTED 0xFF
byte_t selected_partition;

void initalize_list_of_connected_partitions(void);
void add_new_partition_to_list(byte_t medium_type, byte_t medium_number, byte_t filesystem, dword_t first_sector);
void remove_partitions_of_medium_from_list(byte_t medium_type, byte_t medium_number);
byte_t is_partition_connected(byte_t medium_type, byte_t medium_number, byte_t filesystem, dword_t first_sector);
byte_t select_partition(byte_t partition_number);