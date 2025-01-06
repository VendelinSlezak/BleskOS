//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_mouse_read_hid_descriptor(byte_t controller_number, byte_t device_address, byte_t device_speed, byte_t interface, word_t length) {
 usb_control_transfer_with_fixed_data_length(controller_number, device_address, device_speed, 0x81, 0x06, 0x2200, interface, length);
 parse_hid_descriptor((dword_t)usb_setup_packet_data);
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
   keyboard_pressed_control_keys |= KEYBOARD_CTRL;
  }
  else {
   keyboard_pressed_control_keys &= ~KEYBOARD_CTRL;
  }
  if((usb_keyboard_mem[0] & 0x22)!=0x00) {
   keyboard_pressed_control_keys |= KEYBOARD_SHIFT;
  }
  else {
   keyboard_pressed_control_keys &= ~KEYBOARD_SHIFT;
  }
  if((usb_keyboard_mem[0] & 0x44)!=0x00) {
   keyboard_pressed_control_keys |= KEYBOARD_ALT;
  }
  else {
   keyboard_pressed_control_keys &= ~KEYBOARD_ALT;
  }

  usb_keyboard_code = usb_keyboard_mem[2];
  usb_keyboard_code_of_pressed_key = usb_keyboard_layout[usb_keyboard_code];
  for(dword_t i=0; i<6; i++) { //save all keys that are pressed
   usb_keyboard_pressed_keys[i] = usb_keyboard_layout[usb_keyboard_mem[2+i]];
  }
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

void usb_mouse_process_received_data(void) {
 dword_t *mouse_data_mem = (dword_t *) (usb_mouse_data_memory);

 //buttons
 mouse_buttons = 0;
 mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_button_1_data_offset_byte);
 mouse_buttons |= (((*mouse_data_mem >> usb_mouse_button_1_data_offset_shift) & usb_mouse_button_1_data_length)<<0);
 mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_button_2_data_offset_byte);
 mouse_buttons |= (((*mouse_data_mem >> usb_mouse_button_2_data_offset_shift) & usb_mouse_button_2_data_length)<<1);
 mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_button_3_data_offset_byte);
 mouse_buttons |= (((*mouse_data_mem >> usb_mouse_button_3_data_offset_shift) & usb_mouse_button_3_data_length)<<2);
 
 //X movement
 mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_movement_x_data_offset_byte);
 mouse_movement_x = ((*mouse_data_mem >> usb_mouse_movement_x_data_offset_shift) & usb_mouse_movement_x_data_length);
 if(mouse_movement_x>((usb_mouse_movement_x_data_length+1)/2)) {
  mouse_movement_x = (0xFFFFFFFF - (usb_mouse_movement_x_data_length-mouse_movement_x));
 }
 
 //Y movement
 mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_movement_y_data_offset_byte);
 mouse_movement_y = ((*mouse_data_mem >> usb_mouse_movement_y_data_offset_shift) & usb_mouse_movement_y_data_length);
 if(mouse_movement_y>((usb_mouse_movement_y_data_length+1)/2)) {
  mouse_movement_y = (0xFFFFFFFF - (usb_mouse_movement_y_data_length-mouse_movement_y));
 }
 
 //wheel
 if(usb_mouse_movement_wheel_data_length>0) {
  mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_movement_wheel_data_offset_byte);
  mouse_wheel = ((*mouse_data_mem >> usb_mouse_movement_wheel_data_offset_shift) & usb_mouse_movement_wheel_data_length);
  if(mouse_wheel>((usb_mouse_movement_wheel_data_length+1)/2)) {
   mouse_wheel = (0xFFFFFFFF - (usb_mouse_movement_wheel_data_length-mouse_wheel));
  }
 }

 //click button state
 mouse_update_click_button_state();
}