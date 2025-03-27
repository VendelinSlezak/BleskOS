//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_mouse(void) {
    mouse_click_button_state = MOUSE_NO_EVENT;
    mouse_buttons = 0;
    mouse_movement_x = 0;
    mouse_movement_y = 0;
    mouse_wheel_movement = 0;
}

void mouse_prepare_for_next_event(void) {
    mouse_event = STATUS_FALSE;
}

void mouse_update_click_button_state(void) {
    if((mouse_buttons & 0x1)==0x0) {
        mouse_click_button_state = MOUSE_NO_DRAG;
    }
    else {
        if(mouse_click_button_state==MOUSE_NO_DRAG) {
            mouse_click_button_state = MOUSE_CLICK;
            mouse_cursor_x_click = mouse_cursor_x;
            mouse_cursor_y_click = mouse_cursor_y;
        }
        else {
            mouse_click_button_state = MOUSE_DRAG;
        }
    }
}