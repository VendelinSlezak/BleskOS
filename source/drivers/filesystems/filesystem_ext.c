//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_ext(dword_t first_partition_sector) {
 //read ext superblock
 struct ext_superblock_t ext_superblock;
 if(read_storage_medium(first_partition_sector+2, 1, (dword_t)&ext_superblock)==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //check if this is ext filesystem
 if(ext_superblock.ext_signature==EXT_SIGNATURE
    && ext_superblock.total_inodes!=0
    && ext_superblock.total_blocks!=0
    && ext_superblock.blocks_per_group!=0
    && ext_superblock.inodes_per_group!=0
    && ((ext_superblock.total_blocks%ext_superblock.blocks_per_group)==0 ? (ext_superblock.total_blocks/ext_superblock.blocks_per_group) : (ext_superblock.total_blocks/ext_superblock.blocks_per_group)+1)==(ext_superblock.total_inodes/ext_superblock.inodes_per_group)
    && ext_superblock.file_system_state<0b1000 && ext_superblock.file_system_state!=0
    && ext_superblock.error_handling<=3 && ext_superblock.error_handling!=0) {
  //check if we support all required flags of this filesystem
  if(ext_superblock.major_version>0 && (ext_superblock.required_features & ~(EXT_REQUIRED_FEATURE_DIR_TYPE_FIELD))==0) {
   return STATUS_TRUE;
  }
  else {
   log("\nUnmountable ext "); log_var(ext_superblock.major_version); log("."); log_var_with_space(ext_superblock.minor_version); log("filesystem: "); log_hex(ext_superblock.required_features);
  }
 }

 return STATUS_FALSE;
}

void filesystem_ext_read_specific_info(struct connected_partition_info_t *connected_partition_info) {
 //read ext superblock
 struct ext_superblock_t ext_superblock;
 if(read_storage_medium(connected_partition_info->first_sector+2, 1, (dword_t)&ext_superblock)==STATUS_ERROR) {
  return;
 }

 //allocate memory
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (calloc(sizeof(struct ext_specific_info_t)));

 //read useful values
 ext_info->first_partition_sector = connected_partition_info->first_sector;
 ext_info->required_features = ext_superblock.required_features;
 ext_info->number_of_block_groups = (ext_superblock.total_inodes/ext_superblock.inodes_per_group);
 ext_info->number_of_inodes_in_group = ext_superblock.inodes_per_group;

 ext_info->block_size_in_bytes = (1024 << ext_superblock.log_block_size);
 ext_info->block_size_in_sectors = (ext_info->block_size_in_bytes/512);
 if(ext_superblock.major_version==0) {
  ext_info->inode_size_in_bytes = 128;
 }
 else {
  ext_info->inode_size_in_bytes = ext_superblock.inode_size;
 }

 if(ext_info->block_size_in_bytes==1024) {
  ext_info->block_group_descriptor_first_sector = (connected_partition_info->first_sector+4);
 }
 else {
  ext_info->block_group_descriptor_first_sector = (connected_partition_info->first_sector+ext_info->block_size_in_sectors);
 }

 ext_info->loaded_block_group_descriptor_sector = 0xFFFFFFFF;
 ext_info->loaded_inode_table_sector = 0xFFFFFFFF;

 //save pointer to info to connected_partition_info 
 connected_partition_info->filesystem_specific_info_pointer = ((byte_t *)ext_info);

 //copy partition label
 if(ext_superblock.major_version==0 || ext_superblock.volume_name[0]==0) {
  copy_memory((dword_t)&"Partition", (dword_t)&connected_partition_info->partition_label, sizeof("Partition"));
 }
 else {
  for(int i=0; i<10; i++) {
   connected_partition_info->partition_label[i] = ext_superblock.volume_name[i];
  }
  connected_partition_info->partition_label[10] = 0;
 }
 
 //log
 log("\n\nExt filesystem");
 log("\npartition label: "); log(connected_partition_info->partition_label);
 log("\nblock size in kilobytes: "); log_var(ext_info->block_size_in_bytes/1024);
 log("\ninodes in group: "); log_var(ext_info->number_of_inodes_in_group);
}

byte_t read_ext_inode(dword_t inode) {
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
 if(inode==0) {
  return STATUS_ERROR;
 }
 inode--;

 //read sector with descriptor of group where is inode
 #define EXT_NUMBER_OF_BLOCK_GROUP_DESCRIPTORS_IN_ONE_SECTOR 16
 dword_t group_of_inode = (inode/ext_info->number_of_inodes_in_group);
 dword_t sector_with_group_descriptor = (ext_info->block_group_descriptor_first_sector+(group_of_inode/EXT_NUMBER_OF_BLOCK_GROUP_DESCRIPTORS_IN_ONE_SECTOR)*ext_info->block_size_in_sectors);
 
 if(ext_info->loaded_block_group_descriptor_sector!=sector_with_group_descriptor) {
  if(read_storage_medium(sector_with_group_descriptor, 1, (dword_t)&ext_info->block_group_descriptors_sector)==STATUS_ERROR) {
   log("\nExt: can not read sector with block group descriptor");
   return STATUS_ERROR;
  }
  ext_info->loaded_block_group_descriptor_sector = sector_with_group_descriptor;
 }

 //read sector with inode of file
 struct block_group_descriptor_t *block_group_descriptor = (struct block_group_descriptor_t *) ext_info->block_group_descriptors_sector;
 dword_t inode_table_first_sector = ext_info->first_partition_sector+block_group_descriptor[group_of_inode%EXT_NUMBER_OF_BLOCK_GROUP_DESCRIPTORS_IN_ONE_SECTOR].inode_table_first_block*ext_info->block_size_in_sectors;
 dword_t sector_with_inode = (inode_table_first_sector+(inode%ext_info->number_of_inodes_in_group*ext_info->inode_size_in_bytes/512));
 
 if(ext_info->loaded_inode_table_sector!=sector_with_inode) {
  if(read_storage_medium(sector_with_inode, 2, (dword_t)&ext_info->inode_table_sectors)==STATUS_ERROR) {
   log("\nExt: can not read sector with inode");
   return STATUS_ERROR;
  }
  ext_info->loaded_inode_table_sector = sector_with_inode;
 }

 //copy inode
 copy_memory(((dword_t)(&ext_info->inode_table_sectors))+(inode%ext_info->number_of_inodes_in_group*ext_info->inode_size_in_bytes%512), (dword_t)(&ext_info->inode), sizeof(struct ext2_inode_t));
 
 return STATUS_GOOD;
}

byte_t *read_ext_file(dword_t file_location) {
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //read inode
 if(read_ext_inode(file_location)==STATUS_ERROR) {
  log("\nExt: can not read inode "); log_var(file_location);
  return STATUS_ERROR;
 }

 //TODO: check if file data is not in inode
 //TODO: add support for extent tree

 //create DOFS
 struct byte_stream_descriptor_t *descriptor_of_file_sectors = create_descriptor_of_file_sectors(512);
 size_of_loaded_ext_file_in_memory = 0;

 //add direct block pointers sectors
 for(dword_t i=0; i<12; i++) {
  if(ext_info->inode.direct_block_ptr[i]==0) {
   break;
  }
  else {
   add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, ext_info->first_partition_sector+(ext_info->inode.direct_block_ptr[i]*ext_info->block_size_in_sectors), ext_info->block_size_in_sectors);
   size_of_loaded_ext_file_in_memory += ext_info->block_size_in_bytes;
  }
 }

 //add indirect block pointers sectors
 if(ext_add_single_indirect_pointer_blocks(descriptor_of_file_sectors, ext_info->inode.single_indirect_block_ptr)==STATUS_ERROR) {
  log("\nExt: can not read single indirect block pointers");
  return STATUS_ERROR;
 }

 //add double indirect block pointers sectors
 if(ext_add_double_indirect_pointer_blocks(descriptor_of_file_sectors, ext_info->inode.double_indirect_block_ptr)==STATUS_ERROR) {
  log("\nExt: can not read double indirect block pointers");
  return STATUS_ERROR;
 }

 //add triple indirect block pointers sectors
 if(ext_add_triple_indirect_pointer_blocks(descriptor_of_file_sectors, ext_info->inode.triple_indirect_block_ptr)==STATUS_ERROR) {
  log("\nExt: can not read triple indirect block pointers");
  return STATUS_ERROR;
 }

 //read file
 byte_t *file_memory = read_whole_descriptor_of_file_sector(descriptor_of_file_sectors);

 //release memory
 destroy_byte_stream(descriptor_of_file_sectors);

 //return pointer to loaded data or STATUS_ERROR
 return file_memory;
}

byte_t ext_add_single_indirect_pointer_blocks(struct byte_stream_descriptor_t *descriptor_of_file_sectors, dword_t block) {
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 if(block==0) {
  return STATUS_GOOD;
 }

 //load block
 dword_t *block_data = (dword_t *) malloc(ext_info->block_size_in_bytes);
 if(read_storage_medium(ext_info->first_partition_sector+(block*ext_info->block_size_in_sectors), ext_info->block_size_in_sectors, (dword_t)block_data)==STATUS_ERROR) {
  free((dword_t)block_data);
  return STATUS_ERROR;
 }

 //add blocks to descriptor of file sectors
 for(dword_t i=0; i<(ext_info->block_size_in_bytes/4); i++) {
  if(block_data[i]==0) {
   break;
  }
  else {
   add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, ext_info->first_partition_sector+(block_data[i]*ext_info->block_size_in_sectors), ext_info->block_size_in_sectors);
   size_of_loaded_ext_file_in_memory += ext_info->block_size_in_bytes;
  }
 }

 free((dword_t)block_data);
 return STATUS_GOOD;
}

byte_t ext_add_double_indirect_pointer_blocks(struct byte_stream_descriptor_t *descriptor_of_file_sectors, dword_t block) {
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 if(block==0) {
  return STATUS_GOOD;
 }

 //load block
 dword_t *block_data = (dword_t *) malloc(ext_info->block_size_in_bytes);
 if(read_storage_medium(ext_info->first_partition_sector+(block*ext_info->block_size_in_sectors), ext_info->block_size_in_sectors, (dword_t)block_data)==STATUS_ERROR) {
  free((dword_t)block_data);
  return STATUS_ERROR;
 }

 //add blocks to descriptor of file sectors
 for(dword_t i=0; i<(ext_info->block_size_in_bytes/4); i++) {
  if(block_data[i]==0) {
   break;
  }
  else {
   if(ext_add_single_indirect_pointer_blocks(descriptor_of_file_sectors, block_data[i])==STATUS_ERROR) {
    free((dword_t)block_data);
    return STATUS_ERROR;
   }
  }
 }

 free((dword_t)block_data);
 return STATUS_GOOD;
}

byte_t ext_add_triple_indirect_pointer_blocks(struct byte_stream_descriptor_t *descriptor_of_file_sectors, dword_t block) {
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 if(block==0) {
  return STATUS_GOOD;
 }

 //load block
 dword_t *block_data = (dword_t *) malloc(ext_info->block_size_in_bytes);
 if(read_storage_medium(ext_info->first_partition_sector+(block*ext_info->block_size_in_sectors), ext_info->block_size_in_sectors, (dword_t)block_data)==STATUS_ERROR) {
  free((dword_t)block_data);
  return STATUS_ERROR;
 }

 //add blocks to descriptor of file sectors
 for(dword_t i=0; i<(ext_info->block_size_in_bytes/4); i++) {
  if(block_data[i]==0) {
   break;
  }
  else {
   if(ext_add_double_indirect_pointer_blocks(descriptor_of_file_sectors, block_data[i])==STATUS_ERROR) {
    free((dword_t)block_data);
    return STATUS_ERROR;
   }
  }
 }

 free((dword_t)block_data);
 return STATUS_GOOD;
}

byte_t *read_ext_folder(dword_t folder_location) {
 struct ext_specific_info_t *ext_info = (struct ext_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);

 //root folder
 if(folder_location==ROOT_FOLDER) {
  folder_location = 2;
 }

 //read folder data
 byte_t *ext_folder = read_ext_file(folder_location);
 if((dword_t)ext_folder==STATUS_ERROR) {
  log("\nExt: can not read folder data");
  return STATUS_ERROR;
 }

 //count number of files
 number_of_files_in_ext_folder = 0;
 struct ext_folder_entry_t *ext_folder_entry = (struct ext_folder_entry_t *) ext_folder;
 while((dword_t)ext_folder_entry<((dword_t)ext_folder+size_of_loaded_ext_file_in_memory)) {
  //this is not valid entry
  if(ext_folder_entry->inode==0) {
   break;
  }

  //skip . and .. entries
  if(!(ext_folder_entry->name[0]=='.' && ext_folder_entry->name[1]==0)
     && !(ext_folder_entry->name[0]=='.' && ext_folder_entry->name[1]=='.' && ext_folder_entry->name[2]==0)) {
   number_of_files_in_ext_folder++;
  }

  //move pointer to next entry
  ext_folder_entry = (struct ext_folder_entry_t *) ((dword_t)ext_folder_entry+ext_folder_entry->size_of_entry);
 }

 //allocate VFS folder memory
 struct file_descriptor_t *vfs_folder = (struct file_descriptor_t *) (calloc(sizeof(struct file_descriptor_t)*number_of_files_in_ext_folder));

 //convert ext folder to VFS folder
 ext_folder_entry = (struct ext_folder_entry_t *) ext_folder;
 for(dword_t i=0; i<number_of_files_in_ext_folder; i++) {
  //skip . and .. entries
  if((ext_folder_entry->name[0]=='.' && ext_folder_entry->name[1]==0)
     || (ext_folder_entry->name[0]=='.' && ext_folder_entry->name[1]=='.' && ext_folder_entry->name[2]==0)) {
   ext_folder_entry = (struct ext_folder_entry_t *) ((dword_t)ext_folder_entry+ext_folder_entry->size_of_entry);
   i--;
   continue;
  }

  //read inode of file
  if(read_ext_inode(ext_folder_entry->inode)==STATUS_ERROR) {
   log("\nExt: can not read inode of folder entry");
   free((dword_t)ext_folder);
   return STATUS_ERROR;
  }

  vfs_folder[i].partition_number = selected_partition;
  vfs_folder[i].type = NORMAL_FILE;
  if((ext_info->required_features & EXT_REQUIRED_FEATURE_DIR_TYPE_FIELD)==EXT_REQUIRED_FEATURE_DIR_TYPE_FIELD && ext_folder_entry->type_indicator==2) {
   vfs_folder[i].type = FILE_FOLDER;
  }
  vfs_folder[i].file_location = ext_folder_entry->inode;
  //TODO: process flag 64 bit with size
  vfs_folder[i].file_size_in_bytes = ext_info->inode.size_lower;

  convert_unix_time(ext_info->inode.creation_time);
  vfs_folder[i].year_of_creation = math_year;
  vfs_folder[i].month_of_creation = math_month;
  vfs_folder[i].day_of_creation = math_day;

  convert_unix_time(ext_info->inode.modification_time);
  vfs_folder[i].year_of_modification = math_year;
  vfs_folder[i].month_of_modification = math_month;
  vfs_folder[i].day_of_modification = math_day;

  //if size of name is bigger than 256 characters
  if((ext_info->required_features & EXT_REQUIRED_FEATURE_DIR_TYPE_FIELD)==0 && ext_folder_entry->type_indicator!=0) {
   ext_folder_entry->name_length = 0xFF;
  }

  //convert name from UTF-8 to Unicode
  convert_utf_8_to_unicode((dword_t)(&ext_folder_entry->name), (dword_t)(&vfs_folder[i].name), 255);
  vfs_entry_parse_extension_from_name((struct file_descriptor_t *)(&vfs_folder[i]));

  //move pointer to next entry
  ext_folder_entry = (struct ext_folder_entry_t *) ((dword_t)ext_folder_entry+ext_folder_entry->size_of_entry);
 }

 //free allocated memory
 free((dword_t)ext_folder);

 //return virtual file system folder
 return ((byte_t *)vfs_folder);
}