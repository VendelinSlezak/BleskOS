//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define KEY_ESC 0x01
#define KEY_ENTER 0x1C
#define KEY_TAB 0x0F
#define KEY_BACKSPACE 0x0E
#define KEY_CAPSLOCK 0x3A
#define KEY_ESCAPE 0x01
#define KEY_LEFT_SHIFT 0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_LEFT_ALT 0x38
#define KEY_RIGHT_ALT 0xE038
#define KEY_LEFT_CTRL 0x1D
#define KEY_RIGHT_CTRL 0xE01D
#define KEY_NUMBERLOCK 0x45
#define KEY_SCROLLLOCK 0x46

#define KEY_HOME 0xE047
#define KEY_PAGE_UP 0xE049
#define KEY_PAGE_DOWN 0xE051
#define KEY_END 0xE04F
#define KEY_INSERT 0xE052
#define KEY_DELETE 0xE053
#define KEY_UP 0xE048
#define KEY_DOWN 0xE050
#define KEY_LEFT 0xE04B
#define KEY_RIGHT 0xE04D

#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x57
#define KEY_F12 0x58

#define KEY_A 0x1E
#define KEY_B 0x30
#define KEY_C 0x2E
#define KEY_D 0x20
#define KEY_E 0x12
#define KEY_F 0x21
#define KEY_G 0x22
#define KEY_H 0x23
#define KEY_I 0x17
#define KEY_J 0x24
#define KEY_K 0x25
#define KEY_L 0x26
#define KEY_M 0x32
#define KEY_N 0x31
#define KEY_O 0x18
#define KEY_P 0x19
#define KEY_Q 0x10
#define KEY_R 0x13
#define KEY_S 0x1F
#define KEY_T 0x14
#define KEY_U 0x16
#define KEY_V 0x2F
#define KEY_W 0x11
#define KEY_X 0x2D
#define KEY_Y 0x15
#define KEY_Z 0x2C
#define KEY_SPACE 0x39

#define KEY_1 0x02
#define KEY_2 0x03
#define KEY_3 0x04
#define KEY_4 0x05
#define KEY_5 0x06
#define KEY_6 0x07
#define KEY_7 0x08
#define KEY_8 0x09
#define KEY_9 0x0A
#define KEY_0 0x0B

#define KEY_DASH 0x0C
#define KEY_EQUAL 0x0D
#define KEY_LEFT_BRACKET 0x1A
#define KEY_RIGHT_BRACKET 0x1B
#define KEY_BACKSLASH 0x2B
#define KEY_SEMICOLON 0x27
#define KEY_SINGLE_QUOTE 0x28
#define KEY_COMMA 0x33
#define KEY_DOT 0x34
#define KEY_FORWARD_SLASH 0x35
#define KEY_BACK_TICK 0x29

#define KEY_KEYPAD_ASTERISK 0x37
#define KEY_KEYPAD_MINUS 0x4A
#define KEY_KEYPAD_PLUS 0x4E
#define KEY_KEYPAD_DOT 0x53
#define KEY_KEYPAD_ENTER 0xE01C
#define KEY_KEYPAD_FORWARD_SLASH 0xE035
#define KEY_KEYPAD_0 0x52
#define KEY_KEYPAD_1 0x4F
#define KEY_KEYPAD_2 0x50
#define KEY_KEYPAD_3 0x51
#define KEY_KEYPAD_4 0x4B
#define KEY_KEYPAD_5 0x4C
#define KEY_KEYPAD_6 0x4D
#define KEY_KEYPAD_7 0x47
#define KEY_KEYPAD_8 0x48
#define KEY_KEYPAD_9 0x49

#define KEY_POWER_BUTTON 0xE05E
#define KEY_PRINT_SCREEN 0xE037

#define RELEASED_KEY_CODE(key) (key+0x80)

word_t english_keyboard_layout[256] = {
 0, 0, //zero, escape
 '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, //backspace, tab
 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0xA, 0, //enter, left ctrl
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '`', '`', 0, '\\', //left shift
 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', //right shift
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //other control keys
 '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', //numpad
 0 //other keys
};

word_t english_shift_keyboard_layout[256] = {
 0, 0, //zero, escape
 '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0, //backspace, tab
 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0xA, 0, //enter, left ctrl
 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', //left shift
 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', //right shift
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //other control keys
 '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', //numpad
 0 //other keys
};

word_t slovak_keyboard_layout[256] = {
 0, 0, //zero, escape
 '+', 0x13E, 0x161, 0x10D, 0x165, 0x17E, 0xFD, 0xE1, 0xED, 0xE9, '=', 0x0301, 0, 0, //backspace, tab
 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0xFA, 0xE4, 0xA, 0, //enter, left ctrl
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0xF4, 0xA7, ';', 0, 0x148, //left shift
 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, '\\', 0, ' ', //right shift
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //other control keys
 '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', //numpad
 0 //other keys
};

word_t slovak_shift_keyboard_layout[256] = {
 0, 0, //zero, escape
 '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '%', 0x030C, 0, 0, //backspace, tab
 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '/', '(', 0xA, 0, //enter, left ctrl
 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '"', '!', 0xC2B0, 0, ')', //left shift
 'Y', 'X', 'C', 'V', 'B', 'N', 'M', '?', ':', '_', 0, '|', 0, ' ', //right shift
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //other control keys
 '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', //numpad
 0 //other keys
};

struct keyboard_keys_t {
 byte_t shift: 1;
 byte_t ctrl: 1;
 byte_t alt: 1;
 byte_t reserved: 5;
 dword_t pressed_keys[6];
}__attribute__((packed));

struct keyboard_leds_t {
 byte_t capslock: 1;
 byte_t numlock: 1;
 byte_t scrollock: 1;
 byte_t reserved: 5;
}__attribute__((packed));

#define KEYBOARD_CTRL 0
dword_t keyboard_code_of_pressed_key;
word_t keyboard_diacritic_char_for_next_key;
word_t keyboard_unicode_value_of_pressed_key;

struct keyboard_keys_t keyboard_keys_state;
struct keyboard_leds_t keyboard_led_state;

word_t *keyboard_layout_ptr;
word_t *keyboard_shift_layout_ptr;

byte_t keyboard_event;

void initalize_keyboard(void);
void keyboard_prepare_for_next_event(void);
void keyboard_update_keys_state(void);
void keyboard_process_code(dword_t code);
byte_t keyboard_is_key_pressed(dword_t key_value);