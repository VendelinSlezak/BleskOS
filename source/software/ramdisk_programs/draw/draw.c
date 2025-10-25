/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <hardware/groups/graphic_output/graphic_output.h>

/* functions */
dword_t draw_get_default_monitor_id(void) {
    return go_get_device_id(0);
}

void draw_update_default_monitor_screen(void) {
    go_redraw_screen(go_get_device_id(0));
}

void draw_clear_screen(dword_t monitor_id, dword_t color) {
    dword_t mode = go_get_actual_mode(monitor_id);
    dword_t width = go_get_width_of_mode(monitor_id, mode);
    dword_t height = go_get_height_of_mode(monitor_id, mode);
    dword_t *buff = go_get_double_buffer(monitor_id);

    for(int i = 0; i < width*height; i++) {
        *buff = color;
        buff++;
    }
}