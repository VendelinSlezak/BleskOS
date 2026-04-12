/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct monitor monitor_t;
typedef struct monitor_mode monitor_mode_t;

struct monitor_mode {
    uint32_t active_width;
    uint32_t active_height;
    uint32_t bpp;
    uint32_t bytes_per_line;
    uint32_t redraw_cycles;
    void *linear_frame_buffer;

    void (*redraw_full_screen)(monitor_t *monitor, void *double_buffer);
    void (*redraw_part_of_screen)(monitor_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height);
};

struct monitor {
    uint32_t id;

    uint32_t (*change_brightness)(struct monitor *monitor, uint32_t brightness);
    uint32_t brightness;

    uint32_t (*change_resolution)(struct monitor *monitor, monitor_mode_t *mode);
    monitor_mode_t *actual_mode;
    monitor_mode_t *best_mode;
    monitor_mode_t supported_modes[];
};