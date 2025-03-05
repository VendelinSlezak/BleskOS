//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_drivers_of_filesystems(void) {
 array_of_loaded_folders = (struct array_of_loaded_folders_t *) (calloc(sizeof(struct array_of_loaded_folders_t)*MAX_NUMBER_OF_LOADED_FOLDERS));
 array_of_folder_structures = (dword_t *) (calloc(4*MAX_NUMBER_OF_FOLDER_STRUCTURES));
 number_of_folder_structures = 0;
}

void release_loaded_folder_in_array_from_usage_in_structure(word_t folder_number_in_array) {
 if(array_of_loaded_folders[folder_number_in_array].state==AOLF_FOLDER_LOADED) {
  array_of_loaded_folders[folder_number_in_array].number_of_structures_using_this_folder--;
  
  if(array_of_loaded_folders[folder_number_in_array].number_of_structures_using_this_folder==0) {
   free((void *)array_of_loaded_folders[folder_number_in_array].memory_of_entries);
   array_of_loaded_folders[folder_number_in_array].state=AOLF_FREE_ENTRY;
  }
 }
}

byte_t is_filesystem_read_write(byte_t filesystem) {
 if(filesystem==PARTITION_FILESYSTEM_FAT) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

word_t read_folder(dword_t partition_number, dword_t folder_location, dword_t folder_size) {
 //search array if this folder is not already loaded in memory
 for(dword_t i=1; i<MAX_NUMBER_OF_LOADED_FOLDERS; i++) {
  if(array_of_loaded_folders[i].state==AOLF_FOLDER_LOADED && array_of_loaded_folders[i].partition_number==partition_number && array_of_loaded_folders[i].folder_location==folder_location) {
   array_of_loaded_folders[i].number_of_structures_using_this_folder++;
   return i;
  }
 }

 //this folder is not loaded, so read it
 for(dword_t i=1; i<MAX_NUMBER_OF_LOADED_FOLDERS; i++) {
  if(array_of_loaded_folders[i].state==AOLF_FREE_ENTRY) {
   //select partition
   if(select_partition(partition_number)==STATUS_ERROR) {
    logf("\nAOLF: invalid partition");
    return STATUS_ERROR;
   }

   //read folder
   byte_t *folder_memory;
   dword_t number_of_entries;
   dofs_can_user_cancel_action = USER_CAN_NOT_CANCEL_THIS_DOFS_ACTION;
   if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
    folder_memory = read_fat_folder(folder_location);
    number_of_entries = number_of_files_in_fat_folder;
   }
   else if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_EXT) {
    folder_memory = read_ext_folder(folder_location);
    number_of_entries = number_of_files_in_ext_folder;
   }
   else if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_ISO9660) {
    folder_memory = read_iso9660_folder(folder_location, folder_size);
    number_of_entries = number_of_files_in_iso9660_folder;
   }
   else if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_CDDA) {
    folder_memory = read_cdda_root_folder();
    number_of_entries = number_of_files_in_cdda_folder;
   }
   else {
    logf("\nAOLF: not readable filesystem %d on partition %d", connected_partitions[partition_number].filesystem, partition_number);
    return STATUS_ERROR;
   }

   if((dword_t)folder_memory==STATUS_ERROR) {
    logf("\nAOLF: error with reading folder");
    return STATUS_ERROR;
   }

   //create entry
   array_of_loaded_folders[i].state = AOLF_FOLDER_LOADED;
   array_of_loaded_folders[i].partition_number = partition_number;
   array_of_loaded_folders[i].folder_location = folder_location;
   array_of_loaded_folders[i].memory_of_entries = folder_memory;
   array_of_loaded_folders[i].number_of_files = number_of_entries;
   array_of_loaded_folders[i].number_of_structures_using_this_folder = 1;

   //return entry
   return i;
  }
 }

 //all entries are used
 logf("\nAOLF: not enough entries");
 return STATUS_ERROR;
}

byte_t rewrite_folder(dword_t partition_number, dword_t folder_location, dword_t previous_folder_location, struct file_descriptor_t *vfs_folder, dword_t number_of_files_in_vfs_folder) {
 //select partition
 if(select_partition(partition_number)==STATUS_ERROR) {
  logf("\nREWRITE FOLDER: invalid partition");
  return STATUS_ERROR;
 }

 //rewrite folder
 dofs_can_user_cancel_action = USER_CAN_NOT_CANCEL_THIS_DOFS_ACTION;
 if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
  return rewrite_fat_folder(folder_location, previous_folder_location, vfs_folder, number_of_files_in_vfs_folder);
 }

 logf("\nREWRITE FOLDER: not readable filesystem");
 return STATUS_ERROR;
}

dword_t create_folder(dword_t partition_number, dword_t previous_folder_location) {
 //select partition
 if(select_partition(partition_number)==STATUS_ERROR) {
  logf("\nCREATE FOLDER: invalid partition");
  return STATUS_ERROR;
 }

 //rewrite folder
 dofs_can_user_cancel_action = USER_CAN_NOT_CANCEL_THIS_DOFS_ACTION;
 if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
  return create_fat_folder(previous_folder_location);
 }

 logf("\nCREATE FOLDER: not readable filesystem");
 return STATUS_ERROR;
}

byte_t *read_file(dword_t partition_number, dword_t file_location, dword_t file_size) {
 //select partition
 if(select_partition(partition_number)==STATUS_ERROR) {
  logf("\nREAD FILE: invalid partition");
  return STATUS_ERROR;
 }

 //read file
 dofs_can_user_cancel_action = USER_CAN_CANCEL_THIS_DOFS_ACTION;
 dofs_task_completed_number_of_bytes = 0;
 if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
  return read_fat_file(file_location);
 }
 else if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_EXT) {
  return read_ext_file(file_location);
 }
 else if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_ISO9660) {
  return read_iso9660_file(file_location, file_size);
 }
 else if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_CDDA) {
  return read_cdda_file(file_location, file_size);
 }
 else {
  logf("\nREAD FILE: not readable filesystem");
  return STATUS_ERROR;
 }
}

dword_t rewrite_file(dword_t partition_number, dword_t file_location, byte_t *file_memory, dword_t file_size_in_bytes) {
 //select partition
 if(select_partition(partition_number)==STATUS_ERROR) {
  logf("\nREWRITE FILE: invalid partition");
  return STATUS_ERROR;
 }

 //create file
 dofs_can_user_cancel_action = USER_CAN_CANCEL_THIS_DOFS_ACTION;
 dofs_task_completed_number_of_bytes = 0;
 if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
  return rewrite_fat_file(file_location, file_memory, file_size_in_bytes);
 }
 else {
  logf("\nREWRITE FILE: not readable filesystem");
  return STATUS_ERROR;
 }
}

dword_t create_file(dword_t partition_number, byte_t *file_memory, dword_t file_size_in_bytes) {
 //select partition
 if(select_partition(partition_number)==STATUS_ERROR) {
  logf("\nCREATE FILE: invalid partition");
  return STATUS_ERROR;
 }

 //create file
 dofs_can_user_cancel_action = USER_CAN_CANCEL_THIS_DOFS_ACTION;
 dofs_task_completed_number_of_bytes = 0;
 if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
  return create_fat_file(FAT_FIND_FREE_CLUSTER, file_memory, file_size_in_bytes);
 }
 else {
  logf("\nCREATE FILE: not readable filesystem");
  return STATUS_ERROR;
 }
}

byte_t delete_file(dword_t partition_number, dword_t file_location) {
 //select partition
 if(select_partition(partition_number)==STATUS_ERROR) {
  logf("\nDELETE FILE: invalid partition");
  return STATUS_ERROR;
 }

 //delete file
 dofs_can_user_cancel_action = USER_CAN_NOT_CANCEL_THIS_DOFS_ACTION;
 if(connected_partitions[partition_number].filesystem==PARTITION_FILESYSTEM_FAT) {
  return delete_fat_file(file_location);
 }
 else {
  logf("\nDELETE FILE: not readable filesystem");
  return STATUS_ERROR;
 }
}