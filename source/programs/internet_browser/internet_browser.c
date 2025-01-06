//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "draw_webpage.c"
#include "process_html.c"
#include "process_css.c"

void initalize_internet_browser(void) {
 internet_browser_program_interface_memory = create_program_interface_memory(((dword_t)&draw_internet_browser), (PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON | PROGRAM_INTERFACE_FLAG_FOCUS_ON_TEXT_AREA_FROM_NEW_FILE_METHOD));
 internet_browser_webpage_height = screen_height-20-INTERNET_BROWSER_WEBPAGE_START_LINE;
 internet_browser_webpage_width = screen_width-10;
 internet_browser_first_show_line = 0;
 internet_browser_last_show_line = (internet_browser_first_show_line+internet_browser_webpage_height);
 internet_browser_first_show_column = 0;
 internet_browser_last_show_column = (internet_browser_first_show_column+internet_browser_webpage_width);
 internet_browser_url_mem = malloc(2048);
 url_base_mem = calloc(2048);
 url_input_mem = calloc(2048);
 url_output_mem = calloc(2048);
 
 html_title_memory = malloc(1000*2);
 
 html_tag_css_list_mem = calloc(HTML_TAG_CSS_ENTRY_SIZE*1000);
 html_tag_css_list = (dword_t *) html_tag_css_list_mem;
 html_tag_css_list_pointer = 0;
 
 html_list_type_mem = calloc(1000*4);
 html_list_type = (dword_t *) html_list_type_mem;
 html_list_type_pointer = 0;
 html_table_cell_width_list_mem = calloc(1000*4);
 html_table_cell_width_list = (dword_t *) html_table_cell_width_list_mem;
 html_table_cell_width_list_pointer = 0;
 
 css_entries_tag_list_mem = calloc(10000*8);
 css_entries_tag_list_pointer = 0;
 css_entries_id_list_mem = calloc(10000*8);
 css_entries_id_list_pointer = 0;
 css_entries_class_list_mem = calloc(10000*8);
 css_entries_class_list_pointer = 0;

 html_list_of_downloaded_files_mem = calloc(1000*4);
 html_list_of_downloaded_files = (dword_t *) (html_list_of_downloaded_files_mem);
 html_list_of_downloaded_files_pointer = 0;
}

void internet_browser(void) {
 set_program_interface(internet_browser_program_interface_memory);
 program_interface_add_keyboard_event(KEY_F1, (dword_t)internet_browser_open_file);
 program_interface_add_keyboard_event(KEY_F3, (dword_t)internet_browser_new_file);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)internet_browser_close_file);
 program_interface_add_keyboard_event(KEY_UP, (dword_t)internet_browser_key_up_event);
 program_interface_add_keyboard_event(KEY_DOWN, (dword_t)internet_browser_key_down_event);
 program_interface_add_keyboard_event(KEY_HOME, (dword_t)internet_browser_key_home_event);
 program_interface_add_keyboard_event(KEY_END, (dword_t)internet_browser_key_end_event);
 program_interface_add_keyboard_event(KEY_ENTER, (dword_t)internet_browser_key_enter_event);
 program_interface_add_keyboard_event(KEY_P, (dword_t)internet_browser_key_p_event);
 program_interface_add_click_zone_event(INTERNET_BROWSER_CLICK_ZONE_PREVIOUS_PAGE_BUTTON, (dword_t)internet_browser_key_p_event);
 program_interface_redraw();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //close program
  if(keyboard_code_of_pressed_key==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && get_mouse_cursor_click_board_value()==CLICK_ZONE_BACK)) {
   //disable text area
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
    text_area_disable_cursor(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
   }
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //process click to link
  dword_t click_zone = get_mouse_cursor_click_board_value();
  if(mouse_click_button_state==MOUSE_CLICK && click_zone>=0x100000) {
   //convert pointer to href="link" to URL
   convert_relative_url_to_full_url(click_zone);
   byte_t *url_output = (byte_t *) (url_output_mem);
   if(url_output[0]==0) { //not URL parsed
    continue;
   }

   //save actual URL to history
   dword_t *text_area_info = (dword_t *) (get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
   copy_memory_back((get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)+9*2048*2), (get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)+10*2048*2), 9*2048*2+2);
   copy_memory((text_area_info[TEXT_AREA_INFO_MEMORY]), (get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)), 2048*2);

   //copy URL to text area
   word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
   byte_t *url_base = (byte_t *) (url_base_mem);
   for(int i=0; i<2048; i++) {
    text_area_data[i] = url_output[i];
    if(url_output[i]==0) {
     break;
    }
   }

   //load webpage
   internet_browser_load_webpage_from_url_in_text_area();

   //redraw
   program_interface_redraw();
  }

  //process mouse wheel event
  if(mouse_wheel!=0) {
   if(mouse_wheel<0x80000000) {
    internet_browser_key_up_event();
   }
   else {
    internet_browser_key_down_event();
   }
  }
 }
}

void draw_internet_browser(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("Internet browser", "", 0x27A000, WHITE);
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //draw text area
  draw_text_area(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  add_zone_to_click_board(0, 21, screen_width, 10, INTERNET_BROWSER_CLICK_ZONE_TEXT_AREA);
  program_interface_add_text_area(INTERNET_BROWSER_CLICK_ZONE_TEXT_AREA, get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));

  //draw file
  dword_t file_status = get_file_value(INTERNET_BROWSER_FILE_STATUS);
  if(file_status==INTERNET_BROWSER_FILE_STATUS_NOTHING) {
   return;
  }
  else if(file_status==INTERNET_BROWSER_FILE_STATUS_GOOD) {
   //draw webpage
   internet_browser_first_show_line = get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE);
   internet_browser_last_show_line = (internet_browser_first_show_line+internet_browser_webpage_height);
   internet_browser_redraw_webpage(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY));

   //draw scrollbar
   draw_full_square(internet_browser_webpage_width, INTERNET_BROWSER_WEBPAGE_START_LINE, 10, internet_browser_webpage_height, 0x888888);
   if(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)>internet_browser_webpage_height) {
    draw_vertical_scrollbar(internet_browser_webpage_width, INTERNET_BROWSER_WEBPAGE_START_LINE, internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE));
    program_interface_add_vertical_scrollbar(INTERNET_BROWSER_CLICK_ZONE_SCROLLBAR, ((dword_t)(&internet_browser_webpage_height)), get_position_of_file_memory()+INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION*4, get_position_of_file_memory()+INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE*4, ((dword_t)internet_browser_vertical_scrollbar_event));
    add_zone_to_click_board(internet_browser_webpage_width, INTERNET_BROWSER_WEBPAGE_START_LINE, 10, internet_browser_webpage_height, INTERNET_BROWSER_CLICK_ZONE_SCROLLBAR);
   }

   //draw back button if there is page in history
   dword_t file_url_history_memory = get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY);
   dword_t *text_area_info = (dword_t *) (get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
   word_t *url_history = (word_t *) (file_url_history_memory);
   if(are_equal_w_string_w_string((word_t *)(file_url_history_memory), (word_t *)(text_area_info[TEXT_AREA_INFO_MEMORY]))==STATUS_TRUE) {
    url_history = (word_t *) (file_url_history_memory+2048*2);
   }
   if(url_history[0]!=0) {
    set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, 0);
    draw_bottom_line_button("[p] Previous page", INTERNET_BROWSER_CLICK_ZONE_PREVIOUS_PAGE_BUTTON);
   }
  }
  else if(file_status==INTERNET_BROWSER_FILE_STATUS_ERROR_NO_INTERNET) {
   print("No connection to internet", 10, INTERNET_BROWSER_WEBPAGE_START_LINE+8, BLACK);
  }
  else if(file_status==INTERNET_BROWSER_FILE_STATUS_ERROR) {
   print("Error during transfer, check log for detailed informations", 10, INTERNET_BROWSER_WEBPAGE_START_LINE+8, BLACK);
  }
 }
}

void internet_browser_print_message(byte_t *string) {
 print(string, 8, internet_browser_message_line+40, BLACK);
 redraw_part_of_screen(0, internet_browser_message_line+40, screen_width, 10);
 internet_browser_message_line += 10;
}

void internet_browser_open_file(void) {
 //open html file
 if(file_dialog_open("htm html")==FILE_DIALOG_EVENT_EXIT_FILE_NOT_LOADED) {
  return; //file not loaded
 }

 //add file entry
 add_file(file_dialog_file_descriptor->name, 0, 0, 0, 0, 0);

 //convert file and set file variables
 set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_GOOD);
 set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY, convert_html_to_bleskos_webpage((dword_t)file_dialog_open_file_memory, file_dialog_file_descriptor->file_size_in_bytes));
 set_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY, (dword_t)file_dialog_open_file_memory);
 set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH, html_page_height);
 set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, 0);
 set_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY, create_text_area(TEXT_AREA_INPUT_LINE, 2048, 0, 20, screen_width, 10));
 text_area_disable_cursor(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
 if(html_page_height!=0) {
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, calculate_scrollbar_rider_size(internet_browser_webpage_height, html_page_height, internet_browser_webpage_height));
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), html_page_height, internet_browser_webpage_height, 0));
 }
 else {
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, 0);
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, 0);
 }
 word_t *html_title = (word_t *) (html_title_memory);
 if(*html_title!=0) {
  //title from HTML page
  for(dword_t i=0; i<20; i++) {
   set_char_of_file_name(i, html_title[i]);
   if(html_title[i]==0) {
    break;
   }
  }
  set_char_of_file_name(19, 0);
 }
 set_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY, calloc(10*2048*2));
}

void internet_browser_new_file(void) {
 //add file entry
 add_file((word_t *)"N\0e\0w\0 \0t\0a\0b\0\0\0", 0, 0, 0, 0, 0);

 //set file variables
 set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_NOTHING);
 set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY, 0);
 set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH, 0);
 set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, 0);
 set_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY, create_text_area(TEXT_AREA_INPUT_LINE, 2048, 0, 20, screen_width, 10));
 program_interface_element_with_focus = 0; //give text area focus
 set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, 0);
 set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, 0);
 set_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY, 0);
 set_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY, calloc(10*2048*2));
}

void internet_browser_close_file(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  return;
 }
 delete_text_area(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
 free(get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY));
 if(get_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY)!=0) {
  free(get_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY));
 }
 if(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY)!=0) {
  free(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY));
 }
}

void internet_browser_key_up_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)>internet_browser_webpage_height) {
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE)-100);
  if(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE)>0x80000000) {
   set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, 0);
  }
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH), internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE)));
  program_interface_redraw();
 }
}

void internet_browser_key_down_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)>internet_browser_webpage_height) {
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE)+100);
  if(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE)>(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)-internet_browser_webpage_height)) {
   set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, (get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)-internet_browser_webpage_height));
  }
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH), internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE)));
  program_interface_redraw();
 }
}

void internet_browser_key_home_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)>internet_browser_webpage_height) {
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, 0);
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, 0);
  program_interface_redraw();
 }
}

void internet_browser_key_end_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)>internet_browser_webpage_height) {
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, (get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)-internet_browser_webpage_height));
  set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, (internet_browser_webpage_height-get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE)));
  program_interface_redraw();
 }
}

void internet_browser_key_enter_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //save actual URL to history
  dword_t *text_area_info = (dword_t *) (get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  copy_memory_back((get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)+9*2048*2), (get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)+10*2048*2), 9*2048*2+2);
  copy_memory((text_area_info[TEXT_AREA_INFO_MEMORY]), (get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)), 2048*2);

  //load webpage
  internet_browser_load_webpage_from_url_in_text_area();

  //redraw
  text_area_disable_cursor(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  program_interface_redraw();
 }
}

void internet_browser_key_p_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //copy URL from history to text area
  dword_t *text_area_info = (dword_t *) (get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  dword_t file_url_history_memory = get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY);
  if(are_equal_w_string_w_string((word_t *)(file_url_history_memory), (word_t *)(text_area_info[TEXT_AREA_INFO_MEMORY]))==STATUS_TRUE) {
   word_t *url_history = (word_t *) (file_url_history_memory+2048*2);
   if(url_history[0]==0) {
    return;
   }
   copy_memory((file_url_history_memory+2048*2), (text_area_info[TEXT_AREA_INFO_MEMORY]), 2048*2);
  }
  else {
   word_t *url_history = (word_t *) (file_url_history_memory);
   if(url_history[0]==0) {
    return;
   }
   copy_memory((file_url_history_memory), (text_area_info[TEXT_AREA_INFO_MEMORY]), 2048*2);
  }
  copy_memory((file_url_history_memory+2048*2), (file_url_history_memory), 9*2048*2);
  clear_memory(file_url_history_memory+9*2048*2, 2048*2);

  //load webpage
  internet_browser_load_webpage_from_url_in_text_area();

  //redraw
  text_area_disable_cursor(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  program_interface_redraw();
 }
}

void internet_browser_vertical_scrollbar_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0 && get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH)>internet_browser_webpage_height) {
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, get_scrollbar_rider_value(internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH), internet_browser_webpage_height));
  
  //redraw
  
  draw_full_square(0, INTERNET_BROWSER_WEBPAGE_START_LINE, screen_width, internet_browser_webpage_height, WHITE);
  internet_browser_first_show_line = get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE);
  internet_browser_last_show_line = (internet_browser_first_show_line+internet_browser_webpage_height);
  internet_browser_redraw_webpage(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY));
  draw_vertical_scrollbar(internet_browser_webpage_width, INTERNET_BROWSER_WEBPAGE_START_LINE, internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE));
  redraw_screen();
 }
}

void internet_browser_load_webpage_from_url_in_text_area(void) {
 if(internet.status==INTERNET_STATUS_CONNECTED) {
  //close actual opened file
  if(get_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY)!=0) {
   free(get_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY));
  }
  if(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY)!=0) {
   free(get_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY));
  }

  //prepare arrays and variables
  dword_t *text_area_info = (dword_t *) (get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
  byte_t *url = (byte_t *) (internet_browser_url_mem);
  clear_memory(internet_browser_url_mem, 2048);
  clear_memory(url_base_mem, 2048);
  internet_browser_message_line = 0;

  //convert text area to URL
  //TODO: convert also non ASCII chars
  for(int i=0; i<2048; i++) {
   if(*text_area_data==0) {
    break;
   }
   else if(*text_area_data<128) {
    *url = *text_area_data;
    url++;
   }
   text_area_data++;
  }

  //draw clear window with message about loading page
  draw_program_interface("Internet browser", "", 0x27A000, WHITE);
  draw_text_area(get_file_value(INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY));
  internet_browser_print_message("Loading page...");
  redraw_screen();

  //download page file from network
  byte_t file_full_url[MAX_LENGTH_OF_URL];
  dword_t html_memory = STATUS_ERROR, html_size = 0;
  dword_t transfer_number = download_file_from_url((byte_t *)internet_browser_url_mem);
  dword_t timeout = (time_of_system_running+10000);
  while(get_status_of_network_transfer(transfer_number)==NETWORK_TRANSFER_TRANSFERRING_DATA) {
   asm("hlt");

   if(time_of_system_running >= timeout) {
    log("\nInternet browser transfer timeout");
    kill_network_transfer(transfer_number);
    set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_ERROR);
    return;
   }
  }
  if(get_status_of_network_transfer(transfer_number)==NETWORK_TRANSFER_DONE) {
   html_memory = calloc(get_file_size_of_network_transfer(transfer_number)+2);
   html_size = get_file_size_of_network_transfer(transfer_number);
   copy_memory((dword_t)get_file_memory_of_network_transfer(transfer_number), html_memory, html_size);
   copy_memory((dword_t)&network_transfers[transfer_number].url, (dword_t)&file_full_url, MAX_LENGTH_OF_URL);
  }
  //else NETWORK_TRANSFER_ERROR
  close_network_transfer(transfer_number);

  if(html_memory==STATUS_ERROR) {
   if(internet.status==INTERNET_STATUS_DISCONNECTED) {
    set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_ERROR_NO_INTERNET);
   }
   else {
    set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_ERROR);
   }
   return;
  }
  else {
   set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_GOOD);
  }

  //copy resulting URL to text area and history
  text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
  byte_t *url_base = (byte_t *) (url_base_mem);
  for(int i=0; i<2048; i++) {
   text_area_data[i] = file_full_url[i];
   url_base[i] = file_full_url[i];
   if(file_full_url[i]==0) {
    break;
   }
  }
  copy_memory(((dword_t)text_area_data), (get_file_value(INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY)), 2048*2);
  text_area_disable_cursor((dword_t)text_area_info);

  //convert HTML to BleskOS webpage
  internet_browser_print_message("Processing HTML webpage...");
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_MEMORY, convert_html_to_bleskos_webpage(html_memory, html_size));
  
  //set tab name
  word_t *html_title = (word_t *) (html_title_memory);
  if(*html_title!=0) {
   //title from HTML page
   for(dword_t i=0; i<20; i++) {
    set_char_of_file_name(i, html_title[i]);
    if(html_title[i]==0) {
     break;
    }
   }
   set_char_of_file_name(19, 0);
  }
  else {
   set_char_of_file_name(0, 'N');
   set_char_of_file_name(1, 'o');
   set_char_of_file_name(2, ' ');
   set_char_of_file_name(3, 't');
   set_char_of_file_name(4, 'i');
   set_char_of_file_name(5, 't');
   set_char_of_file_name(6, 'l');
   set_char_of_file_name(7, 'e');
   set_char_of_file_name(8, 0);
  }

  //set file variables
  set_file_value(INTERNET_BROWSER_FILE_HTML_MEMORY, html_memory);
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_LENGTH, html_page_height);
  set_file_value(INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE, 0);
  if(html_page_height!=0) {
   set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, calculate_scrollbar_rider_size(internet_browser_webpage_height, html_page_height, internet_browser_webpage_height));
   set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(internet_browser_webpage_height, get_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), html_page_height, internet_browser_webpage_height, 0));
  }
  else {
   set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, 0);
   set_file_value(INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, 0);
  }
 }
 else {
  set_file_value(INTERNET_BROWSER_FILE_STATUS, INTERNET_BROWSER_FILE_STATUS_ERROR_NO_INTERNET);
 }
}