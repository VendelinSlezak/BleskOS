//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_device_list(void) {
 device_list_mem = calloc(DEVICE_LIST_NUMBER_OF_ENTRIES*DEVICE_LIST_SIZE_OF_ENTRY);
 device_list_selected_entry = 0xFFFFFFFF;
 number_of_device_list_entries = 0;

 //add partitions from hard disk
 if(if_storage_medium_exist(MEDIUM_HDD, DEFAULT_MEDIUM)==STATUS_TRUE) {
  select_storage_medium(MEDIUM_HDD, DEFAULT_MEDIUM);
  read_partition_info();
  for(dword_t i=0; i<8; i++) {
   if(partitions[i].type==STORAGE_NO_PARTITION) {
    break; //we went through all partitions
   }

   //add partition
   if(partitions[i].type!=STORAGE_FREE_SPACE && partitions[i].type!=STORAGE_UNKNOWN_FILESYSTEM) {
    add_device_partition_to_device_list(MEDIUM_HDD, DEFAULT_MEDIUM, partitions[i].type, partitions[i].first_sector);
   }
  }
 }

 //add partition from disk in optical drive
 device_list_check_optical_drive();
}

dword_t get_device_list_entry_value(dword_t offset) {
 dword_t *device_list_entry = (dword_t *) (device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*device_list_selected_entry)+(offset*4));
 return *device_list_entry;
}

void set_device_entry_list_value(dword_t offset, dword_t value) {
 dword_t *device_list_entry = (dword_t *) (device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*device_list_selected_entry)+(offset*4));
 *device_list_entry = value;
}

dword_t get_device_list_entry_name_value(dword_t offset) {
 word_t *device_list_entry = (word_t *) (device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*device_list_selected_entry)+(DEVICE_LIST_ENTRY_DEVICE_PARTITION_NAME*4)+(offset*2));
 return *device_list_entry;
}

void set_device_entry_list_name_value(dword_t offset, word_t value) {
 word_t *device_list_entry = (word_t *) (device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*device_list_selected_entry)+(DEVICE_LIST_ENTRY_DEVICE_PARTITION_NAME*4)+(offset*2));
 *device_list_entry = value;
}

void add_device_partition_to_device_list(dword_t device_type, dword_t device_number, dword_t partition_type, dword_t first_partition_sector) {
 if(partition_type==STORAGE_BLESKOS_BOOTABLE) {
  return; //we do not add this partition to device list
 }
 
 for(device_list_selected_entry=0; device_list_selected_entry<DEVICE_LIST_NUMBER_OF_ENTRIES; device_list_selected_entry++) {
  if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==0) {
   //we found free entry
   set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_TYPE, device_type);
   set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER, device_number);
   set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE, partition_type);
   set_device_entry_list_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR, first_partition_sector);

   select_storage_medium(device_type, device_number);
   if(partition_type==STORAGE_FAT) {
    select_fat_partition(first_partition_sector);
    for(dword_t i=0; i<11; i++) {
     set_device_entry_list_name_value(i, partition_label[i]);
    }
    set_device_entry_list_name_value(11, 0);
    set_fat_partition_info_in_device_list_entry();
   }
   else if(partition_type==STORAGE_ISO9660) {
    select_iso9660_partition(first_partition_sector);
    for(dword_t i=0; i<11; i++) {
     set_device_entry_list_name_value(i, partition_label[i]);
    }
    set_device_entry_list_name_value(11, 0);
    set_iso9660_partition_info_in_device_list_entry();
   }
   else if(partition_type==STORAGE_EXT) {
    select_ext_partition(first_partition_sector);
    for(dword_t i=0; i<11; i++) {
     set_device_entry_list_name_value(i, partition_label[i]);
    }
    set_device_entry_list_name_value(11, 0);
    set_ext_partition_info_in_device_list_entry();
   }
   else if(partition_type==STORAGE_CDDA) {
    select_cdda_partition();
    for(dword_t i=0; i<11; i++) {
     set_device_entry_list_name_value(i, partition_label[i]);
    }
    set_device_entry_list_name_value(11, 0);
   }

   number_of_device_list_entries++;
   break;
  }
 }
}

void remove_device_from_device_list(dword_t device_type, dword_t device_number) {
 for(device_list_selected_entry=0; device_list_selected_entry<DEVICE_LIST_NUMBER_OF_ENTRIES; device_list_selected_entry++) {
  if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==device_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)==device_number) {
   //we found entry of this device
   
   //remove it from file dialog
   if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==file_dialog_folder_device_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)==file_dialog_folder_device_number) {
    free(file_dialog_folder_memory);
    file_dialog_folder_memory = 0;
    file_dialog_folder_device_type = 0;
    file_dialog_folder_device_number = 0;
    file_dialog_folder_device_partition_type = 0;
    file_dialog_folder_device_partition_first_sector = 0;
    file_dialog_selected_device_entry = 0xFFFFFFFF;
   }
   
   //remove it from device list
   copy_memory((device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*(device_list_selected_entry+1))), (device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*device_list_selected_entry)), (device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*(DEVICE_LIST_NUMBER_OF_ENTRIES-1)))-(device_list_mem+(DEVICE_LIST_SIZE_OF_ENTRY*(device_list_selected_entry+1))));
   clear_memory((device_list_mem+DEVICE_LIST_SIZE_OF_ENTRY*((DEVICE_LIST_NUMBER_OF_ENTRIES-1))), DEVICE_LIST_SIZE_OF_ENTRY);
   number_of_device_list_entries--;
  }
 }
}

byte_t does_device_list_entry_exist(dword_t device_type, dword_t device_number, dword_t partition_type, dword_t first_partition_sector) {
 for(device_list_selected_entry=0; device_list_selected_entry<DEVICE_LIST_NUMBER_OF_ENTRIES; device_list_selected_entry++) {
  if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==device_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)==device_number && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==partition_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR)==first_partition_sector) {
   return STATUS_TRUE; //we found entry
  }
 }

 return STATUS_FALSE;
}

byte_t select_device_list_entry(dword_t device_type, dword_t device_number, dword_t partition_type, dword_t first_partition_sector) {
 for(device_list_selected_entry=0; device_list_selected_entry<DEVICE_LIST_NUMBER_OF_ENTRIES; device_list_selected_entry++) {
  if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==device_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)==device_number && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==partition_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR)==first_partition_sector) {
   //we found entry
   select_storage_medium(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER));
   if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==STORAGE_FAT) {
    read_fat_partition_info_from_device_list_entry();

    //TODO: !!!!
    partitions[0].type = STORAGE_FAT;
    selected_partition_number = 0;
   }
   else if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==STORAGE_ISO9660) {
    read_iso9660_partition_info_from_device_list_entry();

    //TODO: !!!!
    partitions[0].type = STORAGE_ISO9660;
    selected_partition_number = 0;
   }
   else if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==STORAGE_EXT) {
    read_ext_partition_info_from_device_list_entry();

    //TODO: !!!!
    partitions[0].type = STORAGE_EXT;
    selected_partition_number = 0;
   }
   else if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==STORAGE_CDDA) {
    //TODO: !!!!
    partitions[0].type = STORAGE_CDDA;
    selected_partition_number = 0;
   }
   else {
    partitions[0].type = STORAGE_NO_PARTITION;
    return STATUS_ERROR;
   }

   return STATUS_GOOD;
  }
 }

 return STATUS_ERROR;
}

void device_list_check_optical_drive(void) {
 if(if_storage_medium_exist(MEDIUM_CDROM, DEFAULT_MEDIUM)==STATUS_FALSE) {
  return;
 }

 remove_device_from_device_list(MEDIUM_CDROM, DEFAULT_MEDIUM);
 if(detect_optical_disk()==STATUS_TRUE) {
  select_storage_medium(MEDIUM_CDROM, DEFAULT_MEDIUM);
  read_partition_info();
  for(dword_t i=0; i<8; i++) {
   if(partitions[i].type==STORAGE_NO_PARTITION) {
    break; //we went through all partitions
   }

   //add partition
   if(partitions[i].type!=STORAGE_FREE_SPACE && partitions[i].type!=STORAGE_UNKNOWN_FILESYSTEM) {
    add_device_partition_to_device_list(MEDIUM_CDROM, DEFAULT_MEDIUM, partitions[i].type, partitions[i].first_sector);
   }
  }
 }
}