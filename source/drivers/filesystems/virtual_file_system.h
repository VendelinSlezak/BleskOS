//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_FOLDERS_IN_PATH 50
#define VIEW_FOLDER_LIST 0
#define VIEW_FOLDER_ICONS 1
#define SORT_FOLDER_IN_ASCENDING_ORDER 0
#define SORT_FOLDER_IN_DESCENDING_ORDER 1
#define SORT_FOLDER_BY_NAME 0
#define SORT_FOLDER_BY_EXTENSION 1
#define SORT_FOLDER_BY_SIZE 2
#define SORT_FOLDER_BY_DATE_OF_CREATION 3
#define FOLDER_NO_ENTRY_SELECTED 0xFFFFFFFF
struct folder_descriptor_t {
 byte_t partition_number; //partition where folder is
 
 byte_t view_type; //VIEW_FOLDER_LIST or VIEW_FOLDER_ICONS
 byte_t sort_type: 7; //SORT_FOLDER_BY_NAME, SORT_FOLDER_BY_EXTENSION, SORT_FOLDER_BY_SIZE or SORT_FOLDER_BY_DATE_OF_CREATION
 byte_t sort_direction: 1; //SORT_FOLDER_IN_ASCENDING_ORDER or SORT_FOLDER_IN_DESCENDING_ORDER
 dword_t first_showed_entry;
 dword_t selected_entry;

 byte_t pointer_to_path;
 dword_t path_folder_locations[MAX_NUMBER_OF_FOLDERS_IN_PATH];
 dword_t path_folder_sizes[MAX_NUMBER_OF_FOLDERS_IN_PATH];
 word_t folder_number_in_array_of_loaded_folders;
}__attribute__((packed, aligned(4)));

#define NORMAL_FILE 0
#define FILE_FOLDER 1
struct file_descriptor_t {
 byte_t partition_number; //partition where file is
 byte_t type; //NORMAL_FILE or FILE_FOLDER
 dword_t file_location; //value for locating file on partition e.g. in FAT number of first cluster
 dword_t file_size_in_bytes;

 word_t year_of_creation: 14;
 word_t month_of_creation: 4;
 word_t day_of_creation: 5;
 word_t hour_of_creation: 5;
 word_t minute_of_creation: 6;
 word_t second_of_creation: 6;

 word_t year_of_modification: 14;
 word_t month_of_modification: 4;
 word_t day_of_modification: 5;
 word_t hour_of_modification: 5;
 word_t minute_of_modification: 6;
 word_t second_of_modification: 6;

 word_t name[256];
 word_t extension[10];
}__attribute__((packed, aligned(4)));
struct file_descriptor_t last_loaded_file_descriptor;

void vfs_entry_parse_extension_from_name(struct file_descriptor_t *file_descriptor);
byte_t vfs_does_file_have_this_extension(struct file_descriptor_t *file_descriptor, byte_t *extensions);

void vfs_sort_file_descriptors(struct file_descriptor_t *file_descriptor, dword_t number_of_descriptors, byte_t swap_descriptors(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2));
byte_t swap_descriptors_by_name_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_name_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_extension_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_extension_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_size_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_size_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_date_of_creation_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
byte_t swap_descriptors_by_date_of_creation_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2);
dword_t vfs_sort_folders_at_start(struct folder_descriptor_t *folder_path_structure);
void vfs_sort_folder(struct folder_descriptor_t *folder_path_structure);

struct folder_descriptor_t *vfs_create_folder_path_structure(byte_t partition_number);
struct file_descriptor_t *vfs_get_folder_data_pointer(struct folder_descriptor_t *folder_path_structure);
dword_t vfs_get_folder_number_of_files(struct folder_descriptor_t *folder_path_structure);
void vfs_destroy_folder_path_structure(struct folder_descriptor_t *folder_path_structure);

byte_t vfs_open_folder(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry);
byte_t vfs_go_back_in_folder_path(struct folder_descriptor_t *folder_path_structure);
byte_t vfs_save_folder(struct folder_descriptor_t *folder_path_structure);
byte_t vfs_create_folder(struct folder_descriptor_t *folder_path_structure, word_t *name);
byte_t vfs_delete_folder(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry);

byte_t *vfs_read_file(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry);
byte_t vfs_save_file(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t vfs_create_file(struct folder_descriptor_t *folder_path_structure, word_t *name, byte_t *extension, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t vfs_create_file_by_file_descriptor(struct folder_descriptor_t *folder_path_structure, struct file_descriptor_t file_descriptor, byte_t *file_memory);
byte_t vfs_delete_file(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry);

byte_t *vfs_read_file_show_progress(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry);
byte_t vfs_save_file_show_progress(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t vfs_create_file_show_progress(struct folder_descriptor_t *folder_path_structure, word_t *name, byte_t *extension, byte_t *file_memory, dword_t file_size_in_bytes);
byte_t vfs_create_file_by_file_descriptor_show_progress(struct folder_descriptor_t *folder_path_structure, struct file_descriptor_t file_descriptor, byte_t *file_memory);