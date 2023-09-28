//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void bleskos_main_window_print_item(byte_t *string) {
 print(string, bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, BLACK);
 bleskos_main_window_drawing_line += 15;
}

void bleskos_main_window_draw_item(byte_t *string, dword_t color, byte_t type) {
 add_zone_to_click_board(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, type);
 
 draw_full_square(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, color);
 draw_empty_square(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, BLACK);
 print(string, bleskos_main_window_drawing_column+8, bleskos_main_window_drawing_line+6, BLACK);
 bleskos_main_window_drawing_line += 25;
}

void bleskos_main_window_redraw_time(void) {
 if(time_hour<10) {
  print("0", graphic_screen_x-20-152, graphic_screen_y-30, BLACK);
  print_var(time_hour, graphic_screen_x-20-144, graphic_screen_y-30, BLACK);
 }
 else {
  print_var(time_hour, graphic_screen_x-20-152, graphic_screen_y-30, BLACK);
 }
 if(time_minute<10) {
  print("0", graphic_screen_x-20-128, graphic_screen_y-30, BLACK);
  print_var(time_minute, graphic_screen_x-20-120, graphic_screen_y-30, BLACK);
 }
 else {
  print_var(time_minute, graphic_screen_x-20-128, graphic_screen_y-30, BLACK);
 }
 if(time_second<10) {
  print("0", graphic_screen_x-20-104, graphic_screen_y-30, BLACK);
  print_var(time_second, graphic_screen_x-20-96, graphic_screen_y-30, BLACK);
 }
 else {
  print_var(time_second, graphic_screen_x-20-104, graphic_screen_y-30, BLACK);
 }
 
 print(":  :     /  /", graphic_screen_x-20-136, graphic_screen_y-30, BLACK);
 if(time_day<10) {
  print("0", graphic_screen_x-20-80, graphic_screen_y-30, BLACK);
  print_var(time_day, graphic_screen_x-20-72, graphic_screen_y-30, BLACK);
 }
 else {
  print_var(time_day, graphic_screen_x-20-80, graphic_screen_y-30, BLACK);
 }
 if(time_month<10) {
  print("0", graphic_screen_x-20-56, graphic_screen_y-30, BLACK);
  print_var(time_month, graphic_screen_x-20-48, graphic_screen_y-30, BLACK);
 }
 else {
  print_var(time_month, graphic_screen_x-20-56, graphic_screen_y-30, BLACK);
 }
 print_var(time_year, graphic_screen_x-20-32, graphic_screen_y-30, BLACK);
}

void bleskos_main_window_draw_background(void) {
 clear_screen(0x00C000);
 draw_full_square(0, 0, graphic_screen_x, 10, BLACK);
 draw_full_square(0, 0, 10, graphic_screen_y, BLACK);
 draw_full_square(0, graphic_screen_y-10, graphic_screen_x, 10, BLACK);
 draw_full_square(graphic_screen_x-10, 0, 10, graphic_screen_y, BLACK);
}

void bleskos_main_window_redraw(void) {
 bleskos_main_window_draw_background();
 
 clear_click_board();
 
 print("BleskOS main window", 20, 20, BLACK);

 bleskos_main_window_drawing_line = 40;
 bleskos_main_window_drawing_column = 20;
 bleskos_main_window_print_item("Programs");
 bleskos_main_window_draw_item("[t] Text editor", 0xFFE800, MW_TEXT_EDITOR);
 bleskos_main_window_draw_item("[g] Graphic editor", 0xFFE800, MW_GRAPHIC_EDITOR);
 bleskos_main_window_draw_item("[m] Media viewer", 0xFFE800, MW_MEDIA_VIEWER);
 bleskos_main_window_draw_item("[i] Internet browser", 0xFFE800, MW_INTERNET_BROWSER);

 bleskos_main_window_drawing_line += 25;
 bleskos_main_window_print_item("System");
 bleskos_main_window_draw_item("[F1] System board", 0xFFE800, MW_SYSTEM_BOARD);
 bleskos_main_window_draw_item("[F2] Change keyboard layout", 0xFFE800, MW_CHANGE_KEYBOARD_LAYOUT);
 bleskos_main_window_draw_item("[F12] Shutdown", 0xFFE800, MW_SHUTDOWN);
 
 bleskos_main_window_drawing_line = 40;
 bleskos_main_window_drawing_column = graphic_screen_x_center;
 bleskos_main_window_print_item("Connected hardware");
 if(ac97_present==DEVICE_PRESENT) {
  bleskos_main_window_draw_item("AC97 sound card", 0x00B5FF, 0);
 }
 if(hda_present==DEVICE_PRESENT) {
  bleskos_main_window_draw_item("High Definition Audio sound card", 0x00B5FF, 0);
 }
 if(ide_hdd_base!=0) {
  bleskos_main_window_draw_item("IDE hard disk", 0x00B5FF, 0);
 }
 if(ide_cdrom_base!=0) {
  bleskos_main_window_draw_item("IDE optical disk drive", 0x00B5FF, 0);
 }
 if(ahci_hdd_base!=0) {
  bleskos_main_window_draw_item("AHCI hard disk", 0x00B5FF, 0);
 }
 if(ahci_cdrom_base!=0) {
  bleskos_main_window_draw_item("AHCI optical disk drive", 0x00B5FF, 0);
 }
 if(ethernet_cards_pointer!=0) {
  bleskos_main_window_draw_item("Ethernet card", 0x00B5FF, 0);
 }
 if(usb_mouse_state!=0) {
  bleskos_main_window_draw_item("USB mouse", 0x00B5FF, 0);
 }
 if(usb_keyboard_state!=0) {
  bleskos_main_window_draw_item("USB keyboard", 0x00B5FF, 0);
 }
 for(int i=0; i<10; i++) {
  if(usb_mass_storage_devices[i].type==USB_MSD_INITALIZED) {
   bleskos_main_window_draw_item("USB mass storage device", 0x00B5FF, 0);
  }
  else if(usb_mass_storage_devices[i].type==USB_MSD_ATTACHED) {
   bleskos_main_window_draw_item("USB mass storage device", 0x00B5FF, 0);
  }
 }
 
 print("Keyboard layout:", 20, graphic_screen_y-30, BLACK);
 if(((dword_t)keyboard_layout_ptr)==((dword_t)english_keyboard_layout)) {
  print("English", 20+17*8, graphic_screen_y-30, BLACK);
 }
 else if(((dword_t)keyboard_layout_ptr)==((dword_t)slovak_keyboard_layout)) {
  print("Slovak", 20+17*8, graphic_screen_y-30, BLACK);
 }

 draw_full_square(graphic_screen_x-20-28*8, graphic_screen_y-50, 28*8, 10, 0x00C000);
 if(connected_to_network==NETWORK_CONNECTED) {
  print("You are connected to network", graphic_screen_x-20-28*8, graphic_screen_y-50, BLACK);
 }
 else if(connected_to_network==NETWORK_CONNECTION_FALIED) {
  print("Connection to network failed", graphic_screen_x-20-28*8, graphic_screen_y-50, BLACK);
 }
 
 bleskos_main_window_redraw_time();
}

void bleskos_main_window(void) {
 read_time();

 redraw:
 bleskos_main_window_redraw();
 mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
 draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
 redraw_screen();
 bleskos_main_window_time_redraw = 1;
 
 while(1) {
  wait_for_usb_mouse();
  move_mouse_cursor();

  if(ethernet_link_state_change==1) {
   goto redraw;
  }
  
  if(usb_new_device_detected==1) {
   file_dialog_update_devices();
   goto redraw;
  }
  
  if(keyboard_value==KEY_T) {
   bleskos_main_window_time_redraw = 0;
   text_editor();
   goto redraw;
  }
  if(keyboard_value==KEY_G) {
   bleskos_main_window_time_redraw = 0;
   graphic_editor();
   goto redraw;
  }
  if(keyboard_value==KEY_M) {
   bleskos_main_window_time_redraw = 0;
   media_viewer();
   goto redraw;
  }
  if(keyboard_value==KEY_I) {
   bleskos_main_window_time_redraw = 0;
   internet_browser();
   goto redraw;
  }
  else if(keyboard_value==KEY_F1) {
   bleskos_main_window_time_redraw = 0;
   system_board();
   goto redraw;
  }
  else if(keyboard_value==KEY_F2) {
   bleskos_main_window_change_keyboard_layout();
   goto redraw;
  }
  else if(keyboard_value==KEY_F12) {
   bleskos_main_window_shutdown();
   goto redraw;
  }
  
  //click
  if(mouse_drag_and_drop==MOUSE_CLICK) {   
   dword_t click_value = get_mouse_cursor_click_board_value();
   if(click_value==NO_CLICK) {
    continue;
   }
    
   mouse_drag_and_drop = MOUSE_DRAG;
   if(click_value==MW_TEXT_EDITOR) {
    bleskos_main_window_time_redraw = 0;
    text_editor();
   }
   else if(click_value==MW_GRAPHIC_EDITOR) {
    bleskos_main_window_time_redraw = 0;
    graphic_editor();
   }
   else if(click_value==MW_MEDIA_VIEWER) {
    bleskos_main_window_time_redraw = 0;
    media_viewer();
   }
   else if(click_value==MW_INTERNET_BROWSER) {
    bleskos_main_window_time_redraw = 0;
    internet_browser();
   }
   else if(click_value==MW_SYSTEM_BOARD) {
    bleskos_main_window_time_redraw = 0;
    system_board();
   }
   else if(click_value==MW_CHANGE_KEYBOARD_LAYOUT) {
    bleskos_main_window_change_keyboard_layout();
   }
   if(click_value==MW_SHUTDOWN) {
    bleskos_main_window_shutdown();
   }
   
   goto redraw;
  }
 }
}

void bleskos_main_window_change_keyboard_layout(void) {
 bleskos_main_window_time_redraw = 0;

 clear_click_board();
 bleskos_main_window_draw_background();

 print("Keyboard layouts", 20, 20, BLACK);

 bleskos_main_window_drawing_line = 40;
 bleskos_main_window_drawing_column = 20;
 bleskos_main_window_draw_item("[e] English", 0x00FF00, MW_KEYBOARD_LAYOUT_ENGLISH);
 bleskos_main_window_draw_item("[s] Slovak", 0x00FF00, MW_KEYBOARD_LAYOUT_SLOVAK);

 print("You can go back with ESC", 20, graphic_screen_y-30, BLACK);

 mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
 draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
 redraw_screen();

 while(1) {
  wait_for_usb_mouse();
  move_mouse_cursor();

  if(keyboard_value==KEY_ESC) {
   return;
  }
  else if(keyboard_value==KEY_E) {
   keyboard_layout_ptr = (word_t *) english_keyboard_layout;
   keyboard_shift_layout_ptr = (word_t *) english_shift_keyboard_layout;
   return;
  }
  else if(keyboard_value==KEY_S) {
   keyboard_layout_ptr = (word_t *) slovak_keyboard_layout;
   keyboard_shift_layout_ptr = (word_t *) slovak_shift_keyboard_layout;
   return;
  }

  if(mouse_drag_and_drop==MOUSE_CLICK) {
   dword_t click_value = get_mouse_cursor_click_board_value();
   if(click_value==NO_CLICK) {
    continue;
   }
    
   mouse_drag_and_drop = MOUSE_DRAG;
   if(click_value==MW_KEYBOARD_LAYOUT_ENGLISH) {
    keyboard_layout_ptr = (word_t *) english_keyboard_layout;
    keyboard_shift_layout_ptr = (word_t *) english_shift_keyboard_layout;
    return;
   }
   else if(click_value==MW_KEYBOARD_LAYOUT_SLOVAK) {
    keyboard_layout_ptr = (word_t *) slovak_keyboard_layout;
    keyboard_shift_layout_ptr = (word_t *) slovak_shift_keyboard_layout;
    return;
   }
  }
 }
}

void bleskos_main_window_shutdown(void) {
 bleskos_main_window_time_redraw = 0;

 clear_screen(0x00C000);
 mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);

 if(dialog_yes_no("Do you really want to shut down computer?")==STATUS_TRUE) {
  clear_screen(0x00C000);
  print("Shutting down...", 10, 10, BLACK);
  redraw_screen();
  shutdown();
  wait(5000);
  
  clear_screen(0x00C000);
  print("BleskOS is unable to shut down computer, please hold power button", 10, 10, BLACK);
  redraw_screen();
  while(1) { 
   asm("hlt");
  }
 }
}