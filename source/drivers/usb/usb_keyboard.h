//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_KEYBOARD_UHCI_PORT 1
#define USB_KEYBOARD_OHCI_PORT 2

#define USB_KEYBOARD_ZERO_10 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

dword_t usb_keyboard_layout[256] = {
0, 0, 0, 0,
KEY_A,  KEY_B,  KEY_C,  KEY_D,  KEY_E,  KEY_F,  KEY_G,  KEY_H,  KEY_I,  KEY_J,  KEY_K,  KEY_L,  KEY_M,  KEY_N,  KEY_O,  KEY_P,  KEY_Q,  KEY_R,  KEY_S,  KEY_T,  KEY_U,  KEY_V,  KEY_W,  KEY_X,  KEY_Y,  KEY_Z,
KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE,
KEY_DASH, KEY_EQUAL, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACKSLASH, 0, KEY_SEMICOLON, KEY_SINGLE_QUOTE, KEY_BACK_TICK, KEY_COMMA, KEY_DOT, KEY_FORWARD_SLASH,
KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
0, 0, 0, 0, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, USB_KEYBOARD_ZERO_10, 0, 0, //142 zero bytes
KEY_LEFT_SHIFT, 0, 0, 0, KEY_RIGHT_SHIFT, 0, 0
};

byte_t usb_keyboard_state=0;
byte_t usb_keyboard_controller=0;
byte_t usb_keyboard_port=0;
byte_t usb_keyboard_interface=0;
byte_t usb_keyboard_endpoint=0;
byte_t usb_keyboard_endpoint_length=0;
byte_t usb_keyboard_endpoint_time=0;
byte_t usb_keyboard_endpoint_out=0;
dword_t usb_keyboard_data_memory=0;
dword_t usb_keyboard_check_transfer_descriptor=0;

dword_t usb_keyboard_count=0;
byte_t usb_keyboard_code=0;
dword_t usb_keyboard_value=0;
byte_t usb_keyboard_wait=0;

void wait_for_usb_keyboard(void);
