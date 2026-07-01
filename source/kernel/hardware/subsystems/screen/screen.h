/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/main.h>

typedef enum {
    PART_STATE_MAIN_PANEL = 0,
    PART_STATE_PROGRAM,
    PART_STATE_HORIZONTAL_SPLIT,
    PART_STATE_VERTICAL_SPLIT
} part_state_t;
typedef struct screen_part {
    struct screen_part *parent;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    void *view;
    part_state_t state;
    uint32_t split;
    struct screen_part *first_child;
    struct screen_part *second_child;
    uint32_t is_processed;
} screen_part_t;
typedef struct {
    uint32_t is_active;
    hardware_t *monitor;
    uint32_t width;
    uint32_t height;
    void *buffer;
    void *preview_buffer;
    screen_part_t *global_part;
} view_t;
typedef struct {
    uint32_t number_of_views;
    view_t *views[];
} list_of_views_t;

typedef struct {
    uint32_t start_of_mouse_area;
    uint32_t split_position;
    uint32_t end_of_mouse_area;
} fixed_editing_mode_area_t;

extern uint32_t is_there_screen_subsystem;
extern uint32_t mouse_cursor_x;
extern uint32_t mouse_cursor_y;

#define MINIMAL_PART_SIZE 150