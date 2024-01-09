//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t compare_file_extension(dword_t folder_mem, dword_t entry_number, dword_t extension_memory, dword_t extension_length);
void get_file_extension(dword_t folder_mem, dword_t entry_number);
word_t get_char_of_file_from_folder_entry_name(dword_t folder_mem, dword_t entry_number, dword_t char_offset);
void set_char_of_file_from_folder_entry_name(dword_t folder_mem, dword_t entry_number, dword_t char_offset, word_t char_value);
byte_t is_loaded_file_extension(byte_t *extension);
dword_t get_file_attribute(dword_t folder_mem, dword_t entry_number);
dword_t get_file_starting_entry(dword_t folder_mem, dword_t entry_number);
dword_t get_file_size(dword_t folder_mem, dword_t entry_number);
void set_file_entry_size(dword_t folder_mem, dword_t entry_number, dword_t size);