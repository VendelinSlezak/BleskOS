//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define FILE_MANAGER_DEVICE_LIST_WIDTH 22*8

#define FILE_MANAGER_CLICK_ZONE_DEVICE_ENTRY 100
#define FILE_MANAGER_CLICK_ZONE_DEVICE_LAST_ENTRY 120
#define FILE_MANAGER_CLICK_ZONE_BUTTON_BACK 121
#define FILE_MANAGER_CLICK_ZONE_SCROLLBAR 122
#define FILE_MANAGER_CLICK_ZONE_BUTTON_REFRESH_DEVICES 123
#define FILE_MANAGER_CLICK_ZONE_RENAME_TEXT_AREA 124
#define FILE_MANAGER_CLICK_ZONE_BUTTON_COPY 200
#define FILE_MANAGER_CLICK_ZONE_BUTTON_PASTE 201
#define FILE_MANAGER_CLICK_ZONE_BUTTON_RENAME 202
#define FILE_MANAGER_CLICK_ZONE_BUTTON_DELETE 203
#define FILE_MANAGER_CLICK_ZONE_FIRST_FILE 1000

#define FILE_MANAGER_FILE_DEVICE_TYPE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)
#define FILE_MANAGER_FILE_DEVICE_NUMBER (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)
#define FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+3)
#define FILE_MANAGER_FILE_PATH_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+4)
#define FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+5)
#define FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+6)
#define FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+7)
#define FILE_MANAGER_FILE_FOLDER_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+8)
#define FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+9)
#define FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+10)
#define FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+11)
#define FILE_MANAGER_FILE_PATH_FOLDER_NAMES (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+12)

#define FILE_MANAGER_MAX_PATH_FOLDERS 50

dword_t file_manager_program_interface_memory = 0, file_manager_number_of_files_on_screen = 0, file_manager_number_of_chars_of_file_name = 0, file_manager_scrollbar_heigth = 0;
dword_t file_manager_copy_entry_memory = 0, file_manager_copied_file_memory = 0, file_manager_copied_file_size = 0, file_manager_file_copied = 0;
dword_t file_manager_rename_text_area = 0, file_manager_rename_window_showed = 0;

void initalize_file_manager(void);
void file_manager(void);
void draw_file_manager(void);
void file_manager_new_folder(void);
void file_manager_close_folder(void);
void file_manager_key_f8_event(void);
void file_manager_key_enter_event(void);
void file_manager_key_b_event(void);
void file_manager_key_c_event(void);
void file_manager_key_v_event(void);
void file_manager_key_up_event(void);
void file_manager_key_down_event(void);
void file_manager_verticall_scrollbar_event(void);
void file_manager_print_message(byte_t *message);
void file_manager_compute_number_of_files_in_directory(void);
void file_manager_copy_file(void);
void file_manager_paste_file(void);
void file_manager_rename_file(void);
void file_manager_delete_file(void);