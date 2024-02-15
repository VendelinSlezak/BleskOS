//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "document_llmf.c"
#include "document_mf.c"
#include "odt.c"

void initalize_document_editor(void) {
 document_editor_program_interface_memory = create_program_interface_memory(((dword_t)&draw_document_editor), PROGRAM_INTERFACE_FLAG_NO_SAVE_BUTTON | PROGRAM_INTERFACE_FLAG_NO_NEW_BUTTON);
 document_editor_vertical_scrollbar_height = (graphic_screen_y-PROGRAM_INTERFACE_TOP_LINE_HEIGTH-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGTH);

 initalize_dllmf();
}

void document_editor(void) {
 set_program_interface(document_editor_program_interface_memory);
 program_interface_add_keyboard_event(KEY_F1, (dword_t)document_editor_open_file);
 program_interface_add_keyboard_event(KEY_F4, (dword_t)document_editor_close_file);
 program_interface_add_keyboard_event(KEY_UP, (dword_t)document_editor_key_up_event);
 program_interface_add_keyboard_event(KEY_DOWN, (dword_t)document_editor_key_down_event);
 program_interface_add_keyboard_event(KEY_PAGE_UP, (dword_t)document_editor_key_page_up_event);
 program_interface_add_keyboard_event(KEY_PAGE_DOWN, (dword_t)document_editor_key_page_down_event);
 program_interface_add_keyboard_event(KEY_HOME, (dword_t)document_editor_key_home_event);
 program_interface_add_keyboard_event(KEY_END, (dword_t)document_editor_key_end_event);
 program_interface_redraw();
 
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  //close program
  if(keyboard_value==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && get_mouse_cursor_click_board_value()==CLICK_ZONE_BACK)) {
   return;
  }

  //process interface events
  program_interface_process_keyboard_event();
  program_interface_process_mouse_event();

  //process mouse wheel event
  if(mouse_wheel!=0) {
   if(mouse_wheel<0x80000000) {
    document_editor_key_up_event();
   }
   else {
    document_editor_key_down_event();
   }
  }
 }
}

void draw_document_editor(void) {
 clear_program_interface_before_drawing();
 draw_program_interface("Document viewer", "", 0x00C0FF, 0x555555);

 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  //draw document
  dllmf_draw_first_line = get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE);
  dllmf_draw_last_line = (dllmf_draw_first_line+dllmf_draw_height);
  dllmf_draw_first_column = get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_COLUMN);
  dllmf_draw_last_column = (dllmf_draw_first_column+dllmf_draw_width);
  draw_dllmf(get_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY));

  //add vertical scrollbar
  if(get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)>dllmf_draw_height) {
   draw_vertical_scrollbar(graphic_screen_x-SCROLLBAR_SIZE, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE));
   add_zone_to_click_board(graphic_screen_x-SCROLLBAR_SIZE, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, SCROLLBAR_SIZE, document_editor_vertical_scrollbar_height, DOCUMENT_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR);
   program_interface_add_vertical_scrollbar(DOCUMENT_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR, ((dword_t)&document_editor_vertical_scrollbar_height), get_position_of_file_memory()+DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION*4, get_position_of_file_memory()+DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE*4, ((dword_t)&document_editor_vertical_scrollbar_event));
  }
 }
}

void document_editor_open_file(void) {
 //open file
 file_dialog_open_file_extensions_clear_mem();
 file_dialog_open_file_add_extension("odt");
 dword_t new_file_mem = file_dialog_open();
 if(new_file_mem==0) {
  return; //file not loaded
 }

 //convert file to DMF format
 show_message_window("Opening file...");
 dword_t dmf_memory = 0;
 if(is_loaded_file_extension("odt")==STATUS_TRUE) {
  dmf_memory = convert_odt_to_dmf(new_file_mem, file_dialog_file_size);
 }
 if(dmf_memory==STATUS_ERROR) { //error during converting file
  error_window("Error during opening file, more info in System log");
  free(new_file_mem);
  return;
 }

 //convert DMF to DLLMF
 dword_t dllmf_memory = calloc(dmf_number_of_chars_in_document*DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES+4);
 convert_dmf_to_dllmf(dmf_memory, dllmf_memory);

 //add file
 add_file((word_t *)file_dialog_file_name, 0, 0, 0, 0, 0);
 set_file_value(DOCUMENT_EDITOR_FILE_DMF_MEMORY, dmf_memory);
 set_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY, dllmf_memory);
 set_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT, dllmf_get_document_height(dllmf_memory));
 set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, 0);
 set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_COLUMN, 0);
 free(new_file_mem);
 document_editor_recalculate_scrollbars();
}

void document_editor_close_file(void) {
 free(get_file_value(DOCUMENT_EDITOR_FILE_DMF_MEMORY));
 free(get_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY));
}

void document_editor_key_up_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)==0) {
   return;
  }
  else if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)<=50) {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, 0);
  }
  else {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)-50);
  }
  
  document_editor_recalculate_scrollbars();
  program_interface_redraw();
 }
}

void document_editor_key_down_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)==(get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height)) {
   return;
  }
  else if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)>=((get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height)-50)) {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, (get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height));
  }
  else {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)+50);
  }
  
  document_editor_recalculate_scrollbars();
  program_interface_redraw();
 }
}

void document_editor_key_page_up_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)==0) {
   return;
  }
  else if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)<=dllmf_draw_height) {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, 0);
  }
  else {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)-dllmf_draw_height);
  }
  
  document_editor_recalculate_scrollbars();
  program_interface_redraw();
 }
}

void document_editor_key_page_down_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)==(get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height)) {
   return;
  }
  else if((get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)<=dllmf_draw_height*2) || (get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)>=(get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height*2))) {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, (get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height));
  }
  else {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)+dllmf_draw_height);
  }
  
  document_editor_recalculate_scrollbars();
  program_interface_redraw();
 }
}

void document_editor_key_home_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)!=0) {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, 0);
   document_editor_recalculate_scrollbars();
   program_interface_redraw();
  }
 }
}

void document_editor_key_end_event(void) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
  if(get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)!=(get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height)) {
   set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, (get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT)-dllmf_draw_height));
   document_editor_recalculate_scrollbars();
   program_interface_redraw();
  }
 }
}

void document_editor_vertical_scrollbar_event(void) {
 set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, get_scrollbar_rider_value(document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT), dllmf_draw_height));
 dllmf_draw_first_line = get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE);
 dllmf_draw_last_line = (dllmf_draw_first_line+dllmf_draw_height);
 dllmf_draw_first_column = get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_COLUMN);
 dllmf_draw_last_column = (dllmf_draw_first_column+dllmf_draw_width);
 draw_full_square(dllmf_screen_first_column, dllmf_screen_first_line, dllmf_draw_width, dllmf_draw_height, 0x555555);
 draw_dllmf(get_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY));
 draw_vertical_scrollbar(graphic_screen_x-SCROLLBAR_SIZE, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE));
 redraw_screen();
}

void document_editor_recalculate_scrollbars(void) {
 set_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, calculate_scrollbar_rider_size(document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT), dllmf_draw_height));
 set_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT), dllmf_draw_height, get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)));
}