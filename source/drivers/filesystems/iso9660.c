//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_iso9660(dword_t first_partition_sector) {
 if(read_storage_medium(first_partition_sector+0x10, 1, (dword_t)one_sector)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 dword_t *one_sector_ptr = (dword_t *) (((dword_t)one_sector)+1);
 
 //test signature
 if(*one_sector_ptr==0x30304443) {
  return STATUS_TRUE;
 }
 
 return STATUS_FALSE;
}

void select_iso9660_partition(dword_t first_partition_sector) {
 for(int i=0; i<11; i++) {
  partition_label[i]=0;
 }

 for(int sector=0x10; sector<0x20; sector++) {
  if(read_storage_medium(first_partition_sector+sector, 1, (dword_t)one_sector)==STATUS_ERROR) {
   return;
  }
  if(one_sector[0]==1) {
   //this is primary volume record
   iso9660_root_dir_sector = ((one_sector[162]<<24) | (one_sector[163]<<16) | (one_sector[164]<<8) | (one_sector[165]));
   iso9660_root_dir_length = (((one_sector[170]<<24) | (one_sector[171]<<16) | (one_sector[172]<<8) | (one_sector[173]))/2048);
   for(int i=0; i<11; i++) {
    partition_label[i]=one_sector[40+i];
   }
   return;
  }
 }
}

void set_iso9660_partition_info_in_device_list_entry(void) {
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+0, iso9660_root_dir_sector);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+1, iso9660_root_dir_length);
}

void read_iso9660_partition_info_from_device_list_entry(void) {
 iso9660_root_dir_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+0);
 iso9660_root_dir_length = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+1);
}

dword_t iso9660_read_file(dword_t sector, dword_t length_of_file_in_bytes) {
 //initalize output for user
 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
 }
 
 //compute how many sectors will we read
 dword_t file_sectors = (length_of_file_in_bytes/2048);
 if((length_of_file_in_bytes%2048)!=0) {
  file_sectors++;
 }
 
 //allocate enough memory for file
 dword_t file_mem = calloc(2048*file_sectors);
 if(file_mem==0) {
  return STATUS_ERROR;
 }

 //set good value for balance between performance and informing user about progress
 byte_t how_many_sectors_read_at_once = 0;
 if(file_sectors<512) {
  how_many_sectors_read_at_once = 64;
 }
 else {
  how_many_sectors_read_at_once = 255;
 }
 
 //read file
 dword_t write_file_mem = file_mem, readed_sectors = 0, number_of_file_sectors = file_sectors;
 while(file_sectors>0) {
  if(file_sectors>how_many_sectors_read_at_once) {
   if(read_storage_medium(sector, how_many_sectors_read_at_once, write_file_mem)==STATUS_ERROR) {
    free(file_mem);
    return STATUS_ERROR;
   }
   else {
    file_work_done_percents = (100*readed_sectors/number_of_file_sectors);
    if(file_show_file_work_progress==1) {
     file_dialog_show_progress();
    }
   }
  
   sector+=how_many_sectors_read_at_once;
   readed_sectors+=how_many_sectors_read_at_once;
   file_sectors-=how_many_sectors_read_at_once;
   write_file_mem += (2048*how_many_sectors_read_at_once);
  }
  else {
   if(read_storage_medium(sector, file_sectors, write_file_mem)==STATUS_ERROR) {
    free(file_mem);
    return STATUS_ERROR;
   }

   break;
  }
 }
 
 return file_mem;
}

dword_t read_iso9660_folder(dword_t sector, dword_t length_of_folder_in_bytes) {
 //root directory
 if(sector==ROOT_DIRECTORY) {
  sector = iso9660_root_dir_sector;
  length_of_folder_in_bytes = iso9660_root_dir_length*2048;
 }
 
 //load folder
 dword_t iso9660_folder_mem = iso9660_read_file(sector, length_of_folder_in_bytes);
 if(iso9660_folder_mem==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //count folder entries
 byte_t *iso9660_folder = (byte_t *) iso9660_folder_mem;
 dword_t folder_entries = 0;
 for(int i=0; i<1000000; i++) {
  if(*iso9660_folder==0) {
   break;
  }
  folder_entries++;
  iso9660_folder += (*iso9660_folder);
 }
 
 //allocate memory for converted BleskOS virtual file system folder
 folder_entries++;
 dword_t folder_mem = calloc(folder_entries*256);
 
 //convert ISO9660 folder to BleskOS virtual file system folder
 iso9660_folder = (byte_t *) iso9660_folder_mem;
 byte_t *folder = (byte_t *) folder_mem;
 word_t date=0;
 for(int i=0; i<1000000; i++) {  
  if(iso9660_folder[0]==0) {
   break;
  }
  if(iso9660_folder[32]==0 || iso9660_folder[33]==0 || iso9660_folder[33]==1) { //skip these entries
   iso9660_folder += (iso9660_folder[0]);
   continue;
  }

  //copy name
  for(int j=0, name_length=iso9660_folder[32]; j<name_length; j++) {
   if(j<8) {
    folder[j] = iso9660_folder[33+j];
   }
   if(iso9660_folder[33+j]==';') {
    break; //end of name
   }
   folder[32+j*2] = iso9660_folder[33+j];
  }
  
  //copy starting sector
  folder[21]=iso9660_folder[6];
  folder[20]=iso9660_folder[7];
  folder[27]=iso9660_folder[8];
  folder[26]=iso9660_folder[9];
  
  //copy length of file
  folder[28]=iso9660_folder[17];
  folder[29]=iso9660_folder[16];
  folder[30]=iso9660_folder[15];
  folder[31]=iso9660_folder[14];
  
  //set attributes
  folder[11]=0x20; //normal file
  if((iso9660_folder[25] & 0x2)==0x2) {
   folder[11]=0x10; //directory
  }
  
  //set unused entries
  folder[12]=0;
  folder[13]=0;
  folder[14]=0;
  folder[15]=0;
  folder[22]=0;
  folder[23]=0;
  
  //set date entries
  date = (((iso9660_folder[18]-80)<<9) | ((iso9660_folder[19] & 0xF)<<5) | (iso9660_folder[20] & 0x1F));
  folder[16] = (date & 0xFF);
  folder[17] = ((date>>8) & 0xFF);
  folder[18] = (date & 0xFF);
  folder[19] = ((date>>8) & 0xFF);
  folder[24] = (date & 0xFF);
  folder[25] = ((date>>8) & 0xFF);

  //next entry
  iso9660_folder += (iso9660_folder[0]);
  folder += 256;
 }
 
 free(iso9660_folder_mem);
 return folder_mem;
}
