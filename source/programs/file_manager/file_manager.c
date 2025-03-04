//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_file_manager(void) {
 file_manager_program_interface_memory = create_program_interface_memory(((dword_t)&redraw_file_manager), PROGRAM_INTERFACE_FLAG_NO_OPEN_AND_SAVE_BUTTON);
 file_manager_text_area = (dword_t *) create_text_area(TEXT_AREA_INPUT_LINE, 250, 0, 0, 400, 10);
 event_interface_set_text_area(file_manager_name_input_event_interface, (dword_t)file_manager_text_area);
 file_manager_copied_file_memory = 0;
}

void file_manager(void) {
 set_program_interface(file_manager_program_interface_memory);
 program_interface_add_keyboard_event(KEY_F3, (dword_t)file_manager_new_tab);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)file_manager_close_tab);
 program_interface_redraw();

 while(1) {
  dword_t event = wait_for_event(file_manager_event_interface, redraw_file_manager);

  //process event
  if(event==EVENT_REDRAW) {
   continue;
  }
  else if(event==EVENT_USB_DEVICE_CHANGE) {
   redraw_file_manager();
   continue;
  }

  //close program
  struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
  dword_t click_zone = get_mouse_cursor_click_board_value();
  if(keyboard_code_of_pressed_key==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && click_zone==CLICK_ZONE_BACK)) {
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
    file_manager_folder_descriptor->selected_entry = FOLDER_NO_ENTRY_SELECTED;
   }
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //deselect file by click
  if(   get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0
     && (dword_t)file_manager_folder_descriptor!=0
     && file_manager_folder_descriptor->partition_number!=NO_PARTITION_SELECTED
     && mouse_click_button_state==MOUSE_CLICK
     && file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED
     && get_mouse_cursor_click_board_value()==0) {
   file_manager_folder_descriptor->selected_entry = FOLDER_NO_ENTRY_SELECTED;
   redraw_file_manager();
  }
 }
}

void file_manager_event_click_on_connected_partitions(void) {
 dword_t selected_partition = (get_mouse_cursor_click_board_value()-CLICK_ZONE_FILE_MANAGER_FIRST_CONNECTED_PARTITION);
 int view_type = -1, sort_type = -1, sort_direction = -1;

 //open new tab and load root folder
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  add_file((word_t *)"N\0e\0w\0\0\0", 0, 0, 0, 0, 0);
  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, 0);
  set_file_value(FILE_MANAGER_FOLDER_PATH_NAMES, (dword_t) calloc(20*2*(MAX_NUMBER_OF_FOLDERS_IN_PATH+2)));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);

  word_t *path_names = (word_t *) (get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES));
  path_names[0] = 'R';
  path_names[1] = 'o';
  path_names[2] = 'o';
  path_names[3] = 't';
  goto load_selected_partition;
 }

 //load root folder to new tab
 if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)==0) {
  goto load_selected_partition;
 }

 //test if this is not click on same partition
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if(selected_partition==file_manager_folder_descriptor->partition_number) {
  return;
 }

 //save selected view type, sort type and sort direction
 if((dword_t)file_manager_folder_descriptor!=0) {
  view_type = file_manager_folder_descriptor->view_type;
  sort_type = file_manager_folder_descriptor->sort_type;
  sort_direction = file_manager_folder_descriptor->sort_direction;
 }

 //unload partition
 vfs_destroy_folder_path_structure((struct folder_descriptor_t *)get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //load selected partition
 load_selected_partition:
 set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, (dword_t)vfs_create_folder_path_structure(selected_partition));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
 file_manager_scrollbar_info.showed_document_line = 0;

 //set previously selected view type, sort type and sort direction
 file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if(view_type!=-1) {
  file_manager_folder_descriptor->view_type = view_type;
  file_manager_folder_descriptor->sort_type = sort_type;
  file_manager_folder_descriptor->sort_direction = sort_direction;
  vfs_sort_folder(file_manager_folder_descriptor);
 }

 //test error
 if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)==0) {
  error_window("Error during reading root folder");
  change_file_name_byte_string("Error");
 }
 else {
  change_file_name_byte_string("Root");
 }

 //redraw screen
 redraw_file_manager();
}

void file_manager_event_key_page_up(void) {
 dword_t partition_to_select = NO_PARTITION_SELECTED;

 //add new tab and load root folder
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  //select last connected partition
  for(dword_t i=MAX_NUMBER_OF_CONNECTED_PARTITIONS; i>0; i--) {
   if(connected_partitions[(i-1)].medium_type!=NO_MEDIUM) {
    partition_to_select = (i-1);
    break;
   }
  }
  if(partition_to_select==NO_PARTITION_SELECTED) { //no partition connected
   return;
  }

  //add tab
  add_file((word_t *)"N\0e\0w\0\0\0", 0, 0, 0, 0, 0);
  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, 0);
  set_file_value(FILE_MANAGER_FOLDER_PATH_NAMES, (dword_t) calloc(20*2*(MAX_NUMBER_OF_FOLDERS_IN_PATH+2)));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  word_t *path_names = (word_t *) (get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES));
  path_names[0] = 'R';
  path_names[1] = 'o';
  path_names[2] = 'o';
  path_names[3] = 't';

  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, (dword_t)vfs_create_folder_path_structure(partition_to_select));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  file_manager_scrollbar_info.showed_document_line = 0;

  //test error
  if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)==0) {
   error_window("Error during reading root folder");
   change_file_name_byte_string("Error");
  }
  else {
   change_file_name_byte_string("Root");
  }

  //redraw screen
  redraw_file_manager();
  return;
 }

 //no partition is selected
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if((dword_t)file_manager_folder_descriptor==0) {
  //select last connected partition
  for(dword_t i=MAX_NUMBER_OF_CONNECTED_PARTITIONS; i>0; i--) {
   if(connected_partitions[(i-1)].medium_type!=NO_MEDIUM) {
    partition_to_select = (i-1);
    break;
   }
  }
 }
 else {
  //select previous partition
  for(dword_t i=file_manager_folder_descriptor->partition_number; i>0; i--) {
   if(connected_partitions[(i-1)].medium_type!=NO_MEDIUM) {
    partition_to_select = (i-1);
    break;
   }
  }
  if(file_manager_folder_descriptor->partition_number==partition_to_select) { //this partition is already selected
   return;
  }
 }

 //select partition
 if(partition_to_select!=NO_PARTITION_SELECTED) {
  //save selected view type, sort type and sort direction and unload actual structure
  int view_type = -1, sort_type = -1, sort_direction = -1;
  if((dword_t)file_manager_folder_descriptor!=0) {
   view_type = file_manager_folder_descriptor->view_type;
   sort_type = file_manager_folder_descriptor->sort_type;
   sort_direction = file_manager_folder_descriptor->sort_direction;
   vfs_destroy_folder_path_structure(file_manager_folder_descriptor);
  }

  //try to load root folder from selected partition
  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, (dword_t)vfs_create_folder_path_structure(partition_to_select));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  file_manager_scrollbar_info.showed_document_line = 0;

  //set previously selected view type, sort type and sort direction
  if(view_type!=-1) {
   file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
   file_manager_folder_descriptor->view_type = view_type;
   file_manager_folder_descriptor->sort_type = sort_type;
   file_manager_folder_descriptor->sort_direction = sort_direction;
   vfs_sort_folder(file_manager_folder_descriptor);
  }
  
  //test error
  if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)==0) {
   error_window("Error during reading root folder");
   change_file_name_byte_string("Error");
  }
  else {
   change_file_name_byte_string("Root");
  }

  //redraw screen
  redraw_file_manager();
 }
}

void file_manager_event_key_page_down(void) {
 word_t partition_to_select = NO_PARTITION_SELECTED;

 //add new tab and load root folder
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  //select first connected partition
  for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
   if(connected_partitions[i].medium_type!=NO_MEDIUM) {
    partition_to_select = i;
    break;
   }
  }
  if(partition_to_select==NO_PARTITION_SELECTED) { //no partition connected
   return;
  }

  //add tab
  add_file((word_t *)"N\0e\0w\0\0\0", 0, 0, 0, 0, 0);
  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, 0);
  set_file_value(FILE_MANAGER_FOLDER_PATH_NAMES, (dword_t) calloc(20*2*(MAX_NUMBER_OF_FOLDERS_IN_PATH+2)));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  word_t *path_names = (word_t *) (get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES));
  path_names[0] = 'R';
  path_names[1] = 'o';
  path_names[2] = 'o';
  path_names[3] = 't';

  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, (dword_t)vfs_create_folder_path_structure(partition_to_select));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  file_manager_scrollbar_info.showed_document_line = 0;

  //test error
  if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)==0) {
   error_window("Error during reading root folder");
   change_file_name_byte_string("Error");
  }
  else {
   change_file_name_byte_string("Root");
  }

  //redraw screen
  redraw_file_manager();
  return;
 }

 //no partition is selected
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if((dword_t)file_manager_folder_descriptor==0) {
  //select first connected partition
  for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
   if(connected_partitions[i].medium_type!=NO_MEDIUM) {
    partition_to_select = i;
    break;
   }
  }
 }
 else {
  //select previous partition
  for(dword_t i=(file_manager_folder_descriptor->partition_number+1); i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
   if(connected_partitions[i].medium_type!=NO_MEDIUM) {
    partition_to_select = i;
    break;
   }
  }
  if(file_manager_folder_descriptor->partition_number==partition_to_select) { //this partition is already selected
   return;
  }
 }

 //select partition
 if(partition_to_select!=NO_PARTITION_SELECTED) {
  //save selected view type, sort type and sort direction and unload actual structure
  int view_type = -1, sort_type = -1, sort_direction = -1;
  if((dword_t)file_manager_folder_descriptor!=0) {
   view_type = file_manager_folder_descriptor->view_type;
   sort_type = file_manager_folder_descriptor->sort_type;
   sort_direction = file_manager_folder_descriptor->sort_direction;
   vfs_destroy_folder_path_structure(file_manager_folder_descriptor);
  }

  //try to load root folder from selected partition
  set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, (dword_t)vfs_create_folder_path_structure(partition_to_select));
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  file_manager_scrollbar_info.showed_document_line = 0;

  //set previously selected view type, sort type and sort direction
  if(view_type!=-1) {
   file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
   file_manager_folder_descriptor->view_type = view_type;
   file_manager_folder_descriptor->sort_type = sort_type;
   file_manager_folder_descriptor->sort_direction = sort_direction;
   vfs_sort_folder(file_manager_folder_descriptor);
  }

  //test error
  if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)==0) {
   error_window("Error during reading root folder");
   change_file_name_byte_string("Error");
  }
  else {
   change_file_name_byte_string("Root");
  }

  //redraw screen
  redraw_file_manager();
 }
}

dword_t file_manager_event_key_up(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  return NO_EVENT;
 }

 //nothing to change
 if(file_manager_folder_descriptor->selected_entry==0 && file_manager_folder_descriptor->first_showed_entry==0) {
  return NO_EVENT;
 }

 if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) {
  //move selected entry
  if(file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
   file_manager_folder_descriptor->selected_entry = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1); //select last entry
  }
  else if(file_manager_folder_descriptor->selected_entry>0) {
   file_manager_folder_descriptor->selected_entry--;
  }

  //calculate first showed entry
  if(file_manager_number_of_lines_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
   if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
    file_manager_folder_descriptor->first_showed_entry = file_manager_folder_descriptor->selected_entry;
   }
   else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_lines_on_screen-1)) {
    file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_lines_on_screen+1);
   }
  }

  //set scrollbar
  file_manager_scrollbar_info.showed_document_line = file_manager_folder_descriptor->first_showed_entry;
 }
 else if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) {
  //move selected entry
  if(file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
   file_manager_folder_descriptor->selected_entry = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1); //select last entry
  }
  else if(file_manager_folder_descriptor->selected_entry<file_manager_number_of_columns_on_screen) {
   file_manager_folder_descriptor->selected_entry = 0;
  }
  else {
   file_manager_folder_descriptor->selected_entry -= file_manager_number_of_columns_on_screen;
  }

  //calculate first showed entry
  if(file_manager_number_of_items_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
   if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
    file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen));
   }
   else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_items_on_screen-1)) {
    file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_items_on_screen+(file_manager_number_of_columns_on_screen-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen)));
   }
  }

  //set scrollbar
  file_manager_scrollbar_info.showed_document_line = (file_manager_folder_descriptor->first_showed_entry/file_manager_number_of_columns_on_screen);
 }
 
 //recalculate scrollbar
 scrollbar_struct_calculate_rider_position((&file_manager_scrollbar_info));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, file_manager_scrollbar_info.rider_position);

 //redraw screen
 return EVENT_REDRAW;
}

dword_t file_manager_event_key_down(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  return NO_EVENT;
 }

 if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) {
  //nothing to change
  if(file_manager_folder_descriptor->selected_entry==(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1)
     && file_manager_folder_descriptor->first_showed_entry==(file_manager_folder_descriptor->selected_entry-file_manager_number_of_lines_on_screen+1)) {
   return NO_EVENT;
  }

  //move selected entry
  if(file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
   file_manager_folder_descriptor->selected_entry = 0; //select first entry
  }
  else if(file_manager_folder_descriptor->selected_entry<(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1)) {
   file_manager_folder_descriptor->selected_entry++;
  }

  //calculate first showed entry
  if(file_manager_number_of_lines_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
   if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
    file_manager_folder_descriptor->first_showed_entry = file_manager_folder_descriptor->selected_entry;
   }
   else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_lines_on_screen-1)) {
    file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_lines_on_screen+1);
   }
  }

  //set scrollbar
  file_manager_scrollbar_info.showed_document_line = file_manager_folder_descriptor->first_showed_entry;
 }
 else if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) {
  //nothing to change
  if(file_manager_folder_descriptor->selected_entry==(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1)
     && file_manager_folder_descriptor->first_showed_entry==(file_manager_folder_descriptor->selected_entry-file_manager_number_of_items_on_screen+1)) {
   return NO_EVENT;
  }

  //move selected entry
  if(file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
   file_manager_folder_descriptor->selected_entry = 0; //select first entry
  }
  else if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)<file_manager_number_of_columns_on_screen
          || file_manager_folder_descriptor->selected_entry>=(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-file_manager_number_of_columns_on_screen)) {
   file_manager_folder_descriptor->selected_entry = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1);
  }
  else {
   file_manager_folder_descriptor->selected_entry += file_manager_number_of_columns_on_screen;
  }

  //calculate first showed entry
  if(file_manager_number_of_items_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
   if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
    file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen));
   }
   else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_items_on_screen-1)) {
    file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_items_on_screen+(file_manager_number_of_columns_on_screen-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen)));
   }
  }

  //set scrollbar
  file_manager_scrollbar_info.showed_document_line = (file_manager_folder_descriptor->first_showed_entry/file_manager_number_of_columns_on_screen);
 }

 //recalculate scrollbar
 scrollbar_struct_calculate_rider_position((&file_manager_scrollbar_info));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, file_manager_scrollbar_info.rider_position);

 //redraw screen
 return EVENT_REDRAW;
}

dword_t file_manager_event_key_left(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->view_type!=VIEW_FOLDER_ICONS) {
  return NO_EVENT;
 }

 //move selected entry
 if(file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  file_manager_folder_descriptor->selected_entry = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1); //select last entry
 }
 else if(file_manager_folder_descriptor->selected_entry>0) {
  file_manager_folder_descriptor->selected_entry--;
 }

 //calculate first showed entry
 if(file_manager_number_of_items_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
  if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
   file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen));
  }
  else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_items_on_screen-1)) {
   file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_items_on_screen+(file_manager_number_of_columns_on_screen-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen)));
  }
 }

 //set scrollbar
 file_manager_scrollbar_info.showed_document_line = (file_manager_folder_descriptor->first_showed_entry/file_manager_number_of_columns_on_screen);

 //recalculate scrollbar
 scrollbar_struct_calculate_rider_position((&file_manager_scrollbar_info));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, file_manager_scrollbar_info.rider_position);

 //redraw screen
 return EVENT_REDRAW;
}

dword_t file_manager_event_key_right(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->view_type!=VIEW_FOLDER_ICONS) {
  return NO_EVENT;
 }

 //move selected entry
 if(file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  file_manager_folder_descriptor->selected_entry = 0; //select first entry
 }
 else if(file_manager_folder_descriptor->selected_entry<(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-1)) {
  file_manager_folder_descriptor->selected_entry++;
 }

 //calculate first showed entry
 if(file_manager_number_of_items_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
  if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
   file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen));
  }
  else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_items_on_screen-1)) {
   file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_items_on_screen+(file_manager_number_of_columns_on_screen-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen)));
  }
 }

 //set scrollbar
 file_manager_scrollbar_info.showed_document_line = (file_manager_folder_descriptor->first_showed_entry/file_manager_number_of_columns_on_screen);

 //recalculate scrollbar
 scrollbar_struct_calculate_rider_position((&file_manager_scrollbar_info));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, file_manager_scrollbar_info.rider_position);

 //redraw screen
 return EVENT_REDRAW;
}

dword_t file_manager_event_key_enter(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and file selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  return NO_EVENT;
 }

 //pointer to folder data
 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);

 //folder
 if(folder[file_manager_folder_descriptor->selected_entry].type==FILE_FOLDER) {
  //open folder
  show_message_window("Opening folder...");
  copy_memory((dword_t)folder[file_manager_folder_descriptor->selected_entry].name, get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES)+((file_manager_folder_descriptor->pointer_to_path+1)*20*2), 20*2); //copy name of folder
  if(vfs_open_folder(file_manager_folder_descriptor, file_manager_folder_descriptor->selected_entry)==STATUS_ERROR) {
   error_window("Error during opening folder");
   change_file_name_byte_string("Error");
  }
  else {
   change_file_name_word_string((word_t *)(get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES)+(file_manager_folder_descriptor->pointer_to_path*20*2)));
  }
  file_manager_scrollbar_info.rider_position = 0;
  set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);
  return EVENT_REDRAW;
 }
 else if(folder[file_manager_folder_descriptor->selected_entry].type==NORMAL_FILE) { //normal file
  error_window("This version can not open files directly from File Manager"); //TODO:
 }

 return NO_EVENT;
}

dword_t file_manager_event_click_on_files(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  return NO_EVENT;
 }

 //calculate what file was clicked on
 dword_t selected_file = (get_mouse_cursor_click_board_value()-CLICK_ZONE_FILE_MANAGER_FIRST_ITEM);

 //double click
 if(selected_file==file_manager_folder_descriptor->selected_entry) {
  return file_manager_event_key_enter();
 }
 
 //first click
 file_manager_folder_descriptor->selected_entry = selected_file;
 return EVENT_REDRAW;
}

dword_t file_manager_event_go_to_previous_folder(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and it is not root folder
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->pointer_to_path==0) {
  return NO_EVENT;
 }

 //go to previous folder
 show_message_window("Opening folder...");
 if(vfs_go_back_in_folder_path(file_manager_folder_descriptor)==STATUS_ERROR) {
  error_window("Error during opening folder");
  change_file_name_byte_string("Error");
 }
 else {
  change_file_name_word_string((word_t *)(get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES)+(file_manager_folder_descriptor->pointer_to_path*20*2)));
 }
 file_manager_scrollbar_info.rider_position = 0;
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);

 return EVENT_REDRAW;
}

void file_manager_event_scrollbar_change(dword_t value) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //save new position of scrollbar
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, file_manager_scrollbar_info.rider_position);

 //set first showed entry
 if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) {
  file_manager_folder_descriptor->first_showed_entry = value;
 }
 else if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) {
  file_manager_folder_descriptor->first_showed_entry = (value*file_manager_number_of_columns_on_screen);
 }

 //redraw file manager
 redraw_file_manager();
}

dword_t file_manager_change_view_window(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  return NO_EVENT;
 }

 //draw message window
 clear_click_board();
 draw_message_window(220, 10+16+50+16+50+16+90+20+10);
 program_element_layout_initalize_for_window(220, 10+16+50+16+50+16+90+20+10);
 program_element_layout_add_border_to_area(FIRST_AREA, 10);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Show files as");
 program_element_move_horizontally(FIRST_AREA, 8);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[i] Icons", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_ICONS, ((file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) ? SELECTED_LIST_ENTRY : 0));
 program_element_move_horizontally(FIRST_AREA, 0);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[l] List", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_LIST, ((file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) ? SELECTED_LIST_ENTRY : 1));
 program_element_move_horizontally(FIRST_AREA, 10);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Sort files");
 program_element_move_horizontally(FIRST_AREA, 8);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[a] In ascending order", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_IN_ASCENDING_ORDER, ((file_manager_folder_descriptor->sort_direction==SORT_FOLDER_IN_ASCENDING_ORDER) ? SELECTED_LIST_ENTRY : 0));
 program_element_move_horizontally(FIRST_AREA, 0);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[b] In descending order", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_IN_DESCENDING_ORDER, ((file_manager_folder_descriptor->sort_direction==SORT_FOLDER_IN_DESCENDING_ORDER) ? SELECTED_LIST_ENTRY : 1));
 program_element_move_horizontally(FIRST_AREA, 10);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "by");
 program_element_move_horizontally(FIRST_AREA, 8);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[n] Name", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_NAME, ((file_manager_folder_descriptor->sort_type==SORT_FOLDER_BY_NAME) ? SELECTED_LIST_ENTRY : 0));
 program_element_move_horizontally(FIRST_AREA, 0);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[e] Extension", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_EXTENSION, ((file_manager_folder_descriptor->sort_type==SORT_FOLDER_BY_EXTENSION) ? SELECTED_LIST_ENTRY : 1));
 program_element_move_horizontally(FIRST_AREA, 0);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[s] Size", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_SIZE, ((file_manager_folder_descriptor->sort_type==SORT_FOLDER_BY_SIZE) ? SELECTED_LIST_ENTRY : 2));
 program_element_move_horizontally(FIRST_AREA, 0);
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[d] Date of creation", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_DATE_OF_CREATION, ((file_manager_folder_descriptor->sort_type==SORT_FOLDER_BY_DATE_OF_CREATION) ? SELECTED_LIST_ENTRY : 3));
 program_element_move_horizontally(FIRST_AREA, 10);

 add_button(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[esc] Back", CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_BACK);

 redraw_screen();

 //process events
 while(1) {
  dword_t event = wait_for_event(file_manager_view_window_event_interface, 0);

  if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_BACK) {
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_LIST) {
   file_manager_folder_descriptor->view_type = VIEW_FOLDER_LIST;

   //calculate first showed entry
   file_manager_number_of_lines_on_screen = (program_element_layout_areas_info[SECOND_AREA].height/10);
   file_manager_folder_descriptor->first_showed_entry = 0;
   if(file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED && file_manager_number_of_lines_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
    if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
     file_manager_folder_descriptor->first_showed_entry = file_manager_folder_descriptor->selected_entry;
    }
    else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_lines_on_screen-1)) {
     file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_lines_on_screen+1);
    }
   }
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_ICONS) {
   file_manager_folder_descriptor->view_type = VIEW_FOLDER_ICONS;

   //calculate first showed entry
   file_manager_number_of_columns_on_screen = ((program_element_layout_areas_info[SECOND_AREA].width-30)/(FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+16));
   file_manager_number_of_lines_on_screen = ((program_element_layout_areas_info[SECOND_AREA].height-20)/(FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+30+16));
   file_manager_number_of_items_on_screen = (file_manager_number_of_columns_on_screen*file_manager_number_of_lines_on_screen);
   file_manager_folder_descriptor->first_showed_entry = 0;
   if(file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED && file_manager_number_of_items_on_screen<vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
    if(file_manager_folder_descriptor->first_showed_entry>file_manager_folder_descriptor->selected_entry) {
     file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen));
    }
    else if(file_manager_folder_descriptor->selected_entry>(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_items_on_screen-1)) {
     file_manager_folder_descriptor->first_showed_entry = (file_manager_folder_descriptor->selected_entry-file_manager_number_of_items_on_screen+(file_manager_number_of_columns_on_screen-(file_manager_folder_descriptor->selected_entry%file_manager_number_of_columns_on_screen)));
    }
   }
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_IN_ASCENDING_ORDER) {
   file_manager_folder_descriptor->sort_direction = SORT_FOLDER_IN_ASCENDING_ORDER;
   vfs_sort_folder(file_manager_folder_descriptor);
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_IN_DESCENDING_ORDER) {
   file_manager_folder_descriptor->sort_direction = SORT_FOLDER_IN_DESCENDING_ORDER;
   vfs_sort_folder(file_manager_folder_descriptor);
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_NAME) {
   file_manager_folder_descriptor->sort_type = SORT_FOLDER_BY_NAME;
   vfs_sort_folder(file_manager_folder_descriptor);
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_EXTENSION) {
   file_manager_folder_descriptor->sort_type = SORT_FOLDER_BY_EXTENSION;
   vfs_sort_folder(file_manager_folder_descriptor);
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_SIZE) {
   file_manager_folder_descriptor->sort_type = SORT_FOLDER_BY_SIZE;
   vfs_sort_folder(file_manager_folder_descriptor);
   break;
  }
  else if(event==CLICK_ZONE_FILE_MANAGER_VIEW_WINDOW_SORT_BY_DATE_OF_CREATION) {
   file_manager_folder_descriptor->sort_type = SORT_FOLDER_BY_DATE_OF_CREATION;
   vfs_sort_folder(file_manager_folder_descriptor);
   break;
  }
 }

 //redraw file manager
 return EVENT_REDRAW;
}

dword_t file_manager_preview_window(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and image file is selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  return NO_EVENT;
 }
 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);
 if(vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "jpg png gif qoi bmp")==STATUS_FALSE) {
  return NO_EVENT;
 }

 //get where are next and previous image file
 dword_t next_image_file = 0xFFFFFFFF, previous_image_file = 0xFFFFFFFF;
 reload_preview_window:
 next_image_file = 0xFFFFFFFF;
 for(dword_t i=(file_manager_folder_descriptor->selected_entry+1); i<vfs_get_folder_number_of_files(file_manager_folder_descriptor); i++) {
  if(vfs_does_file_have_this_extension(&folder[i], "jpg png gif qoi bmp")==STATUS_TRUE) {
   next_image_file = i;
   break;
  }
 }
 previous_image_file = 0xFFFFFFFF;
 for(dword_t i=file_manager_folder_descriptor->selected_entry; i>0; i--) {
  if(vfs_does_file_have_this_extension(&folder[(i-1)], "jpg png gif qoi bmp")==STATUS_TRUE) {
   previous_image_file = (i-1);
   break;
  }
 }

 //load and decode image file
 byte_t *image_file = vfs_read_file_show_progress(file_manager_folder_descriptor, file_manager_folder_descriptor->selected_entry);

 dword_t *image = 0;
 if((dword_t)image_file==STATUS_ERROR) {
  image = STATUS_ERROR;
 }
 else {
  show_message_window("Decoding image...");

  if(vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "jpg")==STATUS_TRUE) {
   image = (dword_t *) convert_jpg_to_image_data((dword_t)image_file, folder[file_manager_folder_descriptor->selected_entry].file_size_in_bytes);
  }
  else if(vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "png")==STATUS_TRUE) {
   image = (dword_t *) convert_png_to_image_data((dword_t)image_file, folder[file_manager_folder_descriptor->selected_entry].file_size_in_bytes);
  }
  else if(vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "gif")==STATUS_TRUE) {
   image = (dword_t *) convert_gif_to_image_data((dword_t)image_file, folder[file_manager_folder_descriptor->selected_entry].file_size_in_bytes);
  }
  else if(vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "qoi")==STATUS_TRUE) {
   image = (dword_t *) convert_qoi_to_image_data((dword_t)image_file);
  }
  else if(vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "bmp")==STATUS_TRUE) {
   image = (dword_t *) convert_bmp_to_image_data((dword_t)image_file);
  }

  free((void *)image_file);

  if((dword_t)image==STATUS_ERROR) {
   error_window("Error during decoding image");
  }
 }

 //draw message window
 clear_click_board();
 draw_message_window(620, 420);
 program_element_layout_initalize_for_window(620, 420);
 program_element_layout_add_border_to_area(FIRST_AREA, 10);

 //draw image file name
 word_t image_file_name[76];
 copy_memory((dword_t)(&folder[file_manager_folder_descriptor->selected_entry].name), (dword_t)(&image_file_name), 75*2);
 image_file_name[75] = 0;
 program_element_layout_calculate_floating_element_position(FIRST_AREA, 600, 400, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_TOP_ALIGNMENT, get_number_of_chars_in_unicode_string(image_file_name)*8, 8);
 print_unicode(image_file_name, element_x, element_y, BLACK);
 program_element_layout_skip_space_horizontally(FIRST_AREA, 20);

 //draw image
 if((dword_t)image==STATUS_ERROR) {
  program_element_layout_calculate_floating_element_position(FIRST_AREA, 600, 350, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, (sizeof("Preview not loaded")-1)*8, 8);
  print("Preview not loaded", element_x, element_y, BLACK);
  program_element_layout_skip_space_horizontally(FIRST_AREA, 360);
 }
 else {
  program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, 600, 350);

  //recalculate image size to fit in 600x350 rectangle
  image[IMAGE_INFO_WIDTH] = image[IMAGE_INFO_REAL_WIDTH];
  image[IMAGE_INFO_HEIGHT] = image[IMAGE_INFO_REAL_HEIGHT];
  
  if(image[IMAGE_INFO_REAL_WIDTH]>600) {
   image[IMAGE_INFO_WIDTH] = 600;
   image[IMAGE_INFO_HEIGHT] = (image[IMAGE_INFO_REAL_HEIGHT]*600/image[IMAGE_INFO_REAL_WIDTH]);
   image[IMAGE_INFO_DRAW_HEIGHT] = image[IMAGE_INFO_HEIGHT];
  }
  image[IMAGE_INFO_DRAW_WIDTH] = image[IMAGE_INFO_WIDTH];

  if(image[IMAGE_INFO_HEIGHT]>350) {
   image[IMAGE_INFO_WIDTH] = (image[IMAGE_INFO_WIDTH]*350/image[IMAGE_INFO_HEIGHT]);
   image[IMAGE_INFO_DRAW_WIDTH] = image[IMAGE_INFO_WIDTH];
   image[IMAGE_INFO_HEIGHT] = 350;
  }
  image[IMAGE_INFO_DRAW_HEIGHT] = image[IMAGE_INFO_HEIGHT];

  image[IMAGE_INFO_SCREEN_X] = element_x+((600-image[IMAGE_INFO_WIDTH])/2);
  image[IMAGE_INFO_SCREEN_Y] = element_y+((350-image[IMAGE_INFO_HEIGHT])/2);

  //draw image
  draw_resized_image((dword_t)image);

  program_element_move_horizontally(FIRST_AREA, 10);
 }

 //draw buttons
 program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, 150+10+150, 20);
 if(previous_image_file!=0xFFFFFFFF) {
  draw_button_with_click_zone("[left] Previous", element_x, element_y, 150, 20, CLICK_ZONE_FILE_MANAGER_PREVIEW_WINDOW_LEFT);
 }
 if(next_image_file!=0xFFFFFFFF) {
  draw_button_with_click_zone("[right] Next", element_x+160, element_y, 150, 20, CLICK_ZONE_FILE_MANAGER_PREVIEW_WINDOW_RIGHT);
 }

 redraw_screen();

 //process events
 while(1) {
  dword_t event = wait_for_event(file_manager_preview_window_event_interface, 0);

  //exit
  if(event==CLICK_ZONE_FILE_MANAGER_PREVIEW_WINDOW_BACK) {
   if((dword_t)image!=STATUS_ERROR) {
    free((void *)image);
   }
   return EVENT_REDRAW;
  }

  //move to other file
  if(event==CLICK_ZONE_FILE_MANAGER_PREVIEW_WINDOW_LEFT && previous_image_file!=0xFFFFFFFF) {
   if((dword_t)image!=STATUS_ERROR) {
    free((void *)image);
   }
   file_manager_folder_descriptor->selected_entry = previous_image_file;
   goto reload_preview_window;
  }
  if(event==CLICK_ZONE_FILE_MANAGER_PREVIEW_WINDOW_RIGHT && next_image_file!=0xFFFFFFFF) {
   if((dword_t)image!=STATUS_ERROR) {
    free((void *)image);
   }
   file_manager_folder_descriptor->selected_entry = next_image_file;
   goto reload_preview_window;
  }
 }
}

dword_t file_manager_create_folder_in_folder(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and filesystem is read-write
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || is_filesystem_read_write(connected_partitions[file_manager_folder_descriptor->partition_number].filesystem)==STATUS_FALSE) {
  return NO_EVENT;
 }

 //draw window
 clear_click_board();
 redraw_window:
 draw_message_window(420, 10+16+10+10+20+10);
 program_element_layout_initalize_for_window(420, 10+16+10+10+20+10);
 program_element_layout_add_border_to_area(FIRST_AREA, 10);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Name of new folder:");
 program_element_move_horizontally(FIRST_AREA, 8);

 program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, 10);
 file_manager_text_area[TEXT_AREA_INFO_X] = element_x;
 file_manager_text_area[TEXT_AREA_INFO_Y] = element_y;
 clear_memory(file_manager_text_area[TEXT_AREA_INFO_MEMORY], 250*2);
 text_area_move_cursor_to_start((dword_t)file_manager_text_area);
 draw_text_area((dword_t)file_manager_text_area);
 program_element_move_horizontally(FIRST_AREA, 10);

 program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, 150+10+150, 20);
 draw_button_with_click_zone("[esc] Cancel", element_x, element_y, 150, 20, CLICK_ZONE_FILE_MANAGER_NAME_INPUT_CANCEL);
 draw_button_with_click_zone("[enter] Create", element_x+160, element_y, 150, 20, CLICK_ZONE_FILE_MANAGER_NAME_INPUT_APPROVE);

 redraw_screen();

 while(1) {
  dword_t event = wait_for_event(file_manager_name_input_event_interface, 0);

  if(event==CLICK_ZONE_FILE_MANAGER_NAME_INPUT_CANCEL) {
   return EVENT_REDRAW;
  }

  if(event==CLICK_ZONE_FILE_MANAGER_NAME_INPUT_APPROVE) {
   //check if this is valid name
   word_t *text_area_data = (word_t *) (file_manager_text_area[TEXT_AREA_INFO_MEMORY]);
   for(dword_t i=0; i<250; i++) {
    if(text_area_data[i]=='<'
       || text_area_data[i]=='>'
       || text_area_data[i]==':'
       || text_area_data[i]=='"'
       || text_area_data[i]=='/'
       || text_area_data[i]=='\\'
       || text_area_data[i]=='|'
       || text_area_data[i]=='?'
       || text_area_data[i]=='*') {
     error_window("Please avoid these characters in name: < > : \" / \\ | ? *" );
     goto redraw_window;
    }
   }

   //create folder
   show_message_window("Creating folder...");
   if(vfs_create_folder(file_manager_folder_descriptor, text_area_data)==STATUS_ERROR) {
    error_window("Error during creating new folder");
   }

   return EVENT_REDRAW;
  }
 }
}

dword_t file_manager_delete_file_in_folder(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and filesystem is read-write and some file is selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || is_filesystem_read_write(connected_partitions[file_manager_folder_descriptor->partition_number].filesystem)==STATUS_FALSE || file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  return NO_EVENT;
 }

 //ask if user really wants to delete this file/folder
 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);
 if(folder[file_manager_folder_descriptor->selected_entry].type==NORMAL_FILE) {
  if(dialog_yes_no("Do you really want to delete this file?")==STATUS_TRUE) {
   //delete file
   show_message_window("Deleting file...");
   if(vfs_delete_file(file_manager_folder_descriptor, file_manager_folder_descriptor->selected_entry)==STATUS_ERROR) {
    error_window("Error during deleting file");
   }
  }
 }
 else if(folder[file_manager_folder_descriptor->selected_entry].type==FILE_FOLDER) {
  if(dialog_yes_no("Do you really want to delete this folder?")==STATUS_TRUE) {
   //delete folder
   show_message_window("Deleting folder...");
   if(vfs_delete_folder(file_manager_folder_descriptor, file_manager_folder_descriptor->selected_entry)==STATUS_ERROR) {
    error_window("Error during deleting folder");
   }
  }
 }
 else {
  return NO_EVENT;
 }
 //TODO: delete more files at once

 //unselect entry
 file_manager_folder_descriptor->selected_entry = FOLDER_NO_ENTRY_SELECTED;

 return EVENT_REDRAW;
}

dword_t file_manager_rename_file_in_folder(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and filesystem is read-write and some file is selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || is_filesystem_read_write(connected_partitions[file_manager_folder_descriptor->partition_number].filesystem)==STATUS_FALSE || file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  return NO_EVENT;
 }

 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);

 //draw window
 clear_click_board();
 redraw_window:
 draw_message_window(420, 10+16+10+10+20+10);
 program_element_layout_initalize_for_window(420, 10+16+10+10+20+10);
 program_element_layout_add_border_to_area(FIRST_AREA, 10);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Name of file:");
 program_element_move_horizontally(FIRST_AREA, 8);

 program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, 10);
 file_manager_text_area[TEXT_AREA_INFO_X] = element_x;
 file_manager_text_area[TEXT_AREA_INFO_Y] = element_y;
 copy_memory((dword_t)(&folder[file_manager_folder_descriptor->selected_entry].name), file_manager_text_area[TEXT_AREA_INFO_MEMORY], 250*2);
 text_area_move_cursor_to_start((dword_t)file_manager_text_area);
 draw_text_area((dword_t)file_manager_text_area);
 program_element_move_horizontally(FIRST_AREA, 10);

 program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, 150+10+150, 20);
 draw_button_with_click_zone("[esc] Cancel", element_x, element_y, 150, 20, CLICK_ZONE_FILE_MANAGER_NAME_INPUT_CANCEL);
 draw_button_with_click_zone("[enter] Create", element_x+160, element_y, 150, 20, CLICK_ZONE_FILE_MANAGER_NAME_INPUT_APPROVE);

 redraw_screen();

 while(1) {
  dword_t event = wait_for_event(file_manager_name_input_event_interface, 0);

  if(event==CLICK_ZONE_FILE_MANAGER_NAME_INPUT_CANCEL) {
   return EVENT_REDRAW;
  }

  if(event==CLICK_ZONE_FILE_MANAGER_NAME_INPUT_APPROVE) {
   //check if this is valid name
   word_t *text_area_data = (word_t *) (file_manager_text_area[TEXT_AREA_INFO_MEMORY]);
   for(dword_t i=0; i<250; i++) {
    if(text_area_data[i]=='<'
       || text_area_data[i]=='>'
       || text_area_data[i]==':'
       || text_area_data[i]=='"'
       || text_area_data[i]=='/'
       || text_area_data[i]=='\\'
       || text_area_data[i]=='|'
       || text_area_data[i]=='?'
       || text_area_data[i]=='*') {
     error_window("Please avoid these characters in name: < > : \" / \\ | ? *" );
     goto redraw_window;
    }
   }

   //rename file
   copy_memory(file_manager_text_area[TEXT_AREA_INFO_MEMORY], (dword_t)(&folder[file_manager_folder_descriptor->selected_entry].name), 250*2);
   if(folder[file_manager_folder_descriptor->selected_entry].type==NORMAL_FILE) {
    vfs_entry_parse_extension_from_name((&folder[file_manager_folder_descriptor->selected_entry]));
   }

   //save changes to folder
   show_message_window("Saving folder...");
   if(vfs_save_folder(file_manager_folder_descriptor)==STATUS_ERROR) {
    error_window("Error during saving folder");
   }

   return EVENT_REDRAW;
  }
 }
}

dword_t file_manager_copy_file(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and some file is selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  return NO_EVENT;
 }

 //TODO: copy also folders
 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);
 if(folder[file_manager_folder_descriptor->selected_entry].type!=NORMAL_FILE) {
  return NO_EVENT;
 }

 //read file
 if((dword_t)file_manager_copied_file_memory!=0) {
  free((void *)file_manager_copied_file_memory);
 }
 file_manager_copied_file_memory = vfs_read_file_show_progress(file_manager_folder_descriptor, file_manager_folder_descriptor->selected_entry);
 if((dword_t)file_manager_copied_file_memory==STATUS_ERROR) {
  error_window("Error during reading file");
 }
 else {
  //save file descriptor
  copy_memory((dword_t)(&folder[file_manager_folder_descriptor->selected_entry]), (dword_t)(&file_manager_copied_file_descriptor), sizeof(struct file_descriptor_t));
 }

 return EVENT_REDRAW;
}

dword_t file_manager_paste_file(void) {
 //we can paste only if some file is loaded
 if((dword_t)file_manager_copied_file_memory==STATUS_ERROR) {
  return NO_EVENT;
 }

 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and filesystem is read-write and some file is selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || is_filesystem_read_write(connected_partitions[file_manager_folder_descriptor->partition_number].filesystem)==STATUS_FALSE) {
  return NO_EVENT;
 }

 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);

 //add file
 show_message_window("Pasting file...");
 if(vfs_create_file_by_file_descriptor_show_progress(file_manager_folder_descriptor, file_manager_copied_file_descriptor, file_manager_copied_file_memory)==STATUS_ERROR) {
  error_window("Error during pasting file");
 }

 return EVENT_REDRAW;
}

dword_t file_manager_mouse_wheel_movement_event(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  return NO_EVENT;
 }

 if(mouse_wheel_movement<0x80000000) { //up
  if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) {
   //move first showed entry
   file_manager_folder_descriptor->first_showed_entry -= 10;
   if(file_manager_folder_descriptor->first_showed_entry>0x80000000) {
    file_manager_folder_descriptor->first_showed_entry = 0;
   }

   //set scrollbar
   file_manager_scrollbar_info.showed_document_line = file_manager_folder_descriptor->first_showed_entry;
  }
  else if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) {
   //move first showed entry
   file_manager_folder_descriptor->first_showed_entry -= file_manager_number_of_columns_on_screen;
   if(file_manager_folder_descriptor->first_showed_entry>0x80000000) {
    file_manager_folder_descriptor->first_showed_entry = 0;
   }

   //set scrollbar
   file_manager_scrollbar_info.showed_document_line = (file_manager_folder_descriptor->first_showed_entry/file_manager_number_of_columns_on_screen);
  }
 }
 else { //down
  if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) {
   //move first showed entry
   if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)>file_manager_number_of_lines_on_screen) {
    file_manager_folder_descriptor->first_showed_entry += 10;
    if(file_manager_folder_descriptor->first_showed_entry>(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-file_manager_number_of_lines_on_screen)) {
     file_manager_folder_descriptor->first_showed_entry = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)-file_manager_number_of_lines_on_screen);
    }
   }

   //set scrollbar
   file_manager_scrollbar_info.showed_document_line = file_manager_folder_descriptor->first_showed_entry;
  }
  else if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) {
   //move first showed entry
   if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)>file_manager_number_of_items_on_screen && file_manager_folder_descriptor->first_showed_entry<(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-file_manager_number_of_items_on_screen)) {
    file_manager_folder_descriptor->first_showed_entry += file_manager_number_of_columns_on_screen;
   }

   //set scrollbar
   file_manager_scrollbar_info.showed_document_line = (file_manager_folder_descriptor->first_showed_entry/file_manager_number_of_columns_on_screen);
  }
 }

 //recalculate scrollbar
 scrollbar_struct_calculate_rider_position((&file_manager_scrollbar_info));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, file_manager_scrollbar_info.rider_position);

 //redraw screen
 return EVENT_REDRAW;
}

dword_t file_manager_more_buttons(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  return EVENT_REDRAW;
 }

 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);

 //count number of showed buttons
 dword_t number_of_buttons = 1, list_entry_number = 0; //"Create folder"
 if(file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED) {
  //"Properties" "Rename" "Delete"
  number_of_buttons += 3;

  //"Copy"
  if(folder[file_manager_folder_descriptor->selected_entry].type==NORMAL_FILE) {
   number_of_buttons++;
  }
 }
 //"Paste"
 if((dword_t)file_manager_copied_file_memory!=STATUS_ERROR) {
  number_of_buttons++;
 }

 //draw window
 clear_click_board();
 draw_message_window(220, 10+20*number_of_buttons+10);
 program_element_layout_initalize_for_window(220, 10+20*number_of_buttons+10);
 program_element_layout_add_border_to_area(FIRST_AREA, 10);

 if(file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED) {
  //draw button for showing file properties
  add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[F5] Properties", CLICK_ZONE_FILE_MANAGER_PROPERTIES, list_entry_number);
  program_element_move_horizontally(FIRST_AREA, 0);
  list_entry_number++;

  //draw button for renaming files
  add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[r] Rename", CLICK_ZONE_FILE_MANAGER_RENAME, list_entry_number);
  program_element_move_horizontally(FIRST_AREA, 0);
  list_entry_number++;

  //draw button for deleting files
  add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[del] Delete", CLICK_ZONE_FILE_MANAGER_DELETE, list_entry_number);
  program_element_move_horizontally(FIRST_AREA, 0);
  list_entry_number++;

  //draw button for copying files
  if(folder[file_manager_folder_descriptor->selected_entry].type==NORMAL_FILE) {
   add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[ctrl+c] Copy", CLICK_ZONE_FILE_MANAGER_COPY, list_entry_number);
   program_element_move_horizontally(FIRST_AREA, 0);
   list_entry_number++;
  }
 }

 //draw button for pasting files
 if((dword_t)file_manager_copied_file_memory!=STATUS_ERROR) {
  add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[ctrl+v] Paste", CLICK_ZONE_FILE_MANAGER_PASTE, list_entry_number);
  program_element_move_horizontally(FIRST_AREA, 0);
  list_entry_number++;
 }

 //draw button for creating folders
 add_list_entry(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, 200, "[c] Create folder", CLICK_ZONE_FILE_MANAGER_CREATE_NEW_FOLDER, list_entry_number);

 redraw_screen();

 while(1) {
  dword_t event = wait_for_event(file_manager_more_buttons_event_interface, 0);

  if(event==EVENT_EXIT) {
   return EVENT_REDRAW;
  }
  else if(event!=NO_EVENT) {
   return event;
  }
 }
}

void file_manager_properties(void) {
 struct folder_descriptor_t *file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));

 //this works only if some folder is opened and some file is selected
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 || file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED || file_manager_folder_descriptor->selected_entry==FOLDER_NO_ENTRY_SELECTED) {
  return;
 }

 //draw window
 struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);
 dword_t number_of_lines_of_name_on_screen = (get_number_of_chars_in_unicode_string(folder[file_manager_folder_descriptor->selected_entry].name)/64);
 if((get_number_of_chars_in_unicode_string(folder[file_manager_folder_descriptor->selected_entry].name)%64)!=0) {
  number_of_lines_of_name_on_screen++;
 }
 draw_message_window(10+64*8+10, 10+16+16+(number_of_lines_of_name_on_screen*8)+8+32+32+32+32+24+10);
 program_element_layout_initalize_for_window(10+64*8+10, 10+16+16+(number_of_lines_of_name_on_screen*8)+8+32+32+32+32+24+10);
 program_element_layout_add_border_to_area(FIRST_AREA, 10);

 add_label(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, "Properties of file");
 program_element_move_horizontally(FIRST_AREA, 8);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "File name:");
 program_element_move_horizontally(FIRST_AREA, 8);
 for(dword_t i=0; i<number_of_lines_of_name_on_screen; i++) {
  for(dword_t j=0; j<64; j++) {
   if(folder[file_manager_folder_descriptor->selected_entry].name[i*64+j]==0) {
    break;
   }
   draw_char(folder[file_manager_folder_descriptor->selected_entry].name[i*64+j], program_element_layout_areas_info[FIRST_AREA].actual_element_x+j*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  }
  program_element_move_horizontally(FIRST_AREA, 0);
 }
 program_element_move_horizontally(FIRST_AREA, 0);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "File extension:");
 program_element_move_horizontally(FIRST_AREA, 8);
 if(folder[file_manager_folder_descriptor->selected_entry].type==FILE_FOLDER) {
  print("folder", program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  for(dword_t i=0; i<10; i++) {
   if(folder[file_manager_folder_descriptor->selected_entry].extension[i]==0) {
    break;
   }
   draw_char(folder[file_manager_folder_descriptor->selected_entry].extension[i], program_element_layout_areas_info[FIRST_AREA].actual_element_x+i*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  }
 }
 program_element_move_horizontally(FIRST_AREA, 8);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Location in filesystem:");
 program_element_move_horizontally(FIRST_AREA, 8);
 print_hex(folder[file_manager_folder_descriptor->selected_entry].file_location, program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 program_element_move_horizontally(FIRST_AREA, 8);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "File size in bytes:");
 program_element_move_horizontally(FIRST_AREA, 8);
 print_var(folder[file_manager_folder_descriptor->selected_entry].file_size_in_bytes, program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 program_element_move_horizontally(FIRST_AREA, 8);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Date of creation:");
 program_element_move_horizontally(FIRST_AREA, 8);
 print("    /  /     :  :", program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 print_var(folder[file_manager_folder_descriptor->selected_entry].year_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 if(folder[file_manager_folder_descriptor->selected_entry].month_of_creation>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].month_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+5*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+5*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].month_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+6*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].day_of_creation>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].day_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+8*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+8*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].day_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+9*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].hour_of_creation>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].hour_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+11*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+11*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].hour_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+12*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].minute_of_creation>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].minute_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+14*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+14*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].minute_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+15*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].second_of_creation>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].second_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+17*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+17*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].second_of_creation, program_element_layout_areas_info[FIRST_AREA].actual_element_x+18*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 program_element_move_horizontally(FIRST_AREA, 8);

 add_label(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, "Date of last modification:");
 program_element_move_horizontally(FIRST_AREA, 8);
 print("    /  /     :  :", program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 print_var(folder[file_manager_folder_descriptor->selected_entry].year_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 if(folder[file_manager_folder_descriptor->selected_entry].month_of_modification>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].month_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+5*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+5*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].month_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+6*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].day_of_modification>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].day_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+8*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+8*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].day_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+9*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].hour_of_modification>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].hour_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+11*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+11*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].hour_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+12*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].minute_of_modification>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].minute_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+14*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+14*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].minute_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+15*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 if(folder[file_manager_folder_descriptor->selected_entry].second_of_modification>=10) {
  print_var(folder[file_manager_folder_descriptor->selected_entry].second_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+17*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 else {
  draw_char('0', program_element_layout_areas_info[FIRST_AREA].actual_element_x+17*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
  print_var(folder[file_manager_folder_descriptor->selected_entry].second_of_modification, program_element_layout_areas_info[FIRST_AREA].actual_element_x+18*8, program_element_layout_areas_info[FIRST_AREA].actual_element_y, BLACK);
 }
 program_element_move_horizontally(FIRST_AREA, 8);

 redraw_screen();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  if(keyboard_code_of_pressed_key==KEY_ESC || mouse_click_button_state==MOUSE_CLICK) {
   return;
  }
 }
}

void redraw_file_manager(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("File manager", "", 0x803300, 0xFF6600);

 struct folder_descriptor_t *file_manager_folder_descriptor = 0;
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  file_manager_folder_descriptor = (struct folder_descriptor_t *) (get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 }

 //initalize layout
 program_element_layout_initalize_for_program();
 program_element_layout_split_area_vertically(FIRST_AREA, IN_PIXELS, FILE_MANAGER_WIDTH_OF_CONNECTED_PARTITIONS);
 program_element_layout_draw_background_of_area(FIRST_AREA, 0xB04100);

 //draw down border
 add_button_to_bottom_line_from_left("[esc] Back", CLICK_ZONE_BACK);
 if((dword_t)file_manager_folder_descriptor!=0 && file_manager_folder_descriptor->partition_number!=NO_PARTITION_SELECTED) {
  //if we are not in root folder, add button to go to previous folder
  if(file_manager_folder_descriptor->pointer_to_path!=0) {
   add_button_to_bottom_line_from_left("[b] Go to previous folder", CLICK_ZONE_FILE_MANAGER_GO_TO_PREVIOUS_FOLDER);
  }

  //add button for changing view
  add_button_to_bottom_line_from_left("[v] View", CLICK_ZONE_FILE_MANAGER_CHANGE_VIEW_WINDOW);

  //for images add button for showing preview
  struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);
  if(file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED && vfs_does_file_have_this_extension(&folder[file_manager_folder_descriptor->selected_entry], "jpg png gif qoi bmp")==STATUS_TRUE) {
   add_button_to_bottom_line_from_left("[p] Preview", CLICK_ZONE_FILE_MANAGER_PREVIEW_WINDOW);
  }

  //draw other buttons if filesystem is read-write
  if(is_filesystem_read_write(connected_partitions[file_manager_folder_descriptor->partition_number].filesystem)==STATUS_TRUE) {
   //draw button for opening list of all other buttons
   add_button_to_bottom_line_from_left("More", CLICK_ZONE_FILE_MANAGER_MORE);
  }
  else {
   if(file_manager_folder_descriptor->selected_entry!=FOLDER_NO_ENTRY_SELECTED) {
    //draw button for showing file properties
    add_button_to_bottom_line_from_left("[F5] Properties", CLICK_ZONE_FILE_MANAGER_PROPERTIES);

    //draw button for copying files
    if(folder[file_manager_folder_descriptor->selected_entry].type==NORMAL_FILE) {
     add_button_to_bottom_line_from_left("[ctrl+c] Copy", CLICK_ZONE_FILE_MANAGER_COPY);
    }
   }

   //print to right bottom corner that this is read-only filesystem
   print("Read-only", screen_width-8-9*8, screen_height-13, BLACK);
  }
 }

 //draw connected partitions
 program_element_layout_skip_space_horizontally(FIRST_AREA, 2);
 for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
  if(connected_partitions[i].medium_type!=NO_MEDIUM) {
   //print type of device if needed
   if(i==0 || connected_partitions[i].medium_type!=connected_partitions[i-1].medium_type || connected_partitions[i].medium_number!=connected_partitions[i-1].medium_number) {
    program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, FILE_MANAGER_WIDTH_OF_CONNECTED_PARTITIONS, 20);
    if(connected_partitions[i].medium_type==MEDIUM_HARD_DISK) {
     print("Hard disk", element_x+8, element_y+6, BLACK);
    }
    else if(connected_partitions[i].medium_type==MEDIUM_OPTICAL_DRIVE) {
     print("Optical disk", element_x+8, element_y+6, BLACK);
    }
    else if(connected_partitions[i].medium_type==MEDIUM_USB_MSD) {
     print("USB flash drive", element_x+8, element_y+6, BLACK);
    }
    program_element_move_horizontally(FIRST_AREA, 0);
   }

   //if this is selected partition, draw background
   program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, FILE_MANAGER_WIDTH_OF_CONNECTED_PARTITIONS, 20);
   if((dword_t)file_manager_folder_descriptor!=0 && i==file_manager_folder_descriptor->partition_number) {
    draw_full_square(element_x, element_y, element_width, element_height, RED);
   }

   //print partition name
   draw_char(CHAR_COMMA_IN_MIDDLE_OF_LINE, element_x+8, element_y+6, BLACK);
   if(connected_partitions[i].partition_label[0]!=0) {
    for(dword_t j=0; j<11; j++) {
     if(connected_partitions[i].partition_label[j]==0) {
      break;
     }
     draw_char(connected_partitions[i].partition_label[j], element_x+24+j*8, element_y+6, BLACK);
    }
   }
   else {
    print("Partition", element_x+24, element_y+6, BLACK);
   }
   add_zone_to_click_board(element_x, element_y, element_width, element_height, CLICK_ZONE_FILE_MANAGER_FIRST_CONNECTED_PARTITION+i);
   program_element_move_horizontally(FIRST_AREA, 0);
  }
 }

 //add button for refreshing devices
 program_element_layout_skip_space_horizontally(FIRST_AREA, 10);
 add_button(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, FILE_MANAGER_WIDTH_OF_CONNECTED_PARTITIONS-10, "[F8] Refresh devices", CLICK_ZONE_FILE_MANAGER_REFRESH_DEVICES);

 //in these cases we do not need to draw anything else
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || (dword_t)file_manager_folder_descriptor==0 && file_manager_folder_descriptor->partition_number==NO_PARTITION_SELECTED) {
  redraw_screen();
  return;
 }

 //draw files in folder
 if((dword_t)file_manager_folder_descriptor!=0 && file_manager_folder_descriptor->partition_number!=NO_PARTITION_SELECTED) {
  struct file_descriptor_t *folder = vfs_get_folder_data_pointer(file_manager_folder_descriptor);

  //DRAW LIST
  if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_LIST) {
   program_element_layout_add_border_to_area(SECOND_AREA, 10);

   //calculate how many items will be printed
   file_manager_number_of_lines_on_screen = (program_element_layout_areas_info[SECOND_AREA].height/10);
   if(file_manager_number_of_lines_on_screen>vfs_get_folder_number_of_files(file_manager_folder_descriptor)) {
    file_manager_number_of_lines_on_screen = vfs_get_folder_number_of_files(file_manager_folder_descriptor);
   }

   //add scrollbar if needed
   if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)>(program_element_layout_areas_info[SECOND_AREA].height/10)) {
    //update variables
    file_manager_scrollbar_info.rider_position = get_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION);
    file_manager_scrollbar_info.size = (program_element_layout_areas_info[SECOND_AREA].height/10*10-2);
    program_element_layout_calculate_element_position(SECOND_AREA, ELEMENT_RIGHT_ALIGNMENT, SCROLLBAR_SIZE, file_manager_scrollbar_info.size);
    file_manager_scrollbar_info.x = element_x;
    file_manager_scrollbar_info.y = element_y;
    file_manager_scrollbar_info.full_document_size = vfs_get_folder_number_of_files(file_manager_folder_descriptor);
    file_manager_scrollbar_info.showed_document_size = file_manager_number_of_lines_on_screen;
    scrollbar_struct_calculate_rider_size((&file_manager_scrollbar_info));

    //draw scrollbar
    scrollbar_struct_draw_vertical_scrollbar((&file_manager_scrollbar_info), CLICK_ZONE_FILE_MANAGER_SCROLLBAR);

    //remove space reserved for scrollbar
    program_element_layout_areas_info[SECOND_AREA].width -= (SCROLLBAR_SIZE+5);
   }

   //print items
   if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)==0) {
    add_label(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, "There are no files in this folder");
   }
   for(dword_t i=file_manager_folder_descriptor->first_showed_entry; i<(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_lines_on_screen); i++) {
    //if this is selected item, draw background
    program_element_layout_calculate_element_position(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, program_element_layout_areas_info[SECOND_AREA].width, 10);
    if(i==file_manager_folder_descriptor->selected_entry) {
     draw_full_square(element_x, element_y-1, element_width, element_height, RED);
    }

    //add click zone
    add_zone_to_click_board(element_x, element_y, element_width, element_height, CLICK_ZONE_FILE_MANAGER_FIRST_ITEM+i);

    //print name
    program_element_layout_calculate_element_position(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, program_element_layout_areas_info[SECOND_AREA].width-19*8, 10);
    for(dword_t j=0; j<((program_element_layout_areas_info[SECOND_AREA].width-19*8)/8); j++) {
     if(folder[i].name[j]==0) {
      break;
     }
     draw_char(folder[i].name[j], element_x+j*8, element_y, BLACK);
    }
    program_element_move_vertically(SECOND_AREA, 0);

    //print size
    program_element_layout_calculate_element_position(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, 7*8, 10);
    if(folder[i].type==NORMAL_FILE) {
     if(folder[i].file_size_in_bytes<1024) {
      print_var(folder[i].file_size_in_bytes, element_x, element_y, BLACK);
      print("B", element_x+5*8, element_y, BLACK);
     }
     else if(folder[i].file_size_in_bytes<1024*1024) {
      print_var((folder[i].file_size_in_bytes/1024), element_x, element_y, BLACK);
      print("KB", element_x+5*8, element_y, BLACK);
     }
     else if(folder[i].file_size_in_bytes<1024*1024*1024) {
      print_var((folder[i].file_size_in_bytes/1024/1024), element_x, element_y, BLACK);
      print("MB", element_x+5*8, element_y, BLACK);
     }
     else {
      print_var((folder[i].file_size_in_bytes/1024/1024/1024), element_x, element_y, BLACK);
      print("GB", element_x+5*8, element_y, BLACK);
     }
    }
    program_element_move_vertically(SECOND_AREA, 16);

    //print date of creation
    if(vfs_does_file_have_this_extension((&folder[i]), "cdda")==STATUS_FALSE) {
     program_element_layout_calculate_element_position(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, 10*8, 10);
     print_var(folder[i].year_of_creation, element_x, element_y, BLACK);
     draw_char('/', element_x+4*8, element_y, BLACK);
     if(folder[i].month_of_creation<10) {
      draw_char('0', element_x+5*8, element_y, BLACK);
      print_var(folder[i].month_of_creation, element_x+6*8, element_y, BLACK);
     }
     else {
      print_var(folder[i].month_of_creation, element_x+5*8, element_y, BLACK);
     }
     draw_char('/', element_x+7*8, element_y, BLACK);
     if(folder[i].day_of_creation<10) {
      draw_char('0', element_x+8*8, element_y, BLACK);
      print_var(folder[i].day_of_creation, element_x+9*8, element_y, BLACK);
     }
     else {
      print_var(folder[i].day_of_creation, element_x+8*8, element_y, BLACK);
     }
    }

    //go to next item
    program_element_move_horizontally(SECOND_AREA, 0);
   }
  }
  //DRAW ICONS
  else if(file_manager_folder_descriptor->view_type==VIEW_FOLDER_ICONS) {
   //calculate how many items will be printed
   file_manager_number_of_columns_on_screen = ((program_element_layout_areas_info[SECOND_AREA].width-30)/(FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+16));
   file_manager_number_of_lines_on_screen = ((program_element_layout_areas_info[SECOND_AREA].height-20)/(FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+30+16));
   file_manager_number_of_items_on_screen = (file_manager_number_of_columns_on_screen*file_manager_number_of_lines_on_screen);
   if(file_manager_number_of_items_on_screen>(vfs_get_folder_number_of_files(file_manager_folder_descriptor)-file_manager_folder_descriptor->first_showed_entry)) {
    file_manager_number_of_items_on_screen = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)-file_manager_folder_descriptor->first_showed_entry);
   }

   //add scrollbar if needed
   if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)>file_manager_number_of_items_on_screen) {
    //update variables
    file_manager_scrollbar_info.rider_position = get_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION);
    file_manager_scrollbar_info.size = program_element_layout_areas_info[SECOND_AREA].height;
    file_manager_scrollbar_info.x = screen_width-SCROLLBAR_SIZE;
    file_manager_scrollbar_info.y = program_element_layout_areas_info[SECOND_AREA].actual_element_y;
    file_manager_scrollbar_info.full_document_size = (vfs_get_folder_number_of_files(file_manager_folder_descriptor)/file_manager_number_of_columns_on_screen);
    if((vfs_get_folder_number_of_files(file_manager_folder_descriptor)%file_manager_number_of_columns_on_screen)!=0) {
     file_manager_scrollbar_info.full_document_size++;
    }
    file_manager_scrollbar_info.showed_document_size = file_manager_number_of_lines_on_screen;
    scrollbar_struct_calculate_rider_size((&file_manager_scrollbar_info));

    //draw scrollbar
    scrollbar_struct_draw_vertical_scrollbar((&file_manager_scrollbar_info), CLICK_ZONE_FILE_MANAGER_SCROLLBAR);

    //remove space reserved for scrollbar
    program_element_layout_areas_info[SECOND_AREA].width -= (SCROLLBAR_SIZE+5);
   }

   //print items
   program_element_layout_add_border_to_area(SECOND_AREA, 10);
   if(vfs_get_folder_number_of_files(file_manager_folder_descriptor)==0) {
    add_label(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, "There are no files in this folder");
   }
   for(dword_t i=file_manager_folder_descriptor->first_showed_entry; i<(file_manager_folder_descriptor->first_showed_entry+file_manager_number_of_items_on_screen); i++)  {
    program_element_layout_calculate_element_position(SECOND_AREA, ELEMENT_LEFT_ALIGNMENT, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+30);

    //if selected, draw background
    if(i==file_manager_folder_descriptor->selected_entry) {
     draw_full_square(element_x-4, element_y-4, element_width+8, element_height+8, RED);
    }

    //add click zone
    add_zone_to_click_board(element_x, element_y, element_width, element_height, CLICK_ZONE_FILE_MANAGER_FIRST_ITEM+i);
    
    //draw icon
    dword_t color_of_icon = WHITE; //all unknown file formats are white
    if(folder[i].type==FILE_FOLDER) { //folders are yellow
     color_of_icon = 0xFFED00;
    }
    else if(vfs_does_file_have_this_extension((&folder[i]), "jpg png gif qoi bmp")) { //images are brown
     color_of_icon = 0x902D00;
    }
    else if(vfs_does_file_have_this_extension((&folder[i]), "mp3 wav cdda")) { //sounds are purple
     color_of_icon = 0xA000FF;
    }
    else if(vfs_does_file_have_this_extension((&folder[i]), "mp4 avi mov mod")) { //videos are grey
     color_of_icon = 0xA0A0A0;
    }
    else if(vfs_does_file_have_this_extension((&folder[i]), "doc docx odt rtf pdf")) { //documents are blue
     color_of_icon = 0x00A0FF;
    }
    else if(vfs_does_file_have_this_extension((&folder[i]), "xls xlsx ods")) { //spreadsheets are green
     color_of_icon = 0x33B600;
    }
    else if(vfs_does_file_have_this_extension((&folder[i]), "ppt pptx odp")) { //presentations are orange
     color_of_icon = 0xFF8000;
    }
    draw_full_square(element_x, element_y, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, color_of_icon);
    if(folder[i].type==FILE_FOLDER) {
     program_element_layout_calculate_floating_element_position(SECOND_AREA, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, 6*8, 8);
     print("folder", element_x, element_y, BLACK);
    }
    else if(get_number_of_chars_in_unicode_string(folder[i].extension)<=8) { //do not print extension longer than 8 characters
     program_element_layout_calculate_floating_element_position(SECOND_AREA, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, get_number_of_chars_in_unicode_string(folder[i].extension)*8, 8);
     print_unicode(folder[i].extension, element_x, element_y, BLACK);
    }

    //print name
    word_t name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE+1];
    if(get_number_of_chars_in_unicode_string(folder[i].name)<=FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE) {
     //print first line
     program_element_layout_calculate_floating_element_position(SECOND_AREA, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, 0, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_TOP_ALIGNMENT, get_number_of_chars_in_unicode_string(folder[i].name)*8, 8);
     print_unicode(folder[i].name, element_x, element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8, BLACK);
    }
    else if(get_number_of_chars_in_unicode_string(folder[i].name)<=FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE*2) {
     //print first line
     for(dword_t j=0; j<FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE; j++) {
      name_line[j] = folder[i].name[j];
     }
     name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE] = 0;
     print_unicode(name_line, program_element_layout_areas_info[SECOND_AREA].actual_element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8, BLACK);

     //print second line
     program_element_layout_calculate_floating_element_position(SECOND_AREA, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, 0, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_TOP_ALIGNMENT, get_number_of_chars_in_unicode_string(&folder[i].name[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE])*8, 8);
     print_unicode(&folder[i].name[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE], element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+10, BLACK);
    }
    else if(get_number_of_chars_in_unicode_string(folder[i].name)<=FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE*3) {
     //print first line
     for(dword_t j=0; j<FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE; j++) {
      name_line[j] = folder[i].name[j];
     }
     name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE] = 0;
     print_unicode(name_line, program_element_layout_areas_info[SECOND_AREA].actual_element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8, BLACK);
   
     //print second line
     for(dword_t j=0; j<FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE; j++) {
      name_line[j] = folder[i].name[j+FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE];
     }
     name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE] = 0;
     print_unicode(name_line, program_element_layout_areas_info[SECOND_AREA].actual_element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+10, BLACK);

     program_element_layout_calculate_floating_element_position(SECOND_AREA, FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION, 0, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_TOP_ALIGNMENT, get_number_of_chars_in_unicode_string(&folder[i].name[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE+FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE])*8, 8);
     print_unicode(&folder[i].name[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE+FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE], element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+10+10, BLACK);
    }
    else {
     //print first line
     for(dword_t j=0; j<FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE; j++) {
      name_line[j] = folder[i].name[j];
     }
     name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE] = 0;
     print_unicode(name_line, program_element_layout_areas_info[SECOND_AREA].actual_element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8, BLACK);

     //print second line
     for(dword_t j=0; j<FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE; j++) {
      name_line[j] = folder[i].name[j+FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE];
     }
     name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE] = 0;
     print_unicode(name_line, program_element_layout_areas_info[SECOND_AREA].actual_element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+10, BLACK);
    
     for(dword_t j=0; j<FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE; j++) {
      name_line[j] = folder[i].name[j+FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE+FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE];
     }
     name_line[FILE_MANAGER_NUMBER_OF_CHARS_IN_ICON_LINE] = 0;
     print_unicode(name_line, program_element_layout_areas_info[SECOND_AREA].actual_element_x, program_element_layout_areas_info[SECOND_AREA].actual_element_y+FILE_MANAGER_NUMBER_OF_PIXELS_IN_ICON_DIMENSION+8+10+10, BLACK);
    
    }

    //go to next item
    if((i%file_manager_number_of_columns_on_screen)==(file_manager_number_of_columns_on_screen-1)) { //new line
     program_element_move_horizontally(SECOND_AREA, 16);
    }
    else { //new column
     program_element_move_vertically(SECOND_AREA, 16);
    }
   }
  }
 }

 //redraw screen
 redraw_screen();
}

void file_manager_new_tab(void) {
 add_file((word_t *)"N\0e\0w\0\0\0", 0, 0, 0, 0, 0);
 set_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY, 0);
 set_file_value(FILE_MANAGER_FOLDER_PATH_NAMES, (dword_t) calloc(20*2*(MAX_NUMBER_OF_FOLDERS_IN_PATH+2)));
 set_file_value(FILE_MANAGER_SCROLLBAR_RIDER_POSITION, 0);

 word_t *path_names = (word_t *) (get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES));
 path_names[0] = 'R';
 path_names[1] = 'o';
 path_names[2] = 'o';
 path_names[3] = 't';
}

void file_manager_close_tab(void) {
 if(get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY)!=0) {
  vfs_destroy_folder_path_structure((struct folder_descriptor_t *)get_file_value(FILE_MANAGER_FOLDER_STRUCTURE_MEMORY));
 }
 free((void *)get_file_value(FILE_MANAGER_FOLDER_PATH_NAMES));
}