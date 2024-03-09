//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_mouse_read_hid_descriptor(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t interface, word_t length) {
 usb_control_transfer_with_fixed_data_length(controller_number, device_address, device_speed, 0x81, 0x06, 0x2200, interface, length);
 parse_hid_descriptor(usb_setup_packet_data_mem);
}

void usb_hid_device_set_protocol(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t interface, byte_t protocol) {
 usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x21, 0x0A, protocol, interface);
}

void usb_hid_device_set_idle(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t interface) {
 usb_control_transfer_without_data(controller_number, device_address, device_speed, 0x21, 0x0A, 0x0000, interface);
}

void usb_keyboard_process_new_packet(void) {
 byte_t *usb_keyboard_mem = (byte_t *) (usb_keyboard_data_memory);
 
 if(usb_keyboard_mem[2]!=usb_keyboard_code) { //if not same key   
  //process modifier keys
  if((usb_keyboard_mem[0] & 0x11)!=0x00) {
   keyboard_control_keys |= KEYBOARD_CTRL;
  }
  else {
   keyboard_control_keys &= ~KEYBOARD_CTRL;
  }
  if((usb_keyboard_mem[0] & 0x22)!=0x00) {
   keyboard_control_keys |= KEYBOARD_SHIFT;
  }
  else {
   keyboard_control_keys &= ~KEYBOARD_SHIFT;
  }
  if((usb_keyboard_mem[0] & 0x44)!=0x00) {
   keyboard_control_keys |= KEYBOARD_ALT;
  }
  else {
   keyboard_control_keys &= ~KEYBOARD_ALT;
  }

  usb_keyboard_code = usb_keyboard_mem[2];
  usb_keyboard_value = usb_keyboard_layout[usb_keyboard_code];
  usb_keyboard_count = 0;
  usb_keyboard_packet_received = STATUS_TRUE;
 }
}

void usb_keyboard_process_no_new_packet(void) {
 usb_keyboard_count++;
    
 if(usb_keyboard_count>250+20) { //repeat key after 500 ms every 40 ms
  usb_keyboard_packet_received = STATUS_TRUE;
  usb_keyboard_count = 250;
 }
}

void wait_for_user_input(void) {
 dword_t *mouse_data_mem = (dword_t *) (usb_mouse_data_memory);
 byte_t *keyboard_data_mem = (byte_t *) (usb_keyboard_data_memory);
 
 for(int i=0; i<4; i++) {
  ps2_mouse_data[i]=0;
 }
 for(int i=0; i<8; i++) {
  keyboard_data_mem[i]=0;
 }

 usb_mouse_packet_received = STATUS_FALSE;
 usb_keyboard_packet_received = STATUS_FALSE;
 ps2_mouse_wait = 1;
 ps2_keyboard_wait = 1;
 usb_new_device_detected = STATUS_FALSE;
 ethernet_link_state_change = 0;
 keyboard_value = 0;
 keyboard_unicode = 0;
 mouse_movement_x = 0;
 mouse_movement_y = 0;
 mouse_wheel = 0;
 
 while(usb_mouse_packet_received==STATUS_FALSE && usb_keyboard_packet_received==STATUS_FALSE && ps2_mouse_wait==1 && ps2_keyboard_wait==1 && usb_new_device_detected==STATUS_FALSE && ethernet_link_state_change==0) {
  asm("hlt");

  if(is_ethernet_cable_connected==STATUS_TRUE && connected_to_network==STATUS_FALSE) {
   connect_to_network_with_message();
   break;
  }

  if((ticks_of_processes & 0x80)==0x80) {
   detect_usb_devices_on_hubs(); //detect devices connected to USB HUBs
   if(usb_new_device_detected==STATUS_TRUE) {
    break;
   }
  }

  if(sound_card_detect_headphone_connection_status==STATUS_TRUE) {
   if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
    if(ac97_selected_output==AC97_SPEAKER_OUTPUT && ac97_is_headphone_connected()==STATUS_TRUE) { //headphone was connected
     ac97_set_output(AC97_HEADPHONE_OUTPUT);
    }
    else if(ac97_selected_output==AC97_HEADPHONE_OUTPUT && ac97_is_headphone_connected()==STATUS_FALSE) { //headphone was disconnected
     ac97_set_output(AC97_SPEAKER_OUTPUT);
    }
   }
   else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
    if(hda_selected_output_node==hda_pin_output_node_number && hda_is_headphone_connected()==STATUS_TRUE) { //headphone was connected
     hda_disable_pin_output(hda_codec_number, hda_pin_output_node_number);
     hda_enable_pin_output(hda_codec_number, hda_pin_headphone_node_number);
     hda_selected_output_node = hda_pin_headphone_node_number;
    }
    else if(hda_selected_output_node==hda_pin_headphone_node_number && hda_is_headphone_connected()==STATUS_FALSE) { //headphone was disconnected
     hda_disable_pin_output(hda_codec_number, hda_pin_headphone_node_number);
     hda_enable_pin_output(hda_codec_number, hda_pin_output_node_number);
     hda_selected_output_node = hda_pin_output_node_number;
    }
   }
  }
 }

 //detect changes in USB devices
 if(usb_new_device_detected==STATUS_TRUE) {
  detect_usb_devices();
 }
 
 //process PS/2 mouse (touchpad) event
 if(ps2_mouse_wait==0) {
  //convert data
  mouse_buttons = ps2_mouse_data[0];
  if(ps2_mouse_data[1]<0x80) {
   mouse_movement_x = ps2_mouse_data[1];
  }
  else {
   mouse_movement_x = (0xFFFFFFFF - (0xFF-ps2_mouse_data[1]));
  }
  if(ps2_mouse_data[2]<0x80) {
   mouse_movement_y = (0xFFFFFFFF - ps2_mouse_data[2] + 1);
  }
  else {
   mouse_movement_y = (0x100-ps2_mouse_data[2]);
  }
  if(ps2_mouse_data[3]!=0) {
   if(ps2_mouse_data[3]<0x80) {
    mouse_wheel = (0xFFFFFFFF - ps2_mouse_data[3]);
   }
   else {
    mouse_wheel = (0xFF-ps2_mouse_data[3]+1);
   }
  }
  
  //drag and drop
  if((mouse_buttons & 0x1)==0x0) {
   mouse_click_button_state = MOUSE_NO_DRAG;
  }
  else {
   if(mouse_click_button_state==MOUSE_NO_DRAG) {
    mouse_click_button_state = MOUSE_CLICK;
    mouse_cursor_x_click = mouse_cursor_x;
    mouse_cursor_y_click = mouse_cursor_y;
   }
   else {
    mouse_click_button_state = MOUSE_DRAG;
   }
  }
  
  return;
 }
 
 //process USB mouse event
 if(usb_mouse_packet_received==STATUS_TRUE) {
  //convert data
  mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_buttons_data_offset_byte);
  mouse_buttons = ((*mouse_data_mem >> usb_mouse_buttons_data_offset_shift) & 0x1F);
  
  //drag and drop
  if((mouse_buttons & 0x1)==0x0) {
   mouse_click_button_state = MOUSE_NO_DRAG;
  }
  else {
   if(mouse_click_button_state==MOUSE_NO_DRAG) {
    mouse_click_button_state = MOUSE_CLICK;
    mouse_cursor_x_click = mouse_cursor_x;
    mouse_cursor_y_click = mouse_cursor_y;
   }
   else {
    mouse_click_button_state = MOUSE_DRAG;
   }
  }
  
  mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_movement_x_data_offset_byte);
  mouse_movement_x = ((*mouse_data_mem >> usb_mouse_movement_x_data_offset_shift) & usb_mouse_movement_x_data_length);
  if(mouse_movement_x>((usb_mouse_movement_x_data_length+1)/2)) {
   mouse_movement_x = (0xFFFFFFFF - (usb_mouse_movement_x_data_length-mouse_movement_x));
  }
  
  mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_movement_y_data_offset_byte);
  mouse_movement_y = ((*mouse_data_mem >> usb_mouse_movement_y_data_offset_shift) & usb_mouse_movement_y_data_length);
  if(mouse_movement_y>((usb_mouse_movement_y_data_length+1)/2)) {
   mouse_movement_y = (0xFFFFFFFF - (usb_mouse_movement_y_data_length-mouse_movement_y));
  }
  
  if(usb_mouse_movement_wheel_data_length>0) {
   mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_movement_wheel_data_offset_byte);
   mouse_wheel = ((*mouse_data_mem >> usb_mouse_movement_wheel_data_offset_shift) & usb_mouse_movement_wheel_data_length);
   if(mouse_wheel>((usb_mouse_movement_wheel_data_length+1)/2)) {
    mouse_wheel = (0xFFFFFFFF - (usb_mouse_movement_wheel_data_length-mouse_wheel));
   }
  }
 }
 
 //process USB keyboard event
 if(usb_keyboard_packet_received==STATUS_TRUE) {
  keyboard_process_code(usb_keyboard_value);
 }

 //make screenshot
 if(keyboard_value==KEY_PRINT_SCREEN) {
  copy_memory((dword_t)screen_double_buffer_memory_pointer, screenshoot_image_info_data_mem, screenshoot_image_info_data_length);
  show_system_message("You made a screenshot");
  wait(500);
  remove_system_message();
  screenshot_was_made = STATUS_TRUE;
  screenshot_is_cropped = STATUS_FALSE;
 }
}