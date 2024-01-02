//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ps2_keyboard(void) {
 //initalize device on first channel
 if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_CONNECTED) {
  //translation for first channel is already enabled, so keyboard will send keys according to scan code set 1

  //enable streaming
  write_to_first_ps2_channel(0xF4);
  if(ps2_first_channel_wait_for_ack()==STATUS_GOOD) {
   ps2_first_channel_buffer_pointer = 0;
   ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; //keyboard was successfully initalized
  }
 }

 //initalize device on second channel
 if(ps2_second_channel_device==PS2_CHANNEL_KEYBOARD_CONNECTED) {
  //try to set scan code set 1 because second channel do not have translation to scan code set 1
  write_to_second_ps2_channel(0xF0);
  if(ps2_second_channel_wait_for_ack()==STATUS_ERROR) {
   return;
  }
  write_to_second_ps2_channel(1);
  if(ps2_second_channel_wait_for_ack()==STATUS_ERROR) {
   return; //TODO: support translation from other scan code sets to scan code set 1
  }

  //enable streaming
  write_to_second_ps2_channel(0xF4);
  if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
   ps2_second_channel_buffer_pointer = 0;
   ps2_second_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; //keyboard was successfully initalized
  }
 }
}