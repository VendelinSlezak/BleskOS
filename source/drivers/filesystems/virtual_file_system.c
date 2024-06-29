//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void vfs_entry_parse_extension_from_name(struct file_descriptor_t *file_descriptor) {
 //clear extension
 clear_memory((dword_t)(&file_descriptor->extension), 10*2);

 //parse extension from name
 for(dword_t i=255; i>0; i--) {
  if(file_descriptor->name[i]=='.') { //find dot
   i++; //skip dot

   for(dword_t j=0; j<10; j++) { //copy extension after dot
    if(file_descriptor->name[i+j]==0 || (i+j)>=256) { //end of extension
     break;
    }
    file_descriptor->extension[j] = get_small_char_value(file_descriptor->name[i+j]); //convert big chars to small chars
   }

   break; //extension was copied
  }
 }
}

byte_t vfs_does_file_have_this_extension(struct file_descriptor_t *file_descriptor, byte_t *extensions) {
 //all extensions accepted
 if(extensions[0]==0) {
  return STATUS_TRUE;
 }

 //compare all extensions
 while(*extensions!=0) {
  //compare extension
  for(dword_t i=0; i<10; i++) { //extension can have max 9 charactes
   if(extensions[i]==' ' || extensions[i]==0) { //end of extension = it equals file extension
    return STATUS_TRUE;
   }
   if(file_descriptor->extension[i]!=extensions[i]) {
    break;
   }
  }

  //skip extension
  while(*extensions!=' ') {
   if(*extensions==0) { //this was last extension
    return STATUS_FALSE;
   }
   extensions++;
  }
  extensions++; //skip space
 }
}

void vfs_sort_file_descriptors(struct file_descriptor_t *file_descriptor, dword_t number_of_descriptors, byte_t swap_descriptors(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2)) {
 for(dword_t i=0; i<number_of_descriptors; i++) {
  dword_t descriptor_to_swap = i;

  //find out which descriptor should be at [i] position
  for(dword_t j=(i+1); j<number_of_descriptors; j++) {
   if(swap_descriptors(&file_descriptor[descriptor_to_swap], &file_descriptor[j])==STATUS_TRUE) {
    descriptor_to_swap = j;
   }
  }

  //swap descriptors
  if(i!=descriptor_to_swap) {
   struct file_descriptor_t file_descriptor_in_memory;
   copy_memory((dword_t)(&file_descriptor[i]), (dword_t)(&file_descriptor_in_memory), sizeof(struct file_descriptor_t));
   copy_memory((dword_t)(&file_descriptor[descriptor_to_swap]), (dword_t)(&file_descriptor[i]), sizeof(struct file_descriptor_t));
   copy_memory((dword_t)(&file_descriptor_in_memory), (dword_t)(&file_descriptor[descriptor_to_swap]), sizeof(struct file_descriptor_t));
  }
 }
}

byte_t swap_descriptors_by_name_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 for(dword_t i=0; i<256; i++) {
  //end of first descriptor
  if(file_descriptor1->name[i]==0 || (file_descriptor1->name[i]=='.' && file_descriptor2->name[i]!='.')) {
   return STATUS_FALSE;
  }

  //end of second descriptor, so names are same but second descriptor is shorter
  if(file_descriptor2->name[i]==0 || (file_descriptor2->name[i]=='.' && file_descriptor1->name[i]!='.')) {
   return STATUS_TRUE;
  }

  //sort numbers
  if(is_number(file_descriptor1->name[i])==STATUS_TRUE && is_number(file_descriptor2->name[i])==STATUS_TRUE) {
   //first descriptor should be higher
   if(convert_word_string_to_number((dword_t)(&file_descriptor1->name[i]))<convert_word_string_to_number((dword_t)(&file_descriptor2->name[i]))) {
    return STATUS_FALSE;
   }

   //second descriptor should be higher
   if(convert_word_string_to_number((dword_t)(&file_descriptor1->name[i]))>convert_word_string_to_number((dword_t)(&file_descriptor2->name[i]))) {
    return STATUS_TRUE;
   }
  }
  else { //sort characters
   //first descriptor should be higher
   if(get_small_char_value(file_descriptor1->name[i])<get_small_char_value(file_descriptor2->name[i])) {
    return STATUS_FALSE;
   }

   //second descriptor should be higher
   if(get_small_char_value(file_descriptor1->name[i])>get_small_char_value(file_descriptor2->name[i])) {
    return STATUS_TRUE;
   }
  }
 }

 return STATUS_FALSE;
}

byte_t swap_descriptors_by_name_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 for(dword_t i=0; i<256; i++) {
  //end of first descriptor
  if(file_descriptor1->name[i]==0 || (file_descriptor1->name[i]=='.' && file_descriptor2->name[i]!='.')) {
   return STATUS_FALSE;
  }

  //end of second descriptor, so names are same but second descriptor is shorter
  if(file_descriptor2->name[i]==0 || (file_descriptor2->name[i]=='.' && file_descriptor1->name[i]!='.')) {
   return STATUS_TRUE;
  }

  //sort numbers
  if(is_number(file_descriptor1->name[i])==STATUS_TRUE && is_number(file_descriptor2->name[i])==STATUS_TRUE) {
   //first descriptor should be higher
   if(convert_word_string_to_number((dword_t)(&file_descriptor1->name[i]))>convert_word_string_to_number((dword_t)(&file_descriptor2->name[i]))) {
    return STATUS_FALSE;
   }

   //second descriptor should be higher
   if(convert_word_string_to_number((dword_t)(&file_descriptor1->name[i]))<convert_word_string_to_number((dword_t)(&file_descriptor2->name[i]))) {
    return STATUS_TRUE;
   }
  }
  else { //sort characters
   //first descriptor should be higher
   if(get_small_char_value(file_descriptor1->name[i])>get_small_char_value(file_descriptor2->name[i])) {
    return STATUS_FALSE;
   }

   //second descriptor should be higher
   if(get_small_char_value(file_descriptor1->name[i])<get_small_char_value(file_descriptor2->name[i])) {
    return STATUS_TRUE;
   }
  }
 }

 return STATUS_FALSE;
}

byte_t swap_descriptors_by_extension_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 for(dword_t i=0; i<10; i++) {
  //end of first descriptor
  if(file_descriptor1->extension[i]==0) {
   return STATUS_FALSE;
  }

  //end of second descriptor, so names are same but second descriptor is shorter
  if(file_descriptor2->extension[i]==0) {
   return STATUS_TRUE;
  }

  //first descriptor should be higher
  if(file_descriptor1->extension[i]<file_descriptor2->extension[i]) {
   return STATUS_FALSE;
  }

  //second descriptor should be higher
  if(file_descriptor1->extension[i]>file_descriptor2->extension[i]) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}

byte_t swap_descriptors_by_extension_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 for(dword_t i=0; i<10; i++) {
  //end of first descriptor
  if(file_descriptor1->extension[i]==0) {
   return STATUS_FALSE;
  }

  //end of second descriptor, so names are same but second descriptor is shorter
  if(file_descriptor2->extension[i]==0) {
   return STATUS_TRUE;
  }

  //first descriptor should be higher
  if(file_descriptor1->extension[i]>file_descriptor2->extension[i]) {
   return STATUS_FALSE;
  }

  //second descriptor should be higher
  if(file_descriptor1->extension[i]<file_descriptor2->extension[i]) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}

byte_t swap_descriptors_by_size_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 if(file_descriptor1->file_size_in_bytes<file_descriptor2->file_size_in_bytes) {
  return STATUS_FALSE;
 }
 else {
  return STATUS_TRUE;
 }
}

byte_t swap_descriptors_by_size_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 if(file_descriptor1->file_size_in_bytes>file_descriptor2->file_size_in_bytes) {
  return STATUS_FALSE;
 }
 else {
  return STATUS_TRUE;
 }
}

byte_t swap_descriptors_by_date_of_creation_in_ascending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 dword_t first_descriptor_date_of_creation = (file_descriptor1->year_of_creation<<16 | file_descriptor1->month_of_creation<<8 | file_descriptor1->day_of_creation);
 dword_t second_descriptor_date_of_creation = (file_descriptor2->year_of_creation<<16 | file_descriptor2->month_of_creation<<8 | file_descriptor2->day_of_creation);
 if(first_descriptor_date_of_creation<=second_descriptor_date_of_creation) {
  return STATUS_FALSE;
 }
 else {
  return STATUS_TRUE;
 }
}

byte_t swap_descriptors_by_date_of_creation_in_descending_order(struct file_descriptor_t *file_descriptor1, struct file_descriptor_t *file_descriptor2) {
 dword_t first_descriptor_date_of_creation = (file_descriptor1->year_of_creation<<16 | file_descriptor1->month_of_creation<<8 | file_descriptor1->day_of_creation);
 dword_t second_descriptor_date_of_creation = (file_descriptor2->year_of_creation<<16 | file_descriptor2->month_of_creation<<8 | file_descriptor2->day_of_creation);
 if(first_descriptor_date_of_creation>=second_descriptor_date_of_creation) {
  return STATUS_FALSE;
 }
 else {
  return STATUS_TRUE;
 }
}

dword_t vfs_sort_folders_at_start(struct folder_descriptor_t *folder_path_structure) {
 struct file_descriptor_t *file_descriptor = vfs_get_folder_data_pointer(folder_path_structure);
 dword_t number_of_folders = 0;

 //sort folders at start
 for(dword_t i=0; i<vfs_get_folder_number_of_files(folder_path_structure); i++) {
  if(file_descriptor[i].type!=FILE_FOLDER) {
   dword_t descriptor_to_swap = i;

   //find out next folder
   for(dword_t j=(i+1); j<vfs_get_folder_number_of_files(folder_path_structure); j++) {
    if(file_descriptor[j].type==FILE_FOLDER) {
     descriptor_to_swap = j;
     number_of_folders++;
     break;
    }
   }

   //no more folders to sort
   if(descriptor_to_swap==i) {
    break;
   }

   //swap descriptors
   struct file_descriptor_t file_descriptor_in_memory;
   copy_memory((dword_t)(&file_descriptor[i]), (dword_t)(&file_descriptor_in_memory), sizeof(struct file_descriptor_t));
   copy_memory((dword_t)(&file_descriptor[descriptor_to_swap]), (dword_t)(&file_descriptor[i]), sizeof(struct file_descriptor_t));
   copy_memory((dword_t)(&file_descriptor_in_memory), (dword_t)(&file_descriptor[descriptor_to_swap]), sizeof(struct file_descriptor_t));
  }
  else {
   number_of_folders++;
  }
 }

 //return number of folders in folder
 return number_of_folders;
}

void vfs_sort_folder_by_name(struct folder_descriptor_t *folder_path_structure, byte_t type) {
 struct file_descriptor_t *file_descriptor = vfs_get_folder_data_pointer(folder_path_structure);
 dword_t number_of_folders = vfs_sort_folders_at_start(folder_path_structure);

 if(type==SORT_IN_ASCENDING_ORDER) {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_name_in_ascending_order);
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_name_in_ascending_order);
 }
 else {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_name_in_descending_order);
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_name_in_descending_order);
 }
}

void vfs_sort_folder_by_extension(struct folder_descriptor_t *folder_path_structure, byte_t type) {
 struct file_descriptor_t *file_descriptor = vfs_get_folder_data_pointer(folder_path_structure);
 dword_t number_of_folders = vfs_sort_folders_at_start(folder_path_structure);

 if(type==SORT_IN_ASCENDING_ORDER) {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_name_in_ascending_order); //folders do not have extension
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_extension_in_ascending_order);
 }
 else {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_name_in_ascending_order); //folders do not have extension
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_extension_in_descending_order);
 }
}

void vfs_sort_folder_by_size(struct folder_descriptor_t *folder_path_structure, byte_t type) {
 struct file_descriptor_t *file_descriptor = vfs_get_folder_data_pointer(folder_path_structure);
 dword_t number_of_folders = vfs_sort_folders_at_start(folder_path_structure);

 if(type==SORT_IN_ASCENDING_ORDER) {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_name_in_ascending_order); //folders do not have size
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_size_in_ascending_order);
 }
 else {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_name_in_ascending_order); //folders do not have size
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_size_in_descending_order);
 }
}

void vfs_sort_folder_by_date_of_creation(struct folder_descriptor_t *folder_path_structure, byte_t type) {
 struct file_descriptor_t *file_descriptor = vfs_get_folder_data_pointer(folder_path_structure);
 dword_t number_of_folders = vfs_sort_folders_at_start(folder_path_structure);

 if(type==SORT_IN_ASCENDING_ORDER) {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_date_of_creation_in_ascending_order);
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_date_of_creation_in_ascending_order);
 }
 else {
  vfs_sort_file_descriptors((&file_descriptor[0]), number_of_folders, swap_descriptors_by_date_of_creation_in_descending_order);
  vfs_sort_file_descriptors((&file_descriptor[number_of_folders]), vfs_get_folder_number_of_files(folder_path_structure)-number_of_folders, swap_descriptors_by_date_of_creation_in_descending_order);
 }
}

struct folder_descriptor_t *vfs_create_folder_path_structure(byte_t partition_number) {
 if(number_of_folder_structures>=MAX_NUMBER_OF_FOLDER_STRUCTURES) {
  log("\nVFS: too many folder structures");
  return STATUS_ERROR;
 }

 //read root folder of this partition
 word_t root_folder_aolf_number = read_folder(partition_number, ROOT_FOLDER, 0);
 if(root_folder_aolf_number==STATUS_ERROR) {
  log("\nVFS: error with reading root folder");
  return STATUS_ERROR;
 }

 //allocate memory for structure
 struct folder_descriptor_t *folder_path_structure = (struct folder_descriptor_t *) (calloc(sizeof(struct folder_descriptor_t)));

 //fill structure
 folder_path_structure->partition_number = partition_number;
 folder_path_structure->view_type = VIEW_FOLDER_LIST;
 folder_path_structure->selected_entry = FOLDER_NO_ENTRY_SELECTED;
 folder_path_structure->path_folder_locations[0] = ROOT_FOLDER;
 folder_path_structure->folder_number_in_array_of_loaded_folders = root_folder_aolf_number;

 //add structure to list of structures
 array_of_folder_structures[number_of_folder_structures] = ((dword_t)folder_path_structure);
 number_of_folder_structures++;

 //sort folder files by name
 vfs_sort_folder_by_name(folder_path_structure, SORT_IN_ASCENDING_ORDER);

 //return structure
 return folder_path_structure;
}

struct file_descriptor_t *vfs_get_folder_data_pointer(struct folder_descriptor_t *folder_path_structure) {
 return (struct file_descriptor_t *) (array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries);
}

dword_t vfs_get_folder_number_of_files(struct folder_descriptor_t *folder_path_structure) {
 return array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files;
}

void vfs_destroy_folder_path_structure(struct folder_descriptor_t *folder_path_structure) {
 //unallocate loaded folder in array
 if(folder_path_structure->folder_number_in_array_of_loaded_folders!=0) {
  release_loaded_folder_in_array_from_usage_in_structure(folder_path_structure->folder_number_in_array_of_loaded_folders);
 }

 //remove from list of structures
 for(dword_t i=0; i<number_of_folder_structures; i++) {
  if(array_of_folder_structures[i]==((dword_t)folder_path_structure)) {
   remove_space_from_memory_area((dword_t)(&array_of_folder_structures), MAX_NUMBER_OF_FOLDER_STRUCTURES*4, (dword_t)(&array_of_folder_structures[i]), 4);
   number_of_folder_structures--;
   break;
  }
 }
 
 //unallocate folder structure
 free((dword_t)folder_path_structure);
}

byte_t vfs_open_folder(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //request for reading nonexisting entry
 if(number_of_entry>=vfs_get_folder_number_of_files(folder_path_structure)) {
  return STATUS_ERROR;
 }

 //too many folders in path
 if(folder_path_structure->pointer_to_path>=(MAX_NUMBER_OF_FOLDERS_IN_PATH-1)) {
  log("\nVFS: too many folders in path");
  return STATUS_ERROR;
 }

 //pointer to folder data
 struct file_descriptor_t *folder = (vfs_get_folder_data_pointer(folder_path_structure));

 //read folder
 if(folder[number_of_entry].type==FILE_FOLDER) {
  //read folder data
  word_t opened_folder_aolf_number = read_folder(folder_path_structure->partition_number, folder[number_of_entry].file_location, folder[number_of_entry].file_size_in_bytes);
  if(opened_folder_aolf_number==STATUS_ERROR) {
   log("\nVFS: error with reading folder");
   return STATUS_ERROR;
  }

  //release previous loaded folder
  release_loaded_folder_in_array_from_usage_in_structure(folder_path_structure->folder_number_in_array_of_loaded_folders);

  //add folder to path
  folder_path_structure->pointer_to_path++;
  folder_path_structure->path_folder_locations[folder_path_structure->pointer_to_path] = folder[number_of_entry].file_location;
  folder_path_structure->path_folder_sizes[folder_path_structure->pointer_to_path] = folder[number_of_entry].file_size_in_bytes;
  folder_path_structure->folder_number_in_array_of_loaded_folders = opened_folder_aolf_number;
  folder_path_structure->first_showed_entry = 0;
  folder_path_structure->selected_entry = FOLDER_NO_ENTRY_SELECTED;

  //sort folder files by name
  vfs_sort_folder_by_name(folder_path_structure, SORT_IN_ASCENDING_ORDER);
  
  //folder was successfully loaded
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR; //not folder entry
 }
}

byte_t vfs_go_back_in_folder_path(struct folder_descriptor_t *folder_path_structure) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //we are in root folder
 if(folder_path_structure->pointer_to_path==0) {
  log("\nVFS: can not go back from root folder");
  return STATUS_ERROR;
 }

 //read previous folder
 folder_path_structure->pointer_to_path--;
 word_t previous_folder_aolf_number = read_folder(folder_path_structure->partition_number, folder_path_structure->path_folder_locations[folder_path_structure->pointer_to_path], folder_path_structure->path_folder_sizes[folder_path_structure->pointer_to_path]);
 if(previous_folder_aolf_number==STATUS_ERROR) {
  log("\nVFS: error with reading folder");
  folder_path_structure->pointer_to_path++; //we did not go back in path
  return STATUS_ERROR;
 }

 //release previous loaded folder
 release_loaded_folder_in_array_from_usage_in_structure(folder_path_structure->folder_number_in_array_of_loaded_folders);

 //update values
 folder_path_structure->folder_number_in_array_of_loaded_folders = previous_folder_aolf_number;
 folder_path_structure->first_showed_entry = 0;
 folder_path_structure->selected_entry = FOLDER_NO_ENTRY_SELECTED;

 //sort folder files by name
 vfs_sort_folder_by_name(folder_path_structure, SORT_IN_ASCENDING_ORDER);
 
 //folder was successfully loaded
 return STATUS_GOOD;
}

byte_t vfs_save_folder(struct folder_descriptor_t *folder_path_structure) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 if(folder_path_structure->pointer_to_path==0) {
  //root directory
  if(rewrite_folder(folder_path_structure->partition_number,
                    ROOT_FOLDER,
                    0,
                    (struct file_descriptor_t *) (array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries),
                    array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files)==STATUS_ERROR) {
   log("\nVFS: error with saving folder");
   return STATUS_ERROR;
  }
  else {
   return STATUS_GOOD;
  }
 }
 else {
  //normal directory
  if(rewrite_folder(folder_path_structure->partition_number,
                    folder_path_structure->path_folder_locations[folder_path_structure->pointer_to_path],
                    folder_path_structure->path_folder_locations[(folder_path_structure->pointer_to_path-1)],
                    (struct file_descriptor_t *) (array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries),
                    array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files)==STATUS_ERROR) {
   log("\nVFS: error with saving folder");
   return STATUS_ERROR;
  }
  else {
   return STATUS_GOOD;
  }
 }
}

byte_t vfs_create_folder(struct folder_descriptor_t *folder_path_structure, word_t *name) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //create new folder
 dword_t folder_location = create_folder(folder_path_structure->partition_number, folder_path_structure->path_folder_locations[(folder_path_structure->pointer_to_path-1)]);
 if(folder_location==STATUS_ERROR) {
  log("\nVFS: error during creating folder");
  return STATUS_ERROR;
 }

 //create new entry in folder
 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries = (byte_t *) realloc((dword_t)(vfs_get_folder_data_pointer(folder_path_structure)), sizeof(struct file_descriptor_t)*(vfs_get_folder_number_of_files(folder_path_structure)+1));
 struct file_descriptor_t *vfs_folder_new_entry = (struct file_descriptor_t *) ((dword_t)vfs_get_folder_data_pointer(folder_path_structure)+(sizeof(struct file_descriptor_t)*vfs_get_folder_number_of_files(folder_path_structure)));
 
 vfs_folder_new_entry->partition_number = folder_path_structure->partition_number;
 vfs_folder_new_entry->type = FILE_FOLDER;
 vfs_folder_new_entry->file_location = folder_location;
 vfs_folder_new_entry->file_size_in_bytes = 0; //this works only in FAT, so update if there is write support for other filesystems

 read_time();
 vfs_folder_new_entry->year_of_creation = time_year;
 vfs_folder_new_entry->month_of_creation = time_month;
 vfs_folder_new_entry->day_of_creation = time_day;
 vfs_folder_new_entry->hour_of_creation = time_hour;
 vfs_folder_new_entry->minute_of_creation = time_minute;
 vfs_folder_new_entry->second_of_creation = time_second;

 vfs_folder_new_entry->year_of_modification = time_year;
 vfs_folder_new_entry->month_of_modification = time_month;
 vfs_folder_new_entry->day_of_modification = time_day;
 vfs_folder_new_entry->hour_of_modification = time_hour;
 vfs_folder_new_entry->minute_of_modification = time_minute;
 vfs_folder_new_entry->second_of_modification = time_second;

 for(dword_t i=0; i<255; i++) {
  if(name[i]==0) {
   break;
  }
  vfs_folder_new_entry->name[i] = name[i];
 }

 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files++;

 //save changes to folder
 if(vfs_save_folder(folder_path_structure)==STATUS_ERROR) {
  //erase new entry
  array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files--;
  array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries = (byte_t *) realloc((dword_t)(vfs_get_folder_data_pointer(folder_path_structure)), sizeof(struct file_descriptor_t)*(vfs_get_folder_number_of_files(folder_path_structure)));

  //try to delete created folder, it do not matter if it will work or not
  delete_folder(folder_path_structure->partition_number, folder_location);

  log("\nVFS: error with creating new folder");
  return STATUS_ERROR;
 }

 //new entry was successfully created
 return STATUS_GOOD;
}

byte_t vfs_delete_folder(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 struct file_descriptor_t *folder = (vfs_get_folder_data_pointer(folder_path_structure));
 if(folder[number_of_entry].type!=FILE_FOLDER) { //this is not folder
  return STATUS_ERROR;
 }

 //allocate stream for all files in folder
 struct byte_stream_descriptor_t *list_of_files_in_folder = create_byte_stream(BYTE_STREAM_1_MB_BLOCK);
 dword_t number_of_files_in_list = 0;
 dword_t pointer_to_list = 0;

 //read content of folder
 word_t aolf_number_of_folder = read_folder(folder_path_structure->partition_number, folder[number_of_entry].file_location, folder[number_of_entry].file_size_in_bytes);
 if(aolf_number_of_folder==STATUS_ERROR) {
  log("\nVFS: delete folder error with reading content of folder");
  destroy_byte_stream(list_of_files_in_folder);
  return STATUS_ERROR;
 }
 number_of_files_in_list = array_of_loaded_folders[aolf_number_of_folder].number_of_files;
 add_bytes_to_byte_stream(list_of_files_in_folder, array_of_loaded_folders[aolf_number_of_folder].memory_of_entries, (sizeof(struct file_descriptor_t)*array_of_loaded_folders[aolf_number_of_folder].number_of_files));
 release_loaded_folder_in_array_from_usage_in_structure(aolf_number_of_folder);

 //read all files that are under folder
 while(pointer_to_list<number_of_files_in_list) {
  struct file_descriptor_t *list_of_files_in_folder_pointer = (struct file_descriptor_t *) (list_of_files_in_folder->start_of_allocated_memory);

  if(list_of_files_in_folder_pointer[pointer_to_list].type==FILE_FOLDER) {
   //read content of folder and add it to list
   aolf_number_of_folder = read_folder(folder_path_structure->partition_number, list_of_files_in_folder_pointer[pointer_to_list].file_location, list_of_files_in_folder_pointer[pointer_to_list].file_size_in_bytes);
   if(aolf_number_of_folder==STATUS_ERROR) {
    log("\nVFS: delete folder error with reading content of folder");
    destroy_byte_stream(list_of_files_in_folder);
    return STATUS_ERROR;
   }
   number_of_files_in_list += array_of_loaded_folders[aolf_number_of_folder].number_of_files;
   add_bytes_to_byte_stream(list_of_files_in_folder, array_of_loaded_folders[aolf_number_of_folder].memory_of_entries, (sizeof(struct file_descriptor_t)*array_of_loaded_folders[aolf_number_of_folder].number_of_files));
   release_loaded_folder_in_array_from_usage_in_structure(aolf_number_of_folder);

   if(number_of_files_in_list>10000) {
    log("\nVFS: too many files in folder");
    destroy_byte_stream(list_of_files_in_folder);
    return STATUS_ERROR;
   }
  }

  pointer_to_list++;
 }

 //delete all files under folder
 struct file_descriptor_t *list_of_files_in_folder_pointer = (struct file_descriptor_t *) (list_of_files_in_folder->start_of_allocated_memory);
 for(dword_t i=0; i<number_of_files_in_list; i++) {
  if(list_of_files_in_folder_pointer[i].type==NORMAL_FILE) {
   if(delete_file(folder_path_structure->partition_number, list_of_files_in_folder_pointer[i].file_location)==STATUS_ERROR) {
    log("\nVFS: can not delete file");
    destroy_byte_stream(list_of_files_in_folder);
    return STATUS_ERROR;
   }
  }
  else if(list_of_files_in_folder_pointer[i].type==FILE_FOLDER) {
   if(delete_folder(folder_path_structure->partition_number, list_of_files_in_folder_pointer[i].file_location)==STATUS_ERROR) {
    log("\nVFS: can not delete folder");
    destroy_byte_stream(list_of_files_in_folder);
    return STATUS_ERROR;
   }
  }
 }
 destroy_byte_stream(list_of_files_in_folder);

 //delete folder itself
 if(delete_folder(folder_path_structure->partition_number, folder[number_of_entry].file_location)==STATUS_ERROR) {
  log("\nVFS: can not delete folder itself");
  return STATUS_ERROR;
 }

 //delete file entry
 remove_space_from_memory_area((dword_t)folder, vfs_get_folder_number_of_files(folder_path_structure)*sizeof(struct file_descriptor_t), (dword_t)(&folder[number_of_entry]), sizeof(struct file_descriptor_t));
 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files--;
 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries = (byte_t *) realloc((dword_t)(vfs_get_folder_data_pointer(folder_path_structure)), sizeof(struct file_descriptor_t)*(vfs_get_folder_number_of_files(folder_path_structure)));

 //save changes to folder
 if(vfs_save_folder(folder_path_structure)==STATUS_ERROR) {
  log("\nVFS: error with saving changes to folder");
  return STATUS_ERROR;
 }

 return STATUS_GOOD;
}

byte_t *vfs_read_file(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //request for reading nonexisting entry
 if(number_of_entry>=vfs_get_folder_number_of_files(folder_path_structure)) {
  return STATUS_ERROR;
 }

 //pointer to folder data
 struct file_descriptor_t *folder = (vfs_get_folder_data_pointer(folder_path_structure));

 //read file
 if(folder[number_of_entry].type==NORMAL_FILE) {
  //copy file descriptor
  copy_memory((dword_t)(&folder[number_of_entry]), (dword_t)(&last_loaded_file_descriptor), sizeof(struct file_descriptor_t));
  
  //read file
  return read_file(folder_path_structure->partition_number, folder[number_of_entry].file_location, folder[number_of_entry].file_size_in_bytes);
 }
 else {
  return STATUS_ERROR; //not file entry
 }
}

byte_t vfs_save_file(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry, byte_t *file_memory, dword_t file_size_in_bytes) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //rewrite file
 struct file_descriptor_t *folder = (vfs_get_folder_data_pointer(folder_path_structure));
 if(rewrite_file(folder_path_structure->partition_number, folder[number_of_entry].file_location, file_memory, file_size_in_bytes)==STATUS_ERROR) {
  log("\nVFS: error during rewriting file");
  return STATUS_ERROR;
 }

 //update entry
 folder[number_of_entry].file_size_in_bytes = file_size_in_bytes;

 read_time();
 folder[number_of_entry].year_of_modification = time_year;
 folder[number_of_entry].month_of_modification = time_month;
 folder[number_of_entry].day_of_modification = time_day;
 folder[number_of_entry].hour_of_modification = time_hour;
 folder[number_of_entry].minute_of_modification = time_minute;
 folder[number_of_entry].second_of_modification = time_second;

 //save changes to folder
 if(vfs_save_folder(folder_path_structure)==STATUS_ERROR) {
  log("\nVFS: error with saving changes to folder");
  return STATUS_ERROR;
 }

 //new entry was successfully created
 return STATUS_GOOD;
}

byte_t vfs_create_file(struct folder_descriptor_t *folder_path_structure, word_t *name, byte_t *extension, byte_t *file_memory, dword_t file_size_in_bytes) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //create new file
 dword_t file_location = create_file(folder_path_structure->partition_number, file_memory, file_size_in_bytes);
 if(file_location==STATUS_ERROR) {
  log("\nVFS: error during creating file");
  return STATUS_ERROR;
 }

 //create new entry in folder
 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries = (byte_t *) realloc((dword_t)(vfs_get_folder_data_pointer(folder_path_structure)), sizeof(struct file_descriptor_t)*(vfs_get_folder_number_of_files(folder_path_structure)+1));
 struct file_descriptor_t *vfs_folder_new_entry = (struct file_descriptor_t *) ((dword_t)vfs_get_folder_data_pointer(folder_path_structure)+(sizeof(struct file_descriptor_t)*vfs_get_folder_number_of_files(folder_path_structure)));
 
 vfs_folder_new_entry->partition_number = folder_path_structure->partition_number;
 vfs_folder_new_entry->type = NORMAL_FILE;
 vfs_folder_new_entry->file_location = file_location;
 vfs_folder_new_entry->file_size_in_bytes = file_size_in_bytes;

 read_time();
 vfs_folder_new_entry->year_of_creation = time_year;
 vfs_folder_new_entry->month_of_creation = time_month;
 vfs_folder_new_entry->day_of_creation = time_day;
 vfs_folder_new_entry->hour_of_creation = time_hour;
 vfs_folder_new_entry->minute_of_creation = time_minute;
 vfs_folder_new_entry->second_of_creation = time_second;

 vfs_folder_new_entry->year_of_modification = time_year;
 vfs_folder_new_entry->month_of_modification = time_month;
 vfs_folder_new_entry->day_of_modification = time_day;
 vfs_folder_new_entry->hour_of_modification = time_hour;
 vfs_folder_new_entry->minute_of_modification = time_minute;
 vfs_folder_new_entry->second_of_modification = time_second;

 //copy name
 for(dword_t i=0; i<245; i++) {
  if(name[i]==0 || name[i]=='.' || i==244) {
   //add dot to name
   vfs_folder_new_entry->name[i] = '.';
   i++;

   //copy extension to name field and extension field
   for(dword_t j=0; j<10; j++) {
    if(extension[j]==0) {
     break;
    }
    vfs_folder_new_entry->name[i] = extension[j];
    i++;
    vfs_folder_new_entry->extension[j] = extension[j];
   }

   break;
  }

  //copy name to name field
  vfs_folder_new_entry->name[i] = name[i];
 }

 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files++;

 //save changes to folder
 if(vfs_save_folder(folder_path_structure)==STATUS_ERROR) {
  //erase new entry
  array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files--;
  array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries = (byte_t *) realloc((dword_t)(vfs_get_folder_data_pointer(folder_path_structure)), sizeof(struct file_descriptor_t)*(vfs_get_folder_number_of_files(folder_path_structure)));

  //try to delete created file, it do not matter if it will work or not
  delete_file(folder_path_structure->partition_number, file_location);

  log("\nVFS: error with creating new file");
  return STATUS_ERROR;
 }

 //new entry was successfully created
 return STATUS_GOOD;
}

byte_t vfs_delete_file(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry) {
 if(folder_path_structure->partition_number==NO_PARTITION_SELECTED) {
  log("\nVFS: device was unplugged");
  return STATUS_ERROR;
 }

 //delete file
 struct file_descriptor_t *folder = (vfs_get_folder_data_pointer(folder_path_structure));
 if(folder[number_of_entry].type!=NORMAL_FILE) { //this is not file
  return STATUS_ERROR;
 }
 if(delete_file(folder_path_structure->partition_number, folder[number_of_entry].file_location)==STATUS_ERROR) {
  log("\nVFS: can not delete file");
  return STATUS_ERROR;
 }

 //delete file entry
 remove_space_from_memory_area((dword_t)folder, vfs_get_folder_number_of_files(folder_path_structure)*sizeof(struct file_descriptor_t), (dword_t)(&folder[number_of_entry]), sizeof(struct file_descriptor_t));
 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].number_of_files--;
 array_of_loaded_folders[folder_path_structure->folder_number_in_array_of_loaded_folders].memory_of_entries = (byte_t *) realloc((dword_t)(vfs_get_folder_data_pointer(folder_path_structure)), sizeof(struct file_descriptor_t)*(vfs_get_folder_number_of_files(folder_path_structure)));

 //save changes to folder
 if(vfs_save_folder(folder_path_structure)==STATUS_ERROR) {
  log("\nVFS: error with saving changes to folder");
  return STATUS_ERROR;
 }

 //new entry was successfully created
 return STATUS_GOOD;
}

byte_t *vfs_read_file_show_progress(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry) {
 //load file
 create_task(dofs_show_progress_in_reading, TASK_TYPE_PERIODIC_INTERRUPT, 100);
 byte_t *file_memory = vfs_read_file(file_dialog_folder_descriptor, file_dialog_folder_descriptor->selected_entry);
 destroy_task(dofs_show_progress_in_reading);
 
 if(file_memory==STATUS_ERROR) {
  if(keyboard_code_of_pressed_key==KEY_ESC) {
   show_message_window("User cancelled reading");
   wait(1000);
  }
  else {
   error_window("Error during reading file");
  }
 }

 return file_memory;
}

byte_t vfs_save_file_show_progress(struct folder_descriptor_t *folder_path_structure, dword_t number_of_entry, byte_t *file_memory, dword_t file_size_in_bytes) {
 //load file
 create_task(dofs_show_progress_in_writing, TASK_TYPE_PERIODIC_INTERRUPT, 100);
 byte_t status = vfs_save_file(file_dialog_folder_descriptor, number_of_entry, file_memory, file_size_in_bytes);
 destroy_task(dofs_show_progress_in_writing);
 
 if(status==STATUS_ERROR) {
  if(keyboard_code_of_pressed_key==KEY_ESC) {
   show_message_window("User cancelled writing");
   wait(1000);
  }
  else {
   error_window("Error during rewriting file");
  }
 }

 return status;
}

byte_t vfs_create_file_show_progress(struct folder_descriptor_t *folder_path_structure, word_t *name, byte_t *extension, byte_t *file_memory, dword_t file_size_in_bytes) {
 //load file
 create_task(dofs_show_progress_in_writing, TASK_TYPE_PERIODIC_INTERRUPT, 100);
 byte_t status = vfs_create_file(file_dialog_folder_descriptor, name, extension, file_memory, file_size_in_bytes);
 destroy_task(dofs_show_progress_in_writing);
 
 if(status==STATUS_ERROR) {
  if(keyboard_code_of_pressed_key==KEY_ESC) {
   show_message_window("User cancelled writing");
   wait(1000);
  }
  else {
   error_window("Error during rewriting file");
  }
 }

 return status;
}