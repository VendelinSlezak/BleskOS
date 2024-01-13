//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_file_dialog(void) {
 file_dialog_selected_device_entry = 0xFFFFFFFF;
 file_dialog_folder_device_type = 0;
 file_dialog_folder_device_number = 0;
 file_dialog_folder_device_partition_type = 0;
 file_dialog_folder_device_partition_first_sector = 0;
 file_dialog_folder_path_memory = malloc(FILE_DIALOG_MAX_PATH_FOLDERS*4);
 file_dialog_path_folder_names_memory = calloc(FILE_DIALOG_MAX_PATH_FOLDERS*50*2);
 file_dialog_folder_memory = 0;
 file_dialog_folder_vertical_scrollbar_rider_size = 0;
 file_dialog_array_of_allowed_extension_mem = malloc((FILE_DIALOG_MAX_FILE_EXTENSION_LENGTH+1)*FILE_DIALOG_MAX_OF_ALLOWED_EXTENSIONS);

 file_dialog_number_of_files_on_screen = ((graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH-24-8)/10);
 file_dialog_number_of_chars_of_file_name = ((graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8-20)/8);
 file_dialog_scrollbar_height = (file_dialog_number_of_files_on_screen*10);

 file_dialog_new_name_text_area = create_text_area(TEXT_AREA_INPUT_LINE, 29, graphic_screen_x_center-120, graphic_screen_y_center-5, 240, 10);
}

void redraw_file_dialog(byte_t dialog_type) {
 //clear click board
 clear_click_board();

 //draw background
 clear_screen(0xFF6600);

 //draw borders
 global_color = BLACK;
 draw_full_square(0, 0, graphic_screen_x, 20, 0x00C000);
 draw_straigth_line(0, 20, graphic_screen_x);
 print("File dialog", 8, 6, BLACK);
 draw_full_square(0, graphic_screen_y-19, graphic_screen_x, 19, 0x00C000);
 draw_straigth_line(0, graphic_screen_y-20, graphic_screen_x);
 if(dialog_type==FILE_DIALOG_TYPE_SAVE) {
  dword_t number_of_chars_in_extension = get_number_of_chars_in_unicode_string(file_dialog_file_extension);
  print("Save file with extension:", graphic_screen_x-8-(number_of_chars_in_extension*8)-8-(25*8), 6, BLACK);
  print_unicode(file_dialog_file_extension, graphic_screen_x-8-(number_of_chars_in_extension*8), 6, BLACK);
 }

 //draw "BACK" button
 print("[esc] Back", 8, graphic_screen_y-6-7, BLACK);
 draw_straigth_column(8+10*8+8, graphic_screen_y-20, 20);
 add_zone_to_click_board(0, graphic_screen_y-20, 8+10*8+8, 20, CLICK_ZONE_BACK);

 //draw "SAVE" button
 if(dialog_type==FILE_DIALOG_TYPE_SAVE && file_dialog_folder_memory!=0 && is_filesystem_read_write(file_dialog_folder_device_partition_type)==STATUS_TRUE) {
  print("[s] Save", graphic_screen_x-72, graphic_screen_y-6-7, BLACK);
  draw_straigth_column(graphic_screen_x-80, graphic_screen_y-20, 20);
  add_zone_to_click_board(graphic_screen_x-80, graphic_screen_y-20, 80, 20, FILE_DIALOG_CLICK_ZONE_SAVE_BUTTON);
 }

 //draw connected devices
 draw_full_square(0, 21, FILE_DIALOG_DEVICE_LIST_WIDTH, graphic_screen_y-41, 0xB04100);
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
  if(file_dialog_folder_memory!=0 && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==file_dialog_folder_device_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER)==file_dialog_folder_device_number && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE)==file_dialog_folder_device_partition_type && get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR)==file_dialog_folder_device_partition_first_sector) {
   draw_full_square(0, connected_devices_draw_line, FILE_DIALOG_DEVICE_LIST_WIDTH, 20, RED);
  }

  //print parition name
  add_zone_to_click_board(0, connected_devices_draw_line, FILE_DIALOG_DEVICE_LIST_WIDTH, 20, FILE_DIALOG_CLICK_ZONE_DEVICE_ENTRY+device_list_selected_entry);
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
 draw_button_with_click_zone("[F8] Refresh devices", 5, connected_devices_draw_line, FILE_DIALOG_DEVICE_LIST_WIDTH-10, 20, FILE_DIALOG_CLICK_ZONE_BUTTON_REFRESH_DEVICES);

 //no connected device
 device_list_selected_entry = 0;
 if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==0) {
  if(file_dialog_folder_memory!=0) { //remove actual showed folder
   free(file_dialog_folder_memory);
   file_dialog_folder_memory = 0;
  }
  print("There is no storage device with known filesystem connected", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  redraw_screen();
  return;
 }
 
 //device disconnected
 if(does_device_list_entry_exist(file_dialog_folder_device_type, file_dialog_folder_device_number, file_dialog_folder_device_partition_type, file_dialog_folder_device_partition_first_sector)==STATUS_FALSE) {
  if(file_dialog_folder_memory!=0) { //remove actual showed folder
   free(file_dialog_folder_memory);
   file_dialog_folder_memory = 0;
  }
  print("This device was removed", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  redraw_screen();
  return;
 }

 //draw folder items
 if(file_dialog_folder_memory!=0) {
  dword_t folder_first_showed_entry_offset = (file_dialog_folder_memory+(file_dialog_folder_first_showed_entry*256));
  byte_t *folder8 = (byte_t *) (folder_first_showed_entry_offset);
  word_t *folder16 = (word_t *) (folder_first_showed_entry_offset);
  dword_t *folder32 = (dword_t *) (folder_first_showed_entry_offset);

  //no files in folder
  if(file_dialog_folder_number_of_entries==0) {
   print("There are no files in this directory", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
   redraw_screen();
   return;
  }
  
  //draw up strings
  print("Name", FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  print("Size", (graphic_screen_x-12-72-80-8), PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
  print("Date", (graphic_screen_x-12-80-8), PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);

  //draw selected file background
  if(file_dialog_folder_selected_entry!=NO_FILE_SELECTED && file_dialog_folder_selected_entry>=file_dialog_folder_first_showed_entry && file_dialog_folder_selected_entry<(file_dialog_folder_first_showed_entry+file_dialog_number_of_files_on_screen)) {
   draw_full_square(FILE_DIALOG_DEVICE_LIST_WIDTH+8, (PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+(file_dialog_folder_selected_entry-file_dialog_folder_first_showed_entry)*10), (graphic_screen_x-(FILE_DIALOG_DEVICE_LIST_WIDTH+8)-20), 10, RED);
   
   if(dialog_type==FILE_DIALOG_TYPE_OPEN && file_dialog_folder_device_partition_type==STORAGE_CDDA) {
    set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, 0);
    draw_bottom_line_button("[F12] Read with skipping errors", FILE_DIALOG_CLICK_ZONE_BUTTON_READ_SKIPPING_ERRORS);
   }
  }

  //print file entries
  for(int i=0, draw_line = PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24+1; i<file_dialog_number_of_files_on_screen; i++) {
   //no entry
   if(folder32[(64*i)]==0) {
    break;
   }

   //add click zone
   add_zone_to_click_board(FILE_DIALOG_DEVICE_LIST_WIDTH+8, draw_line-1, (graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8-20), 10, FILE_DIALOG_CLICK_ZONE_FIRST_FILE+i);
   
   //print name
   for(int j=0, draw_column = FILE_DIALOG_DEVICE_LIST_WIDTH+8; j<file_dialog_number_of_chars_of_file_name; j++) {
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
  if(file_dialog_folder_vertical_scrollbar_rider_size!=0) {
   draw_vertical_scrollbar(graphic_screen_x-15, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, file_dialog_scrollbar_height, file_dialog_folder_vertical_scrollbar_position, file_dialog_folder_vertical_scrollbar_rider_size);
   //TODO: program_interface_add_vertical_scrollbar(FILE_DIALOG_CLICK_ZONE_SCROLLBAR, ((dword_t)(&file_dialog_scrollbar_height)), ((dword_t)&file_dialog_folder_vertical_scrollbar_position), ((dword_t)&file_dialog_folder_vertical_scrollbar_rider_size), ((dword_t)&file_dialog_verticall_scrollbar_event));
   add_zone_to_click_board(graphic_screen_x-15, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, 10, file_dialog_scrollbar_height, FILE_DIALOG_CLICK_ZONE_SCROLLBAR);
  }

  //draw back button
  if(file_dialog_folder_path_actual_folder_number!=0) {
   draw_button_with_click_zone("[b] Go back", 5, graphic_screen_y-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH-24, FILE_DIALOG_DEVICE_LIST_WIDTH-10, 24, FILE_DIALOG_CLICK_ZONE_BUTTON_BACK);
  }

 }

 //redraw screen
 redraw_screen();
}

void file_dialog_show_progress(void) {
 draw_full_square(FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, (graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8-8), 16, 0xFF6600);
 draw_full_square(FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, ((graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8-8)*file_work_done_percents/100), 16, 0x000CFF);
 dword_t printed_percents_string_x = 0;
 if(file_work_done_percents<10) {
  printed_percents_string_x = (FILE_DIALOG_DEVICE_LIST_WIDTH+((graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8*5)/2));
 }
 else {
  printed_percents_string_x = (FILE_DIALOG_DEVICE_LIST_WIDTH+((graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8*6)/2));
 }
 print_var(file_work_done_percents, printed_percents_string_x, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+28, BLACK);
 draw_char('%', printed_percents_string_x+16, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+28, BLACK);
 redraw_part_of_screen(FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+24, (graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH-8-8), 16);
}

void file_dialog_print_message(byte_t *message) {
 draw_full_square(FILE_DIALOG_DEVICE_LIST_WIDTH, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, (graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH), (graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH), 0xFF6600);
 print(message, FILE_DIALOG_DEVICE_LIST_WIDTH+8, PROGRAM_INTERFACE_TOP_LINE_HEIGTH+8, BLACK);
 redraw_part_of_screen(FILE_DIALOG_DEVICE_LIST_WIDTH, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, (graphic_screen_x-FILE_DIALOG_DEVICE_LIST_WIDTH), (graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH));
}

void file_dialog_compute_number_of_files_in_directory(void) {
 dword_t *folder = (dword_t *) (file_dialog_folder_memory);
 dword_t number_of_files = 0;
 while(*folder!=0) {
  number_of_files++;
  folder+=64;
 }
 file_dialog_folder_number_of_entries = number_of_files;
 if(number_of_files!=0) {
  file_dialog_folder_vertical_scrollbar_rider_size = calculate_scrollbar_rider_size(file_dialog_scrollbar_height, file_dialog_folder_number_of_entries, file_dialog_number_of_files_on_screen);
  if(file_dialog_folder_vertical_scrollbar_rider_size!=0) {
   file_dialog_folder_vertical_scrollbar_position = calculate_scrollbar_rider_position(file_dialog_scrollbar_height, file_dialog_folder_vertical_scrollbar_rider_size, file_dialog_folder_number_of_entries, file_dialog_number_of_files_on_screen, file_dialog_folder_first_showed_entry);
  }
 }
 else {
  file_dialog_folder_vertical_scrollbar_rider_size = 0;
 } 
}

void file_dialog_read_file_properties(dword_t folder_mem, dword_t file_number) {
 word_t *folder16 = (word_t *) (folder_mem+(file_number*256)+32);
 dword_t *folder32 = (dword_t *) (folder_mem+(file_number*256));
 
 //file size
 file_dialog_file_size = folder32[7];
 
 //file name
 for(int i=0; i<128; i++) {
  file_dialog_file_name[i] = folder16[i];
  if(folder16[i]==0) {
   break;
  }
 }
 
 //file extension
 for(int i=0; i<128; i++) {
  if(folder16[i]=='.') {
   folder16++;
   for(int j=0; j<10; j++) {
    file_dialog_file_extension[j] = folder16[i+j];
    
    if(file_dialog_file_extension[j]<0x41) {
     file_dialog_file_extension[j]=0;
     break;
    }
    else if(file_dialog_file_extension[j]<0x61) { //convert to small chars
     file_dialog_file_extension[j] = (folder16[i+j]+0x20);
    }
   }
   break;
  }
 }
}

void file_dialog_open_file_extensions_clear_mem(void) {
 clear_memory(file_dialog_array_of_allowed_extension_mem, FILE_DIALOG_MAX_FILE_EXTENSION_LENGTH*FILE_DIALOG_MAX_OF_ALLOWED_EXTENSIONS);
 file_dialog_number_of_allowed_extensions = 0;
}

void file_dialog_open_file_add_extension(byte_t *extension) {
 if(file_dialog_number_of_allowed_extensions>=FILE_DIALOG_MAX_OF_ALLOWED_EXTENSIONS) {
  file_dialog_number_of_allowed_extensions = FILE_DIALOG_MAX_OF_ALLOWED_EXTENSIONS;
  return;
 }

 word_t *file_extension_mem = (word_t *) (file_dialog_array_of_allowed_extension_mem+(file_dialog_number_of_allowed_extensions*FILE_DIALOG_MAX_FILE_EXTENSION_LENGTH));
 dword_t extension_length = (get_number_of_chars_in_ascii_string(extension)+1);

 for(dword_t i=0; i<extension_length; i++) {
  file_extension_mem[i]=extension[i];
 }

 file_dialog_number_of_allowed_extensions++;
}

void file_dialog_save_set_extension(byte_t *extension) {
 for(dword_t i=0; i<127; i++) {
  file_dialog_file_extension[i] = extension[i];
  if(extension[i]==0) {
   return;
  }
 }

 file_dialog_file_extension[127] = 0;
}

void file_dialog_process_key_up_key_down(byte_t dialog_type, dword_t key) {
 if(key==KEY_UP) {
  if(file_dialog_folder_selected_entry==NO_FILE_SELECTED) {
   file_dialog_folder_selected_entry = (file_dialog_folder_number_of_entries-1);
   if(file_dialog_folder_selected_entry>(file_dialog_folder_first_showed_entry+file_dialog_number_of_files_on_screen-1)) {
    file_dialog_folder_first_showed_entry = (file_dialog_folder_selected_entry-file_dialog_number_of_files_on_screen+1);
   }
  }
  else {
   if(file_dialog_folder_selected_entry!=0) {
    file_dialog_folder_selected_entry = (file_dialog_folder_selected_entry-1);
   }
   if(file_dialog_folder_selected_entry<file_dialog_folder_first_showed_entry) {
    file_dialog_folder_first_showed_entry = file_dialog_folder_selected_entry;
   }
  }
 }
 else if(key==KEY_DOWN) {
  if(file_dialog_folder_selected_entry==NO_FILE_SELECTED) {
   file_dialog_folder_selected_entry = 0;
   if(file_dialog_folder_selected_entry<file_dialog_folder_first_showed_entry) {
    file_dialog_folder_first_showed_entry = file_dialog_folder_selected_entry;
   }
  }
  else {
   if(file_dialog_folder_selected_entry<(file_dialog_folder_number_of_entries-1)) {
    file_dialog_folder_selected_entry = (file_dialog_folder_selected_entry+1);
   }
   if(file_dialog_folder_selected_entry>(file_dialog_folder_first_showed_entry+file_dialog_number_of_files_on_screen-1)) {
    file_dialog_folder_first_showed_entry = (file_dialog_folder_selected_entry-file_dialog_number_of_files_on_screen+1);
   }
  }
 }

 //recalculate scrollbar
 if(file_dialog_folder_vertical_scrollbar_rider_size!=0) {
  file_dialog_folder_vertical_scrollbar_rider_size = calculate_scrollbar_rider_position(file_dialog_scrollbar_height, file_dialog_folder_vertical_scrollbar_rider_size, file_dialog_folder_number_of_entries, file_dialog_number_of_files_on_screen, file_dialog_folder_first_showed_entry);
 }
 
 //redraw screen
 redraw_file_dialog(dialog_type);
}

dword_t file_dialog_double_click_on_file(byte_t dialog_type, dword_t new_file_memory, dword_t new_file_size) {
 if(file_dialog_folder_number_of_entries!=0 && file_dialog_folder_selected_entry!=NO_FILE_SELECTED && get_char_of_file_from_folder_entry_name(file_dialog_folder_memory, file_dialog_folder_selected_entry, 0)!='.') {
  if(get_file_attribute(file_dialog_folder_memory, file_dialog_folder_selected_entry)==0x10) { //folder
   if(file_dialog_folder_path_actual_folder_number>=FILE_DIALOG_MAX_PATH_FOLDERS) { //too many folders
    error_window("You can not open more than 50 folders");
    redraw_file_dialog((dialog_type & 0xF));
    return STATUS_ERROR;
   }

   //select medium
   if(select_device_list_entry(file_dialog_folder_device_type, file_dialog_folder_device_number, file_dialog_folder_device_partition_type, file_dialog_folder_device_partition_first_sector)==STATUS_ERROR) {
    if(file_dialog_folder_memory!=0) {
     free(file_dialog_folder_memory);
    }
    file_dialog_print_message("Device is not inserted");
    return STATUS_ERROR;
   }

   //read folder
   file_dialog_print_message("Reading directory...");
   dword_t folder_starting_entry = get_file_starting_entry(file_dialog_folder_memory, file_dialog_folder_selected_entry);
   dword_t folder_size = get_file_size(file_dialog_folder_memory, file_dialog_folder_selected_entry);
   dword_t folder_memory = read_folder(folder_starting_entry, folder_size);
   if(folder_memory==STATUS_ERROR) {
    error_window("Error during reading folder");
    redraw_file_dialog((dialog_type & 0xF));
    return STATUS_ERROR;
   }

   //update file values
   file_dialog_folder_path_actual_folder_number = (file_dialog_folder_path_actual_folder_number+1);
   word_t *folder_name = (word_t *) (file_dialog_folder_memory+(file_dialog_folder_selected_entry*256)+32);
   word_t *path_folder_name = (word_t *) (file_dialog_path_folder_names_memory+file_dialog_folder_path_actual_folder_number*20);
   for(dword_t i=0; i<20; i++) {
    path_folder_name[i] = folder_name[i];
   }
   path_folder_name[19] = 0;
   dword_t *folder_path = (dword_t *) (file_dialog_folder_path_memory+(file_dialog_folder_path_actual_folder_number*8));
   folder_path[0] = folder_starting_entry;
   folder_path[1] = folder_size;
   free(file_dialog_folder_memory);
   file_dialog_folder_memory = folder_memory;
   file_dialog_folder_first_showed_entry = 0;
   file_dialog_folder_selected_entry = NO_FILE_SELECTED;
   file_dialog_compute_number_of_files_in_directory();
   
   //show directory on screen
   redraw_file_dialog((dialog_type & 0xF));
   return FILE_DIALOG_FOLDER_WAS_LOADED;
  }
  else { //file
   if(dialog_type==FILE_DIALOG_TYPE_OPEN || dialog_type==FILE_DIALOG_TYPE_OPEN_FILE_SKIPPING_ERRORS) {
    //check if this is file with allowed extension
    if(file_dialog_number_of_allowed_extensions!=0) { //0 = no extensions, open all files
     dword_t allowed_extension = STATUS_FALSE;

     for(dword_t i=0; i<file_dialog_number_of_allowed_extensions; i++) {
      if(compare_file_extension(file_dialog_folder_memory, file_dialog_folder_selected_entry, file_dialog_array_of_allowed_extension_mem+i*FILE_DIALOG_MAX_FILE_EXTENSION_LENGTH, get_number_of_chars_in_ascii_string((byte_t *)(file_dialog_array_of_allowed_extension_mem+i*FILE_DIALOG_MAX_FILE_EXTENSION_LENGTH)))==STATUS_GOOD) {
       allowed_extension = STATUS_TRUE;
       break;
      }
     }

     if(allowed_extension==STATUS_FALSE) {
      error_window("This file type can not be opened in this program");
      redraw_file_dialog((dialog_type & 0xF));
      return STATUS_ERROR;
     }
    }

    //test if there is enough free memory
    dword_t file_size = get_file_size(file_dialog_folder_memory, file_dialog_folder_selected_entry);
    if(all_actual_free_memory_in_bytes<1024*1024*8 || (all_actual_free_memory_in_bytes-(1024*1024*8))<file_size) { //we always leave at least 8 MB of free memory
     error_window("There is not enough free memory, try to close some opened files");
     redraw_file_dialog((dialog_type & 0xF));
     return STATUS_ERROR;
    }

    //select medium
    if(select_device_list_entry(file_dialog_folder_device_type, file_dialog_folder_device_number, file_dialog_folder_device_partition_type, file_dialog_folder_device_partition_first_sector)==STATUS_ERROR) {
     if(file_dialog_folder_memory!=0) {
      free(file_dialog_folder_memory);
     }
     file_dialog_print_message("Device is not inserted");
     return STATUS_ERROR;
    }
    
    //read file
    file_dialog_print_message("Reading file...");
    dword_t file_starting_entry = get_file_starting_entry(file_dialog_folder_memory, file_dialog_folder_selected_entry);
    file_show_file_work_progress = STATUS_TRUE;
    dword_t file_memory = 0;
    if(dialog_type==FILE_DIALOG_TYPE_OPEN_FILE_SKIPPING_ERRORS && file_dialog_folder_device_partition_type==STORAGE_CDDA) {
     file_memory = cdda_read_file_skipping_errors(file_starting_entry, file_size);
    }
    else {
     file_memory = read_file(file_starting_entry, file_size);
    }
    file_show_file_work_progress = STATUS_FALSE;
    if(file_memory==STATUS_ERROR) {
     error_window("Error during reading file");
     redraw_file_dialog((dialog_type & 0xF));
     return STATUS_ERROR;
    }
    file_dialog_read_file_properties(file_dialog_folder_memory, file_dialog_folder_selected_entry);
    return file_memory;
   }
   else if(dialog_type==FILE_DIALOG_TYPE_SAVE && is_filesystem_read_write(file_dialog_folder_device_partition_type)==STATUS_TRUE) {
    //select medium
    if(select_device_list_entry(file_dialog_folder_device_type, file_dialog_folder_device_number, file_dialog_folder_device_partition_type, file_dialog_folder_device_partition_first_sector)==STATUS_ERROR) {
     if(file_dialog_folder_memory!=0) {
      free(file_dialog_folder_memory);
     }
     file_dialog_print_message("Device is not inserted");
     return STATUS_ERROR;
    }

    //dialog window
    if(dialog_yes_no("Do you really want to rewrite this file?")==STATUS_FALSE) {
     redraw_file_dialog((dialog_type & 0xF));
     return STATUS_ERROR;
    }

    //rewrite file
    file_dialog_print_message("Rewriting file...");
    file_show_file_work_progress = STATUS_TRUE;
    if(rewrite_file(get_file_starting_entry(file_dialog_folder_memory, file_dialog_folder_selected_entry), get_file_size(file_dialog_folder_memory, file_dialog_folder_selected_entry), new_file_memory, new_file_size)==STATUS_ERROR) {
     file_show_file_work_progress = STATUS_FALSE;
     error_window("Error during rewriting file");
     redraw_file_dialog((dialog_type & 0xF));
     return STATUS_ERROR;
    }
    file_show_file_work_progress = STATUS_FALSE;
    
    //change file entry
    set_file_entry_size(file_dialog_folder_memory, file_dialog_folder_selected_entry, new_file_size);
    file_dialog_read_file_properties(file_dialog_folder_memory, file_dialog_folder_selected_entry);

    //rewrite folder
    file_dialog_print_message("Saving changes to folder...");
    dword_t *folder_path = (dword_t *) (file_dialog_folder_path_memory+(file_dialog_folder_path_actual_folder_number*8));
    dword_t folder_starting_entry = folder_path[0];
    if(file_dialog_folder_path_actual_folder_number==0) {
     folder_starting_entry = ROOT_DIRECTORY;
    }
    if(rewrite_folder(folder_starting_entry, file_dialog_folder_memory)==STATUS_ERROR) {
     error_window("Error during rewriting folder");
     redraw_file_dialog((dialog_type & 0xF));
     return STATUS_ERROR;
    }
    return STATUS_GOOD;
   }
  }
 }

 return STATUS_ERROR;
}

void file_dialog_folder_back(byte_t dialog_type) {
 if(file_dialog_folder_path_actual_folder_number!=0) {
  //select medium
  if(select_device_list_entry(file_dialog_folder_device_type, file_dialog_folder_device_number, file_dialog_folder_device_partition_type, file_dialog_folder_device_partition_first_sector)==STATUS_ERROR) {
   if(file_dialog_folder_memory!=0) {
    free(file_dialog_folder_memory);
   }
   file_dialog_print_message("Device is not inserted");
   return;
  }
  
  //read folder
  file_dialog_print_message("Reading directory...");
  file_dialog_folder_path_actual_folder_number--;
  dword_t *folder_path = (dword_t *) (file_dialog_folder_path_memory+(file_dialog_folder_path_actual_folder_number*8));
  dword_t folder_starting_entry = folder_path[0];
  dword_t folder_size = folder_path[1];
  if(file_dialog_folder_path_actual_folder_number==0) {
   folder_starting_entry = ROOT_DIRECTORY;
  }
  dword_t folder_memory = read_folder(folder_starting_entry, folder_size);
  if(folder_memory==STATUS_ERROR) {
   error_window("Error during reading folder");
   file_dialog_folder_path_actual_folder_number++; //previous value
   redraw_file_dialog(dialog_type);
   return;
  }

  //update file values
  free(file_dialog_folder_memory);
  file_dialog_folder_memory = folder_memory;
  file_dialog_folder_first_showed_entry = 0;
  file_dialog_folder_selected_entry = NO_FILE_SELECTED;
  file_dialog_compute_number_of_files_in_directory();

  //show directory on screen
  redraw_file_dialog(dialog_type);
 }
}

byte_t file_dialog_save_file(byte_t dialog_type, dword_t new_file_memory, dword_t new_file_size) {
 if(is_filesystem_read_write(file_dialog_folder_device_partition_type)==STATUS_FALSE) { //this filesystem is not read-write
  return STATUS_ERROR;
 }

 //reset text area
 dword_t *text_area_info = (dword_t *) (file_dialog_new_name_text_area);
 word_t *text_area_file_name = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
 clear_memory(text_area_info[TEXT_AREA_INFO_MEMORY], 29*2);
 text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = text_area_info[TEXT_AREA_INFO_MEMORY];

 //show dialog window
 clear_click_board();
 draw_message_window(270, 95);
 print("Write name of new file:", graphic_screen_x_center-120, graphic_screen_y_center-25, BLACK);
 draw_text_area(file_dialog_new_name_text_area);
 draw_button("Cancel", graphic_screen_x_center-100, graphic_screen_y_center+15, 90, 20);
 draw_button("Save", graphic_screen_x_center+10, graphic_screen_y_center+15, 90, 20);
 redraw_screen();

 //file name window
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //do not save
  if(keyboard_value==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y_center+15, graphic_screen_y_center+35, graphic_screen_x_center-100, graphic_screen_x_center-10)==STATUS_TRUE)) {
   return STATUS_ERROR;
  }

  //save file
  if(keyboard_value==KEY_ENTER || (mouse_click_button_state==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y_center+15, graphic_screen_y_center+35, graphic_screen_x_center+10, graphic_screen_x_center+100)==STATUS_TRUE)) {
   if(text_area_file_name[0]==0 || text_area_file_name[0]=='.') { //TODO: more invalid characters
    error_window("Invalid file name");
    return STATUS_ERROR;
   }
   break;
  }

  //process text area events
  text_area_keyboard_event(file_dialog_new_name_text_area);
  text_area_mouse_event(file_dialog_new_name_text_area);
  draw_text_area(file_dialog_new_name_text_area);
  redraw_text_area(file_dialog_new_name_text_area);
 }

 //select medium
 if(select_device_list_entry(file_dialog_folder_device_type, file_dialog_folder_device_number, file_dialog_folder_device_partition_type, file_dialog_folder_device_partition_first_sector)==STATUS_ERROR) {
  if(file_dialog_folder_memory!=0) {
   free(file_dialog_folder_memory);
  }
  file_dialog_print_message("Device is not inserted");
  return STATUS_ERROR;
 }

 //save file
 file_dialog_print_message("Saving file...");
 file_show_file_work_progress = STATUS_TRUE;
 dword_t file_starting_entry = create_file(new_file_memory, new_file_size);
 file_show_file_work_progress = STATUS_FALSE;
 if(file_starting_entry==STATUS_ERROR) {
  error_window("Error during saving file");
  return STATUS_ERROR;
 }

 //create new entry in folder
 file_dialog_folder_memory = realloc(file_dialog_folder_memory, (file_dialog_folder_number_of_entries+2)*256);
 byte_t *folder_new_entry8 = (byte_t *) (file_dialog_folder_memory+file_dialog_folder_number_of_entries*256);
 word_t *folder_new_entry16 = (word_t *) (file_dialog_folder_memory+file_dialog_folder_number_of_entries*256);
 dword_t *folder_new_entry32 = (dword_t *) (file_dialog_folder_memory+file_dialog_folder_number_of_entries*256);
 //short name
 for(int i=0; i<8; i++) {
  folder_new_entry8[i] = text_area_file_name[i];
 }
 //short extension
 folder_new_entry8[8] = file_dialog_file_extension[0];
 folder_new_entry8[9] = file_dialog_file_extension[1];
 folder_new_entry8[10] = file_dialog_file_extension[2];
 //attribute archive
 folder_new_entry8[11] = 0x20;
 //not used
 folder_new_entry8[12] = 0;
 folder_new_entry8[13] = 0;
 //time hour/minute/second
 read_time();
 word_t time_hms = (time_hour<<11 | time_minute<<5 | time_second);
 folder_new_entry16[7] = time_hms;
 folder_new_entry16[11] = time_hms;
 //time year/month/day
 word_t time_ymd = ((time_year-1980)<<9 | time_month<<5 | time_day);
 folder_new_entry16[8] = time_ymd;
 folder_new_entry16[9] = time_ymd;
 folder_new_entry16[12] = time_ymd;
 //file cluster
 folder_new_entry16[10] = (file_starting_entry>>16);
 folder_new_entry16[13] = (file_starting_entry & 0xFFFF);
 //file length
 folder_new_entry32[7] = new_file_size;
 //file name and extension
 for(int i=0; i<50; i++) {
  if(text_area_file_name[i]==0) {
   folder_new_entry16[i+16] = '.';
   folder_new_entry16++;
   for(int j=0; j<10; j++) {
    if(file_dialog_file_extension[j]==0) {
     break;
    }
    folder_new_entry16[i+16+j] = file_dialog_file_extension[j];
   }
   break;
  }
  folder_new_entry16[i+16] = text_area_file_name[i];
 }
 file_dialog_read_file_properties(file_dialog_folder_memory, file_dialog_folder_number_of_entries);
 file_dialog_folder_number_of_entries++;

 //save folder
 file_dialog_print_message("Saving changes to folder...");
 dword_t *folder_path = (dword_t *) (file_dialog_folder_path_memory+(file_dialog_folder_path_actual_folder_number*8));
 dword_t folder_starting_entry = folder_path[0];
 if(file_dialog_folder_path_actual_folder_number==0) {
  folder_starting_entry = ROOT_DIRECTORY;
 }
 if(rewrite_folder(folder_starting_entry, file_dialog_folder_memory)==STATUS_ERROR) {
  error_window("Error during rewriting folder");
  return STATUS_ERROR;
 }
 return STATUS_GOOD;
}

dword_t file_dialog(byte_t dialog_type, dword_t new_file_memory, dword_t new_file_size) {
 redraw_file_dialog(dialog_type);

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //redraw when there is device change
  if(usb_new_device_detected==STATUS_TRUE) {
   redraw_file_dialog(dialog_type);
   continue;
  }

  //close file dialog
  dword_t click_zone = get_mouse_cursor_click_board_value();
  if(keyboard_value==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && click_zone==CLICK_ZONE_BACK)) {
   return STATUS_ERROR;
  }

  //process keyboard event
  if(file_dialog_folder_number_of_entries!=0) {
   if(keyboard_value==KEY_UP) {
    file_dialog_process_key_up_key_down(dialog_type, KEY_UP);
    continue;
   }
   else if(keyboard_value==KEY_DOWN) {
    file_dialog_process_key_up_key_down(dialog_type, KEY_DOWN);
    continue;
   }
  }
  if(keyboard_value==KEY_PAGE_DOWN) {
   if(number_of_device_list_entries==0) {
    continue;
   }
   if(file_dialog_selected_device_entry==0xFFFFFFFF) {
    file_dialog_selected_device_entry = 0;
   }
   else if(file_dialog_selected_device_entry<(number_of_device_list_entries-1)) {
    file_dialog_selected_device_entry++;
   }
   else {
    continue;
   }
   goto select_device_entry;
  }
  else if(keyboard_value==KEY_PAGE_UP) {
   if(number_of_device_list_entries==0) {
    continue;
   }
   if(file_dialog_selected_device_entry==0xFFFFFFFF) {
    file_dialog_selected_device_entry = (number_of_device_list_entries-1);
   }
   else if(file_dialog_selected_device_entry>0) {
    file_dialog_selected_device_entry--;
   }
   else {
    continue;
   }
   goto select_device_entry;
  }
  else if(keyboard_value==KEY_B) {
   file_dialog_folder_back(dialog_type);
   continue;
  }
  else if(keyboard_value==KEY_ENTER && file_dialog_folder_memory!=0 && file_dialog_folder_selected_entry!=NO_FILE_SELECTED) {
   dword_t value = file_dialog_double_click_on_file(dialog_type, new_file_memory, new_file_size);
   if(value==FILE_DIALOG_FOLDER_WAS_LOADED) {
    continue;
   }
   if(dialog_type==FILE_DIALOG_TYPE_OPEN && value!=STATUS_ERROR) {
    return value;
   }
   else if(value==STATUS_GOOD) {
    return STATUS_GOOD;
   }
   continue;
  }
  else if(keyboard_value==KEY_S && dialog_type==FILE_DIALOG_TYPE_SAVE) {
   if(file_dialog_save_file(dialog_type, new_file_memory, new_file_size)==STATUS_GOOD) {
    return STATUS_GOOD;
   }
   redraw_file_dialog(dialog_type); //error during saving file
   continue;
  }
  else if(keyboard_value==KEY_F8) { //refresh device list
   device_list_check_optical_drive();
   redraw_file_dialog(dialog_type);
   continue;
  }
  else if(keyboard_value==KEY_F12 && dialog_type==FILE_DIALOG_TYPE_OPEN && file_dialog_folder_memory!=0 && file_dialog_folder_selected_entry!=NO_FILE_SELECTED && file_dialog_folder_device_partition_type==STORAGE_CDDA) { //CDDA filesystem - read with skipping errors
   if(dialog_yes_no("This may be slow, do you want to continue?")==STATUS_FALSE) {
    redraw_file_dialog(dialog_type);
    continue;
   }
   dword_t value = file_dialog_double_click_on_file(FILE_DIALOG_TYPE_OPEN_FILE_SKIPPING_ERRORS, new_file_memory, new_file_size);
   if(value==FILE_DIALOG_FOLDER_WAS_LOADED) {
    continue;
   }
   if(dialog_type==FILE_DIALOG_TYPE_OPEN && value!=STATUS_ERROR) {
    return value;
   }
   else if(value==STATUS_GOOD) {
    return STATUS_GOOD;
   }
   continue;
  }

  //process mouse event
  if(mouse_click_button_state==MOUSE_CLICK) {
   //refresh device list button
   if(click_zone==FILE_DIALOG_CLICK_ZONE_BUTTON_REFRESH_DEVICES) {
    device_list_check_optical_drive();
    redraw_file_dialog(dialog_type);
    continue;
   }

   //click on device
   if(click_zone>=FILE_DIALOG_CLICK_ZONE_DEVICE_ENTRY && click_zone<=FILE_DIALOG_CLICK_ZONE_DEVICE_LAST_ENTRY) {
    file_dialog_selected_device_entry = (click_zone-FILE_DIALOG_CLICK_ZONE_DEVICE_ENTRY);

    //test if this entry still exist
    select_device_entry:
    device_list_selected_entry = file_dialog_selected_device_entry;
    if(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE)==0) {
     redraw_file_dialog(dialog_type);
     continue;
    }

    //entry exist, read root folder
    select_device_list_entry(get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE), get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR));
     
    //read root directory
    file_dialog_print_message("Reading root directory...");
    dword_t root_folder_memory = read_folder(ROOT_DIRECTORY, 0);
    if(root_folder_memory==STATUS_ERROR) {
     if(file_dialog_folder_memory!=0) { //remove actual showed folder
      free(file_dialog_folder_memory);
      file_dialog_folder_memory = 0;
     }

     error_window("Error during reading root folder of this device");
     redraw_file_dialog(dialog_type);
     continue;
    }

    if(file_dialog_folder_memory!=0) {
     free(file_dialog_folder_memory);
    }

    file_dialog_folder_path_actual_folder_number = 0; //root folder
    file_dialog_folder_memory = root_folder_memory;
    file_dialog_folder_first_showed_entry = 0;
    file_dialog_folder_selected_entry = NO_FILE_SELECTED;
    file_dialog_compute_number_of_files_in_directory();

    file_dialog_folder_device_type = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_TYPE);
    file_dialog_folder_device_number = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_NUMBER);
    file_dialog_folder_device_partition_type = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_TYPE);
    file_dialog_folder_device_partition_first_sector = get_device_list_entry_value(DEVICE_LIST_ENTRY_DEVICE_PARTITION_FIRST_SECTOR);

    //show directory on screen
    redraw_file_dialog(dialog_type);
    continue;
   }

   //select file
   if(file_dialog_folder_memory!=0) {
    if(click_zone>=FILE_DIALOG_CLICK_ZONE_FIRST_FILE) {
     dword_t selected_entry = (file_dialog_folder_first_showed_entry+(click_zone-FILE_DIALOG_CLICK_ZONE_FIRST_FILE));

     //double click
     if(file_dialog_folder_selected_entry==selected_entry) {
      dword_t value = file_dialog_double_click_on_file(dialog_type, new_file_memory, new_file_size);
      if(value==FILE_DIALOG_FOLDER_WAS_LOADED) {
       continue;
      }
      if(dialog_type==FILE_DIALOG_TYPE_OPEN && value!=STATUS_ERROR) {
       return value;
      }
      else if(value==STATUS_GOOD) {
       return STATUS_GOOD;
      }
      continue;
     }

     //first click
     file_dialog_folder_selected_entry = selected_entry;
     redraw_file_dialog(dialog_type);
    }
    else if(click_zone==FILE_DIALOG_CLICK_ZONE_BUTTON_READ_SKIPPING_ERRORS) { //CDDA filesystem - read with skipping errors
     if(dialog_yes_no("This may be slow, do you want to continue?")==STATUS_FALSE) {
      redraw_file_dialog(dialog_type);
      continue;
     }
     dword_t value = file_dialog_double_click_on_file(FILE_DIALOG_TYPE_OPEN_FILE_SKIPPING_ERRORS, new_file_memory, new_file_size);
     if(value==FILE_DIALOG_FOLDER_WAS_LOADED) {
      continue;
     }
     if(dialog_type==FILE_DIALOG_TYPE_OPEN && value!=STATUS_ERROR) {
      return value;
     }
     else if(value==STATUS_GOOD) {
      return STATUS_GOOD;
     }
     continue;
    }
    else if(file_dialog_folder_selected_entry!=NO_FILE_SELECTED) {
     //unselect file
     file_dialog_folder_selected_entry = NO_FILE_SELECTED;
     redraw_file_dialog(dialog_type);
    }
   }

   //back
   if(click_zone==FILE_DIALOG_CLICK_ZONE_BUTTON_BACK) {
    file_dialog_folder_back(dialog_type);
   }

   //save
   if(click_zone==FILE_DIALOG_CLICK_ZONE_SAVE_BUTTON) {
    if(file_dialog_save_file(dialog_type, new_file_memory, new_file_size)==STATUS_GOOD) {
     return STATUS_GOOD;
    }
    redraw_file_dialog(dialog_type); //error during saving file
    continue;
   }
  }
 }
}

dword_t file_dialog_open(void) {
 return file_dialog(FILE_DIALOG_TYPE_OPEN, 0, 0);
}

dword_t file_dialog_save(dword_t file_memory, dword_t file_size) {
 return file_dialog(FILE_DIALOG_TYPE_SAVE, file_memory, file_size);
}