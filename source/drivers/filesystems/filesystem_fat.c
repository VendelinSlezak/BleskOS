//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_fat(dword_t first_partition_sector) {
 //read BPB data
 struct fat16_bpb_t fat16_bpb;
 if(read_storage_medium(first_partition_sector, 1, (dword_t)(&fat16_bpb))==STATUS_ERROR) {
  return STATUS_FALSE;
 }
 
 //TODO: more tests
 if((fat16_bpb.jmp[0]==0xEB || fat16_bpb.jmp[0]==0xE9) && fat16_bpb.jmp[2]==0x90 && fat16_bpb.bytes_per_sector==512 && fat16_bpb.reserved_sectors>0 && fat16_bpb.fat_tables>0 && fat16_bpb.boot_signature==0xAA55) {
  if(fat16_bpb.sectors_per_cluster==1 || (fat16_bpb.sectors_per_cluster%2)==0) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}

void filesystem_fat_read_specific_info(struct connected_partition_info_t *connected_partition_info) {
 log("\n");

 //load first sector of partition with info about FAT
 struct fat16_bpb_t fat16_bpb;
 if(read_storage_medium(connected_partition_info->first_sector, 1, (dword_t)(&fat16_bpb))==STATUS_ERROR) {
  log("\nFAT: can not read first partition sector");
  return;
 }

 //allocate memory
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (calloc(sizeof(struct fat_specific_info_t)));

 //total number of sectors
 fat_info->total_number_of_sectors = fat16_bpb.total_sectors_word;
 if(fat_info->total_number_of_sectors==0) {
  fat_info->total_number_of_sectors = fat16_bpb.total_sectors_dword;
 }

 //size of one cluster
 fat_info->cluster_size_in_sectors = fat16_bpb.sectors_per_cluster;
 if(fat_info->cluster_size_in_sectors==0) {
  log("\nFAT: invalid cluster size");
  free((dword_t)fat_info);
  return;
 }
 fat_info->cluster_size_in_bytes = (fat_info->cluster_size_in_sectors*512);

 //calculate total number of clusters
 fat_info->number_of_clusters = (fat_info->total_number_of_sectors/fat_info->cluster_size_in_sectors);

 //calculate sector of first FAT table
 fat_info->fat_table_first_sector = (connected_partition_info->first_sector + fat16_bpb.reserved_sectors);

 //read number of FAT tables
 fat_info->number_of_fat_tables = fat16_bpb.fat_tables;

 //this variable saves first free cluster for faster creating of files
 fat_info->first_free_cluster = 2;

 //load first sector from FAT table
 if(read_storage_medium(fat_info->fat_table_first_sector, 1, (dword_t)(&fat_info->fat_table_sector))==STATUS_ERROR) {
  log("\nFAT: can not read first FAT table sector");
  free((dword_t)fat_info);
  return;
 }
 //fat_info->loaded_fat_table_sector = 0, already set by calloc()

 //save pointer to info to connected_partition_info 
 connected_partition_info->filesystem_specific_info_pointer = ((byte_t *)fat_info);

 //this is FAT32
 if(fat_info->number_of_clusters>65536) {
  //set type
  fat_info->type = FAT32;
  fat_info->file_ending_entry_value = 0x0FFFFFF8;

  //copy loaded data to FAT32 struct
  struct fat32_bpb_t fat32_bpb;
  copy_memory((dword_t)(&fat16_bpb), (dword_t)(&fat32_bpb), sizeof(struct fat32_bpb_t));

  //read size of one FAT table
  fat_info->fat_table_size_in_sectors = fat32_bpb.fat_table_size;
 
  //read first cluster of root directory
  fat_info->root_directory_location = fat32_bpb.root_dir_cluster;

  //calculate first sector of data
  fat_info->data_first_sector = (fat_info->fat_table_first_sector + (fat32_bpb.fat_tables*fat32_bpb.fat_table_size));
  
  //copy partition label
  for(int i=0; i<11; i++) {
   connected_partition_info->partition_label[i]=fat32_bpb.volume_label[i];
  }
  connected_partition_info->partition_label[10]=0;
 }
 else { //this is FAT12/FAT16
  //set type
  if(fat_info->number_of_clusters<4085) {
   fat_info->type = FAT12;
   fat_info->file_ending_entry_value = 0xFF8;
  }
  else {
   fat_info->type = FAT16;
   fat_info->file_ending_entry_value = 0xFFF8;
  }

  //read size of one FAT table
  fat_info->fat_table_size_in_sectors = fat16_bpb.fat_table_size;

  //read values about root directory
  fat_info->root_directory_location = (fat_info->fat_table_first_sector + (fat16_bpb.fat_tables*fat16_bpb.fat_table_size));
  fat_info->root_directory_size_in_sectors = ((fat16_bpb.root_dir_entries*32)/512);
  if(((fat16_bpb.root_dir_entries*32)%512)!=0) {
   fat_info->root_directory_size_in_sectors++;
  }

  //calculate first sector of data
  fat_info->data_first_sector = (fat_info->root_directory_location + fat_info->root_directory_size_in_sectors);

  //copy partition label
  for(int i=0; i<11; i++) {
   connected_partition_info->partition_label[i]=fat16_bpb.volume_label[i];
  }
  connected_partition_info->partition_label[10]=0;
 }

 //log
 log("\nFAT");
 if(fat_info->number_of_clusters<4085) {
  log("12");
 }
 else if(fat_info->number_of_clusters<=65536) {
  log("16");
 }
 else {
  log("32");
 }
 log(" filesystem");
 log("\ntotal number of sectors: "); log_var(fat_info->total_number_of_sectors);
 log("\ntotal number of clusters: "); log_var(fat_info->number_of_clusters);
 log("\ncluster size in bytes: "); log_var(fat_info->cluster_size_in_sectors*512);
 log("\nroot directory location: "); log_var(fat_info->root_directory_location);
 if(fat_info->type!=FAT32) {
  log("\nroot directory size in sectors: "); log_var(fat_info->root_directory_size_in_sectors);
 }
 log("\npartition label: "); log(connected_partition_info->partition_label);
}

byte_t load_fat_table_sector(dword_t sector) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //sector is already loaded
 if(fat_info->loaded_fat_table_sector==sector) {
  return STATUS_GOOD;
 }
 
 //load FAT table sector to memory
 if(read_storage_medium(fat_info->fat_table_first_sector+sector, 1, (dword_t)(&fat_info->fat_table_sector))==STATUS_ERROR) {
  log("\nFAT: ERROR with reading FAT table sector");
  fat_info->loaded_fat_table_sector = 0xFFFFFFFF;
  return STATUS_ERROR;
 }
 else {
  fat_info->loaded_fat_table_sector = sector;
  return STATUS_GOOD;
 }
}

byte_t save_loaded_fat_table_sector(void) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //save loaded FAT table sector to all FAT tables
 for(dword_t i=0; i<fat_info->number_of_fat_tables; i++) {
  if(write_storage_medium(fat_info->fat_table_first_sector+fat_info->loaded_fat_table_sector+(i*fat_info->fat_table_size_in_sectors), 1, (dword_t)(&fat_info->fat_table_sector))==STATUS_ERROR) {
   log("\nFAT: ERROR with writing FAT table sector");
   return STATUS_ERROR;
  }
 }

 return STATUS_GOOD;
}

byte_t save_and_load_fat_table_sector(dword_t sector) {
 //this method is called by write_fat_table_entry() beacause it guarantee that changes in actually loaded FAT sector will be saved

 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //this sector is already loaded
 if(fat_info->loaded_fat_table_sector==sector) {
  return STATUS_GOOD;
 }

 //save actually loaded sector
 if(save_loaded_fat_table_sector()==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //load requested sector
 return load_fat_table_sector(sector);
}

dword_t read_fat_table_entry(dword_t entry) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 
 //invalid entry returns used entry
 if(entry>=fat_info->number_of_clusters) {
  log("\nFAT: read entry invalid request "); log_hex(entry);
  return 0xFFFFFFFF;
 }

 if(fat_info->type==FAT32) {
  //read FAT table sector with entry
  if(load_fat_table_sector(entry/128)==STATUS_ERROR) {
   return 0xFFFFFFFF;
  }

  //return entry
  dword_t *fat_table_sector = (dword_t *) (fat_info->fat_table_sector);
  return fat_table_sector[entry%128];
 }
 else if(fat_info->type==FAT16) {
  //read FAT table sector with entry
  if(load_fat_table_sector(entry/256)==STATUS_ERROR) {
   return 0xFFFFFFFF;
  }

  //return entry
  word_t *fat_table_sector = (word_t *) (fat_info->fat_table_sector);
  return fat_table_sector[entry%256];
 }
 else if(fat_info->type==FAT12) {
  //read FAT table sector with first byte of entry
  if(load_fat_table_sector(entry*12/4096)==STATUS_ERROR) {
   return 0xFFFFFFFF;
  }

  //read differently according to odd or even entry
  if((entry & 0x1)==0x0) { //odd entry
   word_t entry_value = fat_info->fat_table_sector[(entry*12%4096)/8]; //save first 8 bits

   //read FAT table sector with second byte of entry
   if(load_fat_table_sector((entry*12+8)/4096)==STATUS_ERROR) {
    return 0xFFFFFFFF;
   }

   //return entry
   return (entry_value | (fat_info->fat_table_sector[((entry*12+8)%4096)/8] & 0xF)<<8);
  }
  else { //even entry
   word_t entry_value = (fat_info->fat_table_sector[(entry*12%4096)/8]>>4); //save first 4 bits

   //read FAT table sector with second byte of entry
   if(load_fat_table_sector((entry*12+4)/4096)==STATUS_ERROR) {
    return 0xFFFFFFFF;
   }

   //return entry
   return (entry_value | fat_info->fat_table_sector[((entry*12+4)%4096)/8]<<4);
  }
 }

 return 0xFFFFFFFF;
}

byte_t write_fat_table_entry(dword_t entry, dword_t value) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 
 //invalid entry
 if(entry>=fat_info->number_of_clusters) {
  log("\nFAT: write entry invalid request "); log_hex(entry);
  return STATUS_ERROR;
 }

 if(fat_info->type==FAT32) {
  //read FAT table sector with entry
  if(save_and_load_fat_table_sector(entry/128)==STATUS_ERROR) {
   return STATUS_ERROR;
  }

  //set entry value
  dword_t *fat_table_sector = (dword_t *) (fat_info->fat_table_sector);
  fat_table_sector[entry%128] = value;
 }
 else if(fat_info->type==FAT16) {
  //read FAT table sector with entry
  if(save_and_load_fat_table_sector(entry/256)==STATUS_ERROR) {
   return STATUS_ERROR;
  }

  //set entry value
  word_t *fat_table_sector = (word_t *) (fat_info->fat_table_sector);
  fat_table_sector[entry%256] = value;
 }
 else if(fat_info->type==FAT12) {
  //read FAT table sector with first byte of entry
  if(save_and_load_fat_table_sector(entry*12/4096)==STATUS_ERROR) {
   return STATUS_ERROR;
  }

  //write differently according to odd or even entry
  if((entry & 0x1)==0x0) { //odd entry
   //save first 8 bytes
   fat_info->fat_table_sector[(entry*12%4096)/8] = (value & 0xFF);

   //read FAT table sector with second byte of entry
   if(save_and_load_fat_table_sector((entry*12+8)/4096)==STATUS_ERROR) {
    return STATUS_ERROR;
   }

   //set entry value
   fat_info->fat_table_sector[((entry*12+8)%4096)/8] &= 0xF0;
   fat_info->fat_table_sector[((entry*12+8)%4096)/8] |= ((value>>8) & 0xF);
  }
  else { //even entry
   //save first 4 bytes
   fat_info->fat_table_sector[(entry*12%4096)/8] &= 0x0F;
   fat_info->fat_table_sector[(entry*12%4096)/8] |= ((value & 0xF)<<4);

   //read FAT table sector with second byte of entry
   if(save_and_load_fat_table_sector((entry*12+4)/4096)==STATUS_ERROR) {
    return STATUS_ERROR;
   }

   //set entry value
   fat_info->fat_table_sector[((entry*12+4)%4096)/8] = ((value>>4) & 0xFF);
  }
 }

 //update first free cluster
 if(value==0x00000000 && fat_info->first_free_cluster>entry) {
  fat_info->first_free_cluster = entry;
 }

 return STATUS_GOOD;
}

byte_t *read_fat_file(dword_t cluster) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 size_of_loaded_fat_file_in_memory = 0;

 //create descriptor of file sectors
 struct byte_stream_descriptor_t *descriptor_of_file_sectors = create_descriptor_of_file_sectors(512);

 //get all sectors of file
 for(dword_t i=0; i<fat_info->number_of_clusters; i++) {
  //check invalid cluster value
  if(cluster<2 || cluster>=fat_info->number_of_clusters) {
   log("\nFAT: read file invalid cluster value ");
   log_hex(cluster);
   destroy_byte_stream(descriptor_of_file_sectors);
   return STATUS_ERROR;
  }

  //add sectors of cluster
  add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, fat_info->data_first_sector+((cluster-2)*fat_info->cluster_size_in_sectors), fat_info->cluster_size_in_sectors);
  size_of_loaded_fat_file_in_memory += fat_info->cluster_size_in_bytes;

  //read cluster value in FAT table
  cluster = read_fat_table_entry(cluster);

  //test if this was not last cluster
  if(cluster>=fat_info->file_ending_entry_value) {
   break;
  }
 }

 //read file
 byte_t *file_memory = read_whole_descriptor_of_file_sector(descriptor_of_file_sectors);

 //release memory
 destroy_byte_stream(descriptor_of_file_sectors);

 //return pointer to loaded data or STATUS_ERROR
 return file_memory;
}

dword_t create_fat_file(dword_t first_cluster, byte_t *file_memory, dword_t file_size_in_bytes) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 if(file_size_in_bytes==0) {
  log("\nFAT: write file invalid size");
  return STATUS_ERROR;
 }

 //calculate number of clusters we need to write
 dword_t number_of_clusters_in_file = (file_size_in_bytes/fat_info->cluster_size_in_bytes);
 if((file_size_in_bytes%512)!=0) {
  number_of_clusters_in_file++;
 }

 //initalize all memory structures
 struct byte_stream_descriptor_t *descriptor_of_file_sectors = create_descriptor_of_file_sectors(512);
 struct byte_stream_descriptor_t *clusters_of_file = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 dword_t number_of_finded_free_clusters = 0;

 //save first cluster value if present
 if(first_cluster!=FAT_FIND_FREE_CLUSTER) {
  //save first cluster of file
  add_dword_to_byte_stream(clusters_of_file, first_cluster);

  //save sectors of cluster
  add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, fat_info->data_first_sector+((first_cluster-2)*fat_info->cluster_size_in_sectors), fat_info->cluster_size_in_sectors);
  
  //update variable
  number_of_finded_free_clusters = 1;
 }

 for(dword_t cluster=fat_info->first_free_cluster; cluster<fat_info->number_of_clusters; cluster++) {
  //did we find enough free clusters?
  if(number_of_finded_free_clusters==number_of_clusters_in_file) {
   break;
  }

  //test if this is free cluster
  if(cluster!=first_cluster && read_fat_table_entry(cluster)==0x00000000) {
   //save cluster number
   add_dword_to_byte_stream(clusters_of_file, cluster);

   //save sectors of cluster
   add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, fat_info->data_first_sector+((cluster-2)*fat_info->cluster_size_in_sectors), fat_info->cluster_size_in_sectors);
   
   //update variable
   number_of_finded_free_clusters++;
  }
 }

 //update first free cluster
 dword_t *clusters_of_file_pointer = (dword_t *) (clusters_of_file->start_of_allocated_memory);
 if(first_cluster==FAT_FIND_FREE_CLUSTER) {
  if(fat_info->first_free_cluster<clusters_of_file_pointer[0]) {
   fat_info->first_free_cluster = clusters_of_file_pointer[0];
  }
 }
 else if(number_of_clusters_in_file>1) {
  if(fat_info->first_free_cluster<clusters_of_file_pointer[1]) {
   fat_info->first_free_cluster = clusters_of_file_pointer[1];
  }
 }

 //test if we finded enough free clusters
 if(number_of_finded_free_clusters<number_of_clusters_in_file) {
  log("\nFAT: write file not enough free clusters on medium");
  destroy_byte_stream(clusters_of_file);
  destroy_byte_stream(descriptor_of_file_sectors);
  return STATUS_ERROR;
 }

 //write file data
 if(write_whole_descriptor_of_file_sector(descriptor_of_file_sectors, file_memory)==STATUS_ERROR) {
  log("\nFAT: write file error during writing data of file");
  destroy_byte_stream(clusters_of_file);
  destroy_byte_stream(descriptor_of_file_sectors);
  return STATUS_ERROR;
 }

 //release memory
 destroy_byte_stream(descriptor_of_file_sectors);

 //update entries in FAT table
 first_cluster = clusters_of_file_pointer[0];
 for(dword_t i=0; i<(number_of_clusters_in_file-1); i++) {
  if(write_fat_table_entry(clusters_of_file_pointer[i], clusters_of_file_pointer[i+1])==STATUS_ERROR) { //link to next cluster
   log("\nFAT: write file error during updating FAT entries");
   destroy_byte_stream(clusters_of_file);
   return STATUS_ERROR;
  }
 }

 //set last entry
 if(write_fat_table_entry(clusters_of_file_pointer[number_of_clusters_in_file-1], fat_info->file_ending_entry_value)==STATUS_ERROR) {
  log("\nFAT: write file error during updating FAT entries");
  destroy_byte_stream(clusters_of_file);
  return STATUS_ERROR;
 }

 //release memory
 destroy_byte_stream(clusters_of_file);

 //save all FAT table changes to medium
 if(save_loaded_fat_table_sector()==STATUS_ERROR) {
  log("\nFAT: write file error during updating FAT entries");
  return STATUS_ERROR;
 }

 //return first cluster of file
 return first_cluster;
}

byte_t delete_fat_file(dword_t cluster) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //read all clusters of file
 struct byte_stream_descriptor_t *clusters_of_file = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 for(dword_t i=0; i<fat_info->number_of_clusters; i++) {
  //read cluster entry value
  dword_t cluster_entry_value = read_fat_table_entry(cluster);

  //save number of cluster
  add_dword_to_byte_stream(clusters_of_file, cluster);

  //test if this was last entry of file
  if(cluster_entry_value>=(fat_info->file_ending_entry_value-1) || cluster_entry_value<2) { //end also on bad cluster = (fat_info->file_ending_entry_value-1) or invalid value
   break;
  }

  //go to next cluster
  cluster = cluster_entry_value;
 }

 //delete all clusters of file
 dword_t *clusters_of_file_pointer = (dword_t *) (clusters_of_file->start_of_allocated_memory);
 for(dword_t i=0; i<(clusters_of_file->size_of_stream/4); i++) {
  if(write_fat_table_entry(clusters_of_file_pointer[i], 0x00000000)==STATUS_ERROR) {
   log("\nFAT: delete file error with clearing FAT entry "); log_hex(cluster);
   destroy_byte_stream(clusters_of_file);
   return STATUS_ERROR;
  }
 }
 destroy_byte_stream(clusters_of_file);

 //save all FAT table changes to medium
 if(save_loaded_fat_table_sector()==STATUS_ERROR) {
  log("\nFAT: delete file error during updating FAT entries");
  return STATUS_ERROR;
 }
 
 return STATUS_GOOD;
}

dword_t rewrite_fat_file(dword_t first_cluster, byte_t *file_memory, dword_t file_size_in_bytes) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 
 //delete file
 if(delete_fat_file(first_cluster)==STATUS_ERROR) {
  log("\nFAT: can not delete file to rewrite it");
  return STATUS_ERROR;
 }

 //create file with same cluster
 return create_fat_file(first_cluster, file_memory, file_size_in_bytes);
}

byte_t *read_fat_folder(dword_t cluster) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 struct fat_folder_entry_t *fat_folder;
 
 /* read folder data from partition */
 if(cluster==ROOT_FOLDER) {
  if(fat_info->type==FAT32) {
   fat_folder = (struct fat_folder_entry_t *) (read_fat_file(fat_info->root_directory_location));
  }
  else { //FAT12/16
   struct byte_stream_descriptor_t *descriptor_of_root_folder_sectors = create_descriptor_of_file_sectors(512);
   add_sectors_to_descriptor_of_file_sectors(descriptor_of_root_folder_sectors, fat_info->root_directory_location, fat_info->root_directory_size_in_sectors);
   fat_folder = (struct fat_folder_entry_t *) (read_whole_descriptor_of_file_sector(descriptor_of_root_folder_sectors));
   size_of_loaded_fat_file_in_memory = (fat_info->root_directory_size_in_sectors*512);
   destroy_byte_stream(descriptor_of_root_folder_sectors);
  }
 }
 else {
  //normal folder
  fat_folder = (struct fat_folder_entry_t *) (read_fat_file(cluster));
 }

 //test if there was error
 if((dword_t)fat_folder==STATUS_ERROR) {
  log("\nFAT: reading folder error");
  return STATUS_ERROR;
 }

 /* convert folder data to virtual file system folder */
 struct fat_folder_lfn_entry_t *fat_folder_long_file_entry = (struct fat_folder_lfn_entry_t *) (fat_folder);

 //count basic data
 dword_t number_of_fat_folder_entries = (size_of_loaded_fat_file_in_memory/32); //number according to number of sectors
 number_of_files_in_fat_folder = 0;
 for(dword_t i=0; i<number_of_fat_folder_entries; i++) {
  if(!(fat_folder[i].name[0]=='.' && fat_folder[i].name[1]==' ')
     && !(fat_folder[i].name[0]=='.' && fat_folder[i].name[1]=='.'  && fat_folder[i].name[2]==' ')
     && ((fat_folder[i].attributes & FAT_ATTRIBUTE_FOLDER)==FAT_ATTRIBUTE_FOLDER || (fat_folder[i].attributes & FAT_ATTRIBUTE_ARCHIVE)==FAT_ATTRIBUTE_ARCHIVE)) {
   number_of_files_in_fat_folder++;
  }

  if(fat_folder[i].attributes==0) { //get exact number of entries
   number_of_fat_folder_entries = i;
   break;
  }
 }

 //allocate virtual file system folder
 struct file_descriptor_t *vfs_folder = (struct file_descriptor_t *) (calloc(sizeof(struct file_descriptor_t)*number_of_files_in_fat_folder));

 //convert all entries
 for(dword_t fat_entry=0, vfs_entry=0, lfn_entry=0; fat_entry<number_of_fat_folder_entries; fat_entry++) {
  if(fat_folder[fat_entry].attributes==FAT_ATTRIBUTE_LONG_FILE_NAME) {
   //name is too long
   if(lfn_entry>=19) {
    continue;
   }

   //move chars in actual name to make space for 13 chars at start
   add_space_to_memory_area((dword_t)(&vfs_folder[vfs_entry].name), 256*2, (dword_t)(&vfs_folder[vfs_entry].name), 13*2);

   //copy 13 chars from entry
   copy_memory((dword_t)(&fat_folder_long_file_entry[fat_entry].name_first_5_chars), (dword_t)(&vfs_folder[vfs_entry].name[0]), 5*2);
   copy_memory((dword_t)(&fat_folder_long_file_entry[fat_entry].name_second_6_chars), (dword_t)(&vfs_folder[vfs_entry].name[5]), 6*2);
   copy_memory((dword_t)(&fat_folder_long_file_entry[fat_entry].name_last_2_chars), (dword_t)(&vfs_folder[vfs_entry].name[11]), 2*2);

   //move to next entry
   lfn_entry++;
  }
  else { //entry of normal file
   //set type of file, or skip it
   if((fat_folder[fat_entry].name[0]=='.' && fat_folder[fat_entry].name[1]==' ') || (fat_folder[fat_entry].name[0]=='.' && fat_folder[fat_entry].name[1]=='.'  && fat_folder[fat_entry].name[2]==' ')) {
    continue;
   }
   else if((fat_folder[fat_entry].attributes & FAT_ATTRIBUTE_ARCHIVE)==FAT_ATTRIBUTE_ARCHIVE) {
    vfs_folder[vfs_entry].type = NORMAL_FILE;
   }
   else if((fat_folder[fat_entry].attributes & FAT_ATTRIBUTE_FOLDER)==FAT_ATTRIBUTE_FOLDER) {
    vfs_folder[vfs_entry].type = FILE_FOLDER;
   }
   else { //unknown type of file
    continue;
   }

   //basic info about file
   vfs_folder[vfs_entry].partition_number = selected_partition;
   vfs_folder[vfs_entry].file_location = (fat_folder[fat_entry].low_word_of_cluster | (fat_folder[fat_entry].high_word_of_cluster<<16));
   vfs_folder[vfs_entry].file_size_in_bytes = fat_folder[fat_entry].size_of_file_in_bytes;

   //date and time of creation of file
   vfs_folder[vfs_entry].year_of_creation = (fat_folder[fat_entry].year_of_creation+1980);
   vfs_folder[vfs_entry].month_of_creation = fat_folder[fat_entry].month_of_creation;
   vfs_folder[vfs_entry].day_of_creation = fat_folder[fat_entry].day_of_creation;
   vfs_folder[vfs_entry].hour_of_creation = fat_folder[fat_entry].hour_of_creation;
   vfs_folder[vfs_entry].minute_of_creation = fat_folder[fat_entry].minute_of_creation;
   vfs_folder[vfs_entry].second_of_creation = (fat_folder[fat_entry].second_of_creation*2);

   //date and time of modification of file
   vfs_folder[vfs_entry].year_of_modification = (fat_folder[fat_entry].year_of_modification+1980);
   vfs_folder[vfs_entry].month_of_modification = fat_folder[fat_entry].month_of_modification;
   vfs_folder[vfs_entry].day_of_modification = fat_folder[fat_entry].day_of_modification;
   vfs_folder[vfs_entry].hour_of_modification = fat_folder[fat_entry].hour_of_modification;
   vfs_folder[vfs_entry].minute_of_modification = fat_folder[fat_entry].minute_of_modification;
   vfs_folder[vfs_entry].second_of_modification = (fat_folder[fat_entry].second_of_modification*2);

   //if there were no LFN entries, read name and extension from this entry
   if(vfs_folder[vfs_entry].name[0]==0) {
    //copy name
    for(dword_t i=0; i<8; i++) {
     vfs_folder[vfs_entry].name[i] = fat_folder[fat_entry].name[i];
    }

    //copy extension
    if(vfs_folder[vfs_entry].type==NORMAL_FILE) {
     vfs_folder[vfs_entry].name[8] = '.';
     for(dword_t i=0; i<3; i++) {
      vfs_folder[vfs_entry].name[9+i] = fat_folder[fat_entry].extension[i];
     }
    }
   }

   //parse extension from name
   vfs_entry_parse_extension_from_name((struct file_descriptor_t *)(&vfs_folder[vfs_entry]));

   //move to next entry
   vfs_entry++;
   lfn_entry = 0;
  }
 }

 //free allocated memory
 free((dword_t)fat_folder);

 //return virtual file system folder
 return ((byte_t *)vfs_folder);
}

byte_t rewrite_fat_folder(dword_t folder_location, dword_t previous_folder_location, struct file_descriptor_t *vfs_folder, dword_t number_of_files_in_vfs_folder) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 
 //convert Virtual File System folder to FAT folder
 //allocate memory
 struct fat_folder_entry_t fat_folder_entry;
 struct fat_folder_lfn_entry_t fat_folder_lfn_entry;
 struct byte_stream_descriptor_t *fat_folder_byte_stream = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);

 //get actual time
 read_time();

 //add . and .. to folders
 if(folder_location!=ROOT_FOLDER) {
  //add . entry = pointer to folder itself
  clear_memory((dword_t)(&fat_folder_entry), sizeof(struct fat_folder_entry_t));
  fat_folder_entry.name[0] = '.';
  for(dword_t i=1; i<8; i++) {
   fat_folder_entry.name[i] = ' ';
  }
  for(dword_t i=0; i<3; i++) {
   fat_folder_entry.extension[i] = ' ';
  }
  fat_folder_entry.attributes = FAT_ATTRIBUTE_FOLDER;
  fat_folder_entry.low_word_of_cluster = (folder_location & 0xFFFF);
  fat_folder_entry.high_word_of_cluster = (folder_location >> 16);
  add_bytes_to_byte_stream(fat_folder_byte_stream, (byte_t *)(&fat_folder_entry), sizeof(struct fat_folder_entry_t));

  //add .. entry = pointer to previous folder
  if(previous_folder_location==ROOT_FOLDER) {
   if(fat_info->type==FAT32) {
    previous_folder_location = fat_info->root_directory_location;
   }
   else { //FAT12/16
    previous_folder_location = 0;
   }
  }
  clear_memory((dword_t)(&fat_folder_entry), sizeof(struct fat_folder_entry_t));
  fat_folder_entry.name[0] = '.';
  fat_folder_entry.name[1] = '.';
  for(dword_t i=2; i<8; i++) {
   fat_folder_entry.name[i] = ' ';
  }
  for(dword_t i=0; i<3; i++) {
   fat_folder_entry.extension[i] = ' ';
  }
  fat_folder_entry.attributes = FAT_ATTRIBUTE_FOLDER;
  fat_folder_entry.low_word_of_cluster = (previous_folder_location & 0xFFFF);
  fat_folder_entry.high_word_of_cluster = (previous_folder_location >> 16);
  add_bytes_to_byte_stream(fat_folder_byte_stream, (byte_t *)(&fat_folder_entry), sizeof(struct fat_folder_entry_t));
 }

 //convert entries
 for(dword_t i=0; i<number_of_files_in_vfs_folder; i++) {
  //create fat entry in memory
  //add name
  for(dword_t name_pointer=0; name_pointer<8; name_pointer++) {
   if(vfs_folder[i].name[name_pointer]=='.') {
    break;
   }
   else if(vfs_folder[i].name[name_pointer]>128) {
    fat_folder_entry.name[name_pointer] = '?';
   }
   else {
    fat_folder_entry.name[name_pointer] = get_big_char_value(vfs_folder[i].name[name_pointer]);
   }
  }

  //add extension
  for(dword_t extension_pointer=0; extension_pointer<3; extension_pointer++) {
   fat_folder_entry.extension[extension_pointer] = get_big_char_value(vfs_folder[i].extension[extension_pointer]);
  }

  //set attributes
  if(vfs_folder[i].type==NORMAL_FILE) {
   fat_folder_entry.attributes = FAT_ATTRIBUTE_ARCHIVE;
  }
  else if(vfs_folder[i].type==FILE_FOLDER) {
   fat_folder_entry.attributes = FAT_ATTRIBUTE_FOLDER;
  }
  else {
   fat_folder_entry.attributes = 0;
  }
  fat_folder_entry.reserved = 0;

  //set time of creation
  fat_folder_entry.second_of_creation = (vfs_folder[i].second_of_creation/2);
  fat_folder_entry.minute_of_creation = vfs_folder[i].minute_of_creation;
  fat_folder_entry.hour_of_creation = vfs_folder[i].hour_of_creation;
  fat_folder_entry.day_of_creation = vfs_folder[i].day_of_creation;
  fat_folder_entry.month_of_creation = vfs_folder[i].month_of_creation;
  if(vfs_folder[i].year_of_creation<=1980) {
   fat_folder_entry.year_of_creation = 0;
  }
  else if(vfs_folder[i].year_of_creation>2107) {
   fat_folder_entry.year_of_creation = 0x7F;
  }
  else {
   fat_folder_entry.year_of_creation = (vfs_folder[i].year_of_creation-1980);
  }

  //set time of last access
  fat_folder_entry.day_of_last_access = time_day;
  fat_folder_entry.month_of_last_access = time_month;
  if(time_year<=1980) {
   fat_folder_entry.year_of_last_access = 0;
  }
  else if(time_year>2107) {
   fat_folder_entry.year_of_last_access = 0x7F;
  }
  else {
   fat_folder_entry.year_of_last_access = (time_year-1980);
  }

  //set time of modification
  fat_folder_entry.second_of_modification = (vfs_folder[i].second_of_modification/2);
  fat_folder_entry.minute_of_modification = vfs_folder[i].minute_of_modification;
  fat_folder_entry.hour_of_modification = vfs_folder[i].hour_of_modification;
  fat_folder_entry.day_of_modification = vfs_folder[i].day_of_modification;
  fat_folder_entry.month_of_modification = vfs_folder[i].month_of_modification;
  if(vfs_folder[i].year_of_modification<=1980) {
   fat_folder_entry.year_of_modification = 0;
  }
  else if(vfs_folder[i].year_of_modification>2107) {
   fat_folder_entry.year_of_modification = 0x7F;
  }
  else {
   fat_folder_entry.year_of_modification = (vfs_folder[i].year_of_modification-1980);
  }

  //set cluster and size
  fat_folder_entry.low_word_of_cluster = (vfs_folder[i].file_location & 0xFFFF);
  fat_folder_entry.high_word_of_cluster = (vfs_folder[i].file_location >> 16);
  fat_folder_entry.size_of_file_in_bytes = (vfs_folder[i].file_size_in_bytes);

  //calculate checksum for LFN entries
  byte_t lfn_entry_checksum = 0;
  for(int j=0; j<8; j++) {
   lfn_entry_checksum = (((lfn_entry_checksum & 0x1) ? 0x80 : 0) + (lfn_entry_checksum >> 1) + fat_folder_entry.name[j]);
  }
  for(int j=0; j<3; j++) {
   lfn_entry_checksum = (((lfn_entry_checksum & 0x1) ? 0x80 : 0) + (lfn_entry_checksum >> 1) + fat_folder_entry.extension[j]);
  }

  //calculate varaibles for creating LFN entries
  word_t number_of_characters_in_name = (get_number_of_chars_in_unicode_string(vfs_folder[i].name)+1); //+zero ending
  byte_t number_of_lfn_entries = (number_of_characters_in_name/13);
  if((number_of_characters_in_name%13)!=0) {
   number_of_lfn_entries++;
  }
  word_t file_name[(number_of_lfn_entries*13)];
  for(dword_t j=0; j<(number_of_lfn_entries*13); j++) {
   file_name[j] = 0xFFFF;
  }
  for(dword_t j=0; j<number_of_characters_in_name; j++) {
   file_name[j] = vfs_folder[i].name[j];
  }

  //add LFN entries to folder memory
  for(dword_t lfn_entry=number_of_lfn_entries; lfn_entry>0; lfn_entry--) {
   //create LFN entry in memory
   if(lfn_entry==number_of_lfn_entries) {
    fat_folder_lfn_entry.order = (0x40 | lfn_entry);
   }
   else {
    fat_folder_lfn_entry.order = lfn_entry;
   }
   fat_folder_lfn_entry.attribute = FAT_ATTRIBUTE_LONG_FILE_NAME;
   fat_folder_lfn_entry.entry_type = 0;
   fat_folder_lfn_entry.checksum = lfn_entry_checksum;
   fat_folder_lfn_entry.reserved = 0;

   //copy name
   copy_memory((dword_t)(&file_name[(lfn_entry-1)*13+0]), (dword_t)(&fat_folder_lfn_entry.name_first_5_chars), 5*2);
   copy_memory((dword_t)(&file_name[(lfn_entry-1)*13+5]), (dword_t)(&fat_folder_lfn_entry.name_second_6_chars), 6*2);
   copy_memory((dword_t)(&file_name[(lfn_entry-1)*13+11]), (dword_t)(&fat_folder_lfn_entry.name_last_2_chars), 2*2);

   //add LFN entry to FAT folder in memory
   add_bytes_to_byte_stream(fat_folder_byte_stream, (byte_t *)(&fat_folder_lfn_entry), sizeof(struct fat_folder_lfn_entry_t));
  }

  //add file entry to FAT folder in memory
  add_bytes_to_byte_stream(fat_folder_byte_stream, (byte_t *)(&fat_folder_entry), sizeof(struct fat_folder_entry_t));
 }

 //rewrite folder
 byte_t status = STATUS_ERROR;
 if(folder_location==ROOT_FOLDER) {
  if(fat_info->type==FAT32) {
   if(delete_fat_file(fat_info->root_directory_location)==STATUS_ERROR) {
    log("\nFAT: error with deleting folder");
   }
   else {
    status = create_fat_file(fat_info->root_directory_location, (byte_t *)fat_folder_byte_stream->start_of_allocated_memory, fat_folder_byte_stream->size_of_stream);
   }
  }
  else { //FAT12/16
   if(fat_folder_byte_stream->size_of_stream>(fat_info->root_directory_size_in_sectors*512)) {
    log("\nFAT: root folder is too small");
   }
   else {
    //fill stream with zeroes to be as big as root directory
    for(dword_t i=0; i<((fat_info->root_directory_size_in_sectors*512)-fat_folder_byte_stream->size_of_stream); i++) {
     add_byte_to_byte_stream(fat_folder_byte_stream, 0);
    }

    //rewrite root directory
    struct byte_stream_descriptor_t *descriptor_of_root_folder_sectors = create_descriptor_of_file_sectors(512); //create descriptor
    add_sectors_to_descriptor_of_file_sectors(descriptor_of_root_folder_sectors, fat_info->root_directory_location, fat_info->root_directory_size_in_sectors); //add root directory sectors
    status = write_whole_descriptor_of_file_sector(descriptor_of_root_folder_sectors, (byte_t *)fat_folder_byte_stream->start_of_allocated_memory); //rewrite them
    destroy_byte_stream(descriptor_of_root_folder_sectors); //unallocate descriptor
   }
  }
 }
 else {
  if(delete_fat_file(folder_location)==STATUS_ERROR) {
   log("\nFAT: error with deleting folder");
  }
  else {
   status = create_fat_file(folder_location, (byte_t *)fat_folder_byte_stream->start_of_allocated_memory, fat_folder_byte_stream->size_of_stream);
  }
 }

 destroy_byte_stream(fat_folder_byte_stream);
 return status;
}

dword_t create_fat_folder(dword_t previous_folder_location) {
 struct fat_specific_info_t *fat_info = (struct fat_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //find free cluster
 for(dword_t cluster=fat_info->first_free_cluster; cluster<fat_info->number_of_clusters; cluster++) {
  if(read_fat_table_entry(cluster)==0x00000000) {
   //update first free cluster
   if(fat_info->first_free_cluster<cluster) {
    fat_info->first_free_cluster = cluster;
   }

   //allocate memory for new folder
   struct fat_folder_entry_t *fat_folder = (struct fat_folder_entry_t *) (calloc(fat_info->cluster_size_in_bytes));

   //add . entry = pointer to folder itself
   fat_folder[0].name[0] = '.';
   for(dword_t i=1; i<8; i++) {
    fat_folder[0].name[i] = ' ';
   }
   for(dword_t i=0; i<3; i++) {
    fat_folder[0].extension[i] = ' ';
   }
   fat_folder[0].attributes = FAT_ATTRIBUTE_FOLDER;
   fat_folder[0].low_word_of_cluster = (cluster & 0xFFFF);
   fat_folder[0].high_word_of_cluster = (cluster >> 16);

   //add .. entry = pointer to previous folder
   if(previous_folder_location==ROOT_FOLDER) {
    if(fat_info->type==FAT32) {
     previous_folder_location = fat_info->root_directory_location;
    }
    else { //FAT12/16
     previous_folder_location = 0;
    }
   }
   fat_folder[1].name[0] = '.';
   fat_folder[1].name[1] = '.';
   for(dword_t i=2; i<8; i++) {
    fat_folder[1].name[i] = ' ';
   }
   for(dword_t i=0; i<3; i++) {
    fat_folder[1].extension[i] = ' ';
   }
   fat_folder[1].attributes = FAT_ATTRIBUTE_FOLDER;
   fat_folder[1].low_word_of_cluster = (previous_folder_location & 0xFFFF);
   fat_folder[1].high_word_of_cluster = (previous_folder_location >> 16);

   //write folder data to cluster
   if(write_storage_medium(fat_info->data_first_sector+((cluster-2)*fat_info->cluster_size_in_sectors), fat_info->cluster_size_in_sectors, (dword_t)fat_folder)==STATUS_ERROR) {
    log("\nFAT: create folder error during writing data to cluster");
    free((dword_t)fat_folder);
    return STATUS_ERROR;
   }
   free((dword_t)fat_folder);

   //set cluster in FAT table
   write_fat_table_entry(cluster, fat_info->file_ending_entry_value);
   if(save_loaded_fat_table_sector()==STATUS_ERROR) {
    log("\nFAT: create folder error during updating FAT entries");
    return STATUS_ERROR;
   }

   //return cluster number
   return cluster;
  }
 }

 log("\nFAT: no free cluster for new folder");
 return STATUS_ERROR;
}