//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_ext(dword_t first_partition_sector) {
 if(read_storage_medium(first_partition_sector+2, 1, (dword_t)&ext_one_sector)==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 if(ext_one_sector[0]!=0 && ext_one_sector[1]!=0 && ext_one_sector[8]!=0 && ext_one_sector[10]!=0 && ((ext_one_sector[8]/ext_one_sector[1])==(ext_one_sector[10]/ext_one_sector[0])) && (ext_one_sector[14] & 0xFFFF)==0xEF53 && ((ext_one_sector[14]>>16)<=0b111 && (ext_one_sector[14]>>16)!=0) && ((ext_one_sector[15] & 0xFFFF)<=3 && (ext_one_sector[15] & 0xFFFF)!=0)) {
  if(ext_one_sector[19]!=0 && (ext_one_sector[24])!=0b0010) { //check flags
   return STATUS_ERROR;
  }
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void select_ext_partition(dword_t first_partition_sector) {
 //load sector with superblock
 if(read_storage_medium(first_partition_sector+2, 1, (dword_t)&ext_one_sector)==STATUS_ERROR) {
  return;
 }
 if(ext_one_sector[0]==0) {
  return;
 }
 ext_partition_first_sector = first_partition_sector;

 //read variables
 ext_version = ext_one_sector[19];

 ext_size_of_block_in_bytes = (1024 << ext_one_sector[6]);
 ext_size_of_block_in_sectors = (ext_size_of_block_in_bytes/512);

 if(ext_size_of_block_in_bytes==1024) {
  ext_block_descriptor_first_sector = (first_partition_sector+4);
 }
 else {
  ext_block_descriptor_first_sector = (first_partition_sector+ext_size_of_block_in_sectors);
 }
 ext_block_group_descriptor_size_in_bytes = 32;
 if((ext_one_sector[24] & 0x80)==0x80) { //64 bit feature
  ext_block_group_descriptor_size_in_bytes = 64;
 }

 ext_number_of_block_groups = ((ext_one_sector[8]/ext_one_sector[0]));
 if(ext_number_of_block_groups==0) {
  ext_number_of_block_groups = 1;
 }

 ext_num_of_groups_per_flex_group = 1;
 if((ext_one_sector[24] & 0x200)==0x200) { //flex groups feature
  ext_num_of_groups_per_flex_group = power(2, (ext_one_sector[93] & 0xFF));
 }
 ext_number_of_blocks_in_group = (ext_one_sector[8]*ext_num_of_groups_per_flex_group);
 ext_number_of_inodes_in_group = (ext_one_sector[10]*ext_num_of_groups_per_flex_group);

 //in versions below 1.0
 ext_inode_size_in_bytes = 128;
 for(int i=0; i<12; i++) {
  partition_label[i]=0;
 }

 //versions 1.0+
 if(ext_version!=0) {
  ext_inode_size_in_bytes = ext_one_sector[22];

  byte_t *ext_partition_label = (byte_t *) (((dword_t)&ext_one_sector)+120);
  for(int i=0; i<11; i++) {
   partition_label[i]=ext_partition_label[i];
  }
 }

 //calculate this value for achieving greater preformance
 if(ext_size_of_block_in_sectors<128) {
  ext_max_number_of_blocks_per_one_read = (128/ext_size_of_block_in_sectors);
 }
 else {
  ext_max_number_of_blocks_per_one_read = 1;
 }

 ext_read_folder(ROOT_DIRECTORY, 0xFFFFFFFF);
}

void set_ext_partition_info_in_device_list_entry(void) {
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+0, ext_version);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+1, ext_size_of_block_in_bytes);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+2, ext_size_of_block_in_sectors);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+3, ext_block_descriptor_first_sector);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+4, ext_block_group_descriptor_size_in_bytes);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+5, ext_number_of_block_groups);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+6, ext_number_of_blocks_in_group);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+7, ext_number_of_inodes_in_group);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+8, ext_inode_size_in_bytes);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+9, ext_max_number_of_blocks_per_one_read);
 set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+10, ext_partition_first_sector);
}

void read_ext_partition_info_from_device_list_entry(void) {
 ext_version = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+0);
 ext_size_of_block_in_bytes = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+1);
 ext_size_of_block_in_sectors = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+2);
 ext_block_descriptor_first_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+3);
 ext_block_group_descriptor_size_in_bytes = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+4);
 ext_number_of_block_groups = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+5);
 ext_number_of_blocks_in_group = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+6);
 ext_number_of_inodes_in_group = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+7);
 ext_inode_size_in_bytes = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+8);
 ext_max_number_of_blocks_per_one_read = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+9);
 ext_partition_first_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_UNIQUE_INFO+10);
}

byte_t ext_read_block(dword_t block, dword_t memory) {
 return read_storage_medium(ext_partition_first_sector+block*ext_size_of_block_in_sectors, ext_size_of_block_in_sectors, memory);
}

byte_t ext_read_blocks(dword_t block, dword_t num_of_blocks, dword_t memory) {
 return read_storage_medium(ext_partition_first_sector+block*ext_size_of_block_in_sectors, ext_size_of_block_in_sectors*num_of_blocks, memory);
}

dword_t ext_read_inode(dword_t inode) {
 //read block group decriptor of group where is inode
 dword_t block_group_of_inode = ((inode-1)/ext_number_of_inodes_in_group);
 if(read_storage_medium(ext_block_descriptor_first_sector+(block_group_of_inode*ext_block_group_descriptor_size_in_bytes/512), 1, (dword_t)&ext_one_sector)==STATUS_ERROR) { //read sector with block group descriptor
  return STATUS_ERROR;
 }
 dword_t *block_group_descriptor = (dword_t *) (((dword_t)&ext_one_sector)+(block_group_of_inode*ext_block_group_descriptor_size_in_bytes%512));

 //read block where is inode
 dword_t first_block_of_inodes = (block_group_descriptor[2]);
 dword_t inode_block = (first_block_of_inodes + (ext_inode_size_in_bytes*((inode-1)%ext_number_of_inodes_in_group)/ext_size_of_block_in_bytes));
 dword_t block_mem = malloc(ext_size_of_block_in_bytes);
 if(ext_read_block(inode_block, block_mem)==STATUS_ERROR) {
  log("\next: error reading inode ");
  log_hex(inode);
  log_hex(inode_block);
  free(block_mem);
  return STATUS_ERROR;
 }

 //copy inode from block
 dword_t inode_mem = malloc(ext_inode_size_in_bytes);
 byte_t *inode_in_block = (byte_t *) (block_mem+(ext_inode_size_in_bytes*((inode-1)%ext_number_of_inodes_in_group)%ext_size_of_block_in_bytes));
 byte_t *inode_output = (byte_t *) (inode_mem);
 for(dword_t i=0; i<ext_inode_size_in_bytes; i++) {
  *inode_output = *inode_in_block;
  inode_output++;
  inode_in_block++;
 }
 free(block_mem);
 return inode_mem;
}

byte_t ext_read_indirect_block_pointers(dword_t block) {
 //read block with pointers
 if(ext_read_block(block, ext_indirect_block_memory)==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //read blocks of file
 dword_t *indirect_block = (dword_t *) (ext_indirect_block_memory);
 for(dword_t i=0, number_of_blocks_to_read = 1; i<(ext_size_of_block_in_bytes/4); i+=number_of_blocks_to_read) {
  if(indirect_block[i]==0) {
   return STATUS_GOOD;
  }

  //update info about reading progress on screen
  if(file_show_file_work_progress==1) {
   file_work_done_percents = (100*(ext_already_readed_bytes/1024)/(ext_how_many_bytes_to_read/1024));
   file_dialog_show_progress();
  }

  //if there are multiple blocks immediately after each other, we can read multiple blocks on one request
  number_of_blocks_to_read = 1;
  if(ext_max_number_of_blocks_per_one_read>1) {
   for(dword_t j=0; j<ext_max_number_of_blocks_per_one_read; j++) {
    if(((i+j)!=(ext_size_of_block_in_bytes/4)-1) && indirect_block[i+j+1]==(indirect_block[i+j]+1)) {
     number_of_blocks_to_read++;
    }
    else {
     break;
    }
   }
  }

  //read blocks
  if(ext_read_blocks(indirect_block[i], number_of_blocks_to_read, ext_pointer_to_file_in_memory)==STATUS_ERROR) {
   log("\next: error reading blocks ");
   log_hex(indirect_block[i]);
   log_var(number_of_blocks_to_read);
   return STATUS_ERROR;
  }
  ext_pointer_to_file_in_memory += ext_size_of_block_in_bytes*number_of_blocks_to_read;
  ext_already_readed_bytes += ext_size_of_block_in_bytes*number_of_blocks_to_read;
 }

 return STATUS_GOOD;
}

byte_t ext_read_double_indirect_block_pointers(dword_t block) {
 //read block with pointers
 if(ext_read_block(block, ext_double_indirect_block_memory)==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //read blocks of file
 dword_t *double_indirect_block = (dword_t *) (ext_double_indirect_block_memory);
 for(dword_t i=0; i<(ext_size_of_block_in_bytes/4); i++) {
  if(double_indirect_block[i]==0) {
   return STATUS_GOOD;
  }

  //read indirect block
  if(ext_read_indirect_block_pointers(double_indirect_block[i])==STATUS_ERROR) {
   return STATUS_ERROR;
  }
 }

 return STATUS_GOOD;
}

byte_t ext_read_triple_indirect_block_pointers(dword_t block) {
 //read block with pointers
 if(ext_read_block(block, ext_triple_indirect_block_memory)==STATUS_ERROR) {
  return STATUS_ERROR;
 }

 //read blocks of file
 dword_t *triple_indirect_block = (dword_t *) (ext_triple_indirect_block_memory);
 for(dword_t i=0; i<(ext_size_of_block_in_bytes/4); i++) {
  if(triple_indirect_block[i]==0) {
   return STATUS_GOOD;
  }

  //read double indirect block
  if(ext_read_double_indirect_block_pointers(triple_indirect_block[i])==STATUS_ERROR) {
   return STATUS_ERROR;
  }
 }

 return STATUS_GOOD;
}

dword_t ext_read_file(dword_t inode, dword_t size_in_bytes) {
 ext_file_size_in_bytes = 0;

 //initalize output for user
 file_work_done_percents = 0;
 if(file_show_file_work_progress==1) {
  file_dialog_show_progress();
 }

 //read inode
 dword_t inode_mem = ext_read_inode(inode);
 if(inode_mem==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 dword_t *inode32 = (dword_t *) (inode_mem);
 if(ext_version!=0 && (inode32[8] & 0x4000)!=0x4000 && inode32[27]!=0) {
  return STATUS_ERROR; //file bigger than 4 GB
 }
 dword_t file_mem = 0;
 ext_file_size_in_bytes = inode32[1];
 if(size_in_bytes==0xFFFFFFFF) { //we want to load full file
  file_mem = calloc(ext_file_size_in_bytes-(ext_file_size_in_bytes%ext_size_of_block_in_bytes)+ext_size_of_block_in_bytes); //recalculate to align of blocks
 }
 else {
  file_mem = calloc(size_in_bytes-(size_in_bytes%ext_size_of_block_in_bytes)+ext_size_of_block_in_bytes); //recalculate to align of blocks
 }

 //TODO: read file with extent tree
 if((inode32[8] & 0x80000)==0x80000) {
  log("\nExtent tree");
  free(inode_mem);
  return STATUS_ERROR;
 }

 //read file
 ext_how_many_bytes_to_read = size_in_bytes;
 ext_pointer_to_file_in_memory = file_mem;
 ext_already_readed_bytes = 0;
 ext_indirect_block_memory = malloc(ext_size_of_block_in_bytes);
 ext_double_indirect_block_memory = malloc(ext_size_of_block_in_bytes);
 ext_triple_indirect_block_memory = malloc(ext_size_of_block_in_bytes);
 for(dword_t i=10; i<22; i++) { //read direct block pointers
  if(inode32[i]==0) {
   break;
  }

  //update info about reading progress on screen
  if(file_show_file_work_progress==1 && ext_how_many_bytes_to_read>=1024) {
   file_work_done_percents = (100*(ext_already_readed_bytes/1024)/(ext_how_many_bytes_to_read/1024));
   file_dialog_show_progress();
  }

  //read block
  if(ext_read_block(inode32[i], ext_pointer_to_file_in_memory)==STATUS_ERROR) {
   goto error;
  }
  ext_pointer_to_file_in_memory += ext_size_of_block_in_bytes;
  ext_already_readed_bytes += ext_size_of_block_in_bytes;
 }
 if(inode32[22]!=0) { //read indirect block pointers
  if(ext_read_indirect_block_pointers(inode32[22])==STATUS_ERROR) {
   goto error;
  }
 }
 if(inode32[23]!=0) { //read double indirect block pointers
  if(ext_read_double_indirect_block_pointers(inode32[23])==STATUS_ERROR) {
   goto error;
  }
 }
 if(inode32[24]!=0) { //read triple indirect block pointers
  if(ext_read_triple_indirect_block_pointers(inode32[24])==STATUS_ERROR) {
   goto error;
  }
 }

 //file successfully readed
 free(ext_indirect_block_memory);
 free(ext_double_indirect_block_memory);
 free(ext_triple_indirect_block_memory);
 free(inode_mem);
 if(size_in_bytes!=0xFFFFFFFF) {
  file_mem = realloc(file_mem, size_in_bytes);
 }
 return file_mem;

 error:
 free(ext_indirect_block_memory);
 free(ext_double_indirect_block_memory);
 free(ext_triple_indirect_block_memory);
 free(inode_mem);
 free(file_mem);
 return STATUS_ERROR;
}

dword_t ext_read_folder(dword_t folder_inode, dword_t size_in_bytes) {
 //root directory
 if(folder_inode==ROOT_DIRECTORY) {
  folder_inode = 2;
 }

 //read directory content
 dword_t ext_folder_mem = ext_read_file(folder_inode, 0xFFFFFFFF);
 if(ext_folder_mem==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 ext_folder_mem = realloc(ext_folder_mem, ext_file_size_in_bytes+4); //add zero ending

 //count number of entries
 dword_t *ext_folder32 = (dword_t *) (ext_folder_mem);
 dword_t number_of_entries = 0;
 while(((dword_t)ext_folder32)<(ext_folder_mem+ext_file_size_in_bytes)) {
  if(ext_folder32[0]!=0) {
   number_of_entries++;
   if((ext_folder32[1] & 0xFFFF)==0) {
    break;
   }
   ext_folder32 = (dword_t *) (((dword_t)ext_folder32)+(ext_folder32[1] & 0xFFFF));
  }
  else {
   break;
  }
 }

 //convert from Ext folder to BleskOS virtual file system folder
 dword_t folder_mem = 0;
 if(number_of_entries==0) {
  folder_mem = calloc(256);
 }
 else {
  folder_mem = calloc((number_of_entries*256)+256);

  byte_t *folder = (byte_t *) (folder_mem);
  byte_t *ext_folder8 = (byte_t *) (ext_folder_mem);

  for(dword_t i=0; i<number_of_entries; i++) {
   //clear entry
   for(dword_t i=0; i<32; i++) {
    folder[i]=0;
   }

   //copy name
   word_t *ext_folder16 = (word_t *) (((dword_t)ext_folder8)+4);
   for(dword_t j=0, ext_name_pointer=8; j<(*ext_folder16); j++) {
    //short name
    if(j<8) {
     folder[j] = ext_folder8[8+j];
    }

    //convert from utf-8 to unicode
    if(ext_folder8[ext_name_pointer]==0) {
     break;
    }
    else if(ext_folder8[ext_name_pointer]<0x80) {
     folder[32+j*2] = ext_folder8[ext_name_pointer];
     ext_name_pointer++;
    }
    else if((ext_folder8[ext_name_pointer] & 0xE0)==0xC0) {
     folder[32+j*2] = (((ext_folder8[ext_name_pointer+1] & 0x3F) | ((ext_folder8[ext_name_pointer] & 0x1F)<<6)) & 0xFF);
     folder[32+j*2+1] = (((ext_folder8[ext_name_pointer+1] & 0x3F) | ((ext_folder8[ext_name_pointer] & 0x1F)<<6))>>8);
     ext_name_pointer += 2;
    }
    else if((ext_folder8[ext_name_pointer] & 0xF0)==0xE0){
     folder[32+j*2] = (((ext_folder8[ext_name_pointer+2] & 0x3F) | ((ext_folder8[ext_name_pointer+1] & 0x3F)<<6) | ((ext_folder8[ext_name_pointer] & 0x0F)<<12)) & 0xFF);
     folder[32+j*2+1] = (((ext_folder8[ext_name_pointer+2] & 0x3F) | ((ext_folder8[ext_name_pointer+1] & 0x3F)<<6) | ((ext_folder8[ext_name_pointer] & 0x0F)<<12))>>8);
     ext_name_pointer += 3;
    }
    else {
     break; //unknown char
    }
   }

   //inode
   folder[21]=ext_folder8[3];
   folder[20]=ext_folder8[2];
   folder[27]=ext_folder8[1];
   folder[26]=ext_folder8[0];

   //read inode to get size and date of file
   ext_folder32 = (dword_t *) (((dword_t)ext_folder8));
   dword_t inode_mem = ext_read_inode(*ext_folder32);
   if(inode_mem==STATUS_ERROR) {
    folder[11]=0; //attribute
   }
   else {
    byte_t *inode8 = (byte_t *) (inode_mem);

    //attribute
    if(ext_folder8[7]==0x2) {
     folder[11]=0x10; //directory
    }
    else if(ext_folder8[7]==0x1) {
     folder[11]=0x20; //normal file
    }
    else {
     folder[11]=0x20; //TODO: special file
    }

    //size of file
    folder[28]=inode8[4];
    folder[29]=inode8[5];
    folder[30]=inode8[6];
    folder[31]=inode8[7];

    //date
    dword_t *inode32 = (dword_t *) (((dword_t)inode8)+12);
    convert_unix_time(*inode32);
    if(math_year<=1980) {
     math_year = 0;
    }
    else {
     math_year -= 1980;
    }
    dword_t date = ((math_year<<9) | ((math_month & 0xF)<<5) | (math_day & 0x1F));
    folder[16] = (date & 0xFF);
    folder[17] = ((date>>8) & 0xFF);
    folder[18] = (date & 0xFF);
    folder[19] = ((date>>8) & 0xFF);
    folder[24] = (date & 0xFF);
    folder[25] = ((date>>8) & 0xFF);

    free(inode_mem);
   }

   //next entry
   folder += 256;
   ext_folder8 = (byte_t *) (((dword_t)ext_folder8)+(*ext_folder16));
  }
 }

 free(ext_folder_mem);
 return folder_mem;
}