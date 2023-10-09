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
 text_editor_find_function_text_area_info_mem = create_text_area(TEXT_AREA_INPUT_LINE, 1000, 4, graphic_screen_y-20-4-12, graphic_screen_x-24-160, 10);
 text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
 text_editor_replace_function_text_area_mem = create_text_area(TEXT_AREA_INPUT_LINE, 1000, 4, graphic_screen_y-20-4-12, graphic_screen_x-24-160, 10);
 text_area_disable_cursor(text_editor_replace_function_text_area_mem);
 text_editor_go_to_line_text_area_mem = create_text_area(TEXT_AREA_NUMBER_INPUT, 5, graphic_screen_x_center-24, graphic_screen_y_center, 6*8, 10);
 text_editor_state = TEXT_EDITOR_STATE_TEXT;
 text_editor_more_list_on_screen = STATUS_FALSE;
}

void text_editor(void) {
 set_program_interface(text_editor_program_interface_memory, ((dword_t)&draw_text_editor));
 set_program_value(PROGRAM_INTERFACE_FLAGS, PROGRAM_INTERFACE_FLAG_PERMANENT_FOCUS_ON_TEXT_AREA);
 program_interface_add_keyboard_event(KEY_F1, (dword_t)text_editor_open_file);
 program_interface_add_keyboard_event(KEY_F2, (dword_t)text_editor_save_file);
 program_interface_add_keyboard_event(KEY_F3, (dword_t)text_editor_new_file);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)text_editor_close_file);
 program_interface_add_keyboard_event(KEY_F7, (dword_t)text_editor_key_f7_event);
 program_interface_add_keyboard_event(KEY_F8, (dword_t)text_editor_key_f8_event);
 program_interface_add_keyboard_event(KEY_F9, (dword_t)text_editor_key_f9_event);
 program_interface_add_keyboard_event(KEY_F10, (dword_t)text_editor_key_f10_event);
 program_interface_add_keyboard_event(KEY_F11, (dword_t)text_editor_key_f11_event);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_MORE, (dword_t)text_editor_key_f7_event);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_UNDO, (dword_t)text_editor_click_zone_undo_event);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_REDO, (dword_t)text_editor_click_zone_redo_event);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_MORE_LIST, (dword_t)text_editor_click_zone_more_list);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA, (dword_t)text_area_unfocus_find_and_replace);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND, (dword_t)text_area_unfocus_replace);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE, (dword_t)text_area_unfocus_find);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_NEXT, (dword_t)text_editor_find_next);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_PREVIOUS, (dword_t)text_editor_find_previous);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE_ACTUAL_SELECTION, (dword_t)text_editor_replace_actual_selection);
 program_interface_add_click_zone_event(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE_ALL, (dword_t)text_editor_replace_all);
 text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
 text_area_disable_cursor(text_editor_replace_function_text_area_mem);
 program_interface_redraw();
 
 while(1) {
  wait_for_usb_mouse();
  move_mouse_cursor();

  //close program
  if(keyboard_value==KEY_ESC || (mouse_drag_and_drop==MOUSE_CLICK && get_mouse_cursor_click_board_value()==CLICK_ZONE_BACK)) {
   text_editor_more_list_on_screen = STATUS_FALSE;
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //check if we need to set "unsaved" flag or draw undo/redo button
  if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
   dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
   if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_TEXT_CHANGE_FLAG)==TEXT_AREA_TEXT_CHANGE_FLAG) {
    if((get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & PROGRAM_INTERFACE_FILE_FLAG_SAVED)==PROGRAM_INTERFACE_FILE_FLAG_SAVED) {
     set_file_value(PROGRAM_INTERFACE_FILE_FLAGS, (get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & ~PROGRAM_INTERFACE_FILE_FLAG_SAVED));
     text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_TEXT_CHANGE_FLAG;
     program_interface_redraw();
    }
    if((text_editor_button_undo_on_screen==STATUS_FALSE && text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]>0) || (text_editor_button_redo_on_screen==STATUS_FALSE && text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]<text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER]) || (text_editor_button_undo_on_screen==STATUS_TRUE && text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]==0) || (text_editor_button_redo_on_screen==STATUS_TRUE && text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]==text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER])) {
     program_interface_redraw();
    }
   }
  }
 }
}

void draw_text_editor(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("Text editor", "", 0xB00000, BLACK);
 text_editor_button_undo_on_screen = STATUS_FALSE;
 text_editor_button_redo_on_screen = STATUS_FALSE;
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t text_area_info_memory = get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY);
  dword_t *text_area_info = (dword_t *) (text_area_info_memory);

  //add text area
  if(text_editor_state==TEXT_EDITOR_STATE_TEXT) {
   text_area_info[TEXT_AREA_INFO_HEIGTH]=graphic_screen_y-41;
  }
  else if(text_editor_state==TEXT_EDITOR_STATE_FIND) {
   text_area_info[TEXT_AREA_INFO_HEIGTH]=graphic_screen_y-41-20;
  }
  else if(text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
   text_area_info[TEXT_AREA_INFO_HEIGTH]=graphic_screen_y-41-20-20;
  }
  text_area_info[TEXT_AREA_INFO_REAL_HEIGTH]=text_area_info[TEXT_AREA_INFO_HEIGTH];
  draw_text_area(text_area_info_memory);
  add_zone_to_click_board(0, 21, graphic_screen_x, text_area_info[TEXT_AREA_INFO_REAL_HEIGTH], TEXT_EDITOR_CLICK_ZONE_TEXT_AREA);
  program_interface_add_text_area(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA, text_area_info_memory);
  program_interface_element_with_focus = 0; //set focus on main text area

  //draw find/find and replace area
  dword_t *find_function_text_area_info = (dword_t *) (text_editor_find_function_text_area_info_mem);
  if(text_editor_state==TEXT_EDITOR_STATE_FIND) {
   draw_full_square(0, graphic_screen_y-20-20, graphic_screen_x, 20, 0xFF7000);

   find_function_text_area_info[TEXT_AREA_INFO_Y] = graphic_screen_y-20-16;
   draw_text_area(text_editor_find_function_text_area_info_mem);
   add_zone_to_click_board(4, find_function_text_area_info[TEXT_AREA_INFO_Y], graphic_screen_x-16, 10, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND);
   program_interface_add_text_area(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND, text_editor_find_function_text_area_info_mem);

   draw_button("Next", graphic_screen_x-4-80-8-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12);
   add_zone_to_click_board(graphic_screen_x-4-80-8-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_NEXT);

   draw_button("Previous", graphic_screen_x-4-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12);
   add_zone_to_click_board(graphic_screen_x-4-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_PREVIOUS);
  }
  else if(text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
   draw_full_square(0, graphic_screen_y-20-20-20, graphic_screen_x, 20+20, 0xFF7000);

   find_function_text_area_info[TEXT_AREA_INFO_Y] = graphic_screen_y-20-20-16;
   draw_text_area(text_editor_find_function_text_area_info_mem);
   add_zone_to_click_board(4, graphic_screen_y-20-20-16, find_function_text_area_info[TEXT_AREA_INFO_Y], 10, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND);
   program_interface_add_text_area(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND, text_editor_find_function_text_area_info_mem);

   draw_button("Next", graphic_screen_x-4-80-8-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12);
   add_zone_to_click_board(graphic_screen_x-4-80-8-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_NEXT);

   draw_button("Previous", graphic_screen_x-4-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12);
   add_zone_to_click_board(graphic_screen_x-4-80, find_function_text_area_info[TEXT_AREA_INFO_Y], 80, 12, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_PREVIOUS);

   draw_text_area(text_editor_replace_function_text_area_mem);
   add_zone_to_click_board(4, graphic_screen_y-20-16, graphic_screen_x-16, 10, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE);
   program_interface_add_text_area(TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE, text_editor_replace_function_text_area_mem);

   draw_button("Replace", graphic_screen_x-4-100-8-60, graphic_screen_y-20-16, 65, 12);
   add_zone_to_click_board(graphic_screen_x-4-100-8-60, graphic_screen_y-20-16, 65, 12, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE_ACTUAL_SELECTION);

   draw_button("Replace all", graphic_screen_x-4-100, graphic_screen_y-20-16, 100, 12);
   add_zone_to_click_board(graphic_screen_x-4-100, graphic_screen_y-20-16, 100, 12, TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE_ALL);
  }

  //draw bottom line buttons
  set_program_value(PROGRAM_INTERFACE_BOTTOM_LINE_DRAW_COLUMN, 0);
  draw_bottom_line_button("[F7] More", TEXT_EDITOR_CLICK_ZONE_MORE);
  if(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]>0) {
   draw_bottom_line_button("[ctrl+Z] Undo", TEXT_EDITOR_CLICK_ZONE_UNDO);
   text_editor_button_undo_on_screen = STATUS_TRUE;
  }
  if(text_area_info[TEXT_AREA_INFO_CHANGES_LIST_POINTER]<text_area_info[TEXT_AREA_INFO_CHANGES_LIST_LAST_ENTRY_POINTER]) {
   draw_bottom_line_button("[ctrl+Y] Redo", TEXT_EDITOR_CLICK_ZONE_REDO);
   text_editor_button_redo_on_screen = STATUS_TRUE;
  }
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
 set_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY, create_text_area(TEXT_AREA_NORMAL_DARK, (file_dialog_file_size+100000), 0, 21, graphic_screen_x, graphic_screen_y-41));

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
 set_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY, create_text_area(TEXT_AREA_NORMAL_DARK, 100000, 0, 21, graphic_screen_x, graphic_screen_y-41));
}

void text_editor_close_file(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
  return;
 }
 delete_text_area(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
}

void text_editor_key_f7_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //if is list opened, close it
  if(text_editor_more_list_on_screen==STATUS_TRUE) {
   program_interface_redraw();
   text_editor_more_list_on_screen = STATUS_FALSE;
   return;
  }

  //draw list on screen
  draw_menu_list("[F8] Find\n[F9] Find and replace\n[F10] Go to line\n[F11] Dark mode", COLUMN_OF_FIRST_BUTTON_ON_BOTTOM_LINE, TEXT_EDITOR_CLICK_ZONE_MORE_LIST);
  text_editor_more_list_on_screen = STATUS_TRUE;
 }
}

void text_editor_key_f8_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES!=0)) {
  if(text_editor_state==TEXT_EDITOR_STATE_FIND) {
   text_editor_state = TEXT_EDITOR_STATE_TEXT;
  }
  else {
   text_editor_state = TEXT_EDITOR_STATE_FIND;
   text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
  }
  program_interface_redraw();
 }
}

void text_editor_key_f9_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES!=0)) {
  if(text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
   text_editor_state = TEXT_EDITOR_STATE_TEXT;
  }
  else {
   text_editor_state = TEXT_EDITOR_STATE_FIND_AND_REPLACE;
   text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
   text_area_disable_cursor(text_editor_replace_function_text_area_mem);
  }
  program_interface_redraw();
 }
}

void text_editor_key_f10_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES!=0)) {
  dword_t *number_text_area_info = (dword_t *) (text_editor_go_to_line_text_area_mem);
  word_t *number_text_area_data = (word_t *) (number_text_area_info[TEXT_AREA_INFO_MEMORY]);

  //select everything in number input text area
  number_text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = number_text_area_info[TEXT_AREA_INFO_MEMORY];
  number_text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = number_text_area_info[TEXT_AREA_INFO_MEMORY];
  for(dword_t i=0; i<5; i++) {
   if(number_text_area_data[i]==0) {
    break;
   }
   number_text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=2;
  }

  //draw Go to line dialog
  mouse_cursor_restore_background(mouse_cursor_x, mouse_cursor_y);
  draw_full_square(graphic_screen_x_center-32, graphic_screen_y_center-8-7-8, 8*8+2, 8+7+8+12+8+12+8, 0xFF7000);
  draw_empty_square(graphic_screen_x_center-32, graphic_screen_y_center-8-7-8, 8*8+2, 8+7+8+12+8+12+8, BLACK);
  print("Line:", graphic_screen_x_center-24, graphic_screen_y_center-8-7, BLACK);
  draw_text_area(text_editor_go_to_line_text_area_mem);
  draw_button("Go", graphic_screen_x_center-24, graphic_screen_y_center+12+8, 6*8+2, 12);
  mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
  draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
  redraw_part_of_screen(graphic_screen_x_center-32, graphic_screen_y_center-8-7-8, 8*8, 8+7+8+12+8+12+8);

  while(1) {
   wait_for_usb_mouse();
   move_mouse_cursor();

   if(keyboard_value==KEY_ESC || (mouse_drag_and_drop==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y_center-8-7-8, graphic_screen_y_center+12+8+12+8, graphic_screen_x_center-4*8, graphic_screen_x_center+4*8+2)==STATUS_FALSE)) {
    program_interface_redraw();
    mouse_drag_and_drop = MOUSE_DRAG;
    return;
   }
   else if(keyboard_value==KEY_ENTER || (mouse_drag_and_drop==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y_center+12+8, graphic_screen_y_center+12+8+10, graphic_screen_x_center-24, graphic_screen_x_center-24+6*8+2)==STATUS_TRUE)) {
    dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
    word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
    dword_t line = convert_word_string_to_number(number_text_area_info[TEXT_AREA_INFO_MEMORY]); //get number of line

    //move to start of line
    while(line>0) {
     if(*text_area_data==0xA) {
      line--;
     }
     else if(*text_area_data==0) {
      break;
     }
     text_area_data++;
    }

    //redraw
    text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF; //no selected area
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data); //move cursor
    text_area_set_show_line_and_column((dword_t)text_area_info);
    program_interface_redraw();
    mouse_drag_and_drop = MOUSE_DRAG;
    return;
   }
   else if(keyboard_value!=0) {
    text_area_keyboard_event(text_editor_go_to_line_text_area_mem);
    mouse_cursor_restore_background(mouse_cursor_x, mouse_cursor_y);
    draw_text_area(text_editor_go_to_line_text_area_mem);
    mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
    draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
    redraw_text_area(text_editor_go_to_line_text_area_mem);
   }

   if(mouse_drag_and_drop==MOUSE_CLICK || mouse_drag_and_drop==MOUSE_DRAG) {
    text_area_mouse_event(text_editor_go_to_line_text_area_mem);
    mouse_cursor_restore_background(mouse_cursor_x, mouse_cursor_y);
    draw_text_area(text_editor_go_to_line_text_area_mem);
    mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
    draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
    redraw_text_area(text_editor_go_to_line_text_area_mem);
   }
  }
 }
}

void text_editor_key_f11_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES!=0)) {
  dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  if(text_area_info[TEXT_AREA_INFO_TYPE]==TEXT_AREA_NORMAL) {
   text_area_info[TEXT_AREA_INFO_TYPE] = TEXT_AREA_NORMAL_DARK;
  }
  else {
   text_area_info[TEXT_AREA_INFO_TYPE] = TEXT_AREA_NORMAL;
  }
  program_interface_redraw();
 }
}

void text_editor_click_zone_undo_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES!=0)) {
  text_area_undo(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  draw_text_area(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  program_interface_redraw();
 }
}

void text_editor_click_zone_redo_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES!=0)) {
  text_area_redo(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  draw_text_area(get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  program_interface_redraw();
 }
}

void text_editor_click_zone_more_list(void) {
 dword_t selected_item = get_number_of_clicked_item_from_menu_list(4);

 if(selected_item==0) {
  text_editor_key_f8_event();
 }
 else if(selected_item==1) {
  text_editor_key_f9_event();
 }
 else if(selected_item==2) {
  text_editor_key_f10_event();
 }
 else if(selected_item==3) {
  text_editor_key_f11_event();
 }
}

void text_area_unfocus_find_and_replace(void) {
 text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
 text_area_disable_cursor(text_editor_replace_function_text_area_mem);
 if(text_editor_state==TEXT_EDITOR_STATE_FIND || text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
  draw_text_area(text_editor_find_function_text_area_info_mem);
  redraw_text_area(text_editor_find_function_text_area_info_mem);
 }
 if(text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
  draw_text_area(text_editor_replace_function_text_area_mem);
  redraw_text_area(text_editor_replace_function_text_area_mem);
 }
}

void text_area_unfocus_find(void) {
 text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
 if(text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
  draw_text_area(text_editor_find_function_text_area_info_mem);
  redraw_text_area(text_editor_find_function_text_area_info_mem);
 }
}

void text_area_unfocus_replace(void) {
 text_area_disable_cursor(text_editor_replace_function_text_area_mem);
 if(text_editor_state==TEXT_EDITOR_STATE_FIND_AND_REPLACE) {
  draw_text_area(text_editor_replace_function_text_area_mem);
  redraw_text_area(text_editor_replace_function_text_area_mem);
 }
}

void text_editor_find_next(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  dword_t actual_cursor_position = text_area_info[TEXT_AREA_INFO_CURSOR_POSITION];
  word_t *text_area_data = (word_t *) (actual_cursor_position);
  dword_t *find_text_area_info = (dword_t *) (text_editor_find_function_text_area_info_mem);
  word_t *find_text_area_data = (word_t *) (find_text_area_info[TEXT_AREA_INFO_MEMORY]);

  if(find_text_area_data[0]==0) {
   return; //no string to find
  }
  text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
  text_area_disable_cursor(text_editor_replace_function_text_area_mem);
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF;

  //if we are on match, skip it to find next match
  if(*text_area_data!=0) {
   for(dword_t i=0; i<1001; i++) {
    if(find_text_area_data[i]==0) {
     text_area_data++;
     break;
    }

    if(text_area_data[i]!=find_text_area_data[i]) {
     break; //this is not match
    }
   }
  }

  //search from cursor position to end of document
  while(*text_area_data!=0) {
   for(dword_t i=0; i<1001; i++) {
    if(find_text_area_data[i]==0) {
     //we find match because we are at end of find string
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data);
     text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     program_interface_redraw();
     return;
    }

    if(text_area_data[i]!=find_text_area_data[i]) {
     break; //this is not match
    }
   }

   text_area_data++;
  }

  //search from start of document to cursor position
  text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
  while((dword_t)text_area_data!=actual_cursor_position) {
   for(dword_t i=0; i<1001; i++) {
    if(find_text_area_data[i]==0) {
     //we find match because we are at end of find string
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data);
     text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     program_interface_redraw();
     return;
    }

    if(text_area_data[i]!=find_text_area_data[i]) {
     break; //this is not match
    }
   }

   text_area_data++;
  }

  //no match founded
  error_window("No match founded");
  program_interface_redraw();
 }
}

void text_editor_find_previous(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  dword_t actual_cursor_position = text_area_info[TEXT_AREA_INFO_CURSOR_POSITION];
  word_t *text_area_data = (word_t *) (actual_cursor_position);
  dword_t *find_text_area_info = (dword_t *) (text_editor_find_function_text_area_info_mem);
  word_t *find_text_area_data = (word_t *) (find_text_area_info[TEXT_AREA_INFO_MEMORY]);

  if(find_text_area_data[0]==0) {
   return; //no string to find
  }
  text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
  text_area_disable_cursor(text_editor_replace_function_text_area_mem);
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = 0xFFFFFFFF;

  //if we are on match, skip it to find previous match
  for(dword_t i=0; i<1001; i++) {
   if(find_text_area_data[i]==0) {
    text_area_data--;
    break;
   }

   if(text_area_data[i]!=find_text_area_data[i]) {
    break; //this is not match
   }
  }

  //search from cursor position to start of document
  while((dword_t)text_area_data>=text_area_info[TEXT_AREA_INFO_MEMORY]) {
   for(dword_t i=0; i<1001; i++) {
    if(find_text_area_data[i]==0) {
     //we find match because we are at end of find string
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data);
     text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     program_interface_redraw();
     return;
    }

    if(text_area_data[i]!=find_text_area_data[i]) {
     break; //this is not match
    }
   }

   text_area_data--;
  }

  //search from end of document to cursor position
  text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_MEMORY]);
  while(*text_area_data!=0) {
   text_area_data++;
  }
  while((dword_t)text_area_data!=actual_cursor_position) {
   for(dword_t i=0; i<1001; i++) {
    if(find_text_area_data[i]==0) {
     //we find match because we are at end of find string
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data);
     text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = ((dword_t)text_area_data+(i*2));
     text_area_set_show_line_and_column((dword_t)text_area_info);
     program_interface_redraw();
     return;
    }

    if(text_area_data[i]!=find_text_area_data[i]) {
     break; //this is not match
    }
   }

   text_area_data--;
  }

  //no match founded
  error_window("No match founded");
  program_interface_redraw();
 }
}

void text_editor_replace_actual_selection(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
  dword_t *replace_text_area_info = (dword_t *) (text_editor_replace_function_text_area_mem);
  word_t *replace_text_area_data = (word_t *) (replace_text_area_info[TEXT_AREA_INFO_MEMORY]);

  if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]==0xFFFFFFFF || text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]==text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) {
   text_editor_find_next();
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]==0xFFFFFFFF || text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]==text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]) {
    return; //no area selected and no match founded
   }
  }

  text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
  text_area_disable_cursor(text_editor_replace_function_text_area_mem);

  //calculate length of copied string
  dword_t text_area_length_of_inserted_string_chars = 0;
  for(dword_t i=0; i<1000; i++) {
   if(replace_text_area_data[i]==0) {
    break;
   }
   text_area_length_of_inserted_string_chars+=2;
  }

  //replace string
  if(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]<=(text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_length_of_inserted_string_chars)) {
   //add deleted area data to change list
   text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0;
   if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
    text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1;
    text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
   }
   else {
    text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
   }
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2);
    for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2); i<((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])/2); i++) {
     text_area_add_change_to_list((dword_t)text_area_info, text_area_change_command, *text_area_data, cursor_position);
     text_area_data--;
     cursor_position-=2;
    }
   }
   else {
    text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2);
    for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2); i<((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])/2); i++) {
     text_area_add_change_to_list((dword_t)text_area_info, text_area_change_command, *text_area_data, cursor_position);
     text_area_data--;
     cursor_position-=2;
    }
   }
   text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   
   //delete chars from text area
   if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
    copy_memory((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]+2));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
   }
   else {
    copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+2));
    clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]));
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER];
    text_area_data = (word_t *) ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
   }
   
   //replace
   if(text_area_length_of_inserted_string_chars!=0) {
    copy_memory_back((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_length_of_inserted_string_chars-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_length_of_inserted_string_chars));
   
    //add chars
    copy_memory((replace_text_area_info[TEXT_AREA_INFO_MEMORY]), ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), text_area_length_of_inserted_string_chars);

    //add chars to change list
    text_area_change_command = TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0;
    if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
     text_area_change_command = TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1;
     text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
    }
    else {
     text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
    }
    replace_text_area_data = (word_t *) (replace_text_area_info[TEXT_AREA_INFO_MEMORY]);
    for(dword_t i=0, cursor_position = text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]; i<(text_area_length_of_inserted_string_chars/2); i++) {
     text_area_add_change_to_list((dword_t)text_area_info, text_area_change_command, *replace_text_area_data, cursor_position);
     replace_text_area_data++;
     cursor_position+=2;
    }

    //move cursor
    text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=text_area_length_of_inserted_string_chars;
    text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
   }

   //set change
   text_area_change_type = TEXT_AREA_TEXT_CHANGE;
   text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;

   //calculate actual number of lines and columns
   text_area_calculate_number_of_lines_and_columns((dword_t)text_area_info);

   //go to next match
   text_editor_find_next();
  }

  text_area_set_show_line_and_column((dword_t)text_area_info);
  program_interface_redraw();
 }
}

void text_editor_replace_all(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  dword_t *text_area_info = (dword_t *) (get_file_value(TEXT_EDITOR_FILE_TEXT_AREA_MEMORY));
  word_t *text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]);
  dword_t *find_text_area_info = (dword_t *) (text_editor_find_function_text_area_info_mem);
  word_t *find_text_area_data = (word_t *) (find_text_area_info[TEXT_AREA_INFO_MEMORY]);
  dword_t *replace_text_area_info = (dword_t *) (text_editor_replace_function_text_area_mem);
  word_t *replace_text_area_data = (word_t *) (replace_text_area_info[TEXT_AREA_INFO_MEMORY]);

  if(find_text_area_data[0]==0) {
   return; //no string to find
  }
  text_area_disable_cursor(text_editor_find_function_text_area_info_mem);
  text_area_disable_cursor(text_editor_replace_function_text_area_mem);
  text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_info[TEXT_AREA_INFO_MEMORY];
  text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;

  //calculate length of copied string
  dword_t text_area_length_of_inserted_string_chars = 0;
  for(dword_t i=0; i<1000; i++) {
   if(replace_text_area_data[i]==0) {
    break;
   }
   text_area_length_of_inserted_string_chars+=2;
  }

  //replace all
  while(*text_area_data!=0) {
   for(dword_t i=0; i<1001; i++) {
    if(find_text_area_data[i]==0 && text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]<=(text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_length_of_inserted_string_chars)) {
     //we find match because we are at end of find string
     text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER] = ((dword_t)text_area_data);
     text_area_info[TEXT_AREA_INFO_CURSOR_POSITION] = ((dword_t)text_area_data+(i*2));

     //delete selected area
     //add deleted area data to change list
     text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_0;
     if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
      text_area_change_command = TEXT_AREA_CHANGE_REMOVE_GROUP_OF_CHARS_TOGGLE_1;
      text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
     }
     else {
      text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
     }
     if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
      text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2);
      for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-2); i<((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])/2); i++) {
       text_area_add_change_to_list((dword_t)text_area_info, text_area_change_command, *text_area_data, cursor_position);
       text_area_data--;
       cursor_position-=2;
      }
     }
     else {
      text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2);
      for(dword_t i=0, cursor_position = (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2); i<((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])/2); i++) {
       text_area_add_change_to_list((dword_t)text_area_info, text_area_change_command, *text_area_data, cursor_position);
       text_area_data--;
       cursor_position-=2;
      }
     }
     
     //delete chars from text area
     if(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]>text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]) {
      copy_memory((text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]+2));
      clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION])), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]));
     }
     else {
      copy_memory((text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), (text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+2));
      clear_memory((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-(text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER])), (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]));
      text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]=text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER];
     }
     
     //replace
     if(text_area_length_of_inserted_string_chars!=0) {
      copy_memory_back((text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_length_of_inserted_string_chars-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-2), (text_area_info[TEXT_AREA_INFO_MEMORY_LAST_BYTE]-text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-text_area_length_of_inserted_string_chars));
     
      //add chars
      copy_memory((replace_text_area_info[TEXT_AREA_INFO_MEMORY]), ((dword_t)text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]), text_area_length_of_inserted_string_chars);

      //add chars to change list
      text_area_change_command = TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_0;
      if((text_area_info[TEXT_AREA_INFO_FLAGS] & TEXT_AREA_COPY_TOGGLE_FLAG)==TEXT_AREA_COPY_TOGGLE_FLAG) {
       text_area_change_command = TEXT_AREA_CHANGE_ADD_GROUP_OF_CHARS_TOGGLE_1;
       text_area_info[TEXT_AREA_INFO_FLAGS] &= ~TEXT_AREA_COPY_TOGGLE_FLAG;
      }
      else {
       text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_COPY_TOGGLE_FLAG;
      }
      replace_text_area_data = (word_t *) (replace_text_area_info[TEXT_AREA_INFO_MEMORY]);
      for(dword_t i=0, cursor_position = text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]; i<(text_area_length_of_inserted_string_chars/2); i++) {
       text_area_add_change_to_list((dword_t)text_area_info, text_area_change_command, *replace_text_area_data, cursor_position);
       replace_text_area_data++;
       cursor_position+=2;
      }

      //move cursor
      text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]+=text_area_length_of_inserted_string_chars;
      text_area_info[TEXT_AREA_INFO_SELECTED_AREA_POINTER]=0xFFFFFFFF;
     }

     //set change
     text_area_change_type = TEXT_AREA_TEXT_CHANGE;
     text_area_info[TEXT_AREA_INFO_FLAGS] |= TEXT_AREA_TEXT_CHANGE_FLAG;
     text_area_data = (word_t *) (text_area_info[TEXT_AREA_INFO_CURSOR_POSITION]-2);
     break;
    }

    if(text_area_data[i]!=find_text_area_data[i]) {
     break; //this is not match
    }
   }

   text_area_data++;
  }

  //calculate actual number of lines and columns
  text_area_calculate_number_of_lines_and_columns((dword_t)text_area_info);
  text_area_set_show_line_and_column((dword_t)text_area_info);
  program_interface_redraw();
 }
}