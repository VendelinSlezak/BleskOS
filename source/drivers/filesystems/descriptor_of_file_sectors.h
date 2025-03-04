//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USER_CAN_NOT_CANCEL_THIS_DOFS_ACTION 0
#define USER_CAN_CANCEL_THIS_DOFS_ACTION 1

struct file_sectors_run_t {
 dword_t first_sector_of_run;
 dword_t number_of_sectors_in_run;
}__attribute__((packed));

struct descriptor_of_file_sectors_t {
 word_t size_of_one_sector;
 dword_t number_of_sectors;
 dword_t number_of_runs;
 
 dword_t actual_run;
 dword_t processed_sectors_from_actual_run;

 struct file_sectors_run_t runs[];
}__attribute__((packed));

byte_t dofs_can_user_cancel_action;
dword_t dofs_task_completed_number_of_bytes, dofs_full_task_number_of_bytes;

struct byte_stream_descriptor_t *create_descriptor_of_file_sectors(word_t size_of_one_sector);
void add_sectors_to_descriptor_of_file_sectors(struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream, dword_t first_sector, dword_t number_of_sectors);
byte_t descriptor_of_file_sector_read_sectors(struct descriptor_of_file_sectors_t *descriptor_of_file_sectors, dword_t first_sector, dword_t number_of_sectors, byte_t *memory);
byte_t *read_whole_descriptor_of_file_sector(struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream);
byte_t write_whole_descriptor_of_file_sector(struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream, byte_t *file_memory);
void dofs_show_progress_in_reading(void);
void dofs_show_progress_in_writing(void);