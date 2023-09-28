//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_text_editor(void) {
 text_editor_program_interface_memory = create_program_interface_memory();
}

void text_editor(void) {
 set_program_interface(text_editor_program_interface_memory, ((dword_t)&draw_text_editor));
 set_program_value(PROGRAM_INTERFACE_FLAGS, PROGRAM_INTERFACE_FLAG_PERMANENT_FOCUS_ON_TEXT_AREA);
 program_interface_add_keyboard_event(KEY_F1, (dword_t)text_editor_open_file);
 program_interface_add_keyboard_event(KEY_F2, (dword_t)text_editor_save_file);
 program_interface_add_keyboard_event(KEY_F3, (dword_t)text_editor_new_file);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)text_editor_close_file);
 program_interface_redraw();
 
 while(1) {
  wait_for_usb_mouse();
  move_mouse_cursor();

  //close program
  if(keyboard_value==KEY_ESC || (mouse_drag_and_drop==MOUSE_CLICK && get_mouse_cursor_click_board_value()==CLICK_ZONE_BACK)) {
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //check if we need to set "unsaved" flag
  if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
   dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
   if(text_area_info[TEXT_AREA_INFO_FLAGS]==TEXT_AREA_TEXT_CHANGE && (get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & PROGRAM_INTERFACE_FILE_FLAG_SAVED)==PROGRAM_INTERFACE_FILE_FLAG_SAVED) {
    set_file_value(PROGRAM_INTERFACE_FILE_FLAGS, (get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & ~PROGRAM_INTERFACE_FILE_FLAG_SAVED));
    text_area_info[TEXT_AREA_INFO_FLAGS]=0;
    program_interface_redraw();
   }
  }
 }
}

void draw_text_editor(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("Text editor", "", 0xB00000, WHITE);
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t text_area_info_memory = get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY);

  //add text area
  draw_text_area(text_area_info_memory);
  add_zone_to_click_board(0, 21, graphic_screen_x, graphic_screen_y-41, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA);
  program_interface_add_text_area(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA, text_area_info_memory);
  program_interface_element_with_focus = 0; //give text area focus
 }
}

void text_editor_open_file(void) {
 //open any file
 file_dialog_open_file_extensions_clear_mem();
 dword_t new_file_mem = file_dialog_open();
 if(new_file_mem==0) {
  return; //file not loaded
 }
 new_file_mem = realloc(new_file_mem, file_dialog_file_size+2); //add free char to end

 //add file entry with clear text area with as many characters as bytes of file + 100000 more
 add_file((word_t *)file_dialog_file_name, 0, 0, 0, 0, 0);
 set_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY, create_text_area(TEXT_AREA_NORMAL, (file_dialog_file_size+100000), 0, 21, graphic_screen_x, graphic_screen_y-41));

 //convert file from utf-8 to unicode to text area
 dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
 convert_utf_8_to_unicode(new_file_mem, text_area_info[TEXT_AREA_INFO_MEMORY], (file_dialog_file_size+100000));
 free(new_file_mem);

 //count lines and columns of document
 text_area_calculate_number_of_lines_and_columns(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
}

void text_editor_save_file(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  return;
 }

 //count number of chars in text document
 dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
 word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
 dword_t number_of_chars = 0;
 while(*text_area_data!=0) {
  number_of_chars++;
  text_area_data++;
 }
 if(number_of_chars==0) {
  error_window("You can not save empty file");
  return;
 }

 //convert text area content to utf-8
 convert_unicode_to_utf_8(text_area_info[TEXT_AREA_INFO_MEMORY], number_of_chars);

 //save file
 file_dialog_save_set_extension("txt"); //TODO: more extensions
 file_dialog_save(converted_file_memory, converted_file_size-1); //size is without zero char ending
 free(converted_file_memory);
 set_file_value(PROGRAM_INTERFACE_FILE_FLAGS, PROGRAM_INTERFACE_FILE_FLAG_SAVED);
}

void text_editor_new_file(void) {
 add_file((word_t *)"N\0e\0w\0 \0t\0e\0x\0t\0 \0f\0i\0l\0e\0\0\0", 0, 0, 0, 0, 0);
 set_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY, create_text_area(TEXT_AREA_NORMAL, 100000, 0, 21, graphic_screen_x, graphic_screen_y-41));
}

void text_editor_close_file(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  return;
 }
 delete_text_area(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
}