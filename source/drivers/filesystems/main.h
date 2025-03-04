//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_LOADED_FOLDERS 1000
#define AOLF_FREE_ENTRY 0
#define AOLF_FOLDER_LOADED 1
struct array_of_loaded_folders_t {
 byte_t state;
 byte_t partition_number; //partition where folder is
 dword_t folder_location;

 byte_t *memory_of_entries;
 dword_t number_of_files;
 byte_t number_of_structures_using_this_folder;
}__attribute__((packed));
struct array_of_loaded_folders_t *array_of_loaded_folders;

#define MAX_NUMBER_OF_FOLDER_STRUCTURES 1000
dword_t number_of_folder_structures;
dword_t *array_of_folder_structures;

#define ROOT_FOLDER 0xFFFFFFFF

void initalize_drivers_of_filesystems(void);
void release_loaded_folder_in_array_from_usage_in_structure(word_t folder_number_in_array);
byte_t is_filesystem_read_write(byte_t filesystem);
word_t read_folder(dword_t partition_number, dword_t folder_location, dword_t folder_size);
byte_t rewrite_folder(dword_t partition_number, dword_t folder_location, dword_t previous_folder_location, struct file_descriptor_t *vfs_folder, dword_t number_of_files_in_vfs_folder);
dword_t create_folder(dword_t partition_number, dword_t previous_folder_location);
#define delete_folder(partition_number, file_location) delete_file(partition_number, file_location)
byte_t *read_file(dword_t partition_number, dword_t file_location, dword_t file_size);
dword_t rewrite_file(dword_t partition_number, dword_t file_location, byte_t *file_memory, dword_t file_size_in_bytes);
dword_t create_file(dword_t partition_number, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t delete_file(dword_t partition_number, dword_t file_location);