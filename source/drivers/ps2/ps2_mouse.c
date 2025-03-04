//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ps2_mouse(void) {
 ps2_mouse_enable = STATUS_TRUE;

 //initalize device
 if(ps2_second_channel_device==PS2_CHANNEL_MOUSE_CONNECTED) {
  //send request to enable wheel
  write_to_second_ps2_channel(0xF3);
  ps2_second_channel_wait_for_ack();
  write_to_second_ps2_channel(200);
  ps2_second_channel_wait_for_ack();
  write_to_second_ps2_channel(0xF3);
  ps2_second_channel_wait_for_ack();
  write_to_second_ps2_channel(100);
  ps2_second_channel_wait_for_ack();
  write_to_second_ps2_channel(0xF3);
  ps2_second_channel_wait_for_ack();
  write_to_second_ps2_channel(80);
  ps2_second_channel_wait_for_ack();

  //read device ID to see if device accepted wheel request
  write_to_second_ps2_channel(0xF2);
  if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
   if(ps2_second_channel_wait_for_response()==STATUS_GOOD) {
    if(ps2_second_channel_buffer[1]==3 || ps2_second_channel_buffer[1]==4) { //mouse sends 4 bytes
     ps2_second_channel_mouse_data_bytes = 4;
    }
    else if(ps2_second_channel_buffer[1]==0) { //mouse sends 3 bytes
     ps2_second_channel_mouse_data_bytes = 3;
    }
    else { //unknown device ID
     return;
    }

    //enable streaming
    write_to_second_ps2_channel(0xF4);
    if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
     ps2_second_channel_buffer_pointer = 0;
     ps2_second_channel_device = PS2_CHANNEL_MOUSE_INITALIZED; //mouse was successfully initalized
    }
   }
  }
 }
}

void enable_ps2_mouse(void) {
 if(ps2_second_channel_device==PS2_CHANNEL_MOUSE_INITALIZED) {
  ps2_second_channel_buffer_pointer = 0;
 }
 ps2_mouse_enable = STATUS_TRUE;
}

void disable_ps2_mouse(void) {
 ps2_mouse_enable = STATUS_FALSE;
}

void ps2_mouse_convert_received_data(void) {
 //buttons
 mouse_buttons = ps2_mouse_data[0];

 //X movement
 if(ps2_mouse_data[1]<0x80) {
  mouse_movement_x = ps2_mouse_data[1];
 }
 else {
  mouse_movement_x = (0xFFFFFFFF - (0xFF-ps2_mouse_data[1]));
 }

 //Y movement
 if(ps2_mouse_data[2]<0x80) {
  mouse_movement_y = (0xFFFFFFFF - ps2_mouse_data[2] + 1);
 }
 else {
  mouse_movement_y = (0x100-ps2_mouse_data[2]);
 }

 //wheel
 if(ps2_mouse_data[3]!=0) {
  if(ps2_mouse_data[3]<0x80) {
   mouse_wheel_movement = (0xFFFFFFFF - ps2_mouse_data[3]);
  }
  else {
   mouse_wheel_movement = (0xFF-ps2_mouse_data[3]+1);
  }
 }

 //click button state
 mouse_update_click_button_state();
}