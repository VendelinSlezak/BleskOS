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

 document_editor_style_stack_pointer = (struct document_editor_style_stack_entry *) (calloc(sizeof(struct document_editor_style_stack_entry)*DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STYLE_STACK));
 document_editor_style_stack_number_of_entries = 0;
 document_editor_list_of_styles_pointer = (struct document_editor_list_of_styles_entry *) (calloc(sizeof(struct document_editor_list_of_styles_entry)*DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_LIST_OF_STYLES));
 document_editor_list_of_styles_number_of_entries = 0;
 document_editor_stack_of_lists_pointer = (struct document_editor_stack_of_lists_entry *) (calloc(sizeof(struct document_editor_stack_of_lists_entry)*DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STACK_OF_LISTS));
 document_editor_stack_of_lists_number_of_entries = 0;

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

  //process CTRL+C
  if((keyboard_control_keys & KEYBOARD_CTRL)==KEYBOARD_CTRL && get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)!=0) {
   if(keyboard_value==KEY_C && get_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA)!=0) {
    //release previous copied text
    if(text_area_copy_memory!=0) {
     free(text_area_copy_memory);
    }

    //calculate variables TODO: copy text to Document editor copy memory
    dword_t first_character_memory = 0, last_character_memory = 0;
    if(get_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA)<get_file_value(DOCUMENT_EDITOR_FILE_CURSOR)) {
     first_character_memory = (get_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA));
     last_character_memory = (get_file_value(DOCUMENT_EDITOR_FILE_CURSOR));
    }
    else {
     first_character_memory = (get_file_value(DOCUMENT_EDITOR_FILE_CURSOR));
     last_character_memory = (get_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA));
    }

    //copy text to text area copy memory
    dword_t *dllmf = (dword_t *) (first_character_memory);
    dword_t number_of_characters = 0;
    while(((dword_t)dllmf)<last_character_memory) {
     if(*dllmf==DLLMF_PAGE_CONTENT_END) {
      dllmf++;
     }
     else {
      if(*dllmf!=0xD) {
       number_of_characters++;
      }
      dllmf+=(DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES/4);
     }
    }

    text_area_copy_memory_length = (number_of_characters*2);
    text_area_copy_memory = calloc(text_area_copy_memory_length);
    dllmf = (dword_t *) (first_character_memory);
    word_t *text_area_copy_memory_pointer = (word_t *) (text_area_copy_memory);
    while(((dword_t)dllmf)<last_character_memory) {
     if(*dllmf==DLLMF_PAGE_CONTENT_END) {
      dllmf++;
     }
     else {
      if(*dllmf!=0xD) {
       if(*dllmf==DLLMF_HIDDEN_SPACE) {
        *text_area_copy_memory_pointer = ' ';
       }
       else {
        *text_area_copy_memory_pointer = *dllmf;
       }
       text_area_copy_memory_pointer++;
      }
      dllmf+=(DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES/4);
     }
    }

    continue;
   }

   if(keyboard_value==KEY_A) {
    dword_t *dllmf = (dword_t *) (dllmf_get_data_memory(get_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY)));
    set_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA, ((dword_t)dllmf));
    while(*dllmf!=0) {
     if(*dllmf==DLLMF_PAGE_CONTENT_END) {
      dllmf++;
     }
     else {
      dllmf+=(DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES/4);
     }
    }
    set_file_value(DOCUMENT_EDITOR_FILE_CURSOR, ((dword_t)dllmf));
    program_interface_redraw();

    continue;
   }
  }
  

  //process click in document
  dword_t click_zone = get_mouse_cursor_click_board_value();
  if(mouse_click_button_state==MOUSE_CLICK) {
   if(click_zone>0x100000) {
    set_file_value(DOCUMENT_EDITOR_FILE_CURSOR, click_zone);
    set_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA, click_zone);
    program_interface_redraw();
   }
   else if(click_zone==NO_CLICK) {
    set_file_value(DOCUMENT_EDITOR_FILE_CURSOR, 0);
    set_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA, 0);
    program_interface_redraw();
   }
  }
  else if(mouse_click_button_state==MOUSE_DRAG) {
   if(click_zone>0x100000 && get_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA)!=0) {
    set_file_value(DOCUMENT_EDITOR_FILE_CURSOR, click_zone);
    program_interface_redraw();
   }
  }

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
  dllmf_cursor = get_file_value(DOCUMENT_EDITOR_FILE_CURSOR);
  dllmf_selected_area = get_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA);
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
 dword_t dllmf_memory = calloc((DLLMF_NUM_OF_PAGE_ENTRIES*(DLLMF_PAGE_ENTRY_SIZE+4))+((dmf_number_of_chars_in_document*2)*DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES));
 convert_dmf_to_dllmf(dmf_memory, dllmf_memory);

 //add file
 add_file((word_t *)file_dialog_file_name, 0, 0, 0, 0, 0);
 set_file_value(DOCUMENT_EDITOR_FILE_DMF_MEMORY, dmf_memory);
 set_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY, dllmf_memory);
 set_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT, dllmf_get_document_height(dllmf_memory));
 set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE, 0);
 set_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_COLUMN, 0);
 set_file_value(DOCUMENT_EDITOR_FILE_CURSOR, (dllmf_memory+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE));
 set_file_value(DOCUMENT_EDITOR_FILE_SELECTED_AREA, 0);
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
 add_zone_to_click_board(dllmf_screen_first_column, dllmf_screen_first_line, dllmf_draw_width, dllmf_draw_height, NO_CLICK);
 draw_dllmf(get_file_value(DOCUMENT_EDITOR_FILE_DLLMF_MEMORY));
 draw_vertical_scrollbar(graphic_screen_x-SCROLLBAR_SIZE, PROGRAM_INTERFACE_TOP_LINE_HEIGTH, document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION), get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE));
 redraw_screen();
}

void document_editor_recalculate_scrollbars(void) {
 set_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE, calculate_scrollbar_rider_size(document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT), dllmf_draw_height));
 set_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION, calculate_scrollbar_rider_position(document_editor_vertical_scrollbar_height, get_file_value(DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE), get_file_value(DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT), dllmf_draw_height, get_file_value(DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE)));
}

void document_editor_add_style_to_stack(void) {
 if(document_editor_style_stack_number_of_entries<DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STYLE_STACK) {
  document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries].paragraph_alignment = dmf_paragraph_alignment;
  document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries].character_size = dmf_character_size;
  document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries].character_emphasis = dmf_character_emphasis;
  document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries].character_color = dmf_character_color;
  document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries].character_background_color = dmf_character_background_color;
 }
 if(document_editor_style_stack_number_of_entries<0xFFFFFFFF) {
  document_editor_style_stack_number_of_entries++;
 }
}

void document_editor_take_style_from_stack(void) {
 if(document_editor_style_stack_number_of_entries>0) {
  document_editor_style_stack_number_of_entries--;
 }
 if(document_editor_style_stack_number_of_entries<DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STYLE_STACK) {
  dmf_paragraph_alignment = document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries-1].paragraph_alignment;
  dmf_character_size = document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries-1].character_size;
  dmf_character_emphasis = document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries-1].character_emphasis;
  dmf_character_color = document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries-1].character_color;
  dmf_character_background_color = document_editor_style_stack_pointer[document_editor_style_stack_number_of_entries-1].character_background_color;
 }
}

void document_editor_add_style_to_list(dword_t memory_of_style_name, dword_t memory_of_style_content) {
 if(document_editor_list_of_styles_number_of_entries<DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_LIST_OF_STYLES) {
  document_editor_list_of_styles_pointer[document_editor_list_of_styles_number_of_entries].memory_of_style_name = memory_of_style_name;
  document_editor_list_of_styles_pointer[document_editor_list_of_styles_number_of_entries].memory_of_style_content = memory_of_style_content;
  document_editor_list_of_styles_number_of_entries++;
 }
}