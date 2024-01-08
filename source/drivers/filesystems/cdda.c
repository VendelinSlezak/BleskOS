//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_optical_disk_cdda(void) {
 //TODO: can be this tested more?
 dword_t one_audio_sector = malloc(2352);
 byte_t status = read_audio_cd(0, 1, one_audio_sector);
 free(one_audio_sector);
 return status;
}

void select_cdda_partition(void) {
 byte_t *audio_cd_string = "Audio CD   ";
 for(int i=0; i<11; i++) {
  partition_label[i]=audio_cd_string[i];
 }
}

dword_t cdda_read_file(dword_t sector, dword_t length_of_file_in_bytes) {
 dword_t memory = malloc(length_of_file_in_bytes+2352);
 dword_t memory_pointer = memory;
 dword_t number_of_sectors = (length_of_file_in_bytes/2352);
 dword_t full_number_of_sectors = number_of_sectors, number_of_readed_sectors = 0;

 //show progress
 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
  print("You can press ESC to stop reading", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, BLACK);
  redraw_part_of_screen(FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, 33*8, 8);
 }

 //prepare for possibility of keyboard event
 keyboard_value = 0;
 usb_keyboard_value = 0;

 //read file
 while(1) {
  //read 27 sectors, max value for one transfer request
  if(number_of_sectors>27) {
   if(read_audio_cd(sector, 27, memory_pointer)==STATUS_ERROR) {
    free(memory);
    return STATUS_ERROR;
   }
   number_of_sectors-=27;
   sector+=27;
   number_of_readed_sectors+=27;
   memory_pointer+=(2352*27);
  }
  else {
   if(read_audio_cd(sector, number_of_sectors, memory_pointer)==STATUS_ERROR) {
    free(memory);
    return STATUS_ERROR;
   }
   return memory;
  }

  //update reading progress
  file_work_done_percents = (100*number_of_readed_sectors/full_number_of_sectors);
  if(file_show_file_work_progress==1) {
   file_dialog_show_progress();
  }

  //check if there is not request to cancel reading
  if(keyboard_value==KEY_ESC || usb_keyboard_value==KEY_ESC) {
   free(memory);
   error_window("You cancelled reading");
   return STATUS_ERROR;
  }
 }
}

dword_t cdda_read_file_skipping_errors(dword_t sector, dword_t length_of_file_in_bytes) {
 dword_t memory = calloc(length_of_file_in_bytes+2352);
 dword_t memory_pointer = memory;
 dword_t number_of_sectors = (length_of_file_in_bytes/2352);
 dword_t full_number_of_sectors = number_of_sectors, number_of_readed_sectors = 0;
 dword_t number_of_bad_sectors = 0;

 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
  print("Number of readed sectors: 0", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, BLACK);
  print("Number of bad sectors: 0", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16, BLACK);
  print("Number of all sectors:", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16+16, BLACK);
  print_var(full_number_of_sectors, FILE_DIALOG_DEVICE_LIST_WIDTH+8+23*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16+16, BLACK);
  print("You can press ESC to stop reading", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16+16+16, BLACK);
  redraw_part_of_screen(FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, 40*8, 56);
 }

 //prepare for possibility of keyboard event
 keyboard_value = 0;
 usb_keyboard_value = 0;

 //read file
 for(dword_t i=0; i<number_of_sectors; i++) {
  //read one sector per request
  if(read_audio_cd(sector, 1, memory_pointer)==STATUS_ERROR) {
   //try read it again
   if(read_audio_cd(sector, 1, memory_pointer)==STATUS_ERROR) {
    number_of_bad_sectors++;
   }
  }
  sector++;
  number_of_readed_sectors++;
  memory_pointer+=2352;

  //update reading progress
  file_work_done_percents = (100*number_of_readed_sectors/full_number_of_sectors);
  if(file_show_file_work_progress==1) {
   file_dialog_show_progress();
   draw_full_square(FILE_DIALOG_DEVICE_LIST_WIDTH+8+26*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, 10*8, 8, 0xFF6600);
   print_var(number_of_readed_sectors, FILE_DIALOG_DEVICE_LIST_WIDTH+8+26*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, BLACK);
   redraw_part_of_screen(FILE_DIALOG_DEVICE_LIST_WIDTH+8+26*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8, 10*8, 8);
   draw_full_square(FILE_DIALOG_DEVICE_LIST_WIDTH+8+23*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16, 10*8, 8, 0xFF6600);
   print_var(number_of_bad_sectors, FILE_DIALOG_DEVICE_LIST_WIDTH+8+23*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16, BLACK);
   redraw_part_of_screen(FILE_DIALOG_DEVICE_LIST_WIDTH+8+23*8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+16+8+16, 10*8, 8);
  }

  //check if there is not request to cancel reading
  if(keyboard_value==KEY_ESC || usb_keyboard_value==KEY_ESC) {
   free(memory);
   error_window("You cancelled reading");
   return STATUS_ERROR;
  }
 }

 return memory;
}

dword_t cdda_read_root_folder(void) {
 //we create root folder from content of TOC
 if(read_optical_disk_toc()==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //allocate memory for folder
 dword_t root_folder_mem = calloc((optical_disk_table_of_content.last_track+1)*256);
 byte_t *folder8 = (byte_t *) (root_folder_mem);
 word_t *folder16 = (word_t *) (root_folder_mem);
 dword_t *folder32 = (dword_t *) (root_folder_mem+28);

 //convert tracks to folder entries
 for(dword_t track=0; track<optical_disk_table_of_content.last_track; track++) {
  //add name of entry
  folder8[0] = 'T';
  folder16[16] = 'T';
  folder16[17] = 'r';
  folder16[18] = 'a';
  folder16[19] = 'c';
  folder16[20] = 'k';
  folder16[21] = ' ';
  dword_t name_pointer = 22;
  track++;
  if(track<10) {
   folder16[name_pointer] = (track+'0');
   name_pointer++;
  }
  else {
   folder16[name_pointer] = ((track/10)+'0');
   name_pointer++;
   folder16[name_pointer] = ((track%10)+'0');
   name_pointer++;
  }
  track--;
  folder16[name_pointer] = '.';
  folder16[name_pointer+1] = 'c';
  folder16[name_pointer+2] = 'd';
  folder16[name_pointer+3] = 'd';
  folder16[name_pointer+4] = 'a';

  //first sector
  folder8[26]=(byte_t)(optical_disk_table_of_content.track[track].first_sector);
  folder8[27]=(byte_t)(optical_disk_table_of_content.track[track].first_sector>>8);
  folder8[20]=(byte_t)(optical_disk_table_of_content.track[track].first_sector>>16);
  folder8[21]=(byte_t)(optical_disk_table_of_content.track[track].first_sector>>24);
  
  //length of file
  folder32[0] = ((optical_disk_table_of_content.track[track+1].first_sector-optical_disk_table_of_content.track[track].first_sector)*2352);
  
  //set attributes
  folder8[11]=0x20; //normal file
  
  //next entry
  folder8 += 256;
  folder16 += 128;
  folder32 += 64;
 }

 return root_folder_mem;
}