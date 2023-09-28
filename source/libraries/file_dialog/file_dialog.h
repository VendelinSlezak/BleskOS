//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define FD_NO_DEVICE 0
#define FD_HARD_DISK 1
#define FD_CDROM 2
#define FD_USB_MSD 3

#define FD_DEVICE_ENTRY_LENGTH 40
#define FD_NUMBER_OF_DEVICE_ENTRIES 20
#define FD_NUMBER_OF_MAX_FOLDER_DEPTH 50

#define FD_DEVICE_ENTRY_OFFSET_MEDIUM 0
#define FD_DEVICE_ENTRY_OFFSET_MEDIUM_NUMBER 1
#define FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE 3
#define FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR 4
#define FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS 8
#define FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH 12 //TODO
#define FD_DEVICE_ENTRY_OFFSET_NAME 20

#define NO_FILE_SELECTED 0xFFFFFFFF

#define FD_STATE_NO_DEVICE 0
#define FD_STATE_FOLDER_LOADING 1
#define FD_STATE_FOLDER_LOADED 2
#define FD_STATE_FOLDER_LOADING_ERROR 3
#define FD_STATE_FILE_LOADING 4
#define FD_STATE_FILESYSTEM_UNKNOWN 5
#define FD_STATE_FILESYSTEM_ERROR 6
#define FD_STATE_FOLDER_WRITING 7
#define FD_STATE_FILE_WRITING 8

dword_t file_dialog_devices_mem=0, file_dialog_devices_mem2=0, file_dialog_folder_mem=0, file_dialog_folder_num_of_files=0, file_dialog_selected_file=0;
dword_t folder_path_memory=0, folder_path_depth=0, folder_open_file_extensions_mem=0;
byte_t file_dialog_state=FD_STATE_NO_DEVICE, file_dialog_num_of_devices=0, file_dialog_selected_device=0, file_dialog_save_return_state=0;
dword_t string_no_name_mem;

dword_t file_dialog_file_size = 0;
word_t file_dialog_file_name[128];
byte_t file_dialog_file_extension[10];

void initalize_file_dialog(void);
byte_t file_dialog_is_device_entry_present(byte_t medium, byte_t medium_number, byte_t partition_type, dword_t first_sector, dword_t num_of_sectors);
void file_dialog_update_devices(void);
void file_dialog_open_file_extensions_clear_mem(void);
void file_dialog_open_file_add_extension(byte_t *extension);
void file_dialog_load_folder(void);
void file_dialog_sort_folders_up(void);
void file_dialog_open_folder(dword_t file_entry, dword_t file_size);
void file_dialog_show_progress(void);
void file_dialog_save_set_extension(byte_t *extension);
void file_dialog_read_file_properties(dword_t folder_mem, dword_t file_number);
dword_t file_dialog_window(dword_t new_file_memory, dword_t new_file_size);
dword_t file_dialog_open(void);
dword_t file_dialog_save(dword_t new_file_memory, dword_t new_file_size);
