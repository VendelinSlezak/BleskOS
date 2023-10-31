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

 if(usb_mouse[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
  dword_t *descriptor = (dword_t *) (usb_mouse[0].transfer_descriptor_check);

  if(usb_mouse[0].controller_type==USB_CONTROLLER_UHCI) {
   if((*descriptor & 0x00800000)==0x00000000) {
    usb_mouse_packet_received = STATUS_TRUE;

    //reverse toggle bit
    if((descriptor[1] & 0x00080000)==0x00000000) {
     descriptor[1] |= 0x00080000;
    }
    else {
     descriptor[1] &= (~0x00080000);
    }
    
    //reactivate transfer
    if(usb_mouse[0].device_speed==USB_LOW_SPEED) {
     *descriptor=0x04800000; //low speed transfer
    }
    else {
     *descriptor=0x00800000; //full speed transfer
    }
   }
  }
  else if(usb_mouse[0].controller_type==USB_CONTROLLER_OHCI) {
   dword_t *ed = (dword_t *) (usb_mouse[0].transfer_descriptor_check);

   if(ed[2]!=(ed[1]-0x10)) {
    //no error
    if((ed[2] & 0x1)==0x0) {
     usb_mouse_packet_received = STATUS_TRUE;
    }

    //reactivate transfer
    dword_t *td = (dword_t *) (usb_controllers[usb_mouse[0].controller_number].mem3+1025*16);
    td[0] = ((td[0] & 0x0FFFFFFF) | 0xE0000000);
    td[1] = usb_mouse_data_memory;
    td[2] = 0;
    td[3] = usb_mouse_data_memory+usb_mouse[0].endpoint_size-1;
    ed[2] = (ed[1]-0x10);
   }
  }
  else if(usb_mouse[0].controller_type==USB_CONTROLLER_EHCI) {
   dword_t *td = (dword_t *) (usb_mouse[0].transfer_descriptor_check);

   if((*td & 0x80)!=0x80) {
    //no error
    if((*td & 0x7C)==0x0) {
     usb_mouse_packet_received = STATUS_TRUE;
    }

    //reactivate transfer
    dword_t *qh = (dword_t *) (usb_controllers[usb_mouse[0].controller_number].mem1+1*64);
    for(dword_t i=5; i<16; i++) {
     qh[i]=0;
    }
    qh[3] = (((dword_t)td)-8);
    qh[4] = qh[3];
    td[1] = (usb_mouse_data_memory);
    td[0] = ((td[0] & 0xFE00FF00) | ((usb_mouse[0].endpoint_size)<<16) | 0x80);
   }
  }
 }

 if(usb_keyboard[0].controller_type!=USB_NO_DEVICE_ATTACHED) {
  dword_t *descriptor = (dword_t *) (usb_keyboard[0].transfer_descriptor_check);

  if(usb_keyboard[0].controller_type==USB_CONTROLLER_UHCI) {
   if((*descriptor & 0x00800000)==0x00000000) { //new packet
    usb_keyboard_process_new_packet();
    
    //reverse toggle bit
    if((descriptor[1] & 0x00080000)==0x00000000) {
     descriptor[1] |= 0x00080000;
    }
    else {
     descriptor[1] &= (~0x00080000);
    }
    
    //reactivate transfer
    if(usb_keyboard[0].device_speed==USB_LOW_SPEED) {
     *descriptor=0x04800000; //low speed transfer
    }
    else {
     *descriptor=0x00800000; //full speed transfer
    }
   }
   else if(usb_keyboard_code!=0x00) { //same key is still pressed
    usb_keyboard_process_no_new_packet();
   }
  }
  else if(usb_keyboard[0].controller_type==USB_CONTROLLER_OHCI) {
   dword_t *ed = (dword_t *) (usb_keyboard[0].transfer_descriptor_check);

   if(ed[2]!=(ed[1]-0x10)) {
    if((ed[2] & 0x1)==0x0) { //no error
     usb_keyboard_process_new_packet();
    }
    
    //reactivate transfer
    dword_t *td = (dword_t *) (usb_controllers[usb_keyboard[0].controller_number].mem3+1026*16);
    td[0] = ((td[0] & 0x0FFFFFFF) | 0xE0000000);
    td[1] = usb_keyboard_data_memory;
    td[2] = 0;
    td[3] = usb_keyboard_data_memory+usb_keyboard[0].endpoint_size-1;
    ed[2] = (ed[1]-0x10);
   }
   else if(usb_keyboard_code!=0x00) { //same key is still pressed
    usb_keyboard_process_no_new_packet();
   }
  }
  else if(usb_keyboard[0].controller_type==USB_CONTROLLER_EHCI) {
   dword_t *td = (dword_t *) (usb_keyboard[0].transfer_descriptor_check);

   if((*td & 0x80)!=0x80) {
    //no error
    if((*td & 0x7C)==0x0) {
     usb_keyboard_process_new_packet();
    }
    
    //reactivate transfer
    dword_t *qh = (dword_t *) (usb_controllers[usb_keyboard[0].controller_number].mem1+2*64);
    for(dword_t i=5; i<16; i++) {
     qh[i]=0;
    }
    qh[3] = (((dword_t)td)-8);
    qh[4] = qh[3];
    td[1] = (usb_keyboard_data_memory);
    td[0] = ((td[0] & 0xFE00FF00) | ((usb_keyboard[0].endpoint_size)<<16) | 0x80);
   }
   else if(usb_keyboard_code!=0x00) { //same key is still pressed
    usb_keyboard_process_no_new_packet();
   }
  }
 }
}