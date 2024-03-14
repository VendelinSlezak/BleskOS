//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_keyboard(void) {
 //english layout is default
 keyboard_layout_ptr = (word_t *) (english_keyboard_layout);
 keyboard_shift_layout_ptr = (word_t *) (english_shift_keyboard_layout);

 keyboard_pressed_control_keys = 0;
 keyboard_diacritic_char_for_next_key = 0;
 keyboard_led_state = 0;
 keyboard_change_in_led_state = STATUS_FALSE;
}

void keyboard_process_code(dword_t code) {
 //save code of pressed key
 keyboard_code_of_pressed_key = code;

 //shift
 if(keyboard_code_of_pressed_key==KEY_LEFT_SHIFT || keyboard_code_of_pressed_key==KEY_RIGHT_SHIFT) {
  keyboard_pressed_control_keys |= KEYBOARD_SHIFT;
  return;
 }
 if(keyboard_code_of_pressed_key==RELEASED_KEY_CODE(KEY_LEFT_SHIFT) || keyboard_code_of_pressed_key==RELEASED_KEY_CODE(KEY_RIGHT_SHIFT)) {
  keyboard_pressed_control_keys &= ~KEYBOARD_SHIFT;
  return;
 }
 
 //ctrl
 if(keyboard_code_of_pressed_key==KEY_LEFT_CTRL || keyboard_code_of_pressed_key==KEY_RIGHT_CTRL) {
  keyboard_pressed_control_keys |= KEYBOARD_CTRL;
  return;
 }
 if((keyboard_code_of_pressed_key==RELEASED_KEY_CODE(KEY_LEFT_CTRL) || keyboard_code_of_pressed_key==RELEASED_KEY_CODE(KEY_RIGHT_CTRL))) {
  keyboard_pressed_control_keys &= ~KEYBOARD_CTRL;
  return;
 }

 //alt
 if(keyboard_code_of_pressed_key==KEY_LEFT_ALT || keyboard_code_of_pressed_key==KEY_RIGHT_ALT) {
  keyboard_pressed_control_keys |= KEYBOARD_ALT;
  return;
 }
 if(keyboard_code_of_pressed_key==RELEASED_KEY_CODE(KEY_LEFT_ALT) || keyboard_code_of_pressed_key==RELEASED_KEY_CODE(KEY_RIGHT_ALT)) {
  keyboard_pressed_control_keys &= ~KEYBOARD_ALT;
  return;
 }
 
 //capslock
 if(keyboard_code_of_pressed_key==KEY_CAPSLOCK) {
  keyboard_change_in_led_state = STATUS_TRUE;

  //reverse shift
  if((keyboard_pressed_control_keys & KEYBOARD_CAPSLOCK)==0) {
   keyboard_pressed_control_keys |= KEYBOARD_CAPSLOCK;
   keyboard_led_state |= KEYBOARD_LED_CAPSLOCK;
   return;
  }
  else {
   keyboard_led_state &= ~KEYBOARD_LED_CAPSLOCK;
   keyboard_pressed_control_keys &= ~KEYBOARD_CAPSLOCK;
   return;
  }
 }
 
 //get unicode value
 if((keyboard_code_of_pressed_key & 0xFF)>0x80) {
  keyboard_unicode_value_of_pressed_key = 0; //released keys do not have unicode value
  return;
 }
 if((keyboard_pressed_control_keys & (KEYBOARD_SHIFT | KEYBOARD_CAPSLOCK))==0 || (keyboard_pressed_control_keys & (KEYBOARD_SHIFT | KEYBOARD_CAPSLOCK))==(KEYBOARD_SHIFT | KEYBOARD_CAPSLOCK)) {
  keyboard_unicode_value_of_pressed_key = keyboard_layout_ptr[code];
 }
 else if((keyboard_pressed_control_keys & (KEYBOARD_SHIFT | KEYBOARD_CAPSLOCK))==KEYBOARD_SHIFT || (keyboard_pressed_control_keys & (KEYBOARD_SHIFT | KEYBOARD_CAPSLOCK))==KEYBOARD_CAPSLOCK) {
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
}