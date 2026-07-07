/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/stb_image_implementation.h>

typedef struct {
    uint8_t *start;
    uint8_t *end;
    uint32_t width;
    uint32_t height;
} line_of_text_t;
typedef struct {
    uint8_t *string;
    uint32_t width;
    uint32_t height;
    uint32_t number_of_lines;
    line_of_text_t lines[];
} text_t;

typedef struct click_area_t {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t argument;
    void (*process_click)(screen_part_t *part, uint32_t argument);
} click_area_t;
typedef struct {
    uint32_t number_of_click_areas;
    click_area_t click_areas[];
} click_area_list_t;

typedef enum {
    VERTICAL_BLOCK,
    HORIZONTAL_BLOCK,
    TEXT_BLOCK,
    DYNAMIC_TEXT_BLOCK,
    IMAGE_BLOCK,
    INTERACTIVE_DRAW,
} block_type_t;
typedef enum {
    ALIGN_LEFT = 0,
    ALIGN_TOP,
    ALIGN_CENTER,
    ALIGN_RIGHT,
    ALIGN_BOTTOM,
} alignment_t;
enum {
    ADD_FROM_START,
    ADD_FROM_END,
};
typedef struct block_t {
    struct block_t *parent;

    uint32_t argument;
    void (*process_click)(screen_part_t *part, uint32_t argument);

    block_type_t type;
    void *data;
    text_t *text;

    alignment_t horizontal_alignment;
    alignment_t vertical_alignment;

    int margin_top;
    int margin_bottom;
    int margin_left;
    int margin_right;

    int padding_top;
    int padding_bottom;
    int padding_left;
    int padding_right;

    int border_size;
    uint32_t border_color;

    uint32_t background_color;

    uint32_t text_color;
    
    int min_width;
    int max_width;
    int actual_max_width;
    int inside_width;
    int content_width;
    int actual_width;

    int min_height;
    int max_height;
    int actual_max_height;
    int inside_height;
    int content_height;
    int actual_height;

    uint32_t draw_from_two_ends;
    uint32_t number_of_blocks_from_start;
    uint32_t number_of_blocks_from_end;
    uint32_t number_of_blocks;
    struct block_t *blocks[];
} block_t;