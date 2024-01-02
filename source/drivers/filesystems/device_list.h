//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define DEVICE_LIST_SIZE_OF_ENTRY 256
#define DEVICE_LIST_NUMBER_OF_ENTRIES 20

#define DEVICE_LIST_ENTRY_DEVICE_TYPE 0
#define DEVICE_LIST_ENTRY_DEVICE_NUMBER 1
#define DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE 2
#define DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR 3
#define DEVICE_LIST_ENTRY_DEVICE_PARTITION_NAME 4
#define DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO 12

dword_t device_list_mem = 0;
dword_t device_list_selected_entry = 0;
dword_t number_of_device_list_entries = 0;

void initalize_device_list(void);
dword_t get_device_list_entry_value(dword_t offset);
void set_device_entry_list_value(dword_t offset, dword_t value);
dword_t get_device_list_entry_name_value(dword_t offset);
void set_device_entry_list_name_value(dword_t offset, word_t value);
void add_device_partition_to_device_list(dword_t device_type, dword_t device_number, dword_t partition_type, dword_t first_partition_sector);
void remove_device_from_device_list(dword_t device_type, dword_t device_number);
byte_t does_device_list_entry_exist(dword_t device_type, dword_t device_number, dword_t partition_type, dword_t first_partition_sector);
byte_t select_device_list_entry(dword_t device_type, dword_t device_number, dword_t partition_type, dword_t first_partition_sector);
void device_list_check_optical_drive(void);