//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_iso9660(dword_t first_partition_sector) {
 struct iso9660_volume_descriptor_t iso9660_volume_descriptor;
 for(dword_t i=0; i<256; i++) {
  //read sector
  if(read_storage_medium(first_partition_sector+0x10+i, 1, (dword_t)(&iso9660_volume_descriptor))==STATUS_ERROR) {
   return STATUS_FALSE;
  }

  //check if this is volume descriptor
  if(is_memory_equal_with_memory(iso9660_volume_descriptor.identifier, "CD001", 5)==STATUS_FALSE) {
   return STATUS_FALSE;
  }

  //check if this is primary volume descriptor
  if(iso9660_volume_descriptor.type==ISO9660_PRIMARY_VOLUME_DESCRIPTOR) {
   struct iso9660_primary_volume_descriptor_t *iso9660_primary_volume_descriptor = (struct iso9660_primary_volume_descriptor_t *) (&iso9660_volume_descriptor);
   if(iso9660_primary_volume_descriptor->logical_block_size==2048) {
    return STATUS_TRUE;
   }
   else { //we do not support filesystem with blocks other than 2048 bytes
    return STATUS_FALSE;
   }
  }

  //check if this is last set terminator descriptor
  if(iso9660_volume_descriptor.type==ISO9660_VOLUME_DESCRIPTOR_SET_TERMINATOR) {
   return STATUS_FALSE; //this means that there was no primary volume descriptor
  }
 }

 return STATUS_FALSE;
}

void filesystem_iso9660_read_specific_info(struct connected_partition_info_t *connected_partition_info) {
 //load Primary Volume Descriptor
 struct iso9660_primary_volume_descriptor_t iso9660_primary_volume_descriptor;
 for(dword_t i=0; i<256; i++) {
  //read sector
  if(read_storage_medium(connected_partition_info->first_sector+0x10+i, 1, (dword_t)(&iso9660_primary_volume_descriptor))==STATUS_ERROR) {
   return;
  }

  //check if this is volume descriptor
  if(is_memory_equal_with_memory(iso9660_primary_volume_descriptor.identifier, "CD001", 5)==STATUS_FALSE) {
   return;
  }

  //check if this is primary volume descriptor
  if(iso9660_primary_volume_descriptor.type==ISO9660_PRIMARY_VOLUME_DESCRIPTOR) {
   break;
  }

  //check if this is last set terminator descriptor
  if(iso9660_primary_volume_descriptor.type==ISO9660_VOLUME_DESCRIPTOR_SET_TERMINATOR) {
   return; //this means that there was no primary volume descriptor
  }
 }

 //allocate memory
 struct iso9660_specific_info_t *iso9660_info = (struct iso9660_specific_info_t *) (calloc(sizeof(struct iso9660_specific_info_t)));

 //read root folder informations
 iso9660_info->root_directory_sector = iso9660_primary_volume_descriptor.root_directory_entry.location_of_extent;
 iso9660_info->root_directory_size_in_bytes = iso9660_primary_volume_descriptor.root_directory_entry.data_length;

 //save pointer to info to connected_partition_info 
 connected_partition_info->filesystem_specific_info_pointer = ((byte_t *)iso9660_info);

 //copy partition label
 for(int i=0; i<11; i++) {
  connected_partition_info->partition_label[i]=iso9660_primary_volume_descriptor.volume_identifier[i];
 }
 connected_partition_info->partition_label[10] = 0;

 //log
 log("\n\nISO9660 filesystem");
 log("\nroot directory location: "); log_var(iso9660_info->root_directory_sector);
 log("\nroot directory size in sectors: "); log_var(iso9660_info->root_directory_size_in_bytes/2048);
 log("\npartition label: "); log(connected_partition_info->partition_label);
}

byte_t *read_iso9660_file(dword_t file_location, dword_t file_size_in_bytes) {
 //create descriptor of file sectors
 struct byte_stream_descriptor_t *descriptor_of_file_sectors = create_descriptor_of_file_sectors(2048);

 //add sectors of file
 dword_t file_number_of_sectors = (file_size_in_bytes/2048);
 if((file_size_in_bytes%2048)!=0) {
  file_number_of_sectors++;
 }
 add_sectors_to_descriptor_of_file_sectors(descriptor_of_file_sectors, file_location, file_number_of_sectors);

 //read file
 byte_t *file_memory = read_whole_descriptor_of_file_sector(descriptor_of_file_sectors);

 //release memory
 destroy_byte_stream(descriptor_of_file_sectors);

 //return pointer to loaded data or STATUS_ERROR
 return file_memory;
}

byte_t *read_iso9660_folder(dword_t folder_location, dword_t folder_size_in_bytes) {
 //set variables if this is root folder
 if(folder_location==ROOT_FOLDER) {
  struct iso9660_specific_info_t *iso9660_info = (struct iso9660_specific_info_t *) (connected_partitions[selected_partition].filesystem_specific_info_pointer);
  folder_location = iso9660_info->root_directory_sector;
  folder_size_in_bytes = iso9660_info->root_directory_size_in_bytes;
 }

 //read folder data
 byte_t *iso9660_folder = read_iso9660_file(folder_location, folder_size_in_bytes);
 if((dword_t)iso9660_folder==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 iso9660_folder = (byte_t *) (realloc((dword_t)iso9660_folder, folder_size_in_bytes+256)); //add zero bytes at end

 /* convert ISO9660 folder to BleskOS Virtual File System folder */

 //count number of entries
 struct iso9660_directory_entry_t *iso9660_folder_pointer = (struct iso9660_directory_entry_t *) iso9660_folder;
 iso9660_folder_pointer = (struct iso9660_directory_entry_t *) ((dword_t)iso9660_folder_pointer+iso9660_folder_pointer->length_of_directory_record); //skip '.' entry
 iso9660_folder_pointer = (struct iso9660_directory_entry_t *) ((dword_t)iso9660_folder_pointer+iso9660_folder_pointer->length_of_directory_record); //skip '..' entry
 dword_t number_of_iso9660_folder_entries = (folder_size_in_bytes/32); //max number of entries
 number_of_files_in_iso9660_folder = 0;
 for(dword_t i=0; i<number_of_iso9660_folder_entries; i++) {
  //end of directory
  if(iso9660_folder_pointer->length_of_directory_record==0) {
   break;
  }

  //add entry
  number_of_files_in_iso9660_folder++;

  //move pointer to next entry
  iso9660_folder_pointer = (struct iso9660_directory_entry_t *) ((dword_t)iso9660_folder_pointer+iso9660_folder_pointer->length_of_directory_record);
 }

 //allocate VFS folder memory
 struct file_descriptor_t *vfs_folder = (struct file_descriptor_t *) (calloc(sizeof(struct file_descriptor_t)*number_of_files_in_iso9660_folder));

 //convert all entries
 iso9660_folder_pointer = (struct iso9660_directory_entry_t *) iso9660_folder;
 iso9660_folder_pointer = (struct iso9660_directory_entry_t *) ((dword_t)iso9660_folder_pointer+iso9660_folder_pointer->length_of_directory_record); //skip '.' entry
 iso9660_folder_pointer = (struct iso9660_directory_entry_t *) ((dword_t)iso9660_folder_pointer+iso9660_folder_pointer->length_of_directory_record); //skip '..' entry
 for(dword_t i=0; i<number_of_files_in_iso9660_folder; i++) {
  vfs_folder[i].partition_number = selected_partition;
  vfs_folder[i].type = NORMAL_FILE;
  if((iso9660_folder_pointer->file_flags & 0x2)==0x2) {
   vfs_folder[i].type = FILE_FOLDER;
  }
  vfs_folder[i].file_location = iso9660_folder_pointer->location_of_extent;
  vfs_folder[i].file_size_in_bytes = iso9660_folder_pointer->data_length;

  vfs_folder[i].year_of_creation = (iso9660_folder_pointer->year_of_creation+1900);
  vfs_folder[i].month_of_creation = iso9660_folder_pointer->month_of_creation;
  vfs_folder[i].day_of_creation = iso9660_folder_pointer->day_of_creation;
  vfs_folder[i].hour_of_creation = iso9660_folder_pointer->hour_of_creation;
  vfs_folder[i].minute_of_creation = iso9660_folder_pointer->minute_of_creation;
  vfs_folder[i].second_of_creation = iso9660_folder_pointer->second_of_creation;

  vfs_folder[i].year_of_modification = vfs_folder[i].year_of_creation;
  vfs_folder[i].month_of_modification = vfs_folder[i].month_of_creation;
  vfs_folder[i].day_of_modification = vfs_folder[i].day_of_creation;
  vfs_folder[i].hour_of_modification = vfs_folder[i].hour_of_creation;
  vfs_folder[i].minute_of_modification = vfs_folder[i].minute_of_creation;
  vfs_folder[i].second_of_modification = vfs_folder[i].second_of_creation;

  for(dword_t j=0; j<iso9660_folder_pointer->length_of_file_identifier; j++) {
   if(iso9660_folder_pointer->file_identifier[j]==';' || iso9660_folder_pointer->file_identifier[j]==0) {
    break;
   }
   vfs_folder[i].name[j] = iso9660_folder_pointer->file_identifier[j];
  }
  vfs_entry_parse_extension_from_name((struct file_descriptor_t *)(&vfs_folder[i]));

  //move pointer to next entry
  iso9660_folder_pointer = (struct iso9660_directory_entry_t *) ((dword_t)iso9660_folder_pointer+iso9660_folder_pointer->length_of_directory_record);
 }

 //free allocated memory
 free((dword_t)iso9660_folder);

 //return virtual file system folder
 return ((byte_t *)vfs_folder);
}