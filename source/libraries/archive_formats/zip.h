//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define ZIP_FILE_NOT_FOUNDED 0xFFFFFFFF

#define ZIP_NO_COMPRESSION 0
#define ZIP_COMPRESSION_DEFLATE 8

#define ZIP_LOCAL_FILE_HEADER_SIGNATURE 0x04034B50
#define ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE 0x02014B50
#define ZIP_END_OF_CENTRAL_DIRECTORY_SIGNATURE 0x06054B50

struct zip_local_file_header {
 dword_t signature;
 word_t version_for_extracting;
 word_t general_purpose_flag;
 word_t compression_method;
 word_t file_last_modification_time;
 word_t file_last_modification_date;
 dword_t crc32;
 dword_t uncompressed_size;
 dword_t compressed_size;
 word_t file_name_length;
 word_t extra_field_length;
}__attribute__((packed));

struct zip_central_directory_file_header {
 dword_t signature;
 word_t version_made_by;
 word_t version_for_extracting;
 word_t general_purpose_flag;
 word_t compression_method;
 word_t file_last_modification_time;
 word_t file_last_modification_date;
 dword_t crc32;
 dword_t uncompressed_size;
 dword_t compressed_size;
 word_t file_name_length;
 word_t extra_field_length;
 word_t file_comment_length;
 word_t disk_number_of_file;
 word_t internal_file_attributes;
 dword_t external_file_attributes;
 dword_t relative_offset_to_local_file_header;
}__attribute__((packed));

struct zip_end_of_central_directory {
 dword_t signature;
 word_t number_of_this_disk;
 word_t disk_of_central_directory;
 word_t number_of_central_directory_records_on_this_disk;
 word_t total_number_of_central_directory_records;
 dword_t size_of_central_directory;
 dword_t relative_offset_to_central_directory;
 word_t comment_length;
}__attribute__((packed));

dword_t zip_extracted_file_size = 0;
dword_t new_zip_file_pointer = 0, new_zip_file_size = 0, new_zip_actual_processed_file_number = 0, new_zip_number_of_files = 0, new_zip_size_of_central_directory = 0;
dword_t *new_zip_file_list_of_relative_offsets = 0;

byte_t is_this_zip(dword_t zip_file_memory, dword_t zip_file_size);
dword_t search_for_file_in_zip(dword_t zip_file_memory, dword_t zip_file_size, byte_t *file_name);
dword_t zip_extract_file(dword_t zip_file_memory, dword_t zip_file_size, dword_t entry_number);
dword_t create_zip_file(dword_t number_of_files_inside, dword_t size_of_all_files);
void zip_add_file(byte_t *name, dword_t memory, dword_t size);
void zip_add_central_directory_file_header(byte_t *name, dword_t memory, dword_t size);