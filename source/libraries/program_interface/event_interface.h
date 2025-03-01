//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

enum {
 END_OF_EVENTS = 0,
 KEYBOARD_EVENT_PRESSED_KEY, //number of pressed key, method to be called, return value
 KEYBOARD_EVENT_PRESSED_KEY_WITH_CONTROL_KEY, //control key, number of pressed key, method to be called, return value
 MOUSE_EVENT_CLICK_ON_ZONE, //number of click zone, method to be called, return value
 MOUSE_EVENT_CLICK_ON_ZONES, //number of first click zone in range, number of last click zone in range, method to be called, return value
 mouse_wheel_movement_EVENT, //method to be called, return value
 VERTICAL_SCROLLBAR_EVENT, //click zone of scrollbar, pointer to scrollbar info, method to be called if there is change on scrollbar
 HORIZONTAL_SCROLLBAR_EVENT, //click zone of scrollbar, pointer to scrollbar info, method to be called if there is change on scrollbar
 TEXT_AREA_WITH_PERMANENT_FOCUS_EVENT, //pointer to text area info
 TEXT_AREA_EVENT,
};

enum {
 NO_EVENT = 0,
 RETURN_EVENT_FROM_METHOD,
 EVENT_EXIT,
 EVENT_USB_DEVICE_CHANGE,
 EVENT_REDRAW,
 PROGRAM_DEFINED_EVENTS,
};

dword_t wait_for_event(dword_t *event_interface, void (*redraw_method)(void));
dword_t wait_for_one_event(dword_t *event_interface);

void event_interface_set_text_area(dword_t *event_interface, dword_t text_area_pointer);