/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <syslib.h>
#include <userspace_library/screen.h>

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT,
} horizontal_alignment_t;
typedef enum {
    ALIGN_TOP,
    ALIGN_MIDDLE,
    ALIGN_BOTTOM,
} vertical_alignment_t;

typedef enum {
    PRINTABLE_WORD,
    SPACE_WORD,
    NEW_LINE_WORD,
} word_type_t;
typedef struct {
    word_type_t type;
    uint32_t width_of_word;
    uint32_t height_of_word;
    uint32_t chars_of_word;
    void *start_of_word;
    void *end_of_word;
} word_size_t;
typedef struct {
    uint32_t width_of_line;
    uint32_t height_of_line;
    uint32_t chars_of_line;
    void *start_of_line;
    void *end_of_line;
} line_size_t;
typedef struct {
    uint32_t color;
    uint8_t *text;
    uint32_t number_of_words;
    word_size_t **words;
    uint32_t number_of_lines;
    line_size_t **lines;
} text_t;

typedef struct {
    uint8_t *text;
    uint32_t width_of_text;
    uint32_t height_of_text;
} button_t;

typedef enum {
    ELEMENT_TEXT_BLOCK,
    ELEMENT_BUTTON,
    ELEMENT_DRAW_AREA,
    ELEMENT_HBOX,
    ELEMENT_VBOX,
    ELEMENT_TABLE_BOX,
} element_type_t;
typedef struct {
    buffer_box_t screen_box;
    uint32_t screen_box_x;
    uint32_t screen_box_y;

    horizontal_alignment_t halign;
    vertical_alignment_t valign;

    uint32_t min_width;
    uint32_t max_width;
    uint32_t real_width;
    uint32_t min_height;
    uint32_t max_height;
    uint32_t real_height;

    uint32_t margin_top;
    uint32_t margin_bottom;
    uint32_t margin_left;
    uint32_t margin_right;

    uint32_t border_top;
    uint32_t border_bottom;
    uint32_t border_left;
    uint32_t border_right;
    uint32_t border_color;

    uint32_t padding_top;
    uint32_t padding_bottom;
    uint32_t padding_left;
    uint32_t padding_right;
    uint32_t background_color;

    uint32_t full_content_width;
    uint32_t min_content_width;
    uint32_t max_content_width;
    uint32_t real_content_width;
    uint32_t full_content_height;
    uint32_t min_content_height;
    uint32_t max_content_height;
    uint32_t real_content_height;

    int offset_x;
    int offset_y;

    uint32_t enable_scrollbars;
    uint32_t horizontal_scrollbar_size;
    uint32_t horizontal_scrollbar_slider_size;
    uint32_t horizontal_scrollbar_slider_offset;
    uint32_t vertical_scrollbar_size;
    uint32_t vertical_scrollbar_slider_size;
    uint32_t vertical_scrollbar_slider_offset;

    element_type_t type;
    void *data;
} element_t;

typedef struct {
    void (*redraw_area)(element_t *element, buffer_box_t *box, int x_offset, int y_offset);
    uint32_t (*process_wheel)(element_t *element, int movement);
} draw_area_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t number_of_elements;
} box_part_t;
typedef struct {
    int number_of_elements_from_start;
    int number_of_elements_from_end;
    uint32_t number_of_elements;
    element_t **elements;
    uint32_t wrap;
    uint32_t number_of_parts;
    box_part_t *parts;
} box_t;

typedef struct {
    uint32_t is_active;
    horizontal_alignment_t halign;
    vertical_alignment_t valign;
    uint32_t span_across_columns;
    uint32_t span_across_lines;
} cell_info_t;
typedef struct {
    uint32_t lines;
    uint32_t *line_real_height;
    uint32_t columns;
    uint32_t *column_real_width;
    uint32_t number_of_elements;
    element_t **elements;
    cell_info_t *cell_info;
} table_box_t;

#define ELEMENT_MAX_WIDTH 0x7FFFFFFF
#define ELEMENT_MAX_HEIGHT 0x7FFFFFFF

typedef enum {
    DRAG_VERTICAL_SCROLLBAR,
    DRAG_HORIZONTAL_SCROLLBAR,
} gui_element_action_t;
typedef struct {
    uint32_t button;
    uint32_t top;
    uint32_t bottom;
    uint32_t left;
    uint32_t right;
    element_t *element;
    gui_element_action_t action;
} click_area_t;

typedef enum {
    EVENT_WHEEL_MOVEMENT,
    EVENT_KEY_PRESSED,
} gui_element_event_t;
typedef struct {
    element_t *element;
    gui_element_event_t event;
    uint32_t in_focus;
    uint32_t value;
    void *process_function;
} event_t;