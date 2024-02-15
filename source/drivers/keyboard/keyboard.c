//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_keyboard(void) {
 keyboard_layout_ptr = (word_t *) english_keyboard_layout;
 keyboard_shift_layout_ptr = (word_t *) english_shift_keyboard_layout;
 keyboard_control_keys = 0;
 keyboard_diacritic = 0;
}

void keyboard_process_code(dword_t code) {
 ps2_keyboard_wait = 0;
 keyboard_value = code;
 
 //ctrl
 if(keyboard_value==KEY_LEFT_CTRL || keyboard_value==KEY_RIGHT_CTRL) {
  keyboard_control_keys |= KEYBOARD_CTRL;
  return;
 }
 if((keyboard_value==RELEASED_KEY(KEY_LEFT_CTRL) || keyboard_value==RELEASED_KEY(KEY_RIGHT_CTRL))) {
  keyboard_control_keys &= ~KEYBOARD_CTRL;
  return;
 }
 
 //left and right shift
 if(keyboard_value==KEY_LEFT_SHIFT || keyboard_value==KEY_RIGHT_SHIFT) {
  keyboard_control_keys |= KEYBOARD_SHIFT;
  return;
 }
 if(keyboard_value==RELEASED_KEY(KEY_LEFT_SHIFT) || keyboard_value==RELEASED_KEY(KEY_RIGHT_SHIFT)) {
  keyboard_control_keys &= ~KEYBOARD_SHIFT;
  return;
 }
 
 //capslock
 if(keyboard_value==KEY_CAPSLOCK) {
  if((keyboard_control_keys & KEYBOARD_SHIFT)==0) {
   keyboard_control_keys |= KEYBOARD_SHIFT;
   return;
  }
  else {
   keyboard_control_keys &= ~KEYBOARD_SHIFT;
   return;
  }
 }
 
 //get unicode value
 if((keyboard_value & 0xFF)>0x80) {
  keyboard_unicode = 0; //released key do not have unicode value
  return;
 }
 if((keyboard_control_keys & KEYBOARD_SHIFT)==0) {
  keyboard_unicode = keyboard_layout_ptr[code];
 }
 else if((keyboard_control_keys & KEYBOARD_SHIFT)==KEYBOARD_SHIFT) {
  keyboard_unicode = keyboard_shift_layout_ptr[code];
 }

 //convert char with diacritic
 if(keyboard_unicode==UNICODE_COMBINING_ACUTE || keyboard_unicode==UNICODE_COMBINING_CARON) {
  keyboard_diacritic = keyboard_unicode;
  keyboard_unicode = 0;
 }
 else {
  keyboard_unicode = get_unicode_char_with_diacritic(keyboard_unicode, keyboard_diacritic);
  keyboard_diacritic = 0;
 }
}