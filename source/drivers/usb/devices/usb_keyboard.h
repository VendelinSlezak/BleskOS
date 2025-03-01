//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_KEYBOARD_ZERO_10 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
dword_t usb_keyboard_layout[256] = {
0, 0, 0, 0,
KEY_A,  KEY_B,  KEY_C,  KEY_D,  KEY_E,  KEY_F,  KEY_G,  KEY_H,  KEY_I,  KEY_J,  KEY_K,  KEY_L,  KEY_M,  KEY_N,  KEY_O,  KEY_P,  KEY_Q,  KEY_R,  KEY_S,  KEY_T,  KEY_U,  KEY_V,  KEY_W,  KEY_X,  KEY_Y,  KEY_Z,
KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE,
KEY_DASH, KEY_EQUAL, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACKSLASH, 0, KEY_SEMICOLON, KEY_SINGLE_QUOTE, KEY_BACK_TICK, KEY_COMMA, KEY_DOT, KEY_FORWARD_SLASH,
KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
KEY_PRINT_SCREEN, 0, 0, 0, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, 0, 0, //142 zero bytes
KEY_LEFT_SHIFT, 0, 0, 0, KEY_RIGHT_SHIFT, 0, 0
};

struct usb_keyboard_buffer_t {
 word_t left_ctrl: 1;
 word_t left_shift: 1;
 word_t left_alt: 1;
 word_t left_logo: 1;
 word_t right_ctrl: 1;
 word_t right_shift: 1;
 word_t right_alt: 1;
 word_t reserved: 9;
 byte_t keys[6];
}__attribute__((packed));

dword_t usb_keyboard_packet_received;

byte_t num_of_keyboards = 0;

void usb_keyboard_save_informations(byte_t device_address, struct usb_full_interface_info_t interface);

void usb_keyboard_initalize(byte_t device_address);

void usb_keyboard_set_boot_protocol_success(byte_t device_address);
void usb_keyboard_set_idle_success(byte_t device_address);
void usb_keyboard_initalization_success(byte_t device_address);
void usb_keyboard_interrupt_transfer_successfull(byte_t device_address);

void usb_keyboards_check_pressed_keys(void);
void usb_keyboard_update_led_status_with_transfer_data(byte_t device_address);
void usb_keyboard_set_led_success(byte_t device_address);
void usb_keyboard_led_interrupt_transfer_successfull(byte_t device_address);

void usb_keyboard_stop_initalization(byte_t device_address, byte_t *err_string);
void usb_keyboard_set_boot_protocol_error(byte_t device_address);
void usb_keyboard_set_idle_error(byte_t device_address);
void usb_keyboard_set_led_error(byte_t device_address);