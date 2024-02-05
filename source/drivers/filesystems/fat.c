//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_fat(dword_t first_partition_sector) {
 if(read_storage_medium(first_partition_sector, 1, (dword_t)&fat16_boot_sector)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //TODO: more tests
 if((fat16_boot_sector.jmp[0]==0xEB || fat16_boot_sector.jmp[0]==0xE9) && fat16_boot_sector.jmp[2]==0x90 && fat16_boot_sector.bytes_per_sector==512 && fat16_boot_sector.reserved_sectors>0 && fat16_boot_sector.fat_tables>0 && fat16_boot_sector.boot_signature==0xAA55) {
  if(fat16_boot_sector.sectors_per_cluster==1 || (fat16_boot_sector.sectors_per_cluster%2)==0) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}

void select_fat_partition(dword_t first_partition_sector) {
 //load first sector of partition with info about FAT
 if(read_storage_medium(first_partition_sector, 1, (dword_t)&fat16_boot_sector)==STATUS_ERROR) {
  return;
 }
 
 //initalize basic values
 fat_total_sectors = fat16_boot_sector.total_sectors_word;
 if(fat_total_sectors==0) {
  fat_total_sectors = fat16_boot_sector.total_sectors_dword;
 }
 fat_cluster_length = fat16_boot_sector.sectors_per_cluster;
 fat_cluster_length_in_bytes = (fat16_boot_sector.sectors_per_cluster*fat16_boot_sector.bytes_per_sector);
 fat_number_of_clusters = (fat_total_sectors/fat_cluster_length);
 fat_table_sector = (first_partition_sector + fat16_boot_sector.reserved_sectors);
 
 //load first FAT table sector
 loaded_fat_sector = fat_table_sector;
 if(read_storage_medium(fat_table_sector, 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
  loaded_fat_sector = 0xFFFFFFFF;
  return;
 }
 
 if(fat_number_of_clusters>65536) {
  //load first sector of partition with info about FAT32
  if(read_storage_medium(first_partition_sector, 1, (dword_t)&fat32_boot_sector)==STATUS_ERROR) {
   return;
  }
 
  //initalize values
  fat_root_dir_sector = fat32_boot_sector.root_dir_cluster;
  fat_root_dir_length = fat_get_size_of_file_on_disk(fat32_boot_sector.root_dir_cluster);
  fat_data_sector = (fat_table_sector + (fat32_boot_sector.fat_tables*fat32_boot_sector.fat_table_size));
  for(int i=0; i<11; i++) {
   partition_label[i]=fat32_boot_sector.volume_label[i];
  }
  partition_label[11]=0;

  return;
 }
 
 //FAT12/FAT16
 fat_root_dir_sector = (fat_table_sector + (fat16_boot_sector.fat_tables*fat16_boot_sector.fat_table_size));
 fat_root_dir_length = (((fat16_boot_sector.root_dir_entries*32) + (fat16_boot_sector.bytes_per_sector-1))/fat16_boot_sector.bytes_per_sector);
 fat_data_sector = fat_root_dir_sector + fat_root_dir_length;
 for(int i=0; i<11; i++) {
  partition_label[i]=fat16_boot_sector.volume_label[i];
 }
 partition_label[11]=0;
}

void set_fat_partition_info_in_device_list_entry(void) {
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+0, fat_total_sectors);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+1, fat_cluster_length);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+2, fat_cluster_length_in_bytes);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+3, fat_number_of_clusters);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+4, fat_table_sector);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+5, fat_root_dir_sector);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+6, fat_root_dir_length);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+7, fat_data_sector);
}

void read_fat_partition_info_from_device_list_entry(void) {
 fat_total_sectors = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+0);
 fat_cluster_length = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+1);
 fat_cluster_length_in_bytes = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+2);
 fat_number_of_clusters = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+3);
 fat_table_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+4);
 fat_root_dir_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+5);
 fat_root_dir_length = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+6);
 fat_data_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+7);

 //load first FAT table sector
 loaded_fat_sector = 0xFFFFFFFF;
 if(read_storage_medium(fat_table_sector, 1, (dword_t)fat_table_one_sector)==STATUS_GOOD) {
  loaded_fat_sector = fat_table_sector;
 }
}

dword_t get_fat_entry(dword_t entry) {
 byte_t *fat_table_one_sector8 = (byte_t *) fat_table_one_sector;
 word_t *fat_table_one_sector16 = (word_t *) fat_table_one_sector;

 if(fat_number_of_clusters<4085) {
  dword_t fat12_entry_byte_offset = 0;
  if(entry>0) {
   fat12_entry_byte_offset = ((entry & ~0x1)*3/2);
  }

  //FAT12
  //get first byte
  if(loaded_fat_sector!=(fat_table_sector + (fat12_entry_byte_offset/512))) {
   if(read_storage_medium(fat_table_sector + (fat12_entry_byte_offset/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return 1; //error
   }
   loaded_fat_sector = (fat_table_sector + (fat12_entry_byte_offset/512));
  }
  byte_t fat12_first_cluster_byte = fat_table_one_sector8[(fat12_entry_byte_offset % 512)];
  
  //get second byte
  if(loaded_fat_sector!=(fat_table_sector + ((fat12_entry_byte_offset+1)/512))) {
   if(read_storage_medium(fat_table_sector + ((fat12_entry_byte_offset+1)/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return 1; //error
   }
   loaded_fat_sector = (fat_table_sector + ((fat12_entry_byte_offset+1)/512));
  }
  byte_t fat12_second_cluster_byte = fat_table_one_sector8[((fat12_entry_byte_offset+1) % 512)];
  
  //get third byte
  if(loaded_fat_sector!=(fat_table_sector + ((fat12_entry_byte_offset+2)/512))) {
   if(read_storage_medium(fat_table_sector + ((fat12_entry_byte_offset+2)/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return 1; //error
   }
   loaded_fat_sector = (fat_table_sector + ((fat12_entry_byte_offset+2)/512));
  }
  byte_t fat12_third_cluster_byte = fat_table_one_sector8[((fat12_entry_byte_offset+2) % 512)];
  
  //return cluster
  if((entry & 0x1)==0x0) {
   return (fat12_first_cluster_byte | ((fat12_second_cluster_byte & 0xF)<<8));
  }
  else {
   return ((fat12_third_cluster_byte<<4) | (fat12_second_cluster_byte>>4));
  }
 }
 else if(fat_number_of_clusters<=65536) {
  //FAT16
  //load FAT table sector
  if(loaded_fat_sector!=(fat_table_sector + (entry/256))) {
   if(read_storage_medium(fat_table_sector + (entry/256), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return 1; //error
   }
   loaded_fat_sector = (fat_table_sector + (entry/256));
  }
  
  return fat_table_one_sector16[(entry % 256)];
 }
 else {
  //FAT32
  //load FAT table sector
  if(loaded_fat_sector!=(fat_table_sector + (entry/128))) {
   if(read_storage_medium(fat_table_sector + (entry/128), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return 1; //error
   }
   loaded_fat_sector = (fat_table_sector + (entry/128));
  }
  
  return fat_table_one_sector[(entry % 128)];
 }
}

byte_t set_fat_entry(dword_t entry, dword_t value) {
 byte_t *fat_table_one_sector8 = (byte_t *) fat_table_one_sector;
 word_t *fat_table_one_sector16 = (word_t *) fat_table_one_sector;

 if(fat_number_of_clusters<4085) {
  dword_t fat12_entry_byte_offset = 0;
  if(entry>0) {
   fat12_entry_byte_offset = ((entry & ~0x1)*3/2);
  }

  //FAT12
  if((entry & 0x1)==0x0) {
   //load FAT sector
   if(loaded_fat_sector!=(fat_table_sector + (fat12_entry_byte_offset/512))) {
    if(save_fat_table_sector()==STATUS_ERROR) { //for efficent writing, we save changed values in sector only when we move to another sector
     return STATUS_ERROR;
    }
    if(read_storage_medium(fat_table_sector + (fat12_entry_byte_offset/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
     return STATUS_ERROR; //error during reading FAT table sector
    }
    loaded_fat_sector = (fat_table_sector + (fat12_entry_byte_offset/512));
   }

   //save value
   fat_table_one_sector8[(fat12_entry_byte_offset % 512)] = (value & 0xFF);

   //load FAT sector
   if(loaded_fat_sector!=(fat_table_sector + ((fat12_entry_byte_offset+1)/512))) {
    if(save_fat_table_sector()==STATUS_ERROR) { //for efficent writing, we save changed values in sector only when we move to another sector
     return STATUS_ERROR;
    }
    if(read_storage_medium(fat_table_sector + ((fat12_entry_byte_offset+1)/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
     return STATUS_ERROR; //error during reading FAT table sector
    }
    loaded_fat_sector = (fat_table_sector + ((fat12_entry_byte_offset+1)/512));
   }

   //save value
   fat_table_one_sector8[((fat12_entry_byte_offset+1) % 512)] = ((fat_table_one_sector8[((fat12_entry_byte_offset+1) % 512)] & 0xF0) | ((value >> 8) & 0xF));
   
   return STATUS_GOOD;
  }
  else {
   //load FAT sector
   if(loaded_fat_sector!=(fat_table_sector + ((fat12_entry_byte_offset+1)/512))) {
    if(save_fat_table_sector()==STATUS_ERROR) { //for efficent writing, we save changed values in sector only when we move to another sector
     return STATUS_ERROR;
    }
    if(read_storage_medium(fat_table_sector + ((fat12_entry_byte_offset+1)/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
     return STATUS_ERROR; //error during reading FAT table sector
    }
    loaded_fat_sector = (fat_table_sector + ((fat12_entry_byte_offset+1)/512));
   }

   //save value
   fat_table_one_sector8[((fat12_entry_byte_offset+1) % 512)] = ((fat_table_one_sector8[((fat12_entry_byte_offset+1) % 512)] & 0x0F) | ((value & 0xF) << 4));

   //load FAT sector
   if(loaded_fat_sector!=(fat_table_sector + ((fat12_entry_byte_offset+2)/512))) {
    if(save_fat_table_sector()==STATUS_ERROR) { //for efficent writing, we save changed values in sector only when we move to another sector
     return STATUS_ERROR;
    }
    if(read_storage_medium(fat_table_sector + ((fat12_entry_byte_offset+2)/512), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
     return STATUS_ERROR; //error during reading FAT table sector
    }
    loaded_fat_sector = (fat_table_sector + ((fat12_entry_byte_offset+2)/512));
   }

   //save value
   fat_table_one_sector8[((fat12_entry_byte_offset+2) % 512)] = ((value >> 4) & 0xFF);
   
   return STATUS_GOOD;
  }
 }
 else if(fat_number_of_clusters<=65536) {
  //FAT16
  //load FAT table sector
  if(loaded_fat_sector!=(fat_table_sector + (entry/256))) {
   if(save_fat_table_sector()==STATUS_ERROR) { //for efficent writing, we save changed values in sector only when we move to another sector
    return STATUS_ERROR;
   }
   if(read_storage_medium(fat_table_sector + (entry/256), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return STATUS_ERROR; //error during reading FAT table sector
   }
   loaded_fat_sector = (fat_table_sector + (entry/256));
  }
  
  //write new value
  fat_table_one_sector16[(entry % 256)] = value;
  return STATUS_GOOD;
 }
 else {
  //FAT32
  //load FAT table sector
  if(loaded_fat_sector!=(fat_table_sector + (entry/128))) {
   if(save_fat_table_sector()==STATUS_ERROR) { //for efficent writing, we save changed values in sector only when we move to another sector
    return STATUS_ERROR;
   }
   if(read_storage_medium(fat_table_sector + (entry/128), 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) {
    return STATUS_ERROR; //error
   }
   loaded_fat_sector = (fat_table_sector + (entry/128));
  }
  
  //write new value
  fat_table_one_sector[(entry % 128)] = value;
  return STATUS_GOOD;
 }
}

byte_t save_fat_table_sector(void) {
 if(fat_number_of_clusters<=65536) { //FAT12/16
  if(write_storage_medium(loaded_fat_sector, 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) { //save to first FAT table
   return STATUS_ERROR;
  }
  if(fat16_boot_sector.fat_tables>1) {
   return write_storage_medium(loaded_fat_sector + fat16_boot_sector.fat_table_size , 1, (dword_t)fat_table_one_sector); //save to second FAT table
  }
  else {
   return STATUS_GOOD;
  }
 }
 else { //FAT32
  if(write_storage_medium(loaded_fat_sector, 1, (dword_t)fat_table_one_sector)==STATUS_ERROR) { //save to first FAT table
   return STATUS_ERROR;
  }
  if(fat32_boot_sector.fat_tables>1) {
   return write_storage_medium(loaded_fat_sector + fat32_boot_sector.fat_table_size , 1, (dword_t)fat_table_one_sector); //save to second FAT table
  }
  else {
   return STATUS_GOOD;
  }
 }
}

byte_t read_fat_cluster(dword_t cluster, dword_t memory) {
 return read_storage_medium((fat_data_sector + ((cluster - 2)*fat_cluster_length)), fat_cluster_length, memory);
}

byte_t write_fat_cluster(dword_t cluster, dword_t memory) {
 return write_storage_medium((fat_data_sector + ((cluster - 2)*fat_cluster_length)), fat_cluster_length, memory);
}

dword_t fat_get_size_of_file_on_disk(dword_t cluster) {
 dword_t number_of_clusters=0;
 
 //test errors
 if(cluster<2 || cluster>(fat_number_of_clusters+2) || get_fat_entry(cluster)==0) {
  return 0; //return size in bytes
 }
 
 for(dword_t i=0; i<fat_number_of_clusters; i++) {
  //count actual cluster
  number_of_clusters++;
  
  //get number of next cluster
  cluster = get_fat_entry(cluster);
  
  //test if this is last cluster
  if(fat_number_of_clusters<4085) {
   //FAT12
   if(cluster>=0xFF8 && cluster<=0xFFF) {
    return (number_of_clusters*fat_cluster_length_in_bytes); //return size in bytes
   }
  }
  else if(fat_number_of_clusters<=65536) {
   //FAT16
   if(cluster>=0xFFF8 && cluster<=0xFFFF) {
    return (number_of_clusters*fat_cluster_length_in_bytes); //return size in bytes
   }
  }
  else {
   //FAT32
   if(cluster>=0x0FFFFFF8) {
    return (number_of_clusters*fat_cluster_length_in_bytes); //return size in bytes
   }
  }
  
  //test errors
  if(cluster<2 || cluster>(fat_number_of_clusters+2)) {
   return (number_of_clusters*fat_cluster_length_in_bytes); //return size in bytes
  }
 }

 return (number_of_clusters*fat_cluster_length_in_bytes); //return size in bytes
}

dword_t read_fat_file(dword_t cluster, dword_t size_in_bytes) {
 dword_t file_mem = calloc(size_in_bytes-(size_in_bytes%fat_cluster_length_in_bytes)+fat_cluster_length_in_bytes); //recalculate to size of file on disk
 dword_t file_output_mem_pointer = file_mem;
 dword_t remaining_bytes = size_in_bytes;
 byte_t old_file_work_done_percents = 0;

 //initalize output for user
 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
 }
 
 //check for invalid input
 if(cluster<2 || cluster>(fat_number_of_clusters+2) || get_fat_entry(cluster)<2 || size_in_bytes==0) {
  free(file_mem);
  return STATUS_ERROR;
 }

 //read file clusters
 dword_t number_of_clusters = (size_in_bytes/fat_cluster_length_in_bytes);
 if((size_in_bytes%fat_cluster_length_in_bytes)!=0) {
  number_of_clusters++;
 }
 dword_t file_clusters_list_mem = calloc(number_of_clusters*4+8);
 dword_t *file_cluster_list = (dword_t *) (file_clusters_list_mem);
 for(dword_t i=0; i<number_of_clusters; i++) {
  //test cluster errors
  if(cluster<2 || cluster>(fat_number_of_clusters+2)) {
   free(file_mem);
   free(file_clusters_list_mem);
   return STATUS_ERROR;
  }

  //update information about progress
  if(i!=0) {
   file_work_done_percents = (10*i/number_of_clusters);
   if(file_show_file_work_progress==1 && old_file_work_done_percents!=file_work_done_percents) {
    file_dialog_show_progress();
    old_file_work_done_percents = file_work_done_percents;
   }
  }

  //save cluster number
  *file_cluster_list = cluster;
  file_cluster_list++;

  //get next cluster number
  cluster = get_fat_entry(cluster);

  //was this last cluster?
  if(fat_number_of_clusters<4085) { //FAT12
   if(cluster>=0xFF8 && cluster<=0xFFF) {
    break;
   }
  }
  else if(fat_number_of_clusters<=65536) { //FAT16
   if(cluster>=0xFFF8 && cluster<=0xFFFF) {
    break;
   }
  }
  else { //FAT32
   if(cluster>=0x0FFFFFF8) {
    break;
   }
  }
 }

 //read file
 file_cluster_list = (dword_t *) (file_clusters_list_mem);
 dword_t num_of_clusters_to_read = 0;
 dword_t max_clusters_per_one_read = 1;
 if(fat_cluster_length<128) {
  max_clusters_per_one_read = (128/fat_cluster_length);
 }
 else {
  max_clusters_per_one_read = 1;
 }
 while(*file_cluster_list!=0) {
  //update information about progress
  if((size_in_bytes-remaining_bytes)!=0 && size_in_bytes>=1024) {
   file_work_done_percents = (10+90*((size_in_bytes-remaining_bytes)/1024)/(size_in_bytes/1024));
   if(file_show_file_work_progress==1 && old_file_work_done_percents!=file_work_done_percents) {
    file_dialog_show_progress();
    old_file_work_done_percents = file_work_done_percents;
   }
  }

  //if there are multiple sectors immediately after each other, we can read multiple clusters on one request
  num_of_clusters_to_read = 1;
  if(max_clusters_per_one_read>1) {
   for(dword_t i=0; i<max_clusters_per_one_read; i++) {
    if(file_cluster_list[i+1]==(file_cluster_list[i]+1)) {
     num_of_clusters_to_read++;
    }
    else {
     break;
    }
   }
  }

  //read data of clusters
  if(read_storage_medium((fat_data_sector + ((*file_cluster_list - 2)*fat_cluster_length)), (num_of_clusters_to_read*fat_cluster_length), file_output_mem_pointer)==STATUS_ERROR) {
   free(file_mem);
   free(file_clusters_list_mem);
   return STATUS_ERROR;
  }
  file_output_mem_pointer += (num_of_clusters_to_read*fat_cluster_length_in_bytes);
  remaining_bytes -= (num_of_clusters_to_read*fat_cluster_length_in_bytes);
  file_cluster_list += num_of_clusters_to_read;
 }

 free(file_clusters_list_mem);
 file_mem = realloc(file_mem, size_in_bytes); //realloc to real size of file
 return file_mem; //we succesfully readed file
}

dword_t write_fat_file(dword_t file_mem, dword_t size_in_bytes, dword_t first_cluster) {
 //check invalid request
 if(file_mem==0 || size_in_bytes==0) {
  return STATUS_ERROR;
 }
 
 //initalize variables
 dword_t clusters_of_file = (size_in_bytes/fat_cluster_length_in_bytes);
 if((size_in_bytes%fat_cluster_length_in_bytes)!=0) {
  clusters_of_file++;
 }
 dword_t clusters_numbers_mem = calloc(clusters_of_file*4), remaining_clusters=clusters_of_file;
 dword_t *clusters_numbers_ptr = (dword_t *) clusters_numbers_mem;
 byte_t old_file_work_done_percents = 0;

 //initalize output for user
 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
 }
 
 //set first cluster
 if(first_cluster!=0) {
  *clusters_numbers_ptr=first_cluster;
  clusters_numbers_ptr++;
  remaining_clusters--;
 }

 //found free clusters for file
 for(dword_t cluster=3, cluster_number=0; (remaining_clusters>0 && cluster<(fat_number_of_clusters+2)); cluster++) {
  cluster_number = get_fat_entry(cluster);
  if(cluster_number==1) {
   free(clusters_numbers_mem);
   return STATUS_ERROR;
  }
  if(cluster_number==0 && cluster!=first_cluster) {
   *clusters_numbers_ptr=cluster;
   clusters_numbers_ptr++;
   remaining_clusters--;
  }
 }
 if(remaining_clusters>0) {
  log("\nFAT: not enough free clusters");
  free(clusters_numbers_mem);
  return STATUS_ERROR; //not enough free clusters
 }
 
 //write file
 clusters_numbers_ptr = (dword_t *) clusters_numbers_mem;
 first_cluster = *clusters_numbers_ptr;
 for(int i=0; i<clusters_of_file; i++) { 
  if(write_fat_cluster(*clusters_numbers_ptr, file_mem)==STATUS_ERROR) {
   free(clusters_numbers_mem);
   return STATUS_ERROR;
  }
  file_mem += fat_cluster_length_in_bytes;
  clusters_numbers_ptr++;
  
  file_work_done_percents = (95*i/clusters_of_file);
  if(file_show_file_work_progress==1 && old_file_work_done_percents!=file_work_done_percents) {
   file_dialog_show_progress();
   old_file_work_done_percents = file_work_done_percents;
  }
 }
 
 //save clusters numbers to FAT table
 clusters_numbers_ptr = (dword_t *) clusters_numbers_mem;
 for(int i=0; i<(clusters_of_file-1); i++) {
  if(set_fat_entry(*clusters_numbers_ptr, clusters_numbers_ptr[1])==STATUS_ERROR) {
   free(clusters_numbers_mem);
   return STATUS_ERROR;
  }
  clusters_numbers_ptr++;
  
  file_work_done_percents = 95+(5*i/clusters_of_file);
  if(file_show_file_work_progress==1 && old_file_work_done_percents!=file_work_done_percents) {
   file_dialog_show_progress();
   old_file_work_done_percents = file_work_done_percents;
  }
 }
 
 //write last cluster to FAT table
 if(fat_number_of_clusters<4085) {
  //FAT12
  if(set_fat_entry(*clusters_numbers_ptr, 0xFFF)==STATUS_ERROR) {
   free(clusters_numbers_mem);
   return STATUS_ERROR;
  }
 }
 else if(fat_number_of_clusters<=65536) {
  //FAT16
  if(set_fat_entry(*clusters_numbers_ptr, 0xFFFF)==STATUS_ERROR) {
   free(clusters_numbers_mem);
   return STATUS_ERROR;
  }
 }
 else {
  //FAT32
  if(set_fat_entry(*clusters_numbers_ptr, 0x0FFFFFFF)==STATUS_ERROR) {
   free(clusters_numbers_mem);
   return STATUS_ERROR;
  }
 }

 free(clusters_numbers_mem);
 if(save_fat_table_sector()==STATUS_ERROR) { //save changed values in last sector
  return STATUS_ERROR;
 }
 return first_cluster;
}

dword_t delete_fat_file(dword_t cluster) {
 dword_t last_cluster;

 if(cluster<2 || cluster>(fat_number_of_clusters+2) || get_fat_entry(cluster)<2) {
  log("\nFAT: Invalid cluster delete request ");
  log_var(cluster);
  return STATUS_GOOD; //this file do not exist
 }
 
 for(dword_t i=0; i<fat_number_of_clusters; i++) {  
  //get number of next cluster
  last_cluster = cluster;
  cluster = get_fat_entry(last_cluster);
  
  //delete cluster
  if(set_fat_entry(last_cluster, 0)==STATUS_ERROR) {
   log("\nFAT: Error with setting cluster");
   return STATUS_ERROR;
  }
  
  //test if this is last cluster
  if(fat_number_of_clusters<4085) {
   //FAT12
   if(cluster>=0xFF8 && cluster<=0xFFF) {
    if(save_fat_table_sector()==STATUS_ERROR) { //save changed values in last sector
     log("\nFAT: Error with saving FAT table sector");
     return STATUS_ERROR;
    }
    break;
   }
  }
  else if(fat_number_of_clusters<=65536) {
   //FAT16
   if(cluster>=0xFFF8 && cluster<=0xFFFF) {
    if(save_fat_table_sector()==STATUS_ERROR) { //save changed values in last sector
     log("\nFAT: Error with saving FAT table sector");
     return STATUS_ERROR;
    }
    break;
   }
  }
  else {
   //FAT32
   if(cluster>=0x0FFFFFF8) {
    if(save_fat_table_sector()==STATUS_ERROR) { //save changed values in last sector
     log("\nFAT: Error with saving FAT table sector");
     return STATUS_ERROR;
    }
    break;
   }
  }
  
  //test errors
  if(cluster<2 || cluster>(fat_number_of_clusters+2)) {
   log("\nFAT: Invalid cluster in process of deleting");
   break;
  }
 }

 if(save_fat_table_sector()==STATUS_ERROR) { //save changed values in last sector
  log("\nFAT: Error with saving FAT table sector");
  return STATUS_ERROR;
 }
 return STATUS_GOOD; //we succesfully deleted file
}

dword_t read_fat_folder(dword_t cluster, dword_t size_in_bytes) {
 dword_t fat_folder_mem = 0, folder_mem = 0, folder_entries = 0;
 
 //load folder
 if(cluster==ROOT_DIRECTORY) {
  if(fat_number_of_clusters<=65536) {
   //FAT12/FAT16
   size_in_bytes = fat_root_dir_length*512;
   fat_folder_mem = malloc(size_in_bytes);
   if(read_storage_medium(fat_root_dir_sector, fat_root_dir_length, fat_folder_mem)==STATUS_ERROR) {
    log("\nFAT12/16: Error reading root dir");
    free(fat_folder_mem);
    return STATUS_ERROR;
   }
  }
  else {
   //FAT32
   fat_folder_mem = read_fat_file(fat_root_dir_sector, fat_root_dir_length);
   if(fat_folder_mem==STATUS_ERROR) {
    log("\nFAT32: Error reading root dir");
    return STATUS_ERROR;
   }
   size_in_bytes = fat_root_dir_length;
  }
 }
 else if(cluster<2 || cluster>(fat_number_of_clusters+2)) {
  log("\nFAT: Error invalid folder cluster");
  return STATUS_ERROR;
 }
 else {
  size_in_bytes = fat_get_size_of_file_on_disk(cluster);  
  fat_folder_mem = read_fat_file(cluster, size_in_bytes);
  if(fat_folder_mem==STATUS_ERROR) {
   log("\nFAT: Error reading directory");
   return STATUS_ERROR;
  }
 }

 //count file entries
 byte_t *fat_folder_ptr = (byte_t *) fat_folder_mem;
 for(dword_t i=0; i<size_in_bytes; i+=32) {
  if(fat_folder_ptr[i+11]==0x01 || fat_folder_ptr[i+11]==0x02 || fat_folder_ptr[i+11]==0x04 || fat_folder_ptr[i+11]==0x10 || fat_folder_ptr[i+11]==0x20) {
   folder_entries++;
  }
 }
 
 //allocate memory for folder
 folder_entries++;
 folder_mem = calloc(folder_entries*256);

 //convert from FAT folder to BleskOS virtual file system folder
 fat_folder_ptr = (byte_t *) fat_folder_mem;
 byte_t *folder_mem_ptr = (byte_t *) folder_mem;
 for(dword_t i=0; i<size_in_bytes; i+=32) {
  if(fat_folder_ptr[i+11]==0xF) {
   //move actual name, because LFN entries have name from backward
   for(int j=256, k=256-26; j>32; j--, k--) {
    folder_mem_ptr[j]=folder_mem_ptr[k];
   }
   
   //copy entry name
   for(int j=0; j<10; j++) {
    folder_mem_ptr[32+j]=fat_folder_ptr[i+1+j];
   }
   for(int j=0; j<12; j++) {
    folder_mem_ptr[32+10+j]=fat_folder_ptr[i+14+j];
   }
   for(int j=0; j<4; j++) {
    folder_mem_ptr[32+22+j]=fat_folder_ptr[i+28+j];
   }
  }
  else if(fat_folder_ptr[i+11]==0x01 || fat_folder_ptr[i+11]==0x02 || fat_folder_ptr[i+11]==0x04 || fat_folder_ptr[i+11]==0x10 || fat_folder_ptr[i+11]==0x20) {
   //new entry
   for(int j=0; j<32; j++) {
    folder_mem_ptr[j]=fat_folder_ptr[i+j];
   }
   
   //short file name
   if(folder_mem_ptr[32]==0 && folder_mem_ptr[33]==0) {
    dword_t folder_name_ptr = 0;

    for(dword_t j=0; j<8; j++) {
     if(fat_folder_ptr[i+j]!=' ') {
      folder_mem_ptr[32+folder_name_ptr]=fat_folder_ptr[i+j];
     }
     else {
      break;
     }

     folder_name_ptr+=2;
    }
    if(fat_folder_ptr[i+8]!=' ') { //add extension
     folder_mem_ptr[32+folder_name_ptr]='.';
     folder_mem_ptr[32+folder_name_ptr+2]=fat_folder_ptr[i+8];
     folder_mem_ptr[32+folder_name_ptr+4]=fat_folder_ptr[i+9];
     folder_mem_ptr[32+folder_name_ptr+6]=fat_folder_ptr[i+10];
    }
   }

   folder_mem_ptr += 256;
  }
 }

 //return converted folder
 free(fat_folder_mem);
 return folder_mem;
}

dword_t convert_folder_to_fat_folder(dword_t folder_mem) {
 byte_t *folder = (byte_t *) folder_mem;
 byte_t checksum=0;
 dword_t fat_folder_entries = 0;

 //count number of FAT entries
 for(int i=0; i<1000000; i++, folder += 256) {
  if(folder[32]==0 && folder[33]==0) { //no entry
   break;
  }
  
  fat_folder_entries+=2; //entry with file info + first long file name entry
  
  for(int j=26; j<(26*8); j+=26) {
   if(folder[32+j+0]!=0 || folder[32+j+1]!=0) {
    fat_folder_entries++; //long file name entry
   }
   else {
    //there is zero 
    if(folder[32+j-2]!=0 && folder[32+j-1]!=0) {
     if(folder[32+j-2]!=0xFF && folder[32+j-1]!=0xFF) {
      fat_folder_entries++; //last long file name entry with 0x0000 ending
     }
    }
    break;
   }
  }
 }
 
 //allocate memory for FAT folder
 fat_file_length = (fat_folder_entries*32);
 fat_folder_entries += (fat_cluster_length_in_bytes/32);
 dword_t fat_folder_mem = calloc((((fat_folder_entries*32)/fat_cluster_length_in_bytes)+1)*fat_cluster_length_in_bytes);
 
 //convert entries
 byte_t *fat_folder = (byte_t *) fat_folder_mem;
 folder = (byte_t *) folder_mem;
 for(int i=0, lfn_entries=0; i<1000000; i++) {
  if(folder[32]==0 && folder[33]==0) { //no entry
   break;
  }
  
  //count number of long file entries
  lfn_entries=1;
  for(int j=26; j<(26*8); j+=26) {
   if(folder[32+j+0]!=0 || folder[32+j+1]!=0) {
    lfn_entries++; //long file name entry
   }
   else {
    //there is zero 
    if(folder[32+j-2]!=0 && folder[32+j-1]!=0) {
     if(folder[32+j-2]!=0xFF && folder[32+j-1]!=0xFF) {
      lfn_entries++; //last long file name entry with 0x0000 ending
     }
    }
    break;
   }
  }
  
  //calculate short file name checksum
  checksum=0;
  for(int j=0; j<11; j++) {
   checksum = ((checksum & 0x1) ? 0x80 : 0) + (checksum >> 1) + folder[j];
  }
  
  //create long file name entries
  for(int j=lfn_entries; j>0; j--) {
   if(j==lfn_entries) {
    fat_folder[0]=(0x40 | j);
   }
   else {
    fat_folder[0]=j;
   }
   fat_folder[11]=0xF;
   fat_folder[12]=0;
   fat_folder[13]=checksum;
   fat_folder[26]=0;
   fat_folder[27]=0;
   
   j--;
   for(int k=0; k<10; k++) {
    fat_folder[1+k]=folder[32+j*26+k];
   }
   for(int k=0; k<12; k++) {
    fat_folder[14+k]=folder[32+j*26+10+k];
   }
   for(int k=0; k<4; k++) {
    fat_folder[28+k]=folder[32+j*26+22+k];
   }
   j++;
   
   fat_folder += 32;
  }
  
  //copy file info entry
  for(int j=0; j<32; j++) {
   fat_folder[j]=folder[j];
  }
  fat_folder += 32;
  
  //next folder entry
  folder += 256;
 }
 
 return fat_folder_mem;
}

dword_t rewrite_fat_folder(dword_t cluster, dword_t folder_mem) {
 dword_t fat_folder_mem = convert_folder_to_fat_folder(folder_mem);
 fat_file_length = 0;
 
 //get size of file
 byte_t *fat_folder_mem_ptr = (byte_t *) fat_folder_mem;
 for(int i=0; i<10000; i++, fat_folder_mem_ptr+=32) {
  if(fat_folder_mem_ptr[11]==0) {
   break;
  }
  fat_file_length+=32;
 }

 //root directory
 if(cluster==ROOT_DIRECTORY) {
  if(fat_number_of_clusters<=65536) {
   //allocate memory of size of root directiory
   dword_t root_dir_mem = calloc(512*fat_root_dir_length);
   copy_memory(fat_folder_mem, root_dir_mem, fat_file_length);
  
   //FAT12/FAT16
   if(write_storage_medium(fat_root_dir_sector, fat_root_dir_length, root_dir_mem)==STATUS_ERROR) {
    log("\nFAT: Problem with writing root folder");
    free(root_dir_mem);
    free(fat_folder_mem);
    return STATUS_ERROR;
   }
   free(root_dir_mem);
   free(fat_folder_mem);
   return STATUS_GOOD;
  }
  else {
   //FAT32
   if(delete_fat_file(fat32_boot_sector.root_dir_cluster)==STATUS_ERROR) {
    log("\nFAT: Problem with deleting folder");
    free(fat_folder_mem);
    return STATUS_ERROR;
   }
   if(write_fat_file(fat_folder_mem, fat_file_length, fat32_boot_sector.root_dir_cluster)==STATUS_ERROR) {
    log("\nFAT: Problem with writing folder");
    free(fat_folder_mem);
    return STATUS_ERROR;
   }
   free(fat_folder_mem);
   return STATUS_GOOD;
  }
 }
 else if(cluster<2 || cluster>(fat_number_of_clusters+2)) {
  free(fat_folder_mem);
  return STATUS_ERROR;
 }
 else {
  //normal folder
  if(delete_fat_file(cluster)==STATUS_ERROR) {
   log("\nFAT: Problem with deleting folder");
   free(fat_folder_mem);
   return STATUS_ERROR;
  }
  if(write_fat_file(fat_folder_mem, fat_file_length, cluster)==STATUS_ERROR) {
   log("\nFAT: Problem with writing folder");
   free(fat_folder_mem);
   return STATUS_ERROR;
  }
  free(fat_folder_mem);
  return STATUS_GOOD;
 }
}