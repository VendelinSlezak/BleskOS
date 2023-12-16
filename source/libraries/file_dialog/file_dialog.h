//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NO_FILE_SELECTED 0xFFFFFFFF
#define FILE_DIALOG_FOLDER_WAS_LOADED 0xFFFFFFFE

#define FILE_DIALOG_CLICK_ZONE_DEVICE_ENTRY 100
#define FILE_DIALOG_CLICK_ZONE_DEVICE_LAST_ENTRY 120
#define FILE_DIALOG_CLICK_ZONE_BUTTON_BACK 121
#define FILE_DIALOG_CLICK_ZONE_SCROLLBAR 122
#define FILE_DIALOG_CLICK_ZONE_BUTTON_REFRESH_DEVICES 123
#define FILE_DIALOG_CLICK_ZONE_NEW_NAME_TEXT_AREA 124
#define FILE_DIALOG_CLICK_ZONE_SAVE_BUTTON 125
#define FILE_DIALOG_CLICK_ZONE_FIRST_FILE 1000

#define FILE_DIALOG_TYPE_OPEN 1
#define FILE_DIALOG_TYPE_SAVE 2

#define FILE_DIALOG_DEVICE_LIST_WIDTH 22*8

#define FILE_DIALOG_MAX_PATH_FOLDERS 50

#define FILE_DIALOG_MAX_FILE_EXTENSION_LENGTH 10
#define FILE_DIALOG_MAX_OF_ALLOWED_EXTENSIONS 10

dword_t file_dialog_folder_device_type = 0;
dword_t file_dialog_folder_device_number = 0;
dword_t file_dialog_folder_device_partition_type = 0;
dword_t file_dialog_folder_device_partition_first_sector = 0;
dword_t file_dialog_folder_path_memory = 0;
dword_t file_dialog_path_folder_names_memory = 0;
dword_t file_dialog_folder_path_actual_folder_number = 0;
dword_t file_dialog_folder_vertical_scrollbar_rider_size = 0;
dword_t file_dialog_folder_vertical_scrollbar_position = 0;
dword_t file_dialog_folder_memory = 0;
dword_t file_dialog_folder_first_showed_entry = 0;
dword_t file_dialog_folder_selected_entry = 0;
dword_t file_dialog_folder_number_of_entries = 0;

dword_t file_dialog_file_size = 0;
word_t file_dialog_file_name[128];
word_t file_dialog_file_extension[128];
dword_t file_dialog_array_of_allowed_extension_mem = 0;
dword_t file_dialog_number_of_allowed_extensions = 0;

dword_t file_dialog_number_of_files_on_screen = 0, file_dialog_number_of_chars_of_file_name = 0, file_dialog_scrollbar_heigth = 0;
dword_t file_dialog_window_shown = 0;
dword_t file_dialog_new_name_text_area = 0;

void initalize_file_dialog(void);
void redraw_file_dialog(byte_t dialog_type);
void file_dialog_show_progress(void);
void file_dialog_print_message(byte_t *message);
void file_dialog_compute_number_of_files_in_directory(void);
void file_dialog_read_file_properties(dword_t folder_mem, dword_t file_number);
void file_dialog_open_file_extensions_clear_mem(void);
void file_dialog_open_file_add_extension(byte_t *extension);
void file_dialog_save_set_extension(byte_t *extension);
void file_dialog_process_key_up_key_down(byte_t dialog_type, dword_t key);
dword_t file_dialog_double_click_on_file(byte_t dialog_type, dword_t new_file_memory, dword_t new_file_size);
void file_dialog_folder_back(byte_t dialog_type);
byte_t file_dialog_save_file(byte_t dialog_type, dword_t new_file_memory, dword_t new_file_size);
dword_t file_dialog(byte_t dialog_type, dword_t new_file_memory, dword_t new_file_size);
dword_t file_dialog_open(void);
dword_t file_dialog_save(dword_t file_memory, dword_t file_size);