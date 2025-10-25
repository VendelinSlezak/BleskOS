/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <hardware/devices/monitor/monitor.h>

#define STANDARDIZED_GRAPHIC_OUTPUT_TYPE_TEXT_MODE 1
#define STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE 2

typedef struct {
    dword_t type;
    dword_t width;
    dword_t height;
    dword_t bpp;
    dword_t bytes_per_line;
    void *linear_frame_buffer;
} standardized_graphic_output_t;

typedef struct {
    /* device part */
    monitor_mode_t mode;

    /* controller part */
    dword_t number_of_devices;
    dword_t devices[1];
} e_dgc_attr_t;