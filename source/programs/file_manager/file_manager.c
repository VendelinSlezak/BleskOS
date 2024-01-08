//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_file_manager(void) {
 file_manager_program_interface_memory = create_program_interface_memory();
 file_manager_number_of_files_on_screen = ((graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH-24-8)/10);
 file_manager_number_of_chars_of_file_name = ((graphic_screen_x-FILE_MANAGER_DEVICE_LIST_WIDTH-8-20)/8);
 file_manager_scrollbar_height = (file_manager_number_of_files_on_screen*10);
 file_manager_copy_entry_memory = calloc(256);
 file_manager_copied_file_memory = 0;
 file_manager_file_copied = STATUS_FALSE;
 file_manager_rename_text_area = create_text_area(TEXT_AREA_INPUT_LINE, 220, graphic_screen_x_center-120, graphic_screen_y_center-5, 240, 10);
 file_manager_rename_window_showed = STATUS_FALSE;
}

void file_manager(void) {
 set_program_interface(file_manager_program_interface_memory, ((dword_t)&draw_file_manager));
 set_program_value(PROGRAM_INTERFACE_FLAGS, PROGRAM_INTERFACE_FLAG_NO_OPEN_AND_SAVE_BUTTON);
 program_interface_add_keyboard_event(KEY_F3, (dword_t)file_manager_new_folder);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)file_manager_close_folder);
 program_interface_add_keyboard_event(KEY_F8, (dword_t)file_manager_key_f8_event);
 program_interface_add_keyboard_event(KEY_F9, (dword_t)file_manager_rename_file);
 program_interface_add_keyboard_event(KEY_F10, (dword_t)file_manager_delete_file);
 program_interface_add_keyboard_event(KEY_ENTER, (dword_t)file_manager_key_enter_event);
 program_interface_add_keyboard_event(KEY_B, (dword_t)file_manager_key_b_event);
 program_interface_add_keyboard_event(KEY_C, (dword_t)file_manager_key_c_event);
 program_interface_add_keyboard_event(KEY_V, (dword_t)file_manager_key_v_event);
 program_interface_add_keyboard_event(KEY_UP, (dword_t)file_manager_key_up_event);
 program_interface_add_keyboard_event(KEY_DOWN, (dword_t)file_manager_key_down_event);
 program_interface_add_click_zone_event(FILE_MANAGER_CLICK_ZONE_BUTTON_BACK, (dword_t)file_manager_key_b_event);
 program_interface_add_click_zone_event(FILE_MANAGER_CLICK_ZONE_BUTTON_REFRESH_DEVICES, (dword_t)file_manager_key_f8_event);
 program_interface_add_click_zone_event(FILE_MANAGER_CLICK_ZONE_BUTTON_COPY, (dword_t)file_manager_copy_file);
 program_interface_add_click_zone_event(FILE_MANAGER_CLICK_ZONE_BUTTON_PASTE, (dword_t)file_manager_paste_file);
 program_interface_add_click_zone_event(FILE_MANAGER_CLICK_ZONE_BUTTON_RENAME, (dword_t)file_manager_rename_file);
 program_interface_add_click_zone_event(FILE_MANAGER_CLICK_ZONE_BUTTON_DELETE, (dword_t)file_manager_delete_file);
 program_interface_redraw();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //close program
  dword_t click_zone = get_mouse_cursor_click_board_value();
  if(keyboard_value==KEY_ESC || (mouse_drag_and_drop==MOUSE_CLICK && click_zone==CLICK_ZONE_BACK)) {
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //redraw when there is device change
  if(usb_new_device_detected==STATUS_TRUE) {
   program_interface_redraw();
   continue;
  }

  //process click on device
  if(mouse_drag_and_drop==MOUSE_CLICK && click_zone!=FILE_MANAGER_CLICK_ZONE_SCROLLBAR) {
   if(click_zone>=FILE_MANAGER_CLICK_ZONE_DEVICE_ENTRY && click_zone<=FILE_MANAGER_CLICK_ZONE_DEVICE_LAST_ENTRY) {
    device_list_selected_entry = (click_zone-FILE_MANAGER_CLICK_ZONE_DEVICE_ENTRY);

    //test if this entry still exist
    if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==0) {
     program_interface_redraw();
     continue;
    }

    //entry exist, read root folder
    select_device_list_entry(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR));
     
    //read root directory
    file_manager_print_message("Reading root directory...");
    dword_t root_folder_memory = read_folder(ROOT_DIRECTORY, 0);
    if(root_folder_memory==STATUS_ERROR) {
     error_window("Error during reading root folder of this device");
     program_interface_redraw();
     continue;
    }

    if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) { //create new entry
     //create entry
     add_file((word_t *)"R\0o\0o\0t\0\0\0", 0, 0, 0, 0, 0);
     set_file_value(FILE_MANAGER_FILE_PATH_MEMORY, malloc(FILE_MANAGER_MAX_PATH_FOLDERS*8));
     set_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER, 0);
     set_file_value(FILE_MANAGER_FILE_PATH_FOLDER_NAMES, calloc(FILE_MANAGER_MAX_PATH_FOLDERS*20));
     set_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY, root_folder_memory);
     set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, 0);
     set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
     file_manager_compute_number_of_files_in_directory();
    }
    else { //go back to root directory in actual entry
     set_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER, 0);
     free(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY));
     set_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY, root_folder_memory);
     set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, 0);
     set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
     file_manager_compute_number_of_files_in_directory();
     set_char_of_file_name(0, 'R');
     set_char_of_file_name(1, 'o');
     set_char_of_file_name(2, 'o');
     set_char_of_file_name(3, 't');
     set_char_of_file_name(4, 0);
    }
    set_file_value(FILE_MANAGER_FILE_DEVICE_TYPE, get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE));
    set_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER, get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER));
    set_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE, get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE));
    set_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR, get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR));

    //show directory on screen
    program_interface_redraw();
    continue;
   }

   //select file
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
    if(click_zone>=FILE_MANAGER_CLICK_ZONE_FIRST_FILE) {
     dword_t selected_entry = (get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)+(click_zone-FILE_MANAGER_CLICK_ZONE_FIRST_FILE));

     //double click
     if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)==selected_entry) {
      file_manager_key_enter_event();
      continue;
     }

     //first click
     set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, selected_entry);
     program_interface_redraw();
    }
    else if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=NO_FILE_SELECTED) {
     //unselect file
     set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
     program_interface_redraw();
    }
   }
  }
 }
}

void draw_file_manager(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("File manager", "", 0x803300, 0xFF6600);

 //draw connected devices
 draw_full_square(0, 21, FILE_MANAGER_DEVICE_LIST_WIDTH, graphic_screen_y-41, 0xB04100);
 dword_t connected_devices_draw_line = (PROGRAM_INTERFACE_TOP_LINE_HEIGTH+2), last_device_type = 0, last_device_number = 0;
 for(device_list_selected_entry=0; device_list_selected_entry<DEVICE_LIST_NUMBER_OF_ENTRIES; device_list_selected_entry++) {
  if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==0) {
   break; //end of device list
  }
  
  //if this is new device, print it's name
  if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)!=last_device_type || get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)!=last_device_number) {
   dword_t device_type = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE);
   if(device_type==MEDIUM_HDD) {
    print("Hard disk", 8, connected_devices_draw_line+6, BLACK);
   }
   else if(device_type==MEDIUM_CDROM) {
    print("Optical drive", 8, connected_devices_draw_line+6, BLACK);
   }
   else if(device_type==MEDIUM_USB_MSD) {
    print("USB flash drive", 8, connected_devices_draw_line+6, BLACK);
   }
   connected_devices_draw_line += 20;
  }

  //if this is selected entry, draw background
  if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE) && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)==get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER) && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE) && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR)==get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR)) {
   draw_full_square(0, connected_devices_draw_line, FILE_MANAGER_DEVICE_LIST_WIDTH, 20, RED);
  }

  //print parition name
  add_zone_to_click_board(0, connected_devices_draw_line, FILE_MANAGER_DEVICE_LIST_WIDTH, 20, FILE_MANAGER_CLICK_ZONE_DEVICE_ENTRY+device_list_selected_entry);
  draw_char(CHAR_COMMA_IN_MIDDLE_OF_LINE, 8, connected_devices_draw_line+6, BLACK);
  if(get_device_list_entry_name_value(0)!=0) {
   for(dword_t i=0; i<11; i++) {
    if(get_device_list_entry_name_value(i)==0) {
     break;
    }
    draw_char(get_device_list_entry_name_value(i), 24+i*8, connected_devices_draw_line+6, BLACK);
   }
  }
  else {
   print("Partition", 24, connected_devices_draw_line+6, BLACK);
  }
  connected_devices_draw_line += 20;
 }

 //button "Refresh devices"
 if(connected_devices_draw_line!=PROGRAM_INTERFACE_TOP_LINE_HEIGTH) {
  connected_devices_draw_line+=10; //add some space if there are some entries
 }
 draw_button_with_click_zone("[F8] Refresh devices", 5, connected_devices_draw_line, FILE_MANAGER_DEVICE_LIST_WIDTH-10, 20, FILE_MANAGER_CLICK_ZONE_BUTTON_REFRESH_DEVICES);

 //no connected device
 device_list_selected_entry = 0;
 if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==0) {
  print("There is no storage device with known filesystem connected", FILE_MANAGER_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  return;
 }
 
 //device disconnected
 if(does_device_list_entry_exist(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_FALSE) {
  print("This device was removed", FILE_MANAGER_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  return;
 }

 //draw folder items
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)!=0) {
  dword_t folder_first_showed_entry_offset = (get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)+(get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)*256));
  byte_t *folder8 = (byte_t *) (folder_first_showed_entry_offset);
  word_t *folder16 = (word_t *) (folder_first_showed_entry_offset);
  dword_t *folder32 = (dword_t *) (folder_first_showed_entry_offset);

  //no files in folder
  if(get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)==0) {
   print("There are no files in this directory", FILE_MANAGER_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
   return;
  }
  
  //draw up strings
  print("Name", FILE_MANAGER_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  print("Size", (graphic_screen_x-12-72-80-8), PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  print("Date", (graphic_screen_x-12-80-8), PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);

  //draw selected file background
  if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=NO_FILE_SELECTED && get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)>=get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY) && get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)<(get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)+file_manager_number_of_files_on_screen)) {
   draw_full_square(FILE_MANAGER_DEVICE_LIST_WIDTH+8, (PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)-get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY))*10), (graphic_screen_x-(FILE_MANAGER_DEVICE_LIST_WIDTH+8)-20), 10, RED);
  }

  //print file entries
  for(int i=0, draw_line = PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+1; i<file_manager_number_of_files_on_screen; i++) {
   //no entry
   if(folder32[(64*i)]==0) {
    break;
   }

   //add click zone
   add_zone_to_click_board(FILE_MANAGER_DEVICE_LIST_WIDTH+8, draw_line-1, (graphic_screen_x-FILE_MANAGER_DEVICE_LIST_WIDTH-8-20), 10, FILE_MANAGER_CLICK_ZONE_FIRST_FILE+i);
   
   //print name
   for(int j=0, draw_column = FILE_MANAGER_DEVICE_LIST_WIDTH+8; j<file_manager_number_of_chars_of_file_name; j++) {
    if(folder16[(128*i)+16+j]==0) {
     break; //end of name
    }
    draw_char(folder16[(128*i)+16+j], draw_column, draw_line, BLACK);
    draw_column += 8;
   }
   
   //print size
   if(folder8[(256*i)+11]!=0x10) { //do not write size of folders
    dword_t file_size = folder32[(64*i)+7];

    if(file_size<1024) {
     print_var(file_size, (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("B", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
    else if(file_size<1024*1024) {
     print_var((file_size/1024), (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("KB", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
    else if(file_size<1024*1024*1024) {
     print_var((file_size/1024/1024), (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("MB", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
    else {
     print_var((file_size/1024/1024/1024), (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("GB", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
   }
   
   //print date
   dword_t file_date = folder16[(128*i)+8];
   print_var((file_date >> 9)+1980, (graphic_screen_x-12-80-8), draw_line, BLACK);
   draw_char('/', (graphic_screen_x-12-48-8), draw_line, BLACK);
   if(((file_date >> 5) & 0xF)<10) {
    draw_char('0', (graphic_screen_x-12-40-8), draw_line, BLACK);
    print_var(((file_date >> 5) & 0xF), (graphic_screen_x-12-32-8), draw_line, BLACK);
   }
   else {
    print_var(((file_date >> 5) & 0xF), (graphic_screen_x-12-40-8), draw_line, BLACK);
   }
   draw_char('/', (graphic_screen_x-12-24-8), draw_line, BLACK);
   if((file_date & 0x1F)<10) {
    draw_char('0', (graphic_screen_x-12-16-8), draw_line, BLACK);
    print_var((file_date & 0x1F), (graphic_screen_x-12-8-8), draw_line, BLACK);
   }
   else {
    print_var((file_date & 0x1F), (graphic_screen_x-12-16-8), draw_line, BLACK);
   }
   
   draw_line+=10;
  }

  //draw scrollbar
  if(get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE)!=0) {
   draw_vertical_scrollbar(graphic_screen_x-15, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, file_manager_scrollbar_height, get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE));
   program_interface_add_vertical_scrollbar(FILE_MANAGER_CLICK_ZONE_SCROLLBAR, ((dword_t)(&file_manager_scrollbar_height)), get_position_of_file_memory()+FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION*4, get_position_of_file_memory()+FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE*4, ((dword_t)&file_manager_verticall_scrollbar_event));
   add_zone_to_click_board(graphic_screen_x-15, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, 10, file_manager_scrollbar_height, FILE_MANAGER_CLICK_ZONE_SCROLLBAR);
  }

  //draw back button
  if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)!=0) {
   draw_button_with_click_zone("[b] Go back", 5, graphic_screen_y-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH-24, FILE_MANAGER_DEVICE_LIST_WIDTH-10, 24, FILE_MANAGER_CLICK_ZONE_BUTTON_BACK);
  }

  //draw bottom line buttons
  set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, 0);
  if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=NO_FILE_SELECTED && get_file_attribute(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY))!=ATTRIBUTE_DIRECTORY) {
   draw_bottom_line_button("[ctrl+c] Copy", FILE_MANAGER_CLICK_ZONE_BUTTON_COPY);
  }
  if(is_filesystem_read_write(get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE))==STATUS_TRUE) {
   if(file_manager_file_copied==STATUS_TRUE) {
    draw_bottom_line_button("[ctrl+v] Paste", FILE_MANAGER_CLICK_ZONE_BUTTON_PASTE);
   }
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=NO_FILE_SELECTED) {
    draw_bottom_line_button("[F9] Rename", FILE_MANAGER_CLICK_ZONE_BUTTON_RENAME);
    draw_bottom_line_button("[F10] Delete", FILE_MANAGER_CLICK_ZONE_BUTTON_DELETE);
   }
  }
 }
 
}

void file_manager_new_folder(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE) {
  add_file((word_t *)"N\0e\0w\0\0\0", 0, 0, 0, 0, 0);
  set_file_value(FILE_MANAGER_FILE_DEVICE_TYPE, 0);
  set_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER, 0);
  set_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE, 0);
  set_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR, 0);
  set_file_value(FILE_MANAGER_FILE_PATH_MEMORY, malloc(FILE_MANAGER_MAX_PATH_FOLDERS*8));
  set_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER, 0);
  set_file_value(FILE_MANAGER_FILE_PATH_FOLDER_NAMES, calloc(FILE_MANAGER_MAX_PATH_FOLDERS*20));
  set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, 0);
  set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, 0);
  set_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY, 0);
  set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, 0);
  set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
  set_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES, NO_FILE_SELECTED);
 }
}

void file_manager_close_folder(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE) {
  free(get_file_value(FILE_MANAGER_FILE_PATH_MEMORY));
  free(get_file_value(FILE_MANAGER_FILE_PATH_FOLDER_NAMES));
  if(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)!=0) {
   free(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY));
  }
 }
}

void file_manager_key_f8_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE) {
  device_list_check_optical_drive();
  program_interface_redraw();
 }
}

void file_manager_key_enter_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=NO_FILE_SELECTED && get_char_of_file_from_folder_entry_name(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY), 0)!='.') {
  if(get_file_attribute(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY))==0x10) { //folder
   if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)>=FILE_MANAGER_MAX_PATH_FOLDERS) { //too many folders
    error_window("You can not open more than 50 folders");
    program_interface_redraw();
    return;
   }

   //select medium
   if(select_device_list_entry(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_ERROR) {
    file_manager_print_message("Device is not inserted");
    return;
   }

   //read folder
   file_manager_print_message("Reading directory...");
   dword_t folder_starting_entry = get_file_starting_entry(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
   dword_t folder_size = get_file_size(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
   dword_t folder_memory = read_folder(folder_starting_entry, folder_size);
   if(folder_memory==STATUS_ERROR) {
    error_window("Error during reading folder");
    program_interface_redraw();
    return;
   }

   //update file values
   set_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER, get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)+1);
   word_t *folder_name = (word_t *) (get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)+(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)*256)+32);
   word_t *path_folder_name = (word_t *) (get_file_value(FILE_MANAGER_FILE_PATH_FOLDER_NAMES)+get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*20);
   for(dword_t i=0; i<20; i++) {
    path_folder_name[i] = folder_name[i];
    set_char_of_file_name(i, folder_name[i]);
   }
   path_folder_name[19] = 0;
   set_char_of_file_name(19, 0);
   dword_t *folder_path = (dword_t *) (get_file_value(FILE_MANAGER_FILE_PATH_MEMORY)+(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*8));
   folder_path[0] = folder_starting_entry;
   folder_path[1] = folder_size;
   free(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY));
   set_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY, folder_memory);
   set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, 0);
   set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
   file_manager_compute_number_of_files_in_directory();
   
   //show directory on screen
   program_interface_redraw();
   return;
  }
  else { //file
   error_window("This version can not open files directly from File Manager");
   program_interface_redraw();
   return;
  }
 }
}

void file_manager_key_b_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)!=0) {
  //select medium
  if(select_device_list_entry(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_ERROR) {
   file_manager_print_message("Device is not inserted");
   return;
  }
  
  //read folder
  file_manager_print_message("Reading directory...");
  set_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER, get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)-1);
  dword_t *folder_path = (dword_t *) (get_file_value(FILE_MANAGER_FILE_PATH_MEMORY)+(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*8));
  dword_t folder_starting_entry = folder_path[0];
  dword_t folder_size = folder_path[1];
  if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)==0) {
   folder_starting_entry = ROOT_DIRECTORY;
  }
  dword_t folder_memory = read_folder(folder_starting_entry, folder_size);
  if(folder_memory==STATUS_ERROR) {
   error_window("Error during reading folder");
   set_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER, get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)+1); //previous value
   program_interface_redraw();
   return;
  }

  //update file values
  if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)!=0) {
   word_t *path_folder_name = (word_t *) (get_file_value(FILE_MANAGER_FILE_PATH_FOLDER_NAMES)+get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*20);
   for(dword_t i=0; i<20; i++) {
    set_char_of_file_name(i, path_folder_name[i]);
   }
   set_char_of_file_name(19, 0);
  }
  else {
   set_char_of_file_name(0, 'R');
   set_char_of_file_name(1, 'o');
   set_char_of_file_name(2, 'o');
   set_char_of_file_name(3, 't');
   set_char_of_file_name(4, 0);
  }
  free(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY));
  set_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY, folder_memory);
  set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, 0);
  set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
  file_manager_compute_number_of_files_in_directory();

  //show directory on screen
  program_interface_redraw();
 }
}

void file_manager_key_c_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=NO_FILE_SELECTED && (keyboard_control_keys & KEYBOARD_CTRL)==KEYBOARD_CTRL) {
  file_manager_copy_file();
 }
}

void file_manager_key_v_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && file_manager_file_copied!=STATUS_FALSE && (keyboard_control_keys & KEYBOARD_CTRL)==KEYBOARD_CTRL) {
  file_manager_paste_file();
 }
}

void file_manager_key_up_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)!=0) {
  if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)==NO_FILE_SELECTED) {
   set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)-1);
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)>(get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)+file_manager_number_of_files_on_screen-1)) {
    set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, (get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)-file_manager_number_of_files_on_screen+1));
   }
  }
  else {
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)!=0) {
    set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)-1);
   }
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)<get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)) {
    set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
   }
  }

  //recalculate scrollbar
  if(get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE)!=0) {
   set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(file_manager_scrollbar_height, get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES), file_manager_number_of_files_on_screen, get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)));
  }
  
  //redraw screen
  program_interface_redraw();
 }
}

void file_manager_key_down_event(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)!=0) {
  if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)==NO_FILE_SELECTED) {
   set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, 0);
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)<get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)) {
    set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
   }
  }
  else {
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)<(get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)-1)) {
    set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)+1);
   }
   if(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)>(get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)+file_manager_number_of_files_on_screen-1)) {
    set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, (get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)-file_manager_number_of_files_on_screen+1));
   }
  }

  //recalculate scrollbar
  if(get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE)!=0) {
   set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(file_manager_scrollbar_height, get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES), file_manager_number_of_files_on_screen, get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)));
  }

  //redraw screen
  program_interface_redraw();
 }
}

void file_manager_verticall_scrollbar_event(void) {
 set_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY, get_scrollbar_rider_value(file_manager_scrollbar_height, get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES), file_manager_number_of_files_on_screen));
 program_interface_redraw();
 program_interface_selected_element_number = 0; //select scrollbar
}

void file_manager_print_message(byte_t *message) {
 draw_full_square(FILE_MANAGER_DEVICE_LIST_WIDTH, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, (graphic_screen_x-FILE_MANAGER_DEVICE_LIST_WIDTH), (graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH), 0xFF6600);
 print(message, FILE_MANAGER_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
 redraw_part_of_screen(FILE_MANAGER_DEVICE_LIST_WIDTH, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, (graphic_screen_x-FILE_MANAGER_DEVICE_LIST_WIDTH), (graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH));
}

void file_manager_compute_number_of_files_in_directory(void) {
 dword_t *folder = (dword_t *) (get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY));
 dword_t number_of_files = 0;
 while(*folder!=0) {
  number_of_files++;
  folder+=64;
 }
 set_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES, number_of_files);
 if(number_of_files!=0) {
  set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, calculate_scrollbar_rider_size(file_manager_scrollbar_height, get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES), file_manager_number_of_files_on_screen));
  if(get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE)!=0) {
   set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(file_manager_scrollbar_height, get_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES), file_manager_number_of_files_on_screen, get_file_value(FILE_MANAGER_FILE_FOLDER_FIRST_SHOWED_ENTRY)));
  }
 }
 else {
  set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, 0);
  set_file_value(FILE_MANAGER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, 0);
 } 
}

void file_manager_copy_file(void) {
 //do not copy folders
 if(get_file_attribute(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY))==ATTRIBUTE_DIRECTORY) { //folder
  error_window("This version can not copy folders");
  program_interface_redraw();
  return;
 }

 //free previous file
 if(file_manager_copied_file_memory!=0) {
  free(file_manager_copied_file_memory);
  file_manager_file_copied = STATUS_FALSE;
 }

 //select medium
 if(select_device_list_entry(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_ERROR) {
  file_manager_print_message("Device is not inserted");
  return;
 }

 //read file to memory
 file_manager_print_message("Reading file...");
 file_manager_copied_file_size = get_file_size(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
 file_show_file_work_progress = STATUS_TRUE;
 file_manager_copied_file_memory = read_file(get_file_starting_entry(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)), file_manager_copied_file_size);
 file_show_file_work_progress = STATUS_FALSE;
 if(file_manager_copied_file_memory==STATUS_ERROR) {
  error_window("Error during reading file");
  program_interface_redraw();
  return;
 }

 //save file entry
 copy_memory(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)+(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)*256), file_manager_copy_entry_memory, 256);
 file_manager_file_copied = STATUS_TRUE;
 program_interface_redraw();
}

void file_manager_paste_file(void) {
 if(is_filesystem_read_write(get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE))==STATUS_FALSE || file_manager_file_copied==STATUS_FALSE) {
  return;
 }

 //select medium
 if(select_device_list_entry(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_ERROR) {
  file_manager_print_message("Device is not inserted");
  return;
 }

 //write file
 file_manager_print_message("Writing file...");
 dword_t file_pointer = create_file(file_manager_copied_file_memory, file_manager_copied_file_size);
 if(file_pointer==STATUS_ERROR) {
  error_window("Error during writing file");
  program_interface_redraw();
  return;
 }

 //create new folder entry
 word_t *entry = (word_t *) (file_manager_copy_entry_memory);
 entry[10] = (file_pointer>>16);
 entry[13] = (file_pointer & 0xFFFF);
 dword_t folder_memory = get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY);
 folder_memory = realloc(folder_memory, (get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)+2)*256);
 set_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY, folder_memory);
 copy_memory(file_manager_copy_entry_memory, folder_memory+(get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)*256), 256);

 //save folder
 file_manager_print_message("Saving changes to folder...");
 dword_t *folder_path = (dword_t *) (get_file_value(FILE_MANAGER_FILE_PATH_MEMORY)+(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*8));
 dword_t folder_starting_entry = folder_path[0];
 if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)==0) {
  folder_starting_entry = ROOT_DIRECTORY;
 }
 if(rewrite_folder(folder_starting_entry, folder_memory)==STATUS_ERROR) {
  error_window("Error during rewriting folder");
  program_interface_redraw();
  return;
 }

 //successfull copy
 program_interface_redraw();
}

void file_manager_rename_file(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)==NO_FILE_SELECTED || is_filesystem_read_write(get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE))==STATUS_FALSE) {
  return;
 }

 //copy entry name
 dword_t *text_area_info = (dword_t *) (file_manager_rename_text_area);
 word_t *text_area_data = (word_t *) (text_area_info[0]);
 for(dword_t i=0; i<220; i++) {
  text_area_data[i] = get_char_of_file_from_folder_entry_name(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY), i);
 }

 //show message window
 clear_program_interface_before_drawing();
 

 draw_message_window(270, 95);

 print("Write new name of file:", graphic_screen_x_center-120, graphic_screen_y_center-25, BLACK);

 text_area_disable_cursor(file_manager_rename_text_area);
 draw_text_area(file_manager_rename_text_area);
 program_interface_add_text_area(FILE_MANAGER_CLICK_ZONE_RENAME_TEXT_AREA, file_manager_rename_text_area);
 add_zone_to_click_board(graphic_screen_x_center-120, graphic_screen_y_center-5, 240, 10, FILE_MANAGER_CLICK_ZONE_RENAME_TEXT_AREA);

 draw_button("Cancel", graphic_screen_x_center-100, graphic_screen_y_center+15, 90, 20);
 draw_button("Rename", graphic_screen_x_center+10, graphic_screen_y_center+15, 90, 20);

 file_manager_rename_window_showed = STATUS_TRUE;
 redraw_screen();

 //rename window
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //do not rename
  if(keyboard_value==KEY_ESC || (mouse_drag_and_drop==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y_center+15, graphic_screen_y_center+35, graphic_screen_x_center-100, graphic_screen_x_center-10)==STATUS_TRUE)) {
   file_manager_rename_window_showed = STATUS_FALSE;
   program_interface_redraw();
   return;
  }

  //rename
  if(keyboard_value==KEY_ENTER || (mouse_drag_and_drop==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y_center+15, graphic_screen_y_center+35, graphic_screen_x_center+10, graphic_screen_x_center+100)==STATUS_TRUE)) {
   file_manager_rename_window_showed = STATUS_FALSE;
   if(text_area_data[0]==0 || text_area_data[0]=='.') { //TODO: more invalid characters
    error_window("Invalid file name");
    program_interface_redraw();
    return;
   }
   break;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();
 }

 //select medium
 if(select_device_list_entry(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_ERROR) {
  file_manager_print_message("Device is not inserted");
  return;
 }

 //copy new name to entry
 for(dword_t i=0; i<220; i++) {
  set_char_of_file_from_folder_entry_name(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY), i, text_area_data[i]);
  if(text_area_data[i]==0) {
   break;
  }
 }

 //save folder
 file_manager_print_message("Saving changes to folder...");
 dword_t *folder_path = (dword_t *) (get_file_value(FILE_MANAGER_FILE_PATH_MEMORY)+(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*8));
 dword_t folder_starting_entry = folder_path[0];
 if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)==0) {
  folder_starting_entry = ROOT_DIRECTORY;
 }
 if(rewrite_folder(folder_starting_entry, get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY))==STATUS_ERROR) {
  error_window("Error during rewriting folder");
  program_interface_redraw();
  return;
 }

 //successful rename
 program_interface_redraw();
}

void file_manager_delete_file(void) {
 if(file_manager_rename_window_showed==STATUS_FALSE && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0 || get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)==NO_FILE_SELECTED || is_filesystem_read_write(get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE))==STATUS_FALSE) {
  return;
 }
 if(dialog_yes_no("Do you really want to delete this file?")==STATUS_FALSE) {
  program_interface_redraw();
  return;
 }

 //select medium
 if(select_device_list_entry(get_file_value(FILE_MANAGER_FILE_DEVICE_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_NUMBER), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_TYPE), get_file_value(FILE_MANAGER_FILE_DEVICE_PARTITION_FIRST_SECTOR))==STATUS_ERROR) {
  file_manager_print_message("Device is not inserted");
  return;
 }

 //if this is folder, delete files inside of folder
 if(get_file_attribute(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY))==ATTRIBUTE_DIRECTORY) {
  //read folder
  file_manager_print_message("Reading folder content...");
  dword_t folder_starting_entry = get_file_starting_entry(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
  dword_t folder_size = get_file_size(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY));
  dword_t folder_memory = read_folder(folder_starting_entry, folder_size);
  if(folder_memory==STATUS_ERROR) {
   error_window("Error during reading folder content");
   program_interface_redraw();
   return;
  }
  
  //count number of files in directory
  dword_t *folder = (dword_t *) (folder_memory);
  dword_t number_of_files = 0;
  while(*folder!=0) {
   number_of_files++;
   folder+=64;
  }

  //check if there are not folders inside folder
  for(dword_t i=0; i<number_of_files; i++) {
   if(get_file_attribute(folder_memory, i)==ATTRIBUTE_DIRECTORY && get_char_of_file_from_folder_entry_name(folder_memory, i, 0)!='.') {
    error_window("This version can not delete folder with folders inside");
    free(folder_memory);
    program_interface_redraw();
    return;
   }
  }

  //delete files inside folder
  file_manager_print_message("Deleting folder content...");
  for(dword_t i=0; i<number_of_files; i++) {
   if(get_char_of_file_from_folder_entry_name(folder_memory, i, 0)=='.') { //skip first two entries that represents folder itself and folder above
    continue;
   }

   if(delete_file(get_file_starting_entry(folder_memory, i))==STATUS_ERROR) {
    error_window("Error during deleting content of folder");
    free(folder_memory);
    program_interface_redraw();
    return;
   }
  }

  //free folder memory
  free(folder_memory);
 }

 //delete file
 file_manager_print_message("Deleting file...");
 if(delete_file(get_file_starting_entry(get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY), get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)))==STATUS_ERROR) {
  error_window("Error during deleting content of file");
  program_interface_redraw();
  return;
 }

 //delete file entry from folder
 dword_t entry_offset = (get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)+(get_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY)*256));
 copy_memory(entry_offset+256, entry_offset, (get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)+(get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)*256))-entry_offset-256);
 clear_memory((get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY)+(get_file_value(FILE_MANAGER_FILE_FOLDER_NUMBER_OF_ENTRIES)*256))-256, 256);
 
 //save folder
 file_manager_print_message("Saving changes to folder...");
 dword_t *folder_path = (dword_t *) (get_file_value(FILE_MANAGER_FILE_PATH_MEMORY)+(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)*8));
 dword_t folder_starting_entry = folder_path[0];
 if(get_file_value(FILE_MANAGER_FILE_PATH_ACTUAL_FOLDER_NUMBER)==0) {
  folder_starting_entry = ROOT_DIRECTORY;
 }
 if(rewrite_folder(folder_starting_entry, get_file_value(FILE_MANAGER_FILE_FOLDER_MEMORY))==STATUS_ERROR) {
  error_window("Error during rewriting folder");
  set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
  program_interface_redraw();
  return;
 }

 //successfull deletion
 set_file_value(FILE_MANAGER_FILE_FOLDER_SELECTED_ENTRY, NO_FILE_SELECTED);
 program_interface_redraw();
}