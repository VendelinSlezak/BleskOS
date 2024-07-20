//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PROGRAM_INTERFACE_DRAW_METHOD_POINTER 0
#define PROGRAM_INTERFACE_FLAGS 1
#define PROGRAM_INTERFACE_NUMBER_OF_FILES 2
#define PROGRAM_INTERFACE_SELECTED_FILE 3
#define PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE 4
#define PROGRAM_INTERFACE_SELECTED_CLICK_ZONE 5
#define PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN 6
#define PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER 7

#define PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON (1<<0)
#define PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON (1<<1)
#define PROGRAM_INTERFACE_FLAG_NO_OPEN_AND_SAVE_BUTTON (1<<2)
#define PROGRAM_INTERFACE_FLAG_PERMANENT_FOCUS_ON_TEXT_AREA (1<<3)
#define PROGRAM_INTERFACE_FLAG_FOCUS_ON_TEXT_AREA_FROM_NEW_FILE_METHOD (1<<4)
#define PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED (1<<5)

#define PROGRAM_INTERFACE_FILE_FLAGS 0
#define PROGRAM_INTERFACE_FILE_DEVICE_TYPE 1
#define PROGRAM_INTERFACE_FILE_DEVICE_NUMBER 2
#define PROGRAM_INTERFACE_FILE_PARTITION 3
#define PROGRAM_INTERFACE_FILE_SIZE 4
#define PROGRAM_INTERFACE_FILE_DISK_POINTER 5
#define PROGRAM_INTERFACE_FILE_PROGRAM_AREA 6
#define PROGRAM_INTERFACE_FILE_NAME 54

#define PROGRAM_INTERFACE_FILE_FLAG_SAVED (1<<0)

dword_t create_program_interface_memory(dword_t draw_method_pointer, dword_t flags);
dword_t get_program_value(dword_t value_offset);
void set_program_value(dword_t value_offset, dword_t value);
dword_t get_position_of_file_memory(void);
dword_t get_specific_file_value(dword_t file_number, dword_t value_offset);
dword_t get_file_value(dword_t value_offset);
void set_file_value(dword_t value_offset, dword_t value);
word_t get_char_of_file_name(dword_t value_offset);
void set_char_of_file_name(dword_t value_offset, dword_t value);
void set_file_name_from_file_dialog(void);
void add_file(word_t *file_name, byte_t device_type, byte_t device_number, byte_t partition, dword_t file_size, dword_t file_disk_pointer);
void change_file_name_byte_string(byte_t *name);
void change_file_name_word_string(word_t *name);
void remove_file(dword_t file_number);