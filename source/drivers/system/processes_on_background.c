//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void run_processes_on_background(void) {
 if(ticks_of_processes==0x80) {
  detect_status_change_of_usb_devices();
 }
 else if(ticks_of_processes==0xF0) {
  byte_t time_second_old = time_second;
  read_time_second();
  if(time_second_old!=time_second) {
   read_time();
   if(bleskos_main_window_time_redraw==1) { //redraw time on main window
    mouse_cursor_restore_background(mouse_cursor_x, mouse_cursor_y);
    draw_full_square(graphic_screen_x-20-152, graphic_screen_y-30, 152, 10, 0x00C000);
    bleskos_main_window_redraw_time();
    mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
    draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
    redraw_part_of_screen(graphic_screen_x-20-152, graphic_screen_y-30, 152, 10);
   }
  }
 }

 if(ethernet_timer_monitoring_of_line_status==STATUS_TRUE && ticks_of_processes==0x80) {
  read_ethernet_cable_status();
 }

 if(hda_playing_state==1) {
  if(mmio_ind(hda_output_stream_base + 0x04)>=hda_sound_length) {
   hda_stop_sound();
  }
 }
 else if(ac97_playing_state==1) {
  if(ticks_of_processes==0xFF) {
   ac97_fill_buffer_entry();
  }
 }
 
 if(media_viewer_sound_state==MEDIA_VIEWER_SOUND_STATE_PLAYING) {
  //update counter of played miliseconds
  set_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS, get_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS)+2);

  //everything from file was played
  if(get_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS)>=get_file_value(MEDIA_VIEWER_FILE_SOUND_LENGTH_IN_MS)) {
   set_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS, get_file_value(MEDIA_VIEWER_FILE_SOUND_LENGTH_IN_MS));

   media_viewer_sound_state = MEDIA_VIEWER_SOUND_STATE_STOPPED;
   mouse_cursor_restore_background(mouse_cursor_x, mouse_cursor_y);
   draw_media_viewer();
   mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
   draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);

   redraw_part_of_screen(10, graphic_screen_y-60, graphic_screen_x-20, 40);
  }
  else if(get_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS)>get_file_value(MEDIA_VIEWER_FILE_SOUND_NEXT_UPDATE_MS)) { //update progress in file playing
   mouse_cursor_restore_background(mouse_cursor_x, mouse_cursor_y);

   //print length of played part of file
   dword_t seconds = get_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS)%60000/1000;
   dword_t minutes = get_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS)/60000;
   dword_t hours = minutes/60;
   minutes -= (hours*60);
   draw_full_square(graphic_screen_x-10-24-24-24-24-24-16, graphic_screen_y-37, 64, 8, BLACK);
   if(seconds<10) {
    print("0", graphic_screen_x-82-16, graphic_screen_y-37, WHITE);
    print_var(seconds, graphic_screen_x-82-8, graphic_screen_y-37, WHITE);
   }
   else {
    print_var(seconds, graphic_screen_x-82-16, graphic_screen_y-37, WHITE);
   }
   if(minutes<10) {
    print("0", graphic_screen_x-82-16-24, graphic_screen_y-37, WHITE);
    print_var(minutes, graphic_screen_x-82-16-16, graphic_screen_y-37, WHITE);
   }
   else {
    print_var(minutes, graphic_screen_x-82-16-24, graphic_screen_y-37, WHITE);
   }
   if(hours<10) {
    print("0", graphic_screen_x-82-16-24-24, graphic_screen_y-37, WHITE);
    print_var(hours, graphic_screen_x-82-16-24-16, graphic_screen_y-37, WHITE);
   }
   else {
    print_var(hours, graphic_screen_x-82-16-24-24, graphic_screen_y-37, WHITE);
   }
   print(":  :", graphic_screen_x-10-24-24-24-24-24, graphic_screen_y-37, WHITE);
 
   //draw square of played part of file
   draw_full_square(11, graphic_screen_y-59, ((graphic_screen_x-22)*get_file_value(MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS)/get_file_value(MEDIA_VIEWER_FILE_SOUND_LENGTH_IN_MS)), 8, 0x0900FF);

   mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
   draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);

   redraw_part_of_screen(10, graphic_screen_y-60, graphic_screen_x-20, 40);

   set_file_value(MEDIA_VIEWER_FILE_SOUND_NEXT_UPDATE_MS, get_file_value(MEDIA_VIEWER_FILE_SOUND_NEXT_UPDATE_MS)+100);
   if(get_file_value(MEDIA_VIEWER_FILE_SOUND_NEXT_UPDATE_MS)>get_file_value(MEDIA_VIEWER_FILE_SOUND_LENGTH_IN_MS)) {
    set_file_value(MEDIA_VIEWER_FILE_SOUND_NEXT_UPDATE_MS, get_file_value(MEDIA_VIEWER_FILE_SOUND_LENGTH_IN_MS));
   }
  }
 }
 
 if(usb_mouse_state==USB_MOUSE_UHCI_PORT) {
  dword_t *descriptor = (dword_t *) usb_mouse_check_transfer_descriptor;
  if((*descriptor & 0x00800000)==0x00000000) {
   usb_mouse_wait = 0;

   //reverse toggle bit
   if((descriptor[1] & 0x00080000)==0x00000000) {
    descriptor[1] |= 0x00080000;
   }
   else {
    descriptor[1] &= (~0x00080000);
   }
   
   //reactivate transfer
   if(usb_controllers[usb_mouse_controller].ports_device_speed[usb_mouse_port]==USB_LOW_SPEED) {
    *descriptor=0x04800000; //low speed transfer
   }
   else {
    *descriptor=0x00800000; //full speed transfer
   }
  }
 }
 else if(usb_mouse_state==USB_MOUSE_OHCI_PORT) {
  dword_t *descriptor = (dword_t *) usb_mouse_check_transfer_descriptor;
  
  dword_t *ed = (dword_t *) (usb_controllers[usb_mouse_controller].mem2+1*16);
  if(ed[2]!=(ed[1]-0x10)) {
   if((ed[2] & 0x1)==0x0) { //no error
    usb_mouse_wait = 0;
   }

   //reactivate transfer
   dword_t *td = (dword_t *) (usb_controllers[usb_mouse_controller].mem3+34*16);
   td[0] = ((td[0] & 0x0FFFFFFF) | 0xE0000000);
   td[1] = usb_mouse_data_memory;
   td[2] = 0;
   td[3] = usb_mouse_data_memory+usb_mouse_endpoint_length-1;
   ed[2] = (ed[1]-0x10);
  }
 }
 
 if(usb_keyboard_state==USB_KEYBOARD_UHCI_PORT) {
  dword_t *descriptor = (dword_t *) usb_keyboard_check_transfer_descriptor;
  byte_t *usb_keyboard_mem8 = (byte_t *) usb_keyboard_data_memory;
  
  if((*descriptor & 0x00800000)==0x00000000) {   
   //process data
   if(usb_keyboard_mem8[2]!=usb_keyboard_code) { //if not same key   
    //process modifier keys
    if((usb_keyboard_mem8[0] & 0x11)!=0x00) {
     keyboard_control_keys |= KEYBOARD_CTRL;
    }
    else {
     keyboard_control_keys &= ~KEYBOARD_CTRL;
    }
    if((usb_keyboard_mem8[0] & 0x22)!=0x00) {
     keyboard_control_keys |= KEYBOARD_SHIFT;
    }
    else {
     keyboard_control_keys &= ~KEYBOARD_SHIFT;
    }
    if((usb_keyboard_mem8[0] & 0x44)!=0x00) {
     keyboard_control_keys |= KEYBOARD_ALT;
    }
    else {
     keyboard_control_keys &= ~KEYBOARD_ALT;
    }

    usb_keyboard_code = usb_keyboard_mem8[2];
    usb_keyboard_value = usb_keyboard_layout[usb_keyboard_code];
    usb_keyboard_count = 0;
    usb_keyboard_wait = 0;
   }
   
   //reverse toggle bit
   if((descriptor[1] & 0x00080000)==0x00000000) {
    descriptor[1] |= 0x00080000;
   }
   else {
    descriptor[1] &= (~0x00080000);
   }
   
   //reactivate transfer
   if(usb_controllers[usb_keyboard_controller].ports_device_speed[usb_keyboard_port]==USB_LOW_SPEED) {
    *descriptor=0x04800000; //low speed transfer
   }
   else {
    *descriptor=0x00800000; //full speed transfer
   }
  }
  else if(usb_keyboard_code!=0x00) { //same key is still pressed
   usb_keyboard_count++;
   
   if(usb_keyboard_count>250+20) { //repeat key after 500 ms every 40 ms
    usb_keyboard_wait = 0;
    usb_keyboard_count = 250;
   }
  }
 }
 else if(usb_keyboard_state==USB_KEYBOARD_OHCI_PORT) {
  dword_t *descriptor = (dword_t *) usb_keyboard_check_transfer_descriptor;
  byte_t *usb_keyboard_mem8 = (byte_t *) usb_keyboard_data_memory;
  
  dword_t *ed = (dword_t *) (usb_controllers[usb_keyboard_controller].mem2+2*16);
  if(ed[2]!=(ed[1]-0x10)) {
   if((ed[2] & 0x1)==0x0) { //no error
    //process data
    if(usb_keyboard_mem8[2]!=usb_keyboard_code) { //if not same key   
     //process modifier keys
     if((usb_keyboard_mem8[0] & 0x11)!=0x00) {
      keyboard_control_keys |= KEYBOARD_CTRL;
     }
     else {
      keyboard_control_keys &= ~KEYBOARD_CTRL;
     }
     if((usb_keyboard_mem8[0] & 0x22)!=0x00) {
      keyboard_control_keys |= KEYBOARD_SHIFT;
     }
     else {
      keyboard_control_keys &= ~KEYBOARD_SHIFT;
     }
     if((usb_keyboard_mem8[0] & 0x44)!=0x00) {
      keyboard_control_keys |= KEYBOARD_ALT;
     }
     else {
      keyboard_control_keys &= ~KEYBOARD_ALT;
     }

     usb_keyboard_code = usb_keyboard_mem8[2];
     usb_keyboard_value = usb_keyboard_layout[usb_keyboard_code];
     usb_keyboard_count = 0;
     usb_keyboard_wait = 0;
    }
   }
   
   //reactivate transfer
   dword_t *td = (dword_t *) (usb_controllers[usb_keyboard_controller].mem3+35*16);
   td[0] = ((td[0] & 0x0FFFFFFF) | 0xE0000000);
   td[1] = usb_keyboard_data_memory;
   td[2] = 0;
   td[3] = usb_keyboard_data_memory+usb_keyboard_endpoint_length-1;
   ed[2] = (ed[1]-0x10);
  }
  else if(usb_keyboard_code!=0x00) { //same key is still pressed
   usb_keyboard_count++;
   
   if(usb_keyboard_count>250+20) { //repeat key after 500 ms every 40 ms
    usb_keyboard_wait = 0;
    usb_keyboard_count = 250;
   }
  }
 }
}