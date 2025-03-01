//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ps2_keyboard(void) {
 //clear variables
 clear_memory((dword_t)&ps2_keyboard_keys, sizeof(struct keyboard_keys_t));
 ps2_keyboard_leds.capslock = STATUS_FALSE;
 ps2_keyboard_leds.numlock = STATUS_FALSE;
 ps2_keyboard_leds.scrollock = STATUS_FALSE;

 //initalize device
 if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_CONNECTED) {
  //translation for first channel is already enabled, so keyboard will send keys according to scan code set 1

  //enable streaming
  write_to_first_ps2_channel(0xF4);
  if(ps2_first_channel_wait_for_ack()==STATUS_GOOD) {
   ps2_first_channel_buffer_pointer = 0;
   ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_INITALIZED; //keyboard was successfully initalized
  }
 }

 //add task for changing PS/2 keyboard LEDs
 if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED || ps2_second_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
  create_task(ps2_keyboard_check_led_change, TASK_TYPE_USER_INPUT, 16);
 }
}

void ps2_keyboard_check_led_change(void) {
 //check if LEDs state is same as state of system keyboard
 if(ps2_keyboard_leds.capslock != keyboard_led_state.capslock
    || ps2_keyboard_leds.numlock != keyboard_led_state.numlock
    || ps2_keyboard_leds.scrollock != keyboard_led_state.scrollock) {
  ps2_keyboard_set_leds();
 }
}

void ps2_keyboard_set_leds(void) {
 //create data that will be sended to keyboard
 byte_t data = 0;
 if(keyboard_led_state.scrollock == STATUS_TRUE) {
  data |= (1 << 0);
 }
 if(keyboard_led_state.numlock == STATUS_TRUE) {
  data |= (1 << 1);
 }
 if(keyboard_led_state.capslock == STATUS_TRUE) {
  data |= (1 << 2);
 }

 //set LED state for keyboard
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

 //update variables
 ps2_keyboard_leds.capslock = keyboard_led_state.capslock;
 ps2_keyboard_leds.numlock = keyboard_led_state.numlock;
 ps2_keyboard_leds.scrollock = keyboard_led_state.scrollock;
}

void ps2_keyboard_process_key_value(dword_t key_value) {
 //PAUSE key do not signalize release
 if(key_value > 0xFFFF) {
  return;
 }
 
 //key was pressed
 if((key_value & 0xFF)<0x80) {
  //check if this is not control key
  if(key_value==KEY_LEFT_SHIFT || key_value==KEY_RIGHT_SHIFT) {
   ps2_keyboard_keys.shift = STATUS_TRUE;
   keyboard_update_keys_state();
   return;
  }
  else if(key_value==KEY_LEFT_CTRL || key_value==KEY_RIGHT_CTRL) {
   ps2_keyboard_keys.ctrl = STATUS_TRUE;
   keyboard_update_keys_state();
   return;
  }
  else if(key_value==KEY_LEFT_ALT || key_value==KEY_RIGHT_ALT) {
   ps2_keyboard_keys.alt = STATUS_TRUE;
   keyboard_update_keys_state();
   return;
  }
  else if(key_value==KEY_CAPSLOCK) {
   keyboard_led_state.capslock = ~keyboard_led_state.capslock; //reverse state
   return;
  }

  //check if key is not already pressed
  for(dword_t i=0; i<6; i++) {
   if(ps2_keyboard_keys.pressed_keys[i] == key_value) {
    return;
   }
  }

  //add key to list
  for(dword_t i=0; i<5; i++) {
   ps2_keyboard_keys.pressed_keys[i+1] = ps2_keyboard_keys.pressed_keys[i];
  }
  ps2_keyboard_keys.pressed_keys[0] = key_value;
 }
 //key was released
 else {
  //recalculate key value to pressed key value
  key_value = (key_value-0x80);

  //check if this is not control key
  if(key_value==KEY_LEFT_SHIFT || key_value==KEY_RIGHT_SHIFT) {
   ps2_keyboard_keys.shift = STATUS_FALSE;
   return;
  }
  else if(key_value==KEY_LEFT_CTRL || key_value==KEY_RIGHT_CTRL) {
   ps2_keyboard_keys.ctrl = STATUS_FALSE;
   return;
  }
  else if(key_value==KEY_LEFT_ALT || key_value==KEY_RIGHT_ALT) {
   ps2_keyboard_keys.alt = STATUS_FALSE;
   return;
  }

  //check where do we have this key in list and remove it
  for(dword_t i=0; i<6; i++) {
   if(ps2_keyboard_keys.pressed_keys[i] == key_value) {
    ps2_keyboard_keys.pressed_keys[i] = 0;
    return;
   }
  }

 }
}