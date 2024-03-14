//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void wait_for_user_input(void) {
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
 keyboard_code_of_pressed_key = 0;
 keyboard_unicode_value_of_pressed_key = 0;
 mouse_movement_x = 0;
 mouse_movement_y = 0;
 mouse_wheel = 0;
 
 while(usb_mouse_packet_received==STATUS_FALSE && usb_keyboard_packet_received==STATUS_FALSE && ps2_mouse_wait==1 && ps2_keyboard_wait==1 && usb_new_device_detected==STATUS_FALSE && ethernet_link_state_change==0) {
  asm("hlt");

  //run tasks in scheduler
  scheduler_user_input();

  if(is_ethernet_cable_connected==STATUS_TRUE && connected_to_network==STATUS_FALSE) {
   connect_to_network_with_message();
   break;
  }

  if(usb_new_device_detected==STATUS_TRUE) {
   break;
  }
 }

 //detect changes in USB devices
 if(usb_new_device_detected==STATUS_TRUE) {
  detect_usb_devices();
 }
 
 //process PS/2 mouse (touchpad) event
 if(ps2_mouse_wait==0) {
  ps2_mouse_convert_received_data();
 }
 
 //process USB mouse event
 if(usb_mouse_packet_received==STATUS_TRUE) {
  usb_mouse_process_received_data();
 }
 
 //process USB keyboard event
 if(usb_keyboard_packet_received==STATUS_TRUE) {
  keyboard_process_code(usb_keyboard_code_of_pressed_key);
 }

 //make screenshot
 if(keyboard_code_of_pressed_key==KEY_PRINT_SCREEN) {
  copy_memory((dword_t)screen_double_buffer_memory_pointer, screenshoot_image_info_data_mem, screenshoot_image_info_data_length);
  show_system_message("You made a screenshot");
  wait(500);
  remove_system_message();
  screenshot_was_made = STATUS_TRUE;
  screenshot_is_cropped = STATUS_FALSE;
 }
}