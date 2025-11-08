#ifndef BUILD_HARDWARE_DEVICES_MONITOR_MONITOR_H
#define BUILD_HARDWARE_DEVICES_MONITOR_MONITOR_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MONITOR_H
#define MONITOR_H

typedef struct {
    dword_t active_width;
    dword_t active_height;
    dword_t bpp;
    dword_t bytes_per_line;
    void *linear_frame_buffer;
} monitor_mode_t;

typedef struct {
    dword_t device_id;
    dword_t controller_entity;
    dword_t (*change_monitor_resolution)(monitor_mode_t mode);

    void *vm_of_liner_frame_buffer;
    void *double_buffer;
    dword_t redraw_pixels_per_line;
    dword_t redraw_padding;
    dword_t redraw_cycles;

    dword_t best_mode;
    dword_t selected_mode;
    dword_t number_of_modes;
    monitor_mode_t modes[];
} e_monitor_attr_t;

#endif
dword_t create_monitor_device_entity(dword_t controller_entity, size_t controller_functions[], dword_t number_of_modes, monitor_mode_t *modes, dword_t best_mode);
void initialize_monitor_device(void);
void *monitor_get_double_buffer(dword_t entity_number);
void monitor_redraw_screen_32bpp_no_padding(dword_t entity_number);
void monitor_redraw_screen_32bpp_no_padding_sse(dword_t entity_number);
void monitor_redraw_screen_32bpp_with_padding(dword_t entity_number);
void monitor_redraw_screen_24bpp_no_padding(dword_t entity_number);
void monitor_redraw_screen_24bpp_with_padding(dword_t entity_number);
void monitor_redraw_screen_16bpp_no_padding(dword_t entity_number);
void monitor_redraw_screen_16bpp_with_padding(dword_t entity_number);
void monitor_redraw_screen_15bpp_no_padding(dword_t entity_number);
void monitor_redraw_screen_15bpp_with_padding(dword_t entity_number);
void monitor_redraw_screen_8bpp_no_padding(dword_t entity_number);
void monitor_redraw_screen_8bpp_with_padding(dword_t entity_number);

#endif /* BUILD_HARDWARE_DEVICES_MONITOR_MONITOR_H */
