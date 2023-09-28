//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_file_dialog(void) { 
 //allocate memory
 file_dialog_devices_mem=calloc(FD_NUMBER_OF_DEVICE_ENTRIES*FD_DEVICE_ENTRY_LENGTH);
 file_dialog_devices_mem2=calloc(FD_NUMBER_OF_DEVICE_ENTRIES*FD_DEVICE_ENTRY_LENGTH);
 byte_t *devices_mem = (byte_t *) file_dialog_devices_mem;
 folder_path_memory=calloc(FD_NUMBER_OF_DEVICE_ENTRIES*FD_NUMBER_OF_MAX_FOLDER_DEPTH*8);
 folder_path_depth=0;
 folder_open_file_extensions_mem=calloc(10*11*2);
 
 //initalize folder path memory
 dword_t *folder_path_memory_ptr = (dword_t *) folder_path_memory;
 for(int i=0; i<FD_NUMBER_OF_DEVICE_ENTRIES; i++, folder_path_memory_ptr+=FD_NUMBER_OF_MAX_FOLDER_DEPTH*2) {
  folder_path_memory_ptr[0]=ROOT_DIRECTORY;
 }
 
 //initalize hard disk entries
 file_dialog_num_of_devices=0;
 if(if_storage_medium_exist(MEDIUM_HDD, DEFAULT_MEDIUM)==STATUS_TRUE) {
  select_storage_medium(MEDIUM_HDD, DEFAULT_MEDIUM);
  read_partition_info();
  
  for(int i=0; i<4; i++) {
   if(partitions[i].type!=STORAGE_NO_PARTITION && partitions[i].type!=STORAGE_FREE_SPACE) {
    select_partition(i);
    devices_mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM]=FD_HARD_DISK;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM_NUMBER]=DEFAULT_MEDIUM;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]=partitions[i].type;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+0]=partitions[i].first_sector;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+1]=partitions[i].first_sector>>8;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+2]=partitions[i].first_sector>>16;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+3]=partitions[i].first_sector>>24;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+0]=partitions[i].num_of_sectors;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+1]=partitions[i].num_of_sectors>>8;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+2]=partitions[i].num_of_sectors>>16;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+3]=partitions[i].num_of_sectors>>24;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+0]=0;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+1]=0;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+2]=0;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+3]=0;
    devices_mem[FD_DEVICE_ENTRY_OFFSET_NAME+0]='H';
    devices_mem[FD_DEVICE_ENTRY_OFFSET_NAME+1]='D';
    devices_mem[FD_DEVICE_ENTRY_OFFSET_NAME+2]='D';
    devices_mem[FD_DEVICE_ENTRY_OFFSET_NAME+3]=' ';
    for(int j=0; j<11; j++) {
     devices_mem[FD_DEVICE_ENTRY_OFFSET_NAME+4+j]=partition_label[j];
    }
    devices_mem[FD_DEVICE_ENTRY_OFFSET_NAME+15]=0;
    
    devices_mem += FD_DEVICE_ENTRY_LENGTH;
    file_dialog_num_of_devices++;
   }
  }
 }
 file_dialog_update_devices();

 //initalize values for displaying files
 file_dialog_type_of_displaying_files = FD_DISPLAY_FILES_LIST;
 file_dialog_folder_first_displayed_file_num = 0;
 file_dialog_selected_file = NO_FILE_SELECTED;
 file_dialog_highlighted_file = NO_FILE_SELECTED;
 file_dialog_list_num_of_files_on_screen = ((graphic_screen_y-80)/10);
 file_dialog_list_num_of_chars_of_name = ((graphic_screen_x-158-200)/8);
 file_dialog_icons_files_lines = (graphic_screen_y-72)/110;
 file_dialog_icons_files_columns = (graphic_screen_x-158-8)/80;
 
 //select first medium
 file_dialog_selected_device = 0;
 devices_mem = (byte_t *) file_dialog_devices_mem;
 if(devices_mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM]!=FD_NO_DEVICE) {
  select_storage_medium(devices_mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM], devices_mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM_NUMBER]);
  partitions[0].type=devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE];
  partitions[0].first_sector=devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR];
  partitions[0].num_of_sectors=devices_mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS];
  select_partition(0);
 }
 
 //create no name string for programs
 string_no_name_mem = malloc(16);
 word_t *string_ptr = (word_t *) string_no_name_mem;
 string_ptr[0] = 'N';
 string_ptr[1] = 'o';
 string_ptr[2] = ' ';
 string_ptr[3] = 'n';
 string_ptr[4] = 'a';
 string_ptr[5] = 'm';
 string_ptr[6] = 'e';
 string_ptr[7] = 0;
}

byte_t file_dialog_is_device_entry_present(byte_t medium, byte_t medium_number, byte_t partition_type, dword_t first_sector, dword_t num_of_sectors) {
 byte_t *mem = (byte_t *) file_dialog_devices_mem;
 
 for(int i=0; i<20; i++, mem += FD_DEVICE_ENTRY_LENGTH) {
  if(mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM]==medium && mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM_NUMBER]==medium_number && mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]==partition_type && mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR]==first_sector && mem[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS]==num_of_sectors) {
   value32 = i;
   return STATUS_TRUE;
  } 
 }
 
 return STATUS_FALSE;
}

void file_dialog_update_devices(void) {
 byte_t *mem = (byte_t *) file_dialog_devices_mem;
 byte_t *mem2 = (byte_t *) file_dialog_devices_mem2;
 byte_t num_of_entries=0;
 
 //save actual selected entry
 dword_t selected_entry_data = malloc(FD_DEVICE_ENTRY_LENGTH);
 copy_memory((((dword_t)mem)+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH), selected_entry_data, FD_DEVICE_ENTRY_LENGTH);
 
 //we will build entries in mem2, and we will copy entries from mem1 that are still connected to computer
 
 file_dialog_num_of_devices = 0;
 clear_memory((dword_t)mem2, FD_NUMBER_OF_DEVICE_ENTRIES*FD_DEVICE_ENTRY_LENGTH);
 
 //copy hard disk entries because they are still connected
 for(int i=0; i<FD_NUMBER_OF_DEVICE_ENTRIES; i++) {
  if(mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM]==FD_HARD_DISK) {
   copy_memory(((dword_t)mem), ((dword_t)mem2), FD_DEVICE_ENTRY_LENGTH);
   mem += FD_DEVICE_ENTRY_LENGTH;
   mem2 += FD_DEVICE_ENTRY_LENGTH;
   file_dialog_num_of_devices++;
  }
  else {
   break;
  }
 }
 
 //initalize CDROM entries
 if(if_storage_medium_exist(MEDIUM_CDROM, DEFAULT_MEDIUM)==STATUS_TRUE) {
  select_storage_medium(MEDIUM_CDROM, DEFAULT_MEDIUM);
  if(detect_optical_disk()==STATUS_TRUE) {
   read_partition_info();
   
   for(int i=0; i<4; i++) {
    if(file_dialog_is_device_entry_present(MEDIUM_CDROM, DEFAULT_MEDIUM, partitions[i].type, partitions[i].first_sector, partitions[i].num_of_sectors)==STATUS_TRUE) {
     copy_memory((file_dialog_devices_mem+value32*FD_DEVICE_ENTRY_LENGTH),((dword_t)mem2), FD_DEVICE_ENTRY_LENGTH);
     
     mem2 += FD_DEVICE_ENTRY_LENGTH;
     file_dialog_num_of_devices++;
    }
    else if(partitions[i].type!=STORAGE_NO_PARTITION && partitions[i].type!=STORAGE_FREE_SPACE) {
     select_partition(i);
     mem2[FD_DEVICE_ENTRY_OFFSET_MEDIUM]=FD_CDROM;
     mem2[FD_DEVICE_ENTRY_OFFSET_MEDIUM_NUMBER]=DEFAULT_MEDIUM;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]=partitions[i].type;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+0]=partitions[i].first_sector;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+1]=partitions[i].first_sector>>8;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+2]=partitions[i].first_sector>>16;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+3]=partitions[i].first_sector>>24;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+0]=partitions[i].num_of_sectors;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+1]=partitions[i].num_of_sectors>>8;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+2]=partitions[i].num_of_sectors>>16;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+3]=partitions[i].num_of_sectors>>24;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+0]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+1]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+2]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+3]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+0]='D';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+1]='I';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+2]='S';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+3]='K';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+4]=' ';
     for(int j=0; j<11; j++) {
      mem2[FD_DEVICE_ENTRY_OFFSET_NAME+5+j]=partition_label[j];
     }
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+16]=0;
    
     mem2 += FD_DEVICE_ENTRY_LENGTH;
     file_dialog_num_of_devices++;
    }
   }
  }
 }
 
 //initalize USB MSD entries
 for(int i=0; i<10;	i++) {
  if(usb_mass_storage_devices[i].type==USB_MSD_INITALIZED) {
   select_storage_medium(MEDIUM_USB_MSD, i);
   read_partition_info();
   
   for(int j=0; j<4; j++) {
    if(file_dialog_is_device_entry_present(MEDIUM_USB_MSD, i, partitions[j].type, partitions[j].first_sector, partitions[j].num_of_sectors)==STATUS_TRUE) {
     copy_memory((file_dialog_devices_mem+value32*FD_DEVICE_ENTRY_LENGTH), ((dword_t)mem2), FD_DEVICE_ENTRY_LENGTH);
     
     mem2 += FD_DEVICE_ENTRY_LENGTH;
     file_dialog_num_of_devices++;
    }
    else if(usb_mass_storage_devices[i].partitions_type[j]!=STORAGE_NO_PARTITION && usb_mass_storage_devices[i].partitions_type[j]!=STORAGE_FREE_SPACE) {
     select_partition(j);
     mem2[FD_DEVICE_ENTRY_OFFSET_MEDIUM]=FD_USB_MSD;
     mem2[FD_DEVICE_ENTRY_OFFSET_MEDIUM_NUMBER]=i;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]=usb_mass_storage_devices[i].partitions_type[j];
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+0]=usb_mass_storage_devices[i].partitions_first_sector[j];
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+1]=usb_mass_storage_devices[i].partitions_first_sector[j]>>8;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+2]=usb_mass_storage_devices[i].partitions_first_sector[j]>>16;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_FIRST_SECTOR+3]=usb_mass_storage_devices[i].partitions_first_sector[j]>>24;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+0]=usb_mass_storage_devices[i].partitions_num_of_sectors[j];
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+1]=usb_mass_storage_devices[i].partitions_num_of_sectors[j]>>8;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+2]=usb_mass_storage_devices[i].partitions_num_of_sectors[j]>>16;
     mem2[FD_DEVICE_ENTRY_OFFSET_PARTITION_NUM_OF_SECTORS+3]=usb_mass_storage_devices[i].partitions_num_of_sectors[j]>>24;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+0]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+1]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+2]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_FOLDER_PATH_DEPTH+3]=0;
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+0]='U';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+1]='S';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+2]='B';
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+3]=' ';
     for(int j=0; j<11; j++) {
      mem2[FD_DEVICE_ENTRY_OFFSET_NAME+4+j]=partition_label[j];
     }
     mem2[FD_DEVICE_ENTRY_OFFSET_NAME+15]=0;
     
     mem2 += FD_DEVICE_ENTRY_LENGTH;
     file_dialog_num_of_devices++;
    }
   }
  }
 }
 
 //move mem2 to mem1
 copy_memory(file_dialog_devices_mem2, file_dialog_devices_mem, FD_NUMBER_OF_DEVICE_ENTRIES*FD_DEVICE_ENTRY_LENGTH);
 
 //select same entry as before
 file_dialog_selected_device = 0;
 mem = (byte_t *) file_dialog_devices_mem;
 byte_t *selected_entry_data_mem = (byte_t *) selected_entry_data;
 for(int i=0, found=0; i<10; i++, mem+=FD_DEVICE_ENTRY_LENGTH) {
  if(mem[FD_DEVICE_ENTRY_OFFSET_MEDIUM]==FD_NO_DEVICE) {
   break;
  }
  
  found = 1;
  for(int j=0; j<16; j++) {
   if(mem[j]!=selected_entry_data_mem[j]) {
    found = 0;
    break;
   }
  }
  
  if(found==1) {
   mem = (byte_t *) file_dialog_devices_mem;
   file_dialog_selected_device = i;
   select_storage_medium(mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+0], mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+1]);
   partitions[0].type=mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+3];
   partitions[0].first_sector=(mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+4] | (mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+5]<<8) | (mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+6]<<16) | (mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+7]<<24));
   partitions[0].num_of_sectors=(mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+8] | (mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+9]<<8) | (mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+10]<<16) | (mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+11]<<24));
   select_partition(0);
   file_dialog_selected_file = NO_FILE_SELECTED;
   file_dialog_highlighted_file = NO_FILE_SELECTED;
   break;
  }
 }
 
 free(selected_entry_data);
}

void file_dialog_open_file_extensions_clear_mem(void) {
 clear_memory(folder_open_file_extensions_mem, 10*11*2);
}

void file_dialog_open_file_add_extension(byte_t *extension) {
 word_t *file_extension_mem = (word_t *) folder_open_file_extensions_mem;
 dword_t extension_length=0;
 
 //count extension length
 for(int i=0; i<10; i++) {
  if(extension[i]==0) {
   break;
  }
  extension_length++;
 }
 
 //max 10 entries
 for(int i=0; i<10; i++, file_extension_mem += 11) {
  if(*file_extension_mem==0) {
   *file_extension_mem = extension_length;
   file_extension_mem++;
   
   for(int j=0; j<extension_length; j++) {
    file_extension_mem[j] = extension[j];
   }
   
   return;
  }
 }
}

void file_dialog_load_folder(void) {
 byte_t *devices_mem = (byte_t *) file_dialog_devices_mem;
 dword_t *folder_path = (dword_t *) folder_path_memory;
 
 //free memory of previous folder
 if(file_dialog_folder_mem!=0) {
  free(file_dialog_folder_mem);
  file_dialog_folder_mem = 0;
 }
 
 //load folder
 if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_MEDIUM]==0) {
  file_dialog_state=FD_STATE_NO_DEVICE;
 }
 else if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+3]==STORAGE_UNKNOWN_FILESYSTEM) {
  file_dialog_state=FD_STATE_FILESYSTEM_UNKNOWN;
  file_dialog_folder_num_of_files = 0;
 }
 else {
  file_dialog_state=FD_STATE_FOLDER_LOADING;
  draw_file_dialog();
  redraw_screen();
  
  //get folder path
  dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
  folder_path_depth = devices_mem32[3];
  
  //read root directory
  if(folder_path_depth==0) {
   file_dialog_folder_mem = read_folder(ROOT_DIRECTORY, 0);
  }
  else { //read other directory
   file_dialog_folder_mem = read_folder(folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2], folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2+1]);
  }
  
  file_dialog_folder_num_of_files = 0;
  if(file_dialog_folder_mem==STATUS_ERROR) {
   file_dialog_state=FD_STATE_FOLDER_LOADING_ERROR;
  }
  else {
   file_dialog_state=FD_STATE_FOLDER_LOADED;
   file_dialog_folder_first_displayed_file_num = 0;
   file_dialog_selected_file = NO_FILE_SELECTED;
   file_dialog_highlighted_file = NO_FILE_SELECTED;
  }
  
  //count number of files
  dword_t *folder = (dword_t *) file_dialog_folder_mem;
  file_dialog_folder_num_of_files = 0;
  for(int i=0; i<100000; i++) {
   if(*folder==0) {
    break;
   }
  
   file_dialog_folder_num_of_files++;
   folder+=64;
  }
  
  //sort folder
  file_dialog_sort_folders_up();
 }
}

void file_dialog_sort_folders_up(void) {
 byte_t *folder_mem = (byte_t *) file_dialog_folder_mem;
 
 dword_t entry = malloc(256);
 
 for(int i=0; i<file_dialog_folder_num_of_files; i++) {
  //find folder
  dword_t folder_entry_num = 0xFFFFFFFF;
  for(int j=i; j<file_dialog_folder_num_of_files; j++) {
   if(folder_mem[j*256+32]==0) {
    break;
   }
   
   if(folder_mem[j*256+11]==0x10) {
    folder_entry_num = j;
    break;
   }
  }
  if(folder_entry_num==0xFFFFFFFF) {
   break; //no more folder entries
  }
  
  //swap
  if(folder_entry_num!=i) {
   copy_memory((file_dialog_folder_mem+i*256), entry, 256);
   copy_memory((file_dialog_folder_mem+folder_entry_num*256), (file_dialog_folder_mem+i*256), 256);
   copy_memory(entry, (file_dialog_folder_mem+folder_entry_num*256), 256);
  }
 }
 
 free(entry);
}

void file_dialog_open_folder(dword_t file_entry, dword_t file_size) {
 dword_t *folder_path = (dword_t *) folder_path_memory;
 dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
 
 if(devices_mem32[3]>=50) {
  error_window("You can not open more than 50 folders");
  file_dialog_state=FD_STATE_FOLDER_LOADED;
  redraw_file_dialog();
  return;
 }
    
 file_dialog_state=FD_STATE_FOLDER_LOADING;
 redraw_file_dialog();
      
 //add path
 devices_mem32[3]++;
 dword_t folder_path_depth = devices_mem32[3];
 folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2]=file_entry;
 folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2+1]=file_size;
     
 //load folder
 file_dialog_load_folder();
 if(file_dialog_state==FD_STATE_FOLDER_LOADING_ERROR) {
  error_window("Error during loading file");
  file_dialog_state=FD_STATE_FOLDER_LOADED;
  devices_mem32[3]--;
 }     
 redraw_file_dialog();
}

void file_dialog_show_progress(void) {
 draw_full_square(158, 60, (graphic_screen_x-158-8), 16, 0x884E10);
 draw_full_square(158, 60, ((graphic_screen_x-158-8)*file_work_done_percents/100), 16, 0x000CFF);
 if(file_work_done_percents<10) {
  print_var(file_work_done_percents, 142+((graphic_screen_x-158-8)/2), 64, BLACK);
  draw_char('%', 158+((graphic_screen_x-158-8)/2), 64, BLACK);
 }
 else {
  print_var(file_work_done_percents, 134+((graphic_screen_x-158-8)/2), 64, BLACK);
  draw_char('%', 158+((graphic_screen_x-158-8)/2), 64, BLACK);
 }
 redraw_part_of_screen(158, 60, (graphic_screen_x-158-8), 16);
}

void file_dialog_save_set_extension(byte_t *extension) {
 for(int i=0; i<10; i++) {
  file_dialog_file_extension[i]=0;
 }
 
 for(int i=0; i<10; i++) {
  if(extension[i]==0) {
   return;
  }
  file_dialog_file_extension[i]=extension[i];
 }
}

void file_dialog_read_file_properties(dword_t folder_mem, dword_t file_number) {
 word_t *folder16 = (word_t *) (folder_mem+(file_number*256)+32);
 dword_t *folder32 = (dword_t *) (folder_mem+(file_number*256));
 
 //file size
 file_dialog_file_size = folder32[7];
 
 //file name
 for(int i=0; i<128; i++) {
  file_dialog_file_name[i] = folder16[i];
  if(folder16[i]==0) {
   break;
  }
 }
 
 //file extension
 for(int i=0; i<128; i++) {
  if(folder16[i]=='.') {
   folder16++;
   for(int j=0; j<10; j++) {
    file_dialog_file_extension[j] = folder16[i+j];
    
    if(file_dialog_file_extension[j]<0x41) {
     file_dialog_file_extension[j]=0;
     break;
    }
    else if(file_dialog_file_extension[j]<0x61) { //convert to small chars
     file_dialog_file_extension[j] = (folder16[i+j]+0x20);
    }
   }
   break;
  }
 }
}

dword_t file_dialog_window(dword_t new_file_memory, dword_t new_file_size) {
 byte_t *devices_mem = (byte_t *) file_dialog_devices_mem;
 word_t *file_extension_mem = (word_t *) folder_open_file_extensions_mem;
 dword_t *folder_path = (dword_t *) folder_path_memory;
 dword_t *devices_mem32;
 file_show_file_work_progress = 0;
 
 //TODO: test optical disk
 for(int i=0; i<FD_NUMBER_OF_DEVICE_ENTRIES; i++) {
  if(devices_mem[i*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_MEDIUM]==FD_CDROM) {
   if(detect_optical_disk()==STATUS_FALSE) {
    file_dialog_update_devices();
   }
   break;
  }
 }
 
 //load folder
 file_dialog_load_folder();
 draw_file_dialog();
     
 //initalize mouse cursor
 mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
 draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
 
 //redraw screen
 redraw_screen();
 
 while(1) {
  wait_for_usb_mouse();
  move_mouse_cursor();

  if(keyboard_value==KEY_F12 || usb_new_device_detected==1) {
   file_dialog_update_devices();
   file_dialog_load_folder();
   draw_file_dialog();
   redraw_screen();
   usb_new_device_detected=0;
   continue;
  }
  else if(keyboard_value==KEY_ESC) {
   return 0;
  }
  else if(keyboard_value==KEY_PAGE_UP && file_dialog_selected_device>0) {
   file_dialog_selected_device--;
   select_storage_medium(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+0], devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+1]);
   partitions[0].type=devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+3];
   partitions[0].first_sector=(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+4] | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+5]<<8) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+6]<<16) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+7]<<24));
   partitions[0].num_of_sectors=(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+8] | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+9]<<8) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+10]<<16) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+11]<<24));
   select_partition(0);
   file_dialog_selected_file = NO_FILE_SELECTED;
   file_dialog_highlighted_file = NO_FILE_SELECTED;
     
   file_dialog_load_folder();
   redraw_file_dialog();
   continue;
  }
  else if(keyboard_value==KEY_PAGE_DOWN && file_dialog_num_of_devices!=0 && file_dialog_selected_device<(file_dialog_num_of_devices-1)) {
   file_dialog_selected_device++;
   select_storage_medium(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+0], devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+1]);
   partitions[0].type=devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+3];
   partitions[0].first_sector=(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+4] | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+5]<<8) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+6]<<16) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+7]<<24));
   partitions[0].num_of_sectors=(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+8] | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+9]<<8) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+10]<<16) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+11]<<24));
   select_partition(0);
   file_dialog_selected_file = NO_FILE_SELECTED;
   file_dialog_highlighted_file = NO_FILE_SELECTED;
     
   file_dialog_load_folder();
   redraw_file_dialog();
   continue;
  }
  
  //with this state you can only refresh devices
  if(file_dialog_state==FD_STATE_NO_DEVICE) {
   continue;
  }
  
  if(file_dialog_state==FD_STATE_FOLDER_LOADED || file_dialog_state==FD_STATE_FOLDER_LOADING_ERROR) {
   if(keyboard_value==KEY_B) {
    dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
    if(devices_mem32[3]!=0) {
     devices_mem32[3]--;
     file_dialog_load_folder();
     redraw_file_dialog();
     continue;
    }
   }
  }
  
  //with this state files are shown
  if(file_dialog_state==FD_STATE_FOLDER_LOADED && file_dialog_folder_num_of_files>0) {
   //TODO: works only in list state
   if(keyboard_value==KEY_UP) {
    if(file_dialog_selected_file==NO_FILE_SELECTED) {
     file_dialog_selected_file=file_dialog_folder_num_of_files-1;
     if(file_dialog_selected_file<file_dialog_list_num_of_files_on_screen) {
      file_dialog_folder_first_displayed_file_num=0;
      file_dialog_highlighted_file=file_dialog_folder_num_of_files-1;
     }
     else {
      file_dialog_folder_first_displayed_file_num=(file_dialog_folder_num_of_files-file_dialog_list_num_of_files_on_screen);
      file_dialog_highlighted_file=(file_dialog_list_num_of_files_on_screen-1);
     }
     redraw_file_dialog();
     continue;
    }
    else if(file_dialog_selected_file>0 && file_dialog_folder_num_of_files!=0) {
     file_dialog_selected_file--;
     if(file_dialog_highlighted_file>0) {
      file_dialog_highlighted_file--;
     }
     else {
      file_dialog_folder_first_displayed_file_num--;
     }
     redraw_file_dialog();
     continue;
    }
   }
   
   if(keyboard_value==KEY_DOWN) {
    if(file_dialog_selected_file==NO_FILE_SELECTED) {
     file_dialog_selected_file=file_dialog_folder_first_displayed_file_num;
     file_dialog_highlighted_file=0;
     redraw_file_dialog();
    }
    else if(file_dialog_folder_num_of_files!=0 && file_dialog_selected_file<(file_dialog_folder_num_of_files-1)) {
     file_dialog_selected_file++;
     if(file_dialog_highlighted_file<(file_dialog_list_num_of_files_on_screen-1)) {
      file_dialog_highlighted_file++;
     }
     else {
      file_dialog_folder_first_displayed_file_num++;
     }
     redraw_file_dialog();
    }
   }
   
   //TODO: with icons shown all keys
   
   if(keyboard_value==KEY_ENTER && file_dialog_highlighted_file!=NO_FILE_SELECTED) {
    dword_t old_file_entry = get_file_starting_entry(file_dialog_folder_mem, file_dialog_selected_file);
    dword_t old_file_size = get_file_size(file_dialog_folder_mem, file_dialog_selected_file);
        
    //check if this is not folder
    if(get_file_attribute(file_dialog_folder_mem, file_dialog_selected_file)==0x10) {
     file_dialog_open_folder(old_file_entry, old_file_size);
     continue;
    }
        
    //click on file
    if(file_dialog_window_type==FD_TYPE_OPEN) { //OPEN FILE DIALOG
     //check if this is file with allowed extension
     if(file_extension_mem[0]!=0) { //0 = no extensions, open all files
      for(int i=0; i<10; i++) {
       if(file_extension_mem[i*11]==0) {
        error_window("This file type can not be opened in this program");
        file_dialog_state=FD_STATE_FOLDER_LOADED;
        break;
       }
       
       if(compare_file_extension(file_dialog_folder_mem, file_dialog_selected_file, folder_open_file_extensions_mem+i*22+2, file_extension_mem[i*11])==STATUS_TRUE) {
        file_dialog_state=FD_STATE_FILE_LOADING;
        break;
       }
      }
      if(file_dialog_state==FD_STATE_FOLDER_LOADED) {
       redraw_file_dialog();
       continue;
      }
     }
        
     file_dialog_state=FD_STATE_FILE_LOADING;
     redraw_file_dialog();

     file_show_file_work_progress = 1;
     dword_t file_memory = read_file(old_file_entry, old_file_size);
     file_show_file_work_progress = 0;
     if(file_memory==STATUS_ERROR) {
      error_window("Error during loading file");
      file_dialog_state=FD_STATE_FOLDER_LOADED;
      redraw_file_dialog();
      continue;
     }
     else {
      file_dialog_read_file_properties(file_dialog_folder_mem, file_dialog_selected_file);
      return file_memory;
     }
    }
    else if(file_dialog_window_type==FD_TYPE_SAVE) { //SAVE FILE DIALOG
     //if read-only filesystem
     if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]!=STORAGE_FAT) {
      error_window("This filesystem is read-only");
      file_dialog_state=FD_STATE_FOLDER_LOADED;
      redraw_file_dialog();
      continue;
     }
    
     //rewrite
     file_dialog_state=FD_STATE_FILE_WRITING;
     redraw_file_dialog();
     file_show_file_work_progress = 1;
     dword_t file_memory = rewrite_file(old_file_entry, old_file_size, new_file_memory, new_file_size);
     file_show_file_work_progress = 0;
     if(file_memory==STATUS_ERROR) {
      error_window("Error during rewriting file");
      file_dialog_state=FD_STATE_FOLDER_LOADED;
      redraw_file_dialog();
      continue;
     }
     else {
      //update entry and save folder
      //TODO: time of modification
      set_file_entry_size(file_dialog_folder_mem, file_dialog_selected_file, new_file_size);
      
      dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
      folder_path_depth = devices_mem32[3];
      if(rewrite_folder(folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2], file_dialog_folder_mem)==STATUS_ERROR) {
       error_window("Error during rewriting folder");
       file_dialog_state=FD_STATE_FOLDER_LOADED;
       redraw_file_dialog();
       continue;
      }
      
      //set informations about saved file
      file_dialog_save_return_state = STATUS_GOOD;
      file_dialog_read_file_properties(file_dialog_folder_mem, file_dialog_selected_file);
      
      return STATUS_GOOD;
     }
    }
   }
  }
  
  if(keyboard_value==KEY_S && file_dialog_window_type==FD_TYPE_SAVE) {
   //if read-only filesystem
   if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]!=STORAGE_FAT) {
    error_window("This filesystem is read-only");
    file_dialog_state=FD_STATE_FOLDER_LOADED;
    redraw_file_dialog();
    continue;
   }
   
   dword_t file_name_text_area = create_text_area(TEXT_AREA_INPUT_LINE, 50, graphic_screen_x-411, 5, 401, 10);
   while(1) {
    //TODO: with mouse cursor
    draw_text_area(file_name_text_area);
    redraw_part_of_screen(graphic_screen_x-411, 5, 403, 12);
    wait_for_usb_keyboard();
    if(keyboard_value==KEY_ESC) {
     delete_text_area(file_name_text_area);
     break;
    }
    else if(keyboard_value==KEY_ENTER) {
     //save file
     file_dialog_state=FD_STATE_FILE_WRITING;
     redraw_file_dialog();
     file_show_file_work_progress = 1;
     dword_t file_memory = create_file(new_file_memory, new_file_size);
     file_show_file_work_progress = 0;
     if(file_memory==STATUS_ERROR) {
      error_window("Error during rewriting file");
      file_dialog_state=FD_STATE_FOLDER_LOADED;
      redraw_file_dialog();
      continue;
     }
    
     //create new entry in folder
     file_dialog_folder_mem = realloc(file_dialog_folder_mem, (file_dialog_folder_num_of_files+2)*256);
     byte_t *folder_new_entry8 = (byte_t *) (file_dialog_folder_mem+file_dialog_folder_num_of_files*256);
     word_t *folder_new_entry16 = (word_t *) (file_dialog_folder_mem+file_dialog_folder_num_of_files*256);
     dword_t *folder_new_entry32 = (dword_t *) (file_dialog_folder_mem+file_dialog_folder_num_of_files*256);
     dword_t *file_name_text_area_ptr = (dword_t *) file_name_text_area;
     word_t *file_name_text_area_data = (word_t *) (file_name_text_area_ptr[0]);
     //short name
     for(int i=0; i<8; i++) {
      folder_new_entry8[i] = file_name_text_area_data[i];
     }
     //short extension
     folder_new_entry8[8] = file_dialog_file_extension[0];
     folder_new_entry8[9] = file_dialog_file_extension[1];
     folder_new_entry8[10] = file_dialog_file_extension[2];
     //attribute archive
     folder_new_entry8[11] = 0x20;
     //not used
     folder_new_entry8[12] = 0;
     folder_new_entry8[13] = 0;
     //time hour/minute/second
     read_time();
     word_t time_hms = (time_hour<<11 | time_minute<<5 | time_second);
     folder_new_entry16[7] = time_hms;
     folder_new_entry16[11] = time_hms;
     //time year/month/day
     word_t time_ymd = ((time_year-1980)<<9 | time_month<<5 | time_day);
     folder_new_entry16[8] = time_ymd;
     folder_new_entry16[9] = time_ymd;
     folder_new_entry16[12] = time_ymd;
     //file cluster
     folder_new_entry16[10] = (file_memory>>16);
     folder_new_entry16[13] = (file_memory & 0xFFFF);
     //file length
     folder_new_entry32[7] = new_file_size;
     //file name and extension
     for(int i=0; i<50; i++) {
      if(file_name_text_area_data[i]==0) {
       folder_new_entry16[i+16] = '.';
       folder_new_entry16++;
       for(int j=0; j<10; j++) {
        if(file_dialog_file_extension[j]==0) {
         break;
        }
        folder_new_entry16[i+16+j] = file_dialog_file_extension[j];
       }
       break;
      }
      folder_new_entry16[i+16] = file_name_text_area_data[i];
     }
     delete_text_area(file_name_text_area);
     file_dialog_folder_num_of_files++;
     
     //rewrite folder
     dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
     folder_path_depth = devices_mem32[3];
     if(rewrite_folder(folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2], file_dialog_folder_mem)==STATUS_ERROR) {
      error_window("Error during rewriting folder");
      file_dialog_state=FD_STATE_FOLDER_LOADED;
      break;
     }
     
     //set informations about saved file
     file_dialog_save_return_state = STATUS_GOOD;
     file_dialog_read_file_properties(file_dialog_folder_mem, (file_dialog_folder_num_of_files-1));
     
     return STATUS_GOOD;
    }
    text_area_keyboard_event(file_name_text_area);
   }
   redraw_file_dialog();
   continue;
  }
  
  //click
  if(mouse_drag_and_drop==MOUSE_CLICK) {   
   //devices
   if(is_mouse_in_zone(48, (graphic_screen_y-32), 0, 150)==STATUS_TRUE) {
    mouse_drag_and_drop = MOUSE_DRAG;
      
    if(((mouse_cursor_y-48)/25)<file_dialog_num_of_devices) {
     if(file_dialog_selected_device==((mouse_cursor_y-48)/25)) {
      continue;
     }
    
     file_dialog_selected_device = ((mouse_cursor_y-48)/25);
     select_storage_medium(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+0], devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+1]);
     partitions[0].type=devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+3];
     partitions[0].first_sector=(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+4] | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+5]<<8) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+6]<<16) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+7]<<24));
     partitions[0].num_of_sectors=(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+8] | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+9]<<8) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+10]<<16) | (devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+11]<<24));
     select_partition(0);
     file_dialog_selected_file = NO_FILE_SELECTED;
     file_dialog_highlighted_file = NO_FILE_SELECTED;
     
     file_dialog_load_folder();
     redraw_file_dialog();
     continue;
    }
   }
  
   //file zone
   if(file_dialog_state==FD_STATE_FOLDER_LOADED && file_dialog_folder_num_of_files>0) {
    if(is_mouse_in_zone(48, (graphic_screen_y-32), 158, (graphic_screen_x-8))==STATUS_TRUE) {
     mouse_drag_and_drop = MOUSE_DRAG;
     
     if(file_dialog_type_of_displaying_files==FD_DISPLAY_FILES_LIST) {
      if(((mouse_cursor_y-48)/10)<file_dialog_folder_num_of_files) { //click on file
       if(((mouse_cursor_y-48)/10)==file_dialog_highlighted_file) {
        dword_t old_file_entry = get_file_starting_entry(file_dialog_folder_mem, file_dialog_selected_file);
        dword_t old_file_size = get_file_size(file_dialog_folder_mem, file_dialog_selected_file);
        
        //check if this is not folder
        if(get_file_attribute(file_dialog_folder_mem, file_dialog_selected_file)==0x10) {
         file_dialog_open_folder(old_file_entry, old_file_size);
         continue;
        }
        
        if(file_dialog_window_type==FD_TYPE_OPEN) { //OPEN FILE DIALOG
         //check if this is file with allowed extension
         if(file_extension_mem[0]!=0) { //0 = no extensions, open all files
          for(int i=0; i<10; i++) {
           if(file_extension_mem[i*11]==0) {
            error_window("This file type can not be opened in this program");
            file_dialog_state=FD_STATE_FOLDER_LOADED;
            break;
           }
       
           if(compare_file_extension(file_dialog_folder_mem, file_dialog_selected_file, folder_open_file_extensions_mem+i*22+2, file_extension_mem[i*11])==STATUS_TRUE) {
            file_dialog_state=FD_STATE_FILE_LOADING;
            break;
           }
          }
          if(file_dialog_state==FD_STATE_FOLDER_LOADED) {
           redraw_file_dialog();
           continue;
          }
         }
         
         file_dialog_state=FD_STATE_FILE_LOADING;
         redraw_file_dialog();

         file_show_file_work_progress = 1;
         dword_t file_memory = read_file(old_file_entry, old_file_size);
         file_show_file_work_progress = 0;
         if(file_memory==STATUS_ERROR) {
          error_window("Error during loading file");
          file_dialog_state=FD_STATE_FOLDER_LOADED;
          redraw_file_dialog();
          continue;
         }
         else {
          file_dialog_read_file_properties(file_dialog_folder_mem, file_dialog_selected_file);
          return file_memory;
         }
        }
        else if(file_dialog_window_type==FD_TYPE_SAVE) { //SAVE FILE DIALOG
         //if read-only filesystem
         if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]!=STORAGE_FAT) {
          error_window("This filesystem is read-only");
          file_dialog_state=FD_STATE_FOLDER_LOADED;
          redraw_file_dialog();
          continue;
         }
    
         //rewrite
         file_dialog_state=FD_STATE_FILE_WRITING;
         redraw_file_dialog();
         file_show_file_work_progress = 1;
         dword_t file_memory = rewrite_file(old_file_entry, old_file_size, new_file_memory, new_file_size);
         file_show_file_work_progress = 0;
         if(file_memory==STATUS_ERROR) {
          error_window("Error during rewriting file");
          file_dialog_state=FD_STATE_FOLDER_LOADED;
          redraw_file_dialog();
          continue;
         }
         else {
          //update entry and save folder
          //TODO: time of modification
          set_file_entry_size(file_dialog_folder_mem, file_dialog_selected_file, new_file_size);
      
          dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
          folder_path_depth = devices_mem32[3];
          if(rewrite_folder(folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2], file_dialog_folder_mem)==STATUS_ERROR) {
           error_window("Error during rewriting folder");
           file_dialog_state=FD_STATE_FOLDER_LOADED;
           redraw_file_dialog();
           continue;
          }
          
          //set informations about saved file
          file_dialog_save_return_state = STATUS_GOOD;
          file_dialog_read_file_properties(file_dialog_folder_mem, file_dialog_selected_file);
      
          return STATUS_GOOD;
         }
        }
       }
       else {
        file_dialog_highlighted_file = ((mouse_cursor_y-48)/10);
        file_dialog_selected_file = (file_dialog_folder_first_displayed_file_num+file_dialog_highlighted_file);
        redraw_file_dialog();
        continue;
       }
      }
      else if(file_dialog_highlighted_file!=NO_FILE_SELECTED) { //click not on file
       file_dialog_selected_file = NO_FILE_SELECTED;
       file_dialog_highlighted_file = NO_FILE_SELECTED;
       redraw_file_dialog();
       continue;
      }
     }
     else if(file_dialog_type_of_displaying_files==FD_DISPLAY_FILES_ICONS) {
      dword_t fd_icons_selected_file = (((mouse_cursor_y-40)/110)*file_dialog_icons_files_columns)+((mouse_cursor_x-158)/80);
      
      //click on file
      if(mouse_cursor_x<(158+file_dialog_icons_files_columns*80) && (file_dialog_folder_first_displayed_file_num+fd_icons_selected_file)<file_dialog_folder_num_of_files) {
       if(file_dialog_highlighted_file==fd_icons_selected_file) {
        dword_t old_file_entry = get_file_starting_entry(file_dialog_folder_mem, file_dialog_selected_file);
        dword_t old_file_size = get_file_size(file_dialog_folder_mem, file_dialog_selected_file);
        
        //check if this is not folder
        if(get_file_attribute(file_dialog_folder_mem, file_dialog_selected_file)==0x10) {
         file_dialog_open_folder(old_file_entry, old_file_size);
         continue;
        }
      
        if(file_dialog_window_type==FD_TYPE_OPEN) { //OPEN FILE DIALOG
         //check if this is file with allowed extension
         if(file_extension_mem[0]!=0) { //0 = no extensions, open all files
          for(int i=0; i<10; i++) {
           if(file_extension_mem[i*11]==0) {
            error_window("This file type can not be opened in this program");
            file_dialog_state=FD_STATE_FOLDER_LOADED;
            break;
           }
       
           if(compare_file_extension(file_dialog_folder_mem, file_dialog_selected_file, folder_open_file_extensions_mem+i*22+2, file_extension_mem[i*11])==STATUS_TRUE) {
            file_dialog_state=FD_STATE_FILE_LOADING;
            break;
           }
          }
          if(file_dialog_state==FD_STATE_FOLDER_LOADED) {
           redraw_file_dialog();
           continue;
          }
         }
        
         file_dialog_state=FD_STATE_FILE_LOADING;
         redraw_file_dialog();

         file_show_file_work_progress = 1;
         dword_t file_memory = read_file(old_file_entry, old_file_size);
         file_show_file_work_progress = 0;
         if(file_memory==STATUS_ERROR) {
          error_window("Error during loading file");
          file_dialog_state=FD_STATE_FOLDER_LOADED;
          redraw_file_dialog();
          continue;
         }
         else {
          file_dialog_read_file_properties(file_dialog_folder_mem, file_dialog_selected_file);
          return file_memory;
         }
        }
        else if(file_dialog_window_type==FD_TYPE_SAVE) { //SAVE FILE DIALOG
         //if read-only filesystem
         if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]!=STORAGE_FAT) {
          error_window("This filesystem is read-only");
          file_dialog_state=FD_STATE_FOLDER_LOADED;
          redraw_file_dialog();
          continue;
         }
    
         //rewrite
         file_dialog_state=FD_STATE_FILE_WRITING;
         redraw_file_dialog();
         file_show_file_work_progress = 1;
         dword_t file_memory = rewrite_file(old_file_entry, old_file_size, new_file_memory, new_file_size);
         file_show_file_work_progress = 0;
         if(file_memory==STATUS_ERROR) {
          error_window("Error during rewriting file");
          file_dialog_state=FD_STATE_FOLDER_LOADED;
          redraw_file_dialog();
          continue;
         }
         else {
          //update entry and save folder
          //TODO: time of modification
          set_file_entry_size(file_dialog_folder_mem, file_dialog_selected_file, new_file_size);
      
          dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
          folder_path_depth = devices_mem32[3];
          if(rewrite_folder(folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2], file_dialog_folder_mem)==STATUS_ERROR) {
           error_window("Error during rewriting folder");
           file_dialog_state=FD_STATE_FOLDER_LOADED;
           redraw_file_dialog();
           continue;
          }
          
          //set informations about saved file
          file_dialog_save_return_state = STATUS_GOOD;
          file_dialog_read_file_properties(file_dialog_folder_mem, file_dialog_selected_file);
      
          return STATUS_GOOD;
         }
        }
       }
      
       file_dialog_highlighted_file = fd_icons_selected_file;
       file_dialog_selected_file = (file_dialog_folder_first_displayed_file_num+fd_icons_selected_file);
       redraw_file_dialog();
       continue;
      }
      else { //click not on file
       file_dialog_selected_file = NO_FILE_SELECTED;
       file_dialog_highlighted_file = NO_FILE_SELECTED;
       redraw_file_dialog();
       continue;
      }
     }
    }
    else if(file_dialog_highlighted_file!=NO_FILE_SELECTED) { //click not to file zone
     file_dialog_selected_file = NO_FILE_SELECTED;
     file_dialog_highlighted_file = NO_FILE_SELECTED;
     redraw_file_dialog();
     continue;
    }
   }
   
   //TODO: move scrollbar
   
   //buttons
   if(is_mouse_in_zone(graphic_screen_y-24-32, graphic_screen_y-24-8, 0, 150)==STATUS_TRUE) { 
    mouse_drag_and_drop = MOUSE_DRAG;
      
    devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
    if(devices_mem32[3]!=0) {
     devices_mem32[3]--;
     file_dialog_load_folder();
     redraw_file_dialog();
     continue;
    }
   }
   
   if(is_mouse_in_zone(graphic_screen_y-24, graphic_screen_y, graphic_screen_x-120, graphic_screen_x)==STATUS_TRUE && file_dialog_window_type==FD_TYPE_SAVE) {
    mouse_drag_and_drop = MOUSE_DRAG;
    
    //if read-only filesystem
    if(devices_mem[file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH+FD_DEVICE_ENTRY_OFFSET_PARTITION_TYPE]!=STORAGE_FAT) {
     error_window("This filesystem is read-only");
     file_dialog_state=FD_STATE_FOLDER_LOADED;
     redraw_file_dialog();
     continue;
    }
   
    dword_t file_name_text_area = create_text_area(TEXT_AREA_INPUT_LINE, 50, graphic_screen_x-411, 5, 401, 10);
    while(1) {
     //TODO: with mouse cursor
     draw_text_area(file_name_text_area);
     redraw_part_of_screen(graphic_screen_x-411, 5, 403, 12);
     wait_for_usb_keyboard();
     if(keyboard_value==KEY_ESC) {
      delete_text_area(file_name_text_area);
      break;
     }
     else if(keyboard_value==KEY_ENTER) {
      //save file
      file_dialog_state=FD_STATE_FILE_WRITING;
      redraw_file_dialog();
      file_show_file_work_progress = 1;
      dword_t file_memory = create_file(new_file_memory, new_file_size);
      file_show_file_work_progress = 0;
      if(file_memory==STATUS_ERROR) {
       error_window("Error during rewriting file");
       file_dialog_state=FD_STATE_FOLDER_LOADED;
       redraw_file_dialog();
       continue;
      }
     
      //create new entry in folder
      file_dialog_folder_mem = realloc(file_dialog_folder_mem, (file_dialog_folder_num_of_files+2)*256);
      byte_t *folder_new_entry8 = (byte_t *) (file_dialog_folder_mem+file_dialog_folder_num_of_files*256);
      word_t *folder_new_entry16 = (word_t *) (file_dialog_folder_mem+file_dialog_folder_num_of_files*256);
      dword_t *folder_new_entry32 = (dword_t *) (file_dialog_folder_mem+file_dialog_folder_num_of_files*256);
      dword_t *file_name_text_area_ptr = (dword_t *) file_name_text_area;
      word_t *file_name_text_area_data = (word_t *) (file_name_text_area_ptr[0]);
      //short name
      for(int i=0; i<8; i++) {
       folder_new_entry8[i] = file_name_text_area_data[i];
      }
      //short extension
      folder_new_entry8[8] = file_dialog_file_extension[0];
      folder_new_entry8[9] = file_dialog_file_extension[1];
      folder_new_entry8[10] = file_dialog_file_extension[2];
      //attribute archive
      folder_new_entry8[11] = 0x20;
      //not used
      folder_new_entry8[12] = 0;
      folder_new_entry8[13] = 0;
      //time hour/minute/second
      read_time();
      word_t time_hms = (time_hour<<11 | time_minute<<5 | time_second);
      folder_new_entry16[7] = time_hms;
      folder_new_entry16[11] = time_hms;
      //time year/month/day
      word_t time_ymd = ((time_year-1980)<<9 | time_month<<5 | time_day);
      folder_new_entry16[8] = time_ymd;
      folder_new_entry16[9] = time_ymd;
      folder_new_entry16[12] = time_ymd;
      //file cluster
      folder_new_entry16[10] = (file_memory>>16);
      folder_new_entry16[13] = (file_memory & 0xFFFF);
      //file length
      folder_new_entry32[7] = new_file_size;
      //file name and extension
      for(int i=0; i<50; i++) {
       if(file_name_text_area_data[i]==0) {
        folder_new_entry16[i+16] = '.';
        folder_new_entry16++;
        for(int j=0; j<10; j++) {
         if(file_dialog_file_extension[j]==0) {
          break;
         }
         folder_new_entry16[i+16+j] = file_dialog_file_extension[j];
        }
        break;
       }
       folder_new_entry16[i+16] = file_name_text_area_data[i];
      }
      delete_text_area(file_name_text_area);
      file_dialog_folder_num_of_files++;
      
      //rewrite folder
      dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
      folder_path_depth = devices_mem32[3];
      if(rewrite_folder(folder_path[file_dialog_selected_device*FD_NUMBER_OF_MAX_FOLDER_DEPTH*2+folder_path_depth*2], file_dialog_folder_mem)==STATUS_ERROR) {
       error_window("Error during rewriting folder");
       file_dialog_state=FD_STATE_FOLDER_LOADED;
       break;
      }
      
      //set informations about saved file
      file_dialog_save_return_state = STATUS_GOOD;
      file_dialog_read_file_properties(file_dialog_folder_mem, (file_dialog_folder_num_of_files-1));
      
      return STATUS_GOOD;
     }
     text_area_keyboard_event(file_name_text_area);
    }
    redraw_file_dialog();
    continue;
   }

  }

 }
}

dword_t file_dialog_open(void) {
 file_dialog_window_type = FD_TYPE_OPEN;
 return file_dialog_window(0, 0);
}

dword_t file_dialog_save(dword_t file_memory, dword_t file_size) {
 file_dialog_window_type = FD_TYPE_SAVE;
 file_dialog_save_return_state = STATUS_ERROR;
 return file_dialog_window(file_memory, file_size);
}
