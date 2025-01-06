//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_list_of_connected_partitions(void) {
 connected_partitions = (struct connected_partition_info_t *) (calloc(sizeof(struct connected_partition_info_t)*MAX_NUMBER_OF_CONNECTED_PARTITIONS));
 selected_partition = NO_PARTITION_SELECTED;
}

void add_new_partition_to_list(byte_t medium_type, byte_t medium_number, byte_t filesystem, dword_t first_sector) {
 for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
  if(connected_partitions[i].medium_type==NO_MEDIUM) {
   //add basic info about partition
   connected_partitions[i].medium_type = medium_type;
   connected_partitions[i].medium_number = medium_number;
   connected_partitions[i].filesystem = filesystem;
   connected_partitions[i].first_sector = first_sector;

   //read partition specific info
   if(connected_partitions[i].filesystem==PARTITION_FILESYSTEM_FAT) {
    filesystem_fat_read_specific_info((struct connected_partition_info_t *)(&connected_partitions[i]));
   }
   else if(connected_partitions[i].filesystem==PARTITION_FILESYSTEM_EXT) {
    filesystem_ext_read_specific_info((struct connected_partition_info_t *)(&connected_partitions[i]));
   }
   else if(connected_partitions[i].filesystem==PARTITION_FILESYSTEM_ISO9660) {
    filesystem_iso9660_read_specific_info((struct connected_partition_info_t *)(&connected_partitions[i]));
   }
   else if(connected_partitions[i].filesystem==PARTITION_FILESYSTEM_CDDA) {
    filesystem_cdda_read_specific_info((struct connected_partition_info_t *)(&connected_partitions[i]));
   }

   return;
  }
 }
}

void remove_partitions_of_medium_from_list(byte_t medium_type, byte_t medium_number) {
 for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
  if(connected_partitions[i].medium_type==medium_type && connected_partitions[i].medium_number==medium_number) {
   connected_partitions[i].medium_type = NO_MEDIUM;
   connected_partitions[i].filesystem = 0;

   //unselect partition if it was selected
   if(selected_partition==i) {
    selected_partition = NO_PARTITION_SELECTED;
   }

   //free memory allocated by filesystem driver
   if(connected_partitions[i].filesystem_specific_info_pointer!=0) {
    free((dword_t)connected_partitions[i].filesystem_specific_info_pointer);
   }

   //free all loaded folders from array
   for(dword_t j=0; j<MAX_NUMBER_OF_LOADED_FOLDERS; j++) {
    if(array_of_loaded_folders[j].state==AOLF_FOLDER_LOADED && array_of_loaded_folders[j].partition_number==i) {
     free((dword_t)array_of_loaded_folders[j].memory_of_entries);
     array_of_loaded_folders[j].state = AOLF_FREE_ENTRY;
    }
   }

   //clear all folder structures using this partition
   for(dword_t j=0; j<number_of_folder_structures; j++) {
    struct folder_descriptor_t *folder_path_structure = (struct folder_descriptor_t *) ((dword_t)array_of_folder_structures[j]);
    if(folder_path_structure->partition_number==i) {
     folder_path_structure->partition_number = NO_PARTITION_SELECTED;
     folder_path_structure->folder_number_in_array_of_loaded_folders = 0;
    }
   }
  }
 }
}

byte_t is_partition_connected(byte_t medium_type, byte_t medium_number, byte_t filesystem, dword_t first_sector) {
 for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
  if(connected_partitions[i].medium_type==medium_type && connected_partitions[i].medium_number==medium_number && connected_partitions[i].filesystem==filesystem && connected_partitions[i].first_sector==first_sector) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}

byte_t select_partition(byte_t partition_number) {
 if(selected_partition==partition_number) {
  return STATUS_GOOD; //this partition is already selected
 }

 if(connected_partitions[partition_number].medium_type==NO_MEDIUM) {
  return STATUS_ERROR;
 }

 //select storage medium to be able to work with partition
 select_storage_medium(connected_partitions[partition_number].medium_type, connected_partitions[partition_number].medium_number);

 //save partition number
 selected_partition = partition_number;

 return STATUS_GOOD;
}