//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
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

void bleskos_main_window_redraw_sound_volume(void) {
 bleskos_main_window_drawing_line = 40;
 bleskos_main_window_drawing_column = graphic_screen_x_center;
 bleskos_main_window_print_item("Sound volume");
 add_zone_to_click_board(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, MW_SOUND_VOLUME);
 draw_full_square(bleskos_main_window_drawing_column+5, bleskos_main_window_drawing_line, 295, 20, 0x00C000);
 draw_full_square(bleskos_main_window_drawing_column+5, bleskos_main_window_drawing_line, 285*sound_volume/100, 20, WHITE);
 draw_full_square(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 5, 20, BLACK);
 draw_full_square(bleskos_main_window_drawing_column+290, bleskos_main_window_drawing_line, 5, 20, BLACK);
 draw_empty_square(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, BLACK);
 if(sound_volume<10) {
  print_var(sound_volume, bleskos_main_window_drawing_column+295/2-4, bleskos_main_window_drawing_line+6, BLACK);
 }
 else if(sound_volume<100) {
  print_var(sound_volume, bleskos_main_window_drawing_column+295/2-8, bleskos_main_window_drawing_line+6, BLACK);
 }
 else {
  print("100", bleskos_main_window_drawing_column+295/2-12, bleskos_main_window_drawing_line+6, BLACK);
 }

 redraw_part_of_screen(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20);
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
 bleskos_main_window_draw_item("[f] File manager", 0xFFE800, MW_FILE_MANAGER);

 bleskos_main_window_drawing_line += 25;
 bleskos_main_window_print_item("Tools");
 bleskos_main_window_draw_item("[r] Screenshooter", 0xFFE800, MW_SCREENSHOOTER);

 bleskos_main_window_drawing_line += 25;
 bleskos_main_window_print_item("System");
 bleskos_main_window_draw_item("[F1] System board", 0xFFE800, MW_SYSTEM_BOARD);
 bleskos_main_window_draw_item("[F2] Change keyboard layout", 0xFFE800, MW_CHANGE_KEYBOARD_LAYOUT);
 bleskos_main_window_draw_item("[F12] Shutdown", 0xFFE800, MW_SHUTDOWN);
 
 bleskos_main_window_drawing_line = 40;
 bleskos_main_window_drawing_column = graphic_screen_x_center;
 bleskos_main_window_print_item("Sound volume");
 add_zone_to_click_board(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, MW_SOUND_VOLUME);
 draw_full_square(bleskos_main_window_drawing_column+5, bleskos_main_window_drawing_line, 285*sound_volume/100, 20, WHITE);
 draw_full_square(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 5, 20, BLACK);
 draw_full_square(bleskos_main_window_drawing_column+290, bleskos_main_window_drawing_line, 5, 20, BLACK);
 draw_empty_square(bleskos_main_window_drawing_column, bleskos_main_window_drawing_line, 295, 20, BLACK);
 if(sound_volume<10) {
  print_var(sound_volume, bleskos_main_window_drawing_column+295/2-4, bleskos_main_window_drawing_line+6, BLACK);
 }
 else if(sound_volume<100) {
  print_var(sound_volume, bleskos_main_window_drawing_column+295/2-8, bleskos_main_window_drawing_line+6, BLACK);
 }
 else {
  print("100", bleskos_main_window_drawing_column+295/2-12, bleskos_main_window_drawing_line+6, BLACK);
 }
 bleskos_main_window_drawing_line += 25;

 if(ethernet_selected_card!=0xFF) {
  bleskos_main_window_drawing_line += 10;
  bleskos_main_window_print_item("Network");
  if(is_ethernet_cable_connected==STATUS_FALSE) {
   bleskos_main_window_draw_item("Ethernet cable is not connected", 0x00C000, 0);
  }
  else if(connected_to_network==NETWORK_CONNECTION_FALIED) {
   bleskos_main_window_draw_item("Connection to network failed", 0x00C000, 0);
  }
  else if(connected_to_network==NETWORK_CONNECTED) {
   bleskos_main_window_draw_item("You are connected to network", 0x00C000, 0);
  }
 }

 if(ps2_first_channel_device==PS2_CHANNEL_MOUSE_INITALIZED || ps2_second_channel_device==PS2_CHANNEL_MOUSE_INITALIZED) {
  bleskos_main_window_drawing_line += 10;
  bleskos_main_window_print_item("Touchpad");
  if(ps2_mouse_enable==STATUS_TRUE) {
   bleskos_main_window_draw_item("[F10] Disable touchpad", 0x00B5FF, MW_ENABLE_DISABLE_TOUCHPAD);
  }
  else {
   bleskos_main_window_draw_item("[F10] Enable touchpad", 0x00B5FF, MW_ENABLE_DISABLE_TOUCHPAD);
  }
 }

 if(ide_cdrom_base!=0) { //TODO: also AHCI
  bleskos_main_window_drawing_line += 10;
  bleskos_main_window_print_item("Optical drive");
  bleskos_main_window_draw_item("[F11] Eject optical disk drive", 0x00B5FF, MW_EJECT_OPTICAL_DISK_DRIVE);
 }

 if(usb_controllers_pointer!=0) {
  bleskos_main_window_drawing_line += 10;
  bleskos_main_window_print_item("Connected USB devices");
  for(int i=0; i<10; i++) {
   if(usb_hubs[i].controller_type!=USB_NO_DEVICE_ATTACHED) {
    bleskos_main_window_draw_item("USB hub", 0x00B5FF, 0);
   }
  }
  if(usb_mouse[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
   bleskos_main_window_draw_item("USB mouse", 0x00B5FF, 0);
  }
  if(usb_keyboard[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
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
 }
 
 print("Keyboard layout:", 20, graphic_screen_y-30, BLACK);
 if(((dword_t)keyboard_layout_ptr)==((dword_t)english_keyboard_layout)) {
  print("English", 20+17*8, graphic_screen_y-30, BLACK);
 }
 else if(((dword_t)keyboard_layout_ptr)==((dword_t)slovak_keyboard_layout)) {
  print("Slovak", 20+17*8, graphic_screen_y-30, BLACK);
 }
 
 bleskos_main_window_redraw_time();
}

void bleskos_main_window(void) {
 read_time();

 redraw:
 bleskos_main_window_time_redraw = 0;
 bleskos_main_window_redraw();
 redraw_screen();
 bleskos_main_window_time_redraw = 1;
 
 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  if(ethernet_link_state_change==1) {
   goto redraw;
  }
  
  if(usb_new_device_detected==1) {
   goto redraw;
  }
  
  if(keyboard_value==KEY_T) {
   bleskos_main_window_time_redraw = 0;
   text_editor();
   goto redraw;
  }
  else if(keyboard_value==KEY_G) {
   bleskos_main_window_time_redraw = 0;
   graphic_editor();
   goto redraw;
  }
  else if(keyboard_value==KEY_M) {
   bleskos_main_window_time_redraw = 0;
   media_viewer();
   goto redraw;
  }
  else if(keyboard_value==KEY_I) {
   bleskos_main_window_time_redraw = 0;
   internet_browser();
   goto redraw;
  }
  else if(keyboard_value==KEY_F) {
   bleskos_main_window_time_redraw = 0;
   file_manager();
   goto redraw;
  }
  else if(keyboard_value==KEY_R) {
   bleskos_main_window_time_redraw = 0;
   screenshooter();
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
  else if(keyboard_value==KEY_F10) {
   bleskos_main_window_enable_disable_touchpad();
   goto redraw;
  }
  else if(keyboard_value==KEY_F11 && ide_cdrom_base!=0) { //TODO: also AHCI
   eject_optical_disk();
   continue;
  }
  else if(keyboard_value==KEY_F12) {
   bleskos_main_window_shutdown();
   goto redraw;
  }
  else if(keyboard_value==KEY_RIGHT) {
   if(sound_volume>90) {
    sound_volume = 100;
   }
   else {
    sound_volume += 10;
   }
   sound_set_volume(sound_volume);
   bleskos_main_window_redraw_sound_volume();
  }
  else if(keyboard_value==KEY_LEFT) {
   if(sound_volume<10) {
    sound_volume = 0;
   }
   else {
    sound_volume -= 10;
   }
   sound_set_volume(sound_volume);
   bleskos_main_window_redraw_sound_volume();
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
   else if(click_value==MW_FILE_MANAGER) {
    bleskos_main_window_time_redraw = 0;
    file_manager();
   }
   else if(click_value==MW_SCREENSHOOTER) {
    bleskos_main_window_time_redraw = 0;
    screenshooter();
   }
   else if(click_value==MW_SYSTEM_BOARD) {
    bleskos_main_window_time_redraw = 0;
    system_board();
   }
   else if(click_value==MW_CHANGE_KEYBOARD_LAYOUT) {
    bleskos_main_window_change_keyboard_layout();
   }
   else if(click_value==MW_SHUTDOWN) {
    bleskos_main_window_shutdown();
   }
   else if(click_value==MW_SOUND_VOLUME) {
    if(mouse_cursor_x<=graphic_screen_x_center+5) {
     sound_set_volume(0);
    }
    else if(mouse_cursor_x>=graphic_screen_x_center+290) {
     sound_set_volume(100);
    }
    else {
     sound_set_volume((100*(mouse_cursor_x-graphic_screen_x_center-5)/285));
    }
   }
   else if(click_value==MW_ENABLE_DISABLE_TOUCHPAD) {
    bleskos_main_window_enable_disable_touchpad();
   }
   else if(click_value==MW_EJECT_OPTICAL_DISK_DRIVE) {
    eject_optical_disk();
   }
   
   goto redraw;
  }

  //drag
  if(mouse_drag_and_drop==MOUSE_DRAG) {
   dword_t click_value = get_mouse_cursor_click_board_value();
   if(click_value==NO_CLICK) {
    continue;
   }
   else if(click_value==MW_SOUND_VOLUME) {
    if(mouse_cursor_x<=graphic_screen_x_center+5) {
     sound_set_volume(0);
    }
    else if(mouse_cursor_x>=graphic_screen_x_center+290) {
     sound_set_volume(100);
    }
    else {
     sound_set_volume((100*(mouse_cursor_x-graphic_screen_x_center-5)/285));
    }
    bleskos_main_window_redraw_sound_volume();
   }
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

 redraw_screen();

 while(1) {
  wait_for_user_input();
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

void bleskos_main_window_enable_disable_touchpad(void) {
 if(ps2_mouse_enable==STATUS_TRUE) {
  disable_ps2_mouse();
 }
 else {
  enable_ps2_mouse();
 }
}