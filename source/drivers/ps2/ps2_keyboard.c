//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
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
  if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
   write_to_second_ps2_channel(1);
   if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
    //enable streaming
    write_to_second_ps2_channel(0xF4);
    if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
     ps2_second_channel_buffer_pointer = 0;
     ps2_second_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; //keyboard was successfully initalized
    }
   }
  }
 }

 //add task for changing PS/2 keyboard LEDs
 if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED || ps2_second_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
  create_task(ps2_keyboard_check_led_change, TASK_TYPE_USER_INPUT, 16);
 }
}

void ps2_keyboard_set_leds(void) {
 byte_t data = 0;
 if((keyboard_led_state & KEYBOARD_LED_SCROLLOCK)==KEYBOARD_LED_SCROLLOCK) {
  data |= 0x1;
 }
 if((keyboard_led_state & KEYBOARD_LED_NUMBERLOCK)==KEYBOARD_LED_NUMBERLOCK) {
  data |= 0x2;
 }
 if((keyboard_led_state & KEYBOARD_LED_CAPSLOCK)==KEYBOARD_LED_CAPSLOCK) {
  data |= 0x4;
 }

 //set LED state for keyboard on first channel
 if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
  ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_CONNECTED; //ACK will not be recognized as pressed key

  write_to_first_ps2_channel(0xED);
  if(ps2_first_channel_wait_for_ack()==STATUS_GOOD) {
   write_to_first_ps2_channel(data);
   if(ps2_first_channel_wait_for_ack()==STATUS_ERROR) {
    log("\nPS/2: LED change data not ACKed");
   }
  }
  else {
   log("\nPS/2: 0xED not ACKed");
  }

  ps2_first_channel_buffer_pointer = 0;
  ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; //keyboard can function normally
 }

 //set LED state for keyboard on second channel
 if(ps2_second_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
  ps2_second_channel_device = PS2_CHANNEL_KEYBOARD_CONNECTED; //ACK will not be recognized as pressed key

  write_to_second_ps2_channel(0xED);
  if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
   write_to_second_ps2_channel(data);
   if(ps2_second_channel_wait_for_ack()==STATUS_ERROR) {
    log("\nPS/2: LED change data not ACKed");
   }
  }
  else {
   log("\nPS/2: 0xED not ACKed");
  }

  ps2_second_channel_buffer_pointer = 0;
  ps2_second_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; //keyboard can function normally
 }

 keyboard_change_in_led_state = STATUS_FALSE;
}

void ps2_keyboard_check_led_change(void) {
 if(keyboard_change_in_led_state==STATUS_TRUE) {
  ps2_keyboard_set_leds();
 }
}

void ps2_keyboard_save_key_value(dword_t key_value) {
 if(key_value > 0xFFFF) {
  return;
 }
 
 //key was pressed
 if((key_value & 0xFF)<0x80) {
  for(dword_t i=0; i<number_of_keys_pressed_on_ps2_keyboard; i++) {
   if(ps2_keyboard_pressed_keys[i]==key_value) { //key was already pressed and is saved
    return;
   }
  }

  if(number_of_keys_pressed_on_ps2_keyboard<10) {
   ps2_keyboard_pressed_keys[number_of_keys_pressed_on_ps2_keyboard]=key_value;
   number_of_keys_pressed_on_ps2_keyboard++;
  }
 }
 else { //key was released
  for(dword_t i=0; i<number_of_keys_pressed_on_ps2_keyboard; i++) {
   if(ps2_keyboard_pressed_keys[i]==(key_value-0x80)) {
    remove_space_from_memory_area((dword_t)&ps2_keyboard_pressed_keys, 10*4, (dword_t)&ps2_keyboard_pressed_keys[i], 4);
    number_of_keys_pressed_on_ps2_keyboard--;
    return;
   }
  }
 }
}