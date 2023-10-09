//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void wait_for_usb_mouse(void) {
 dword_t *mouse_data_mem = (dword_t *) usb_mouse_data_memory;
 byte_t *keyboard_data_mem = (byte_t *) usb_keyboard_data_memory;
 
 for(int i=0; i<4; i++) {
  ps2_mouse_data[i]=0;
 }
 
 for(int i=0; i<8; i++) {
  keyboard_data_mem[i]=0;
 }

 usb_mouse_wait = 1;
 usb_keyboard_wait = 1;
 ps2_mouse_wait = 1;
 ps2_keyboard_wait = 1;
 usb_new_device_detected = 0;
 ethernet_link_state_change = 0;
 keyboard_value = 0;
 keyboard_unicode = 0;
 mouse_movement_x = 0;
 mouse_movement_y = 0;
 mouse_wheel = 0;
 
 while(usb_mouse_wait==1 && usb_keyboard_wait==1 && ps2_mouse_wait==1 && ps2_keyboard_wait==1 && usb_new_device_detected==0 && ethernet_link_state_change==0) {
  asm("hlt");
  if(is_ethernet_cable_connected==STATUS_TRUE && connected_to_network==STATUS_FALSE) {
   connect_to_network_with_message();
   break;
  }
 }
 
 if(usb_new_device_detected==1) {
  detect_usb_devices();
 }
 
 if(ps2_keyboard_wait==0) {
  return;
 }
 
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
   mouse_drag_and_drop = MOUSE_NO_DRAG;
  }
  else {
   if(mouse_drag_and_drop==MOUSE_NO_DRAG) {
    mouse_drag_and_drop = MOUSE_CLICK;
    mouse_cursor_x_click = mouse_cursor_x;
    mouse_cursor_y_click = mouse_cursor_y;
   }
   else {
    mouse_drag_and_drop = MOUSE_DRAG;
   }
  }
  
  return;
 }
 
 if(usb_mouse_wait==0) {
  //convert data
  mouse_data_mem = (dword_t *) (usb_mouse_data_memory+usb_mouse_buttons_data_offset_byte);
  mouse_buttons = ((*mouse_data_mem >> usb_mouse_buttons_data_offset_shift) & 0x1F);
  
  //drag and drop
  if((mouse_buttons & 0x1)==0x0) {
   mouse_drag_and_drop = MOUSE_NO_DRAG;
  }
  else {
   if(mouse_drag_and_drop==MOUSE_NO_DRAG) {
    mouse_drag_and_drop = MOUSE_CLICK;
    mouse_cursor_x_click = mouse_cursor_x;
    mouse_cursor_y_click = mouse_cursor_y;
   }
   else {
    mouse_drag_and_drop = MOUSE_DRAG;
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
 
 if(usb_keyboard_wait==0) {
  keyboard_process_code(usb_keyboard_value);
 }
}
