//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_keyboard(void) {
 //english layout is default
 keyboard_layout_ptr = (word_t *) (english_keyboard_layout);
 keyboard_shift_layout_ptr = (word_t *) (english_shift_keyboard_layout);

 //clear variables
 keyboard_diacritic_char_for_next_key = 0;
 keyboard_event = STATUS_FALSE;
 keyboard_led_state.capslock = STATUS_FALSE;
 keyboard_led_state.scrollock = STATUS_FALSE;
 keyboard_led_state.numlock = STATUS_FALSE;
}

void keyboard_prepare_for_next_event(void) {
 keyboard_event = STATUS_FALSE;
 keyboard_code_of_pressed_key = 0;
 keyboard_unicode_value_of_pressed_key = 0;
}

void keyboard_update_keys_state(void) {
 keyboard_keys_state.shift = 0;
 keyboard_keys_state.ctrl = 0;
 keyboard_keys_state.alt = 0;

 keyboard_keys_state.shift |= ps2_keyboard_keys.shift;
 keyboard_keys_state.ctrl |= ps2_keyboard_keys.ctrl;
 keyboard_keys_state.alt |= ps2_keyboard_keys.alt;

 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].keyboard.is_present == STATUS_TRUE) {
   keyboard_keys_state.shift |= usb_devices[i].keyboard.keys.shift;
   keyboard_keys_state.ctrl |= usb_devices[i].keyboard.keys.ctrl;
   keyboard_keys_state.alt |= usb_devices[i].keyboard.keys.alt;
  }
 }
}

void keyboard_process_code(dword_t code) {
 //save code of pressed key
 keyboard_code_of_pressed_key = code;
 
 //get unicode value
 if((keyboard_code_of_pressed_key & 0xFF)>0x80) {
  keyboard_unicode_value_of_pressed_key = 0; //released keys do not have unicode value
  return;
 }
 if(keyboard_keys_state.shift == keyboard_led_state.capslock) {
  keyboard_unicode_value_of_pressed_key = keyboard_layout_ptr[code];
 }
 else {
  keyboard_unicode_value_of_pressed_key = keyboard_shift_layout_ptr[code];
 }

 //convert char with diacritic
 if(keyboard_unicode_value_of_pressed_key==UNICODE_COMBINING_ACUTE || keyboard_unicode_value_of_pressed_key==UNICODE_COMBINING_CARON) {
  keyboard_diacritic_char_for_next_key = keyboard_unicode_value_of_pressed_key;
  keyboard_unicode_value_of_pressed_key = 0;
 }
 else {
  keyboard_unicode_value_of_pressed_key = get_unicode_char_with_diacritic(keyboard_unicode_value_of_pressed_key, keyboard_diacritic_char_for_next_key);
  keyboard_diacritic_char_for_next_key = 0;
 }

 //inform wait_for_user() from source/drivers/system/user_input.c that there was key pressed on keyboard
 keyboard_event = STATUS_TRUE;
}

byte_t keyboard_is_key_pressed(dword_t key_value) {
 //check PS/2 keyboard
 for(dword_t i=0; i<6; i++) {
  if(ps2_keyboard_keys.pressed_keys[i]==key_value) {
   return STATUS_TRUE;
  }
 }

 //check USB keyboards
 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  if(usb_devices[i].keyboard.is_present == STATUS_TRUE) {
   //check USB keyboard
   for(dword_t i=0; i<6; i++) {
    if(usb_devices[i].keyboard.keys.pressed_keys[i]==key_value) {
     return STATUS_TRUE;
    }
   }
  }
 }

}