/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/devices/cpu/scheduler.h>

#define BREAKING_POINT_FOR_LAYOUT 500

typedef struct {
    int y_offset;
    uint32_t y_offset_range;
    uint32_t is_there_vertical_scrollbar;
    uint32_t vertical_scrollbar_range;
    uint32_t vertical_scrollbar_position;
    uint32_t vertical_scrollbar_area_size;
    uint32_t vertical_scrollbar_size;
    uint32_t is_vertical_scrollbar_dragged;
    uint32_t initial_grab_y_position;

    uint32_t show_remaining_programs;

    uint32_t show_shutdown_dialog;
} main_panel_data_t;