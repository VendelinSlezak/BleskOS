//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_program_interface(void) {
 program_interface_keyboard_events_list_mem = calloc(512*4);
 program_interface_click_zone_events_list_mem = calloc(1000*8);
 program_interface_num_of_click_zone_events = 0;
 program_interface_elements_list_mem = calloc(32*100);
 program_interface_num_of_elements = 0;
 program_element_layout_areas_info = (struct program_element_layout_area_info *) (calloc(sizeof(struct program_element_layout_area_info) * PROGRAM_ELEMENT_LAYOUT_MAX_NUM_OF_AREAS));
}

void set_program_interface(dword_t program_interface_mem) {
 program_interface_memory_pointer = program_interface_mem;
 program_interface_draw_method_pointer = get_program_value(PROGRAM_INTERFACE_DRAW_METHOD_POINTER);
 clear_memory(program_interface_keyboard_events_list_mem, 512*4);
 program_interface_num_of_click_zone_events = 0;
 program_interface_num_of_elements = 0;
 program_interface_selected_element_number = 0xFFFFFFFF;
 program_interface_element_with_focus = 0xFFFFFFFF;
}

void clear_program_interface_before_drawing(void) {
 clear_click_board();
 program_interface_num_of_elements = 0;
 program_interface_selected_element_number = 0xFFFFFFFF;
 program_interface_element_with_focus = 0xFFFFFFFF;
}

void program_interface_add_keyboard_event(dword_t key, dword_t method) {
 if(key>0xFFFF) {
  return;
 }
 else if(key>0xFF) {
  key = (256 + (key & 0xFF));
 }
 dword_t *keyboard_events = (dword_t *) (program_interface_keyboard_events_list_mem);
 keyboard_events[key] = method;
}

void program_interface_add_click_zone_event(dword_t click_zone, dword_t method) {
 if(program_interface_num_of_click_zone_events>=1000) {
  return;
 }
 dword_t *click_zone_events = (dword_t *) (program_interface_click_zone_events_list_mem+program_interface_num_of_click_zone_events*8);
 click_zone_events[0] = click_zone;
 click_zone_events[1] = method;
 program_interface_num_of_click_zone_events++;
}

void program_interface_add_vertical_scrollbar(dword_t click_zone, dword_t pointer_to_scrollbar_height, dword_t pointer_to_rider_position, dword_t pointer_to_rider_size, dword_t scrollbar_draw_method_pointer) {
 if(program_interface_num_of_elements>=100) {
  return;
 }
 dword_t *elements = (dword_t *) (program_interface_elements_list_mem+program_interface_num_of_elements*32);
 elements[0] = ELEMENT_VERTICAL_SCROLLBAR;
 elements[1] = click_zone;
 elements[2] = pointer_to_scrollbar_height;
 elements[3] = pointer_to_rider_position;
 elements[4] = pointer_to_rider_size;
 elements[5] = scrollbar_draw_method_pointer;
 program_interface_num_of_elements++;
}

void program_interface_add_horizontal_scrollbar(dword_t click_zone, dword_t pointer_to_scrollbar_width, dword_t pointer_to_rider_position, dword_t pointer_to_rider_size, dword_t scrollbar_draw_method_pointer) {
 if(program_interface_num_of_elements>=100) {
  return;
 }
 dword_t *elements = (dword_t *) (program_interface_elements_list_mem+program_interface_num_of_elements*32);
 elements[0] = ELEMENT_HORIZONTAL_SCROLLBAR;
 elements[1] = click_zone;
 elements[2] = pointer_to_scrollbar_width;
 elements[3] = pointer_to_rider_position;
 elements[4] = pointer_to_rider_size;
 elements[5] = scrollbar_draw_method_pointer;
 program_interface_num_of_elements++;
}

void program_interface_add_text_area(dword_t click_zone, dword_t text_area_info_mem) {
 if(program_interface_num_of_elements>=100) {
  return;
 }
 dword_t *elements = (dword_t *) (program_interface_elements_list_mem+program_interface_num_of_elements*32);
 elements[0] = ELEMENT_TEXT_AREA;
 elements[1] = click_zone;
 elements[2] = text_area_info_mem;
 program_interface_num_of_elements++;
}

void program_interface_redraw(void) {
 extern void call(dword_t method);
 call(program_interface_draw_method_pointer);
 redraw_screen();
}

void program_interface_process_keyboard_event(void) {
 extern void call(dword_t method);
 dword_t *keyboard_events = (dword_t *) (program_interface_keyboard_events_list_mem);

 if(keyboard_code_of_pressed_key==0) {
  return;
 }
 
 //process keyboard event for selected element
 if(program_interface_element_with_focus!=0xFFFFFFFF) {
  dword_t *element = (dword_t *) (program_interface_elements_list_mem+program_interface_element_with_focus*32);

  if(element[0]==ELEMENT_TEXT_AREA && (keyboard_code_of_pressed_key & 0xFF)<RELEASED_KEY_CODE(0) && (keyboard_code_of_pressed_key<KEY_F1 || keyboard_code_of_pressed_key>KEY_F10) && keyboard_code_of_pressed_key!=KEY_F11 && keyboard_code_of_pressed_key!=KEY_F12) { //we do not process released keys or F keys
   //do not process enter if INPUT line
   dword_t *text_area_info = (dword_t *) (element[2]);
   if((text_area_info[TEXT_AREA_INFO_TYPE]!=TEXT_AREA_INPUT_LINE && text_area_info[TEXT_AREA_INFO_TYPE]!=TEXT_AREA_NUMBER_INPUT) || keyboard_code_of_pressed_key!=KEY_ENTER) {
    //process event
    text_area_keyboard_event(element[2]);
 
    //redraw
    draw_text_area(element[2]);
    redraw_text_area(element[2]);
    redraw_mouse_cursor();
   }
  }
 }

 //test if keyboard events are not disabled
 if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED)==PROGRAM_INTERFACE_FLAG_KEYBOARD_EVENTS_DISABLED) {
  return;
 }

 //call method of keyboard event
 dword_t pressed_key = keyboard_code_of_pressed_key;
 if(pressed_key>0xFFFF) {
  return;
 }
 else if(pressed_key>0xFF) {
  pressed_key = (256 + (pressed_key & 0xFF));
 }
 if(keyboard_events[pressed_key]!=0) {
  if(pressed_key==KEY_F1) {
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)>=10) {
    error_window("You can not open more than 10 files");
    program_interface_redraw();
    return;
   }

   call(keyboard_events[pressed_key]);
   program_interface_redraw();
  }
  else if(pressed_key==KEY_F2) {
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
    return;
   }

   call(keyboard_events[pressed_key]);
   program_interface_redraw();
  }
  else if(pressed_key==KEY_F3) {
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)>=10) {
    error_window("You can not open more than 10 files");
    program_interface_redraw();
    return;
   }

   call(keyboard_events[pressed_key]);
   dword_t actual_element_with_focus = program_interface_element_with_focus;
   program_interface_redraw();
   if((get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_FOCUS_ON_TEXT_AREA_FROM_NEW_FILE_METHOD)==PROGRAM_INTERFACE_FLAG_FOCUS_ON_TEXT_AREA_FROM_NEW_FILE_METHOD) {
    program_interface_element_with_focus = actual_element_with_focus;
   }
  }
  else if(pressed_key==KEY_F4) {
   if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)==0) {
    return;
   }

   //test if is file saved
   if((get_file_value(PROGRAM_INTERFACE_FILE_FLAGS) & PROGRAM_INTERFACE_FILE_FLAG_SAVED)==STATUS_FALSE) {
    if(dialog_yes_no("Do you want to close unsaved file?")==STATUS_FALSE) {
     program_interface_redraw();
     return;
    }
   }

   //close file
   call(keyboard_events[KEY_F4]); //call program method for closing file
   remove_file(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)); //remove file from file area
   if(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)!=1) {
    set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, (get_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE)-1)); //we need to select file below
   }

   //redraw
   program_interface_redraw();
  }
  else {
   call(keyboard_events[pressed_key]);
  }
 }
 else if(pressed_key==KEY_F5) {
  if(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)<get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)) {
   if(get_program_value(PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER)!=0) {
    call(get_program_value(PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER)); //method for process things when file is deselected
   }
   set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)+1); //select next file
   program_interface_redraw();
  }
 }
 else if(pressed_key==KEY_F6) {
  if(get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)>1) {
   if(get_program_value(PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER)!=0) {
    call(get_program_value(PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER)); //method for process things when file is deselected
   }
   set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)-1); //select previous file
   program_interface_redraw();
  }
 }

}

void program_interface_process_mouse_event(void) {
 extern void call(dword_t method);
 dword_t *keyboard_events = (dword_t *) (program_interface_keyboard_events_list_mem);
 dword_t click_zone = get_mouse_cursor_click_board_value();

 if(mouse_click_button_state==MOUSE_NO_EVENT) {
  program_interface_selected_element_number = 0xFFFFFFFF;
  program_interface_text_area_selected_scrollbar = 0;
 }
 else if(mouse_click_button_state==MOUSE_CLICK) {
  //unfocus text area
  if(program_interface_element_with_focus!=0xFFFFFFFF && (get_program_value(PROGRAM_INTERFACE_FLAGS) & PROGRAM_INTERFACE_FLAG_PERMANENT_FOCUS_ON_TEXT_AREA)==STATUS_FALSE) {
   dword_t *element = (dword_t *) (program_interface_elements_list_mem+program_interface_element_with_focus*32);

   if(element[0]==ELEMENT_TEXT_AREA && element[1]!=click_zone) {
    text_area_disable_cursor(element[2]);
 
    //redraw
    draw_text_area(element[2]);
    redraw_text_area(element[2]);
    redraw_mouse_cursor();
   }
  }

  //save selected zone
  set_program_value(PROGRAM_INTERFACE_SELECTED_CLICK_ZONE, click_zone);
  program_interface_selected_element_number = 0xFFFFFFFF;
  program_interface_element_with_focus = 0xFFFFFFFF;
  program_interface_text_area_selected_scrollbar = 0;

  //process click
  if(click_zone==NO_CLICK) {
   return;
  }
  else if(click_zone<10) { //default interface buttons
   if(click_zone==CLICK_ZONE_OPEN) {
    keyboard_code_of_pressed_key = KEY_F1;
    program_interface_process_keyboard_event();
   }
   else if(click_zone==CLICK_ZONE_SAVE) {
    keyboard_code_of_pressed_key = KEY_F2;
    program_interface_process_keyboard_event();
   }
   else if(click_zone==CLICK_ZONE_NEW) {
    keyboard_code_of_pressed_key = KEY_F3;
    program_interface_process_keyboard_event();
   }
  }
  else if(click_zone>=CLICK_ZONE_FIRST_FILE && click_zone<=CLICK_ZONE_LAST_FILE && (get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)!=(click_zone-CLICK_ZONE_FIRST_FILE+1))) { //select file
   if(get_program_value(PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER)!=0) {
    call(get_program_value(PROGRAM_INTERFACE_CHANGE_BETWEEN_FILES_METHOD_POINTER)); //method for process things when file is deselected
   }
   set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, (click_zone-CLICK_ZONE_FIRST_FILE+1)); //select file
   program_interface_redraw();
  }
  else if(click_zone>=CLICK_ZONE_CLOSE_FIRST_FILE && click_zone<=CLICK_ZONE_CLOSE_LAST_FILE) { //close file
   if(keyboard_events[KEY_F4]!=0) {
    //test if is file saved
    if((get_specific_file_value((click_zone-CLICK_ZONE_CLOSE_FIRST_FILE+1), PROGRAM_INTERFACE_FILE_FLAGS) & PROGRAM_INTERFACE_FILE_FLAG_SAVED)==STATUS_FALSE) {
     if(dialog_yes_no("Do you want to close unsaved file?")==STATUS_FALSE) {
      program_interface_redraw();
      return;
     }
    }

    //close file
    set_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE));
    set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, (click_zone-CLICK_ZONE_CLOSE_FIRST_FILE+1));
    call(keyboard_events[KEY_F4]); //call program method for closing file
    set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, get_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE));
    remove_file((click_zone-CLICK_ZONE_CLOSE_FIRST_FILE+1)); //remove file from file area
    if((click_zone-CLICK_ZONE_CLOSE_FIRST_FILE+1)<=get_program_value(PROGRAM_INTERFACE_SELECTED_FILE) && (get_program_value(PROGRAM_INTERFACE_SELECTED_FILE)!=1)) {
     set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, (get_program_value(PROGRAM_INTERFACE_SELECTED_FILE_SAVE_VALUE)-1)); //we need to select file below
    }

    //redraw
    program_interface_redraw();
   }
  }
  else {
   //method for click zone
   dword_t *click_zone_events = (dword_t *) (program_interface_click_zone_events_list_mem);
   for(dword_t i=0; i<program_interface_num_of_click_zone_events; i++, click_zone_events+=2) {
    if(click_zone_events[0]==click_zone) {
     call(click_zone_events[1]);
     break;
    }
   }

   //find if is some element selected
   dword_t *elements = (dword_t *) (program_interface_elements_list_mem);
   for(dword_t i=0; i<program_interface_num_of_elements; i++, elements+=8) {
    if(elements[1]==click_zone) {
     program_interface_selected_element_number = i;
     program_interface_element_with_focus = i;
     break;
    }
   }

   //process click on element
   dword_t *element = (dword_t *) (program_interface_elements_list_mem+program_interface_selected_element_number*32);
   if(element[0]==ELEMENT_TEXT_AREA) {
    dword_t *text_area_info = (dword_t *) (element[2]);

    //find if this is not click on scrollbar
    if(text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE]!=0 && is_mouse_in_zone(text_area_info[TEXT_AREA_INFO_Y], text_area_info[TEXT_AREA_INFO_Y]+text_area_info[TEXT_AREA_INFO_REAL_HEIGHT], text_area_info[TEXT_AREA_INFO_X]+text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_X]+text_area_info[TEXT_AREA_INFO_REAL_WIDTH])==STATUS_TRUE) {
     program_interface_text_area_selected_scrollbar = ELEMENT_VERTICAL_SCROLLBAR;
     program_interface_text_area_info_memory = element[2];
    }
    else if(text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE]!=0 && is_mouse_in_zone(text_area_info[TEXT_AREA_INFO_Y]+text_area_info[TEXT_AREA_INFO_HEIGHT], text_area_info[TEXT_AREA_INFO_Y]+text_area_info[TEXT_AREA_INFO_REAL_HEIGHT], text_area_info[TEXT_AREA_INFO_X], text_area_info[TEXT_AREA_INFO_X]+text_area_info[TEXT_AREA_INFO_WIDTH])==STATUS_TRUE) {
     program_interface_text_area_selected_scrollbar = ELEMENT_HORIZONTAL_SCROLLBAR;
     program_interface_text_area_info_memory = element[2];
    }
    else {
     //process event in text area
     text_area_mouse_event(element[2]);

     //redraw
     draw_text_area(element[2]);
     redraw_text_area(element[2]);
     redraw_mouse_cursor();
    }
   }
  }

 }
 else if(mouse_click_button_state==MOUSE_DRAG) {
  if(program_interface_text_area_selected_scrollbar!=0) {
   dword_t *text_area_info = (dword_t *) (program_interface_text_area_info_memory);

   if(program_interface_text_area_selected_scrollbar==ELEMENT_VERTICAL_SCROLLBAR) {
    //calculate actual rider position
    dword_t movement = (mouse_cursor_y_dnd-mouse_cursor_y_previous_dnd);
    text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION] += movement;
    if(movement<0x80000000 && text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION]>(text_area_info[TEXT_AREA_INFO_REAL_HEIGHT]-text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE])) {
     text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION] = (text_area_info[TEXT_AREA_INFO_REAL_HEIGHT]-text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE]);
    }
    else if(text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION]>0x80000000) {
     text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION] = 0;
    }

    //calculate change of first show line in text area
    text_area_info[TEXT_AREA_INFO_FIRST_SHOW_LINE] = get_scrollbar_rider_value(text_area_info[TEXT_AREA_INFO_REAL_HEIGHT], text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_SIZE], text_area_info[TEXT_AREA_INFO_VERTICAL_SCROLLBAR_RIDER_POSITION], text_area_info[TEXT_AREA_INFO_NUMBER_OF_LINES]*10, text_area_info[TEXT_AREA_INFO_HEIGHT]);

    //redraw
    draw_text_area(program_interface_text_area_info_memory);
    redraw_screen();
   }
   else if(program_interface_text_area_selected_scrollbar==ELEMENT_HORIZONTAL_SCROLLBAR) {
    //calculate actual rider position
    dword_t movement = (mouse_cursor_x_dnd-mouse_cursor_x_previous_dnd);
    text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION] += movement;
    if(movement<0x80000000 && text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION]>(text_area_info[TEXT_AREA_INFO_WIDTH]-text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE])) {
     text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION] = (text_area_info[TEXT_AREA_INFO_WIDTH]-text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE]);
    }
    else if(text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION]>0x80000000) {
     text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION] = 0;
    }

    //calculate change of first show column in text area
    text_area_info[TEXT_AREA_INFO_FIRST_SHOW_COLUMN] = get_scrollbar_rider_value(text_area_info[TEXT_AREA_INFO_WIDTH], text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_SIZE], text_area_info[TEXT_AREA_INFO_HORIZONTAL_SCROLLBAR_RIDER_POSITION], text_area_info[TEXT_AREA_INFO_NUMBER_OF_COLUMNS]*8, text_area_info[TEXT_AREA_INFO_WIDTH]);
 
    //redraw
    draw_text_area(program_interface_text_area_info_memory);
    redraw_screen();
   }
  }
  else if(program_interface_selected_element_number!=0xFFFFFFFF) {
   dword_t *element = (dword_t *) (program_interface_elements_list_mem+program_interface_selected_element_number*32);

   //process movement on element
   if(element[0]==ELEMENT_VERTICAL_SCROLLBAR) {
    dword_t *scrollbar_size = (dword_t *) (element[2]);
    dword_t *rider_position = (dword_t *) (element[3]);
    dword_t *rider_size = (dword_t *) (element[4]);
    dword_t movement = (mouse_cursor_y_dnd-mouse_cursor_y_previous_dnd);

    //calculate actual rider position
    *rider_position += movement;
    if(movement<0x80000000 && *rider_position>(*scrollbar_size-*rider_size)) {
     *rider_position = (*scrollbar_size-*rider_size);
    }
    else if(*rider_position>0x80000000) {
     *rider_position = 0;
    }

    //call scrollbar redraw method
    call(element[5]);
   }
   else if(element[0]==ELEMENT_HORIZONTAL_SCROLLBAR) {
    dword_t *scrollbar_size = (dword_t *) (element[2]);
    dword_t *rider_position = (dword_t *) (element[3]);
    dword_t *rider_size = (dword_t *) (element[4]);
    dword_t movement = (mouse_cursor_x_dnd-mouse_cursor_x_previous_dnd);

    //calculate actual rider position
    *rider_position += movement;
    if(movement<0x80000000 && *rider_position>(*scrollbar_size-*rider_size)) {
     *rider_position = (*scrollbar_size-*rider_size);
    }
    else if(*rider_position>0x80000000) {
     *rider_position = 0;
    }

    //call scrollbar redraw method
    call(element[5]);
   }
   else if(element[0]==ELEMENT_TEXT_AREA) {
    //process event
    text_area_mouse_event(element[2]);
    
    //redraw
    draw_text_area(element[2]);
    redraw_text_area(element[2]);
    redraw_mouse_cursor();
   }
  }
 }

 if(mouse_wheel_movement!=0) {
  if(program_interface_element_with_focus!=0xFFFFFFFF) {
   dword_t *element = (dword_t *) (program_interface_elements_list_mem+program_interface_element_with_focus*32);

   if(element[0]==ELEMENT_TEXT_AREA) {
    //process event
    text_area_mouse_event(element[2]);
    
    //redraw
    draw_text_area(element[2]);
    redraw_text_area(element[2]);
    redraw_mouse_cursor();
   }
  }
 }
}

byte_t dialog_yes_no(byte_t *string) {
 draw_dialog_yes_no(string);
 redraw_screen();
 
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();
  
  if(keyboard_code_of_pressed_key==KEY_ESC) {
   return STATUS_FALSE;
  }
  else if(keyboard_code_of_pressed_key==KEY_ENTER) {
   return STATUS_TRUE;
  }
  if(mouse_click_button_state==MOUSE_CLICK) {
   if(is_mouse_in_zone(screen_y_center-38+2+16+7+16, screen_y_center-38+2+16+7+16+17, screen_x_center-10-80, screen_x_center-10)==STATUS_TRUE) {
    return STATUS_TRUE;
   }
   else if(is_mouse_in_zone(screen_y_center-38+2+16+7+16, screen_y_center-38+2+16+7+16+17, screen_x_center+10, screen_x_center+10+80)==STATUS_TRUE) {
    return STATUS_FALSE;
   }
  }
 }
}

void error_window(byte_t *string) {
 message_window(string);
 redraw_screen();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  if(keyboard_code_of_pressed_key==KEY_ESC || keyboard_code_of_pressed_key==KEY_ENTER || mouse_click_button_state==MOUSE_CLICK) {
   return;
  }
 }
}

dword_t get_number_of_clicked_item_from_menu_list(dword_t number_of_items) {
 return ((mouse_cursor_y-(screen_height-20-number_of_items*20))/20);
}

dword_t window_for_choosing_file_format(dword_t number_of_formats, byte_t *formats_string) {
 //draw window
 draw_message_window(200, 10+10+10+number_of_formats*20+10);
 program_layout_set_dimensions_window(200, 10+10+10+number_of_formats*20+10);
 print("Choose file format:", program_layout_draw_x, program_layout_draw_y, BLACK);
 program_layout_add_text_line();
 draw_list_background(program_layout_draw_x, program_layout_draw_y, program_layout_width, number_of_formats);
 byte_t *format_strings_2 = formats_string;
 for(dword_t i=0; i<number_of_formats; i++, format_strings_2+=(get_number_of_chars_in_ascii_string(format_strings_2)+1)) {
  draw_list_item(format_strings_2);
 }
 redraw_screen();

 //wait
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  if(keyboard_code_of_pressed_key!=0) {
   if(keyboard_code_of_pressed_key==KEY_ESC) {
    return 0xFFFFFFFF;
   }

   keyboard_unicode_value_of_pressed_key = get_small_char_value(keyboard_unicode_value_of_pressed_key);
   format_strings_2 = formats_string;
   for(dword_t i=0; i<number_of_formats; i++, format_strings_2+=(get_number_of_chars_in_ascii_string(format_strings_2)+1)) {
    if(keyboard_unicode_value_of_pressed_key==format_strings_2[1]) { //there is key for this format
     return i;
    }
   }
  }

  if(mouse_click_button_state==MOUSE_CLICK) {
   if(is_mouse_in_zone(screen_y_center-((10+10+10+number_of_formats*20+10)/2), screen_y_center+((10+10+10+number_of_formats*20+10)/2), screen_x_center-100, screen_x_center+100)==STATUS_FALSE) {
    return 0xFFFFFFFF;
   }
   else if(is_mouse_in_zone(program_layout_draw_y, program_layout_draw_y+number_of_formats*20, program_layout_draw_x, program_layout_draw_x+program_layout_width)) {
    return ((mouse_cursor_y-program_layout_draw_y)/20);
   }
  }
 }
}