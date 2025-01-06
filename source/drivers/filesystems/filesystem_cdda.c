//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_optical_disk_cdda(void) {
 byte_t sector[2352];
 return read_audio_cd(0, 1, (dword_t)(&sector)); //try to read first audio sector, if it succeeds, optical disk is CDDA
}

void filesystem_cdda_read_specific_info(struct connected_partition_info_t *connected_partition_info) {
 //set partition label
 copy_memory((dword_t)(&"Audio CD"), (dword_t)(&connected_partition_info->partition_label), sizeof("Audio CD"));
}

byte_t *read_cdda_file(dword_t file_location, dword_t file_size_in_bytes) {
 //create descriptor of file sectors
 struct byte_stream_descriptor_t *descriptor_of_file_sectors = create_descriptor_of_file_sectors(2352);

 //add sectors of file
 add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, file_location, (file_size_in_bytes/2352));

 //read file
 byte_t *file_memory = read_whole_descriptor_of_file_sector(descriptor_of_file_sectors);
 spin_down_optical_drive();

 //release memory
 destroy_byte_stream(descriptor_of_file_sectors);

 //return pointer to loaded data or STATUS_ERROR
 return file_memory;
}

byte_t *read_cdda_file_skipping_errors(dword_t file_location, dword_t file_size_in_bytes) {
 //allocate memory for file
 byte_t *file_memory = (byte_t *) calloc(file_size_in_bytes);
 dword_t file_memory_pointer = (dword_t) file_memory;

 //read all sectors without caring about errors
 keyboard_code_of_pressed_key = 0;
 dofs_full_task_number_of_bytes = file_size_in_bytes;
 dofs_task_completed_number_of_bytes = 0;
 for(dword_t i=0; i<(file_size_in_bytes/2352); i++) {
  //check if user cancelled request
  if(keyboard_code_of_pressed_key==KEY_ESC) {
   spin_down_optical_drive();
   free((dword_t)file_memory);
   return STATUS_ERROR;
  }

  //read one sector
  read_audio_cd(file_location, 1, file_memory_pointer);
  file_location++;
  file_memory_pointer+=2352;
  dofs_task_completed_number_of_bytes+=2352;
 }
 spin_down_optical_drive();

 //return pointer to file memory
 return file_memory;
}

byte_t *read_cdda_root_folder(void) {
 //we will create root folder from content of TOC
 if(read_optical_disk_toc()==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //allocate memory for virtual filesystem folder
 number_of_files_in_cdda_folder = optical_disk_table_of_content.last_track;
 struct file_descriptor_t *vfs_folder = (struct file_descriptor_t *) (calloc(sizeof(struct file_descriptor_t)*number_of_files_in_cdda_folder));

 //convert TOC to VFS folder
 for(dword_t i=0; i<number_of_files_in_cdda_folder; i++) {
  vfs_folder[i].partition_number = selected_partition;
  vfs_folder[i].type = NORMAL_FILE;
  vfs_folder[i].file_location = optical_disk_table_of_content.track[i].first_sector;
  vfs_folder[i].file_size_in_bytes = ((optical_disk_table_of_content.track[i+1].first_sector-optical_disk_table_of_content.track[i].first_sector)*2352);

  vfs_folder[i].name[0] = 'T';
  vfs_folder[i].name[1] = 'r';
  vfs_folder[i].name[2] = 'a';
  vfs_folder[i].name[3] = 'c';
  vfs_folder[i].name[4] = 'k';
  vfs_folder[i].name[5] = ' ';
  byte_t name_pointer = 0;
  if((i+1)<10) {
   vfs_folder[i].name[6] = ((i+1)+'0');
   name_pointer = 7;
  }
  else {
   vfs_folder[i].name[6] = (((i+1)/10)+'0');
   vfs_folder[i].name[7] = (((i+1)%10)+'0');
   name_pointer = 8;
  }
  vfs_folder[i].name[name_pointer+0] = '.';
  vfs_folder[i].name[name_pointer+1] = 'c';
  vfs_folder[i].name[name_pointer+2] = 'd';
  vfs_folder[i].name[name_pointer+3] = 'd';
  vfs_folder[i].name[name_pointer+4] = 'a';
  vfs_entry_parse_extension_from_name((struct file_descriptor_t *)(&vfs_folder[i]));
 }

 //return virtual file system folder
 return ((byte_t *)vfs_folder);
}