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

 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
 }

 while(1) {
  //read 100 sectors
  if(number_of_sectors>100) {
   if(read_audio_cd(sector, 100, memory_pointer)==STATUS_ERROR) {
    free(memory);
    return STATUS_ERROR;
   }
   number_of_sectors-=100;
   sector+=100;
   number_of_readed_sectors+=100;
   memory_pointer+=(2352*100);
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
 }
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