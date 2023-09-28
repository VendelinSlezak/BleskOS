//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define USB_MOUSE_UHCI_PORT 1
#define USB_MOUSE_OHCI_PORT 2

byte_t usb_mouse_state=0;
byte_t usb_mouse_controller=0;
byte_t usb_mouse_port=0;
byte_t usb_mouse_endpoint=0;
byte_t usb_mouse_endpoint_length=0;
byte_t usb_mouse_endpoint_time=0;
dword_t usb_mouse_data_memory=0;
dword_t usb_mouse_check_transfer_descriptor=0;

byte_t usb_mouse_wait=0;

dword_t usb_mouse_buttons_data_offset_byte;
dword_t usb_mouse_buttons_data_offset_shift;
dword_t usb_mouse_movement_x_data_offset_byte;
dword_t usb_mouse_movement_x_data_offset_shift;
dword_t usb_mouse_movement_x_data_length;
dword_t usb_mouse_movement_y_data_offset_byte;
dword_t usb_mouse_movement_y_data_offset_shift;
dword_t usb_mouse_movement_y_data_length;
dword_t usb_mouse_movement_wheel_data_offset_byte;
dword_t usb_mouse_movement_wheel_data_offset_shift;
dword_t usb_mouse_movement_wheel_data_length;

void wait_for_usb_mouse(void);
