/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <syslib.h>
#include <userspace_library/bleskalloc.h>
#include <userspace_library/screen.h>
#include <userspace_library/bitmap_font.h>
#include <userspace_library/logging.h>
#include <userspace_library/libc/string.h>

/* local variables */
uint32_t number_of_click_areas;
click_area_t **click_areas;
uint32_t number_of_key_events;
key_event_t **key_events;

/* functions */
void process_event(vh_human_input_event_t *event) {
    switch(event->type) {
        case VH_HUMAN_INPUT_EVENT_BUTTON_PRESSED: {
            uint32_t button = event->argument1;
            uint32_t x = event->argument2;
            uint32_t y = event->argument3;
            for(uint32_t i = 0; i < number_of_click_areas; i++) {
                click_area_t *area = click_areas[i];
                if(area->button == button && area->top >= y && area->bottom <= y && area->left >= x && area->right <= x) {
                    area->process_click(area);
                }
            }
            break;
        }
        case VH_HUMAN_INPUT_EVENT_KEY_PRESSED: {
            uint32_t key = event->argument1;
            uint32_t unicode = event->argument2;
            for(uint32_t i = 0; i < number_of_key_events; i++) {
                key_event_t *key_event = key_events[i];
                if(key_event->key == key && key_event->state == VH_HUMAN_INPUT_EVENT_KEY_PRESSED) {
                    key_event->process_key(key_event->identifier, unicode);
                }
            }
            break;
        }
        case VH_HUMAN_INPUT_EVENT_KEY_RELEASED: {
            uint32_t key = event->argument1;
            uint32_t unicode = event->argument2;
            for(uint32_t i = 0; i < number_of_key_events; i++) {
                key_event_t *key_event = key_events[i];
                if(key_event->key == key && key_event->state == VH_HUMAN_INPUT_EVENT_KEY_RELEASED) {
                    key_event->process_key(key_event->identifier, unicode);
                }
            }
            break;
        }
    }
}

void add_click_area(uint32_t button, uint32_t x, uint32_t y, uint32_t width, uint32_t height, void (*process_click)(click_area_t *area), uint32_t identifier) {
    click_area_t *area = malloc(sizeof(click_area_t));
    area->button = button;
    area->left = x;
    area->top = y;
    area->right = (x + width - 1);
    area->bottom = (y + height - 1);
    area->process_click = process_click;
    area->identifier = identifier;

    click_areas = realloc(click_areas, sizeof(click_area_t *) * (number_of_click_areas + 1));
    click_areas[number_of_click_areas] = area;
    number_of_click_areas++;
}

void add_key_event(uint32_t key, uint32_t state, void (*process_key)(uint32_t identifier, uint32_t unicode), uint32_t identifier) {
    key_event_t *event = malloc(sizeof(key_event_t));
    event->key = key;
    event->state = state;
    event->process_key = process_key;
    event->identifier = identifier;

    key_events = realloc(key_events, sizeof(key_event_t *) * (number_of_key_events + 1));
    key_events[number_of_key_events] = event;
    number_of_key_events++;
}

void draw_vertical_scrollbar(buffer_box_t *box, uint32_t x, uint32_t y, uint32_t size, uint32_t slider_size, uint32_t slider_offset) {
    draw_square(box, x, y, 10, size, 0xFFAAAAAA);
    draw_square(box, x, y + slider_offset, 10, slider_size, 0xFF888888);
}

void draw_horizontal_scrollbar(buffer_box_t *box, uint32_t x, uint32_t y, uint32_t size, uint32_t slider_size, uint32_t slider_offset) {
    draw_square(box, x, y, size, 10, 0xFFAAAAAA);
    draw_square(box, x + slider_offset, y, slider_size, 10, 0xFF888888);
}

element_t *create_text_block(uint8_t *string, uint32_t color) {
    text_t *text = malloc(sizeof(text_t));
    text->text = string;
    text->color = color;

    // count size of all words for easier manipulation with alignment
    while(*string != '\0') {
        word_size_t *word_size = malloc(sizeof(word_size_t));
        text->words = realloc(text->words, sizeof(word_size_t *) * (text->number_of_words + 1));
        text->words[text->number_of_words++] = word_size;

        word_size->height_of_word = 16;
        word_size->start_of_word = string;
        if(*string == '\n') {
            word_size->type = NEW_LINE_WORD;
            word_size->width_of_word = 0;
            word_size->chars_of_word++;
            word_size->end_of_word = string;
            string++;
            continue;
        }
        else if(*string == ' ') {
            word_size->type = SPACE_WORD;
            while(*string == ' ') {
                word_size->width_of_word += 8;
                word_size->chars_of_word++;
                string++;
            }
        }
        else {
            word_size->type = PRINTABLE_WORD;
            while(*string != ' ' && *string != '\n' && *string != '\0') {
                word_size->width_of_word += 8;
                word_size->chars_of_word++;
                string++;
            }
        }
        word_size->end_of_word = string;
    }

    element_t *element = calloc(sizeof(element_t), 1);
    element->type = ELEMENT_TEXT_BLOCK;
    element->data = text;

    // count size
    element->min_width = 8; // one character
    string = text->text;
    while(*string != '\0') {
        uint32_t line_width = 0;
        while(*string != '\n' || *string != '\0') {
            line_width += 8;
            string++;
            element->max_height += 16; // max height is when every character is on new line
        }
        if(line_width > element->max_width) {
            element->max_width = line_width; // biggest possible line
        }
        element->min_height += 16; // line height
    }

    return element;
}

void recalculate_text_block(text_t *text, uint32_t width) {
    uint32_t x = 0, char_width = 8, char_height = 16;

    // free old lines
    for(int i = 0; i < text->number_of_lines; i++) {
        free(text->lines[i]);
        free(text->lines);
    }
    text->number_of_lines = 0;

    // recalculate lines to fit into width
    uint32_t number_of_processed_words = 0;
    while(number_of_processed_words < text->number_of_words) {
        line_size_t *line = malloc(sizeof(line_size_t));
        line->height_of_line = char_height;
        line->start_of_line = text->words[number_of_processed_words]->start_of_word;
        line->chars_of_line = 0;
        line->end_of_line = text->words[number_of_processed_words]->start_of_word;
        x = 0;
        text->lines = realloc(text->lines, sizeof(line_size_t *) * (text->number_of_lines + 1));
        text->lines[text->number_of_lines++] = line;

        while(number_of_processed_words < text->number_of_words) {
            word_size_t *word = text->words[number_of_processed_words];

            // process new line
            if(word->type == NEW_LINE_WORD) {
                number_of_processed_words++;
                break;
            }

            // process free space
            if(word->type == SPACE_WORD) {
                if(x == 0) { // start of line, skip all spaces
                    number_of_processed_words++;
                    continue;
                }
                if(x + word->width_of_word > width) { // spaces do not fit into this line, go to new line
                    number_of_processed_words++;
                    break;
                }
                else { // spaces do fit into this line, skip them
                    line->width_of_line += word->width_of_word;
                    line->chars_of_line += word->chars_of_word;
                    line->end_of_line = word->end_of_word;
                    x += word->width_of_word;
                    number_of_processed_words++;
                    continue;
                }
            }

            // process characters
            if(x + word->width_of_word > width) { // word does not fit into this line
                if(x == 0) { // we are at start of line, print all word across lines until it fits
                    uint8_t *string = word->start_of_word;
                    while((void *)string < word->end_of_word) {
                        if((x + char_width) > width) { // this character does not fit into this line, create new line
                            line = malloc(sizeof(line_size_t));
                            line->height_of_line = char_height;
                            line->start_of_line = text->words[number_of_processed_words]->start_of_word;
                            line->chars_of_line = 0;
                            line->end_of_line = text->words[number_of_processed_words]->start_of_word;
                            x = 0;
                            text->lines = realloc(text->lines, sizeof(line_size_t *) * (text->number_of_lines + 1));
                            text->lines[text->number_of_lines++] = line;
                        }
                        line->width_of_line += char_width;
                        line->chars_of_line++;
                        line->end_of_line = string;
                        x += char_width;
                    }
                    number_of_processed_words++;
                    break;
                }
                else { // this is at the end of line, go to new line
                    break;
                }
            }
            else { // word does fit into this line, print
                line->width_of_line += word->width_of_word;
                line->chars_of_line += word->chars_of_word;
                line->end_of_line = word->end_of_word;
                x += word->width_of_word;
                number_of_processed_words++;
                continue;
            }
        }
    }
}

void draw_text_block(buffer_box_t *box, text_t *text, uint32_t start_x, uint32_t y, uint32_t width, horizontal_alignment_t alignment, uint32_t color) {
    for(int i = 0; i < text->number_of_lines; i++) {
        line_size_t *line = text->lines[i];

        uint32_t x = 0;
        if(alignment == ALIGN_LEFT) {
            x = 0;
        }
        else if(alignment == ALIGN_CENTER) {
            x = ((width / 2) - (line->width_of_line / 2));
        }
        else if(alignment == ALIGN_RIGHT) {
            x = (width - line->width_of_line);
        }
        x += start_x;

        uint8_t *string = line->start_of_line;
        uint8_t *end = line->end_of_line;
        while(string < end) {
            draw_bitmap_char(box, x, y, *string++, color);
            x += 8; // character width
        }
        y += line->height_of_line;
    }
}

element_t *create_button(uint8_t *text) {
    button_t *button = malloc(sizeof(button_t));
    button->text = text;
    button->width_of_text = strlen(text) * 8;
    button->height_of_text = 16;
    element_t *element = calloc(sizeof(element_t), 1);
    element->type = ELEMENT_BUTTON;
    element->data = button;
    element->real_content_width = (9 + button->width_of_text + 9);
    element->real_content_height = (1 + 2 + button->height_of_text + 2 + 1);
    element_set_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    element_set_content_dimensions(element, element->real_content_width, element->real_content_width, element->real_content_height, element->real_content_height);
    return element;
}

void draw_button(buffer_box_t *box, button_t *button, uint32_t x, uint32_t y) {
    uint32_t width = (9 + button->width_of_text + 9);
    uint32_t height = (1 + 2 + button->height_of_text + 2 + 1);
    uint32_t height1 = (height / 3);
    uint32_t height3 = (height / 3);
    uint32_t height2 = (height - height1 - height3);
    draw_square(box, x, y, width, height1, 0xFFCCCCCC);
    draw_square(box, x, y + height1, width, height2, 0xFFBBBBBB);
    draw_square(box, x, y + height1 + height2, width, height3, 0xFFAAAAAA);
    draw_line(box, x, y, x + width, y, 0xFF000000);
    draw_line(box, x, y + height - 1, x + width, y + height - 1, 0xFF000000);
    draw_line(box, x, y, x, y + height - 1, 0xFF000000);
    draw_line(box, x + width - 1, y, x + width - 1, y + height - 1, 0xFF000000);
    draw_bitmap_string(box, x + 9, y + 1 + 2, button->text, 0xFF000000);
}

element_t *create_draw_area(uint32_t identifier, void (*redraw_area)(uint32_t identifier, buffer_box_t *box, int x_offset, int y_offset), dimensions_t (*get_content_dimensions)(uint32_t identifier)) {
    draw_area_t *draw_area = malloc(sizeof(draw_area_t));
    draw_area->identifier = identifier;
    draw_area->redraw_area = redraw_area;
    draw_area->get_content_dimensions = get_content_dimensions;
    dimensions_t dim = get_content_dimensions(identifier);
    element_t *element = calloc(sizeof(element_t), 1);
    element->type = ELEMENT_DRAW_AREA;
    element->data = draw_area;
    element->full_content_width = dim.width;
    element->full_content_height = dim.height;
    element->real_content_width = 0;
    element->real_content_height = 0;
    element_set_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    element_set_content_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    return element;
}

element_t *create_hbox(uint32_t end_offset) {
    box_t *hbox = malloc(sizeof(box_t));
    hbox->number_of_elements = 0;
    hbox->number_of_elements_from_start = 0;
    hbox->number_of_elements_from_end = 0;
    hbox->end_offset = end_offset;
    hbox->wrap = false;
    hbox->number_of_parts = 0;
    element_t *element = calloc(sizeof(element_t), 1);
    element->type = ELEMENT_HBOX;
    element->data = hbox;
    element_set_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    element_set_content_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    return element;
}

element_t *create_vbox(uint32_t end_offset) {
    box_t *vbox = malloc(sizeof(box_t));
    vbox->number_of_elements = 0;
    vbox->number_of_elements_from_start = 0;
    vbox->number_of_elements_from_end = 0;
    vbox->end_offset = end_offset;
    vbox->wrap = false;
    vbox->number_of_parts = 0;
    element_t *element = calloc(sizeof(element_t), 1);
    element->type = ELEMENT_VBOX;
    element->data = vbox;
    element_set_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    element_set_content_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    return element;
}

element_t *create_table(uint32_t columns, uint32_t lines) {
    table_box_t *table = malloc(sizeof(table_box_t));
    table->lines = lines;
    table->line_real_height = calloc(sizeof(uint32_t), lines);
    table->columns = columns;
    table->column_real_width = calloc(sizeof(uint32_t), columns);
    table->number_of_elements = (lines * columns);
    table->elements = calloc(sizeof(element_t *), table->number_of_elements);
    table->cell_info = calloc(sizeof(cell_info_t), table->number_of_elements);
    for(int i = 0; i < table->number_of_elements; i++) {
        table->cell_info[i].is_active = true;
        table->cell_info[i].halign = ALIGN_LEFT;
        table->cell_info[i].valign = ALIGN_TOP;
        table->cell_info[i].span_across_columns = 1;
        table->cell_info[i].span_across_lines = 1;
    }
    element_t *element = calloc(sizeof(element_t), 1);
    element->type = ELEMENT_TABLE_BOX;
    element->data = table;
    element_set_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    element_set_content_dimensions(element, 0, ELEMENT_MAX_WIDTH, 0, ELEMENT_MAX_HEIGHT);
    return element;
}

void add_element_to_box_from_start(element_t *box_element, element_t *element) {
    box_t *box = box_element->data;
    box->elements = realloc(box->elements, sizeof(element_t *) * (++box->number_of_elements));
    memmove(box->elements[box->number_of_elements_from_start + 1], box->elements[box->number_of_elements_from_start], sizeof(element_t *) * box->number_of_elements_from_end);
    box->elements[box->number_of_elements_from_start++] = element;
}

void add_element_to_box_from_end(element_t *box_element, element_t *element) {
    box_t *box = box_element->data;
    box->elements = realloc(box->elements, sizeof(element_t *) * (++box->number_of_elements));
    box->elements[box->number_of_elements - 1] = element;
    box->number_of_elements_from_end++;
}

void add_element_to_table_box(element_t *table_box_element, uint32_t column, uint32_t line, element_t *element) {
    table_box_t *table = table_box_element->data;
    table->elements[line * table->columns + column] = element;
}

void element_set_margin(element_t *element, uint32_t top, uint32_t bottom, uint32_t left, uint32_t right) {
    element->margin_top = top;
    element->margin_bottom = bottom;
    element->margin_left = left;
    element->margin_right = right;
}

void element_set_padding(element_t *element, uint32_t top, uint32_t bottom, uint32_t left, uint32_t right) {
    element->padding_top = top;
    element->padding_bottom = bottom;
    element->padding_left = left;
    element->padding_right = right;
}

void element_set_border(element_t *element, uint32_t top, uint32_t bottom, uint32_t left, uint32_t right, uint32_t color) {
    element->border_top = top;
    element->border_bottom = bottom;
    element->border_left = left;
    element->border_right = right;
    element->border_color = color;
}

void element_set_dimensions(element_t *element, uint32_t min_w, uint32_t max_w, uint32_t min_h, uint32_t max_h) {
    element->min_width = min_w;
    element->max_width = max_w;
    element->min_height = min_h;
    element->max_height = max_h;
}

void element_set_content_dimensions(element_t *element, uint32_t min_w, uint32_t max_w, uint32_t min_h, uint32_t max_h) {
    element->min_content_width = min_w;
    element->max_content_width = max_w;
    element->min_content_height = min_h;
    element->max_content_height = max_h;
}

void element_enable_scrollbars(element_t *element) {
    element->enable_scrollbars = true;
    element->vertical_scrollbar_offset = 0;
    element->offset_x = 0;
    element->horizontal_scrollbar_offset = 0;
    element->offset_y = 0;
}

void cell_set_alignment(element_t *element, uint32_t column, uint32_t line, horizontal_alignment_t halign, vertical_alignment_t valign) {
    table_box_t *table = element->data;
    uint32_t cell_offset = (line * table->columns + column);
    table->cell_info[cell_offset].halign = halign;
    table->cell_info[cell_offset].valign = valign;
}

void table_merge_cells(element_t *element, uint32_t column, uint32_t line, uint32_t span_across_columns, uint32_t span_across_lines) {
    table_box_t *table = element->data;
    uint32_t cell_offset = (line * table->columns + column);
    for(int i = 0; i < span_across_lines; i++) {
        for(int j = 0; j < span_across_columns; j++) {
            table->cell_info[cell_offset + (i * table->columns) + j].is_active = false;
        }
    }
    table->cell_info[cell_offset].is_active = true;
    table->cell_info[cell_offset].span_across_columns = span_across_columns;
    table->cell_info[cell_offset].span_across_lines = span_across_lines;
}

void recalculate_element(element_t *element) {
    switch(element->type) {
        case ELEMENT_TEXT_BLOCK: {
            text_t *text = element->data;
            recalculate_text_block(text, element->real_content_width);
            element->full_content_width = 0;
            element->full_content_height = 0;
            for(int i = 0; i < text->number_of_lines; i++) {
                element->full_content_height += text->lines[i]->height_of_line;
                element->full_content_width = (text->lines[i]->width_of_line > element->full_content_width) ? text->lines[i]->width_of_line : element->full_content_width;
            }
            break;
        }
        case ELEMENT_BUTTON: {
            break;
        }
        case ELEMENT_DRAW_AREA: {
            draw_area_t *draw_area = element->data;
            dimensions_t dim = draw_area->get_content_dimensions(draw_area->identifier);
            element->full_content_width = dim.width;
            element->full_content_height = dim.height;
            break;
        }
        case ELEMENT_HBOX: {
            box_t *box = element->data;
            element->full_content_width = 0;
            element->full_content_height = 0;
            if(box->number_of_elements == 0) {
                break;
            }
            if(box->number_of_parts != NULL) {
                free(box->parts);
            }
            box->number_of_parts = 0;

            for(int i = 0; i < box->number_of_elements; i++) {
                element_t *inside_element = box->elements[i];
                recalculate_element(inside_element);
                element->full_content_width += inside_element->real_width;
                element->full_content_height = (inside_element->real_height > element->full_content_height) ? inside_element->real_height : element->full_content_height;
            }
            if(element->full_content_width < box->end_offset) {
                element->full_content_width = box->end_offset;
            }
            else if(box->wrap == true) {
                // count number of parts
                uint32_t part_width = 0;
                for(int i = 0; i < box->number_of_elements; i++) {
                    element_t *inside_element = box->elements[i];
                    if((part_width + inside_element->real_width) > box->end_offset) {
                        box->number_of_parts++;
                        if(part_width > 0) { // this element is the first in a new part
                            part_width = inside_element->real_width;
                        }
                        else { // this element is only element in this part
                            part_width = 0;
                        }
                    }
                    else {
                        part_width += inside_element->real_width;
                    }
                }
                if(part_width > 0) {
                    box->number_of_parts++; // there is at least one element not counted in any part
                }
                box->parts = calloc(sizeof(box_part_t), box->number_of_parts);

                // count dimensions and number of elements in each part
                uint32_t part = 0;
                for(int i = 0; i < box->number_of_elements; i++) {
                    element_t *inside_element = box->elements[i];
                    if((box->parts[part].width + inside_element->real_width) > box->end_offset) {
                        if(box->parts[part].number_of_elements > 0) { // this element is the first in a new part
                            part++;
                            box->parts[part].width = inside_element->real_width;
                            box->parts[part].height = inside_element->real_height;
                            box->parts[part].number_of_elements = 1;
                        }
                        else { // this element is only element in this part
                            box->parts[part].width = inside_element->real_width;
                            box->parts[part].height = inside_element->real_height;
                            box->parts[part].number_of_elements = 1;
                            part++;
                        }
                    }
                    else {
                        box->parts[part].width += inside_element->real_width;
                        box->parts[part].height = (inside_element->real_height > box->parts[part].height) ? inside_element->real_height : box->parts[part].height;
                        box->parts[part].number_of_elements++;
                    }
                }

                // count dimensions of whole element
                element->full_content_width = 0;
                element->full_content_height = 0;
                for(int i = 0; i < box->number_of_parts; i++) {
                    element->full_content_width = (box->parts[i].width > element->full_content_width) ? box->parts[i].width : element->full_content_width;
                    element->full_content_height += box->parts[i].height;
                }
            }
            // else as some content is bigger than shown part so element won't be shown full
            break;
        }
        case ELEMENT_VBOX: {
            box_t *box = element->data;
            element->full_content_width = 0;
            element->full_content_height = 0;
            if(box->number_of_elements == 0) {
                break;
            }
            if(box->number_of_parts != NULL) {
                free(box->parts);
            }
            box->number_of_parts = 0;

            for(int i = 0; i < box->number_of_elements; i++) {
                element_t *inside_element = box->elements[i];
                recalculate_element(inside_element);
                element->full_content_width = (inside_element->real_width > element->full_content_width) ? inside_element->real_width : element->full_content_width;
                element->full_content_height += inside_element->real_height;
            }
            if(element->full_content_height < box->end_offset) {
                element->full_content_height = box->end_offset;
            }
            else if(box->wrap == true) {
                // count number of parts
                uint32_t part_height = 0;
                for(int i = 0; i < box->number_of_elements; i++) {
                    element_t *inside_element = box->elements[i];
                    if((part_height + inside_element->real_height) > box->end_offset) {
                        box->number_of_parts++;
                        if(part_height > 0) { // this element is the first in a new part
                            part_height = inside_element->real_height;
                        }
                        else { // this element is only element in this part
                            part_height = 0;
                        }
                    }
                    else {
                        part_height += inside_element->real_height;
                    }
                }
                if(part_height > 0) {
                    box->number_of_parts++; // there is at least one element not counted in any part
                }
                box->parts = calloc(sizeof(box_part_t), box->number_of_parts);

                // count dimensions and number of elements in each part
                uint32_t part = 0;
                for(int i = 0; i < box->number_of_elements; i++) {
                    element_t *inside_element = box->elements[i];
                    if((box->parts[part].height + inside_element->real_height) > box->end_offset) {
                        if(box->parts[part].number_of_elements > 0) { // this element is the first in a new part
                            part++;
                            box->parts[part].width = inside_element->real_width;
                            box->parts[part].height = inside_element->real_height;
                            box->parts[part].number_of_elements = 1;
                        }
                        else { // this element is only element in this part
                            box->parts[part].width = inside_element->real_width;
                            box->parts[part].height = inside_element->real_height;
                            box->parts[part].number_of_elements = 1;
                            part++;
                        }
                    }
                    else {
                        box->parts[part].width = (inside_element->real_width > box->parts[part].width) ? inside_element->real_width : box->parts[part].width;
                        box->parts[part].height += inside_element->real_height;
                        box->parts[part].number_of_elements++;
                    }
                }

                // count dimensions of whole element
                element->full_content_width = 0;
                element->full_content_height = 0;
                for(int i = 0; i < box->number_of_parts; i++) {
                    element->full_content_width += box->parts[i].width;
                    element->full_content_height = (box->parts[i].height > element->full_content_height) ? box->parts[i].height : element->full_content_height;
                }
            }
            // else as some content is bigger than shown part so element won't be shown full
        }
        case ELEMENT_TABLE_BOX: {
            table_box_t *table = element->data;
            memset(table->line_real_height, 0, table->lines * sizeof(uint32_t));
            memset(table->column_real_width, 0, table->columns * sizeof(uint32_t));

            // recalculate size of all cells
            for(int line = 0; line < table->lines; line++) {
                for(int column = 0; column < table->columns; column++) {
                    uint32_t cell_offset = (line * table->columns + column);
                    element_t *cell = table->elements[cell_offset];
                    cell_info_t *info = &table->cell_info[cell_offset];
                    if(cell == NULL || info->is_active == false) {
                        continue;
                    }
                    recalculate_element(cell);

                    uint32_t actual_width_of_columns = 0;
                    for(int i = 0; i < info->span_across_columns; i++) {
                        actual_width_of_columns += table->column_real_width[column + i];
                    }
                    if(actual_width_of_columns < cell->real_width) {
                        uint32_t real_width = cell->real_width - actual_width_of_columns;
                        uint32_t width_part_for_column = real_width / info->span_across_columns;
                        for(int i = 0; i < (info->span_across_columns - 1); i++) {
                            table->column_real_width[column + i] += width_part_for_column;
                            real_width -= width_part_for_column;
                        }
                        table->column_real_width[column + info->span_across_columns - 1] += real_width;
                    }

                    uint32_t actual_height_of_lines = 0;
                    for(int i = 0; i < info->span_across_lines; i++) {
                        actual_height_of_lines += table->line_real_height[line + i];
                    }
                    if(actual_height_of_lines < cell->real_height) {
                        uint32_t real_height = cell->real_height - actual_height_of_lines;
                        uint32_t height_part_for_line = real_height / info->span_across_lines;
                        for(int i = 0; i < (info->span_across_lines - 1); i++) {
                            table->line_real_height[line + i] += height_part_for_line;
                            real_height -= height_part_for_line;
                        }
                        table->line_real_height[line + info->span_across_lines - 1] += real_height;
                    }
                }
            }

            // calculate size of whole table
            element->full_content_width = 0;
            for(int i = 0; i < table->lines; i++) {
                element->full_content_width += table->column_real_width[i];
            }
            
            element->full_content_height = 0;
            for(int i = 0; i < table->lines; i++) {
                element->full_content_height += table->line_real_height[i];
            }
            break;
        }
    }

    // calculate real content width and height according to maximal and minimal values
    element->real_content_width = (element->max_content_width < element->full_content_width) ? element->max_content_width : element->full_content_width;
    element->real_content_width = (element->min_content_width > element->real_content_width) ? element->min_content_width : element->real_content_width;
    element->real_content_height = (element->max_content_height < element->full_content_height) ? element->max_content_height : element->full_content_height;
    element->real_content_height = (element->min_content_height > element->real_content_height) ? element->min_content_height : element->real_content_height;

    // calculate real width and height according to maximal and minimal values
    element->real_width = element->margin_left + element->border_left + element->padding_left + element->real_content_width + element->padding_right + element->border_right + element->margin_right;
    element->real_height = element->margin_top + element->border_top + element->padding_top + element->real_content_height + element->padding_bottom + element->border_bottom + element->margin_bottom;
    element->real_width = (element->max_width < element->real_width) ? element->max_width : element->real_width;
    element->real_width = (element->min_width > element->real_width) ? element->min_width : element->real_width;
    element->real_height = (element->max_height < element->real_height) ? element->max_height : element->real_height;
    element->real_height = (element->min_height > element->real_height) ? element->min_height : element->real_height;
}

// box defines square on screen where element will be drawn, x and y are offsets relative to box
void draw_element(buffer_box_t *box, element_t *element, uint32_t x, uint32_t y) {
    // draw borders
    if(element->border_top != 0) {
        draw_square(box,
                    x + element->margin_left,
                    y + element->margin_top,
                    element->border_left + element->padding_left + element->real_content_width + element->padding_right + element->border_right,
                    element->border_top,
                    element->border_color);
    }
    if(element->border_bottom != 0) {
        draw_square(box,
                    x + element->margin_left,
                    y + element->margin_top + element->border_top + element->padding_top + element->real_content_height + element->padding_bottom,
                    element->border_left + element->padding_left + element->real_content_width + element->padding_right + element->border_right,
                    element->border_bottom,
                    element->border_color);
    }
    if(element->border_left != 0) {
        draw_square(box,
                    x + element->margin_left,
                    y + element->margin_top,
                    element->border_left,
                    element->border_top + element->padding_top + element->real_content_height + element->padding_bottom + element->border_bottom,
                    element->border_color);
    }
    if(element->border_right != 0) {
        draw_square(box,
                    x + element->margin_left + element->border_left + element->padding_left + element->real_content_width + element->padding_right,
                    y + element->margin_top,
                    element->border_right,
                    element->border_top + element->padding_top + element->real_content_height + element->padding_bottom + element->border_bottom,
                    element->border_color);
    }

    // draw backgroung
    if(element->background_color != NULL) {
        draw_square(box,
                    x + element->margin_left + element->border_left,
                    y + element->margin_top + element->border_top,
                    element->padding_left + element->real_content_width + element->padding_right,
                    element->padding_top + element->real_content_height + element->padding_bottom,
                    element->background_color);
    }

    // draw content
    x += element->margin_left + element->border_left + element->padding_left;
    y += element->margin_top + element->border_top + element->padding_top;
    switch(element->type) {
        case ELEMENT_TEXT_BLOCK: {
            text_t *text = element->data;
            draw_text_block(box, element->data, x, y, element->real_content_width, element->halign, text->color);
            break;
        }
        case ELEMENT_BUTTON: {
            draw_button(box, element->data, x, y);
            break;
        }
        case ELEMENT_DRAW_AREA: {
            draw_area_t *draw_area = element->data;
            buffer_box_t draw_area_box = {
                .buffer = malloc(element->real_content_width * element->real_content_height * 4),
                .buffer_width = element->real_content_width,
                .buffer_height = element->real_content_height,
                .x = 0,
                .y = 0,
                .width = element->real_content_width,
                .height = element->real_content_height,
            };
            draw_area->redraw_area(draw_area->identifier, &draw_area_box, element->offset_x, element->offset_y);
            copy_buffer_between_boxes(&draw_area_box, box);
            free(draw_area_box.buffer);
            break;
        }
        case ELEMENT_HBOX: {
            box_t *hbox = element->data;
            position_t pos = calculate_real_position_in_box(box, x, y, element->real_content_width, element->real_content_height);
            if(pos.drawable == false) {
                break;
            }
            buffer_box_t screen_hbox = {
                .buffer = box->buffer,
                .buffer_width = box->buffer_width,
                .buffer_height = box->buffer_height,
                .x = box->x + pos.left,
                .y = box->y + pos.top,
                .width = pos.width,
                .height = pos.height,
            };
            if(hbox->wrap == false || hbox->number_of_parts == 0) {
                if(element->full_content_width >= hbox->end_offset) {
                    uint32_t x_offset = 0;
                    for(int i = 0; i < hbox->number_of_elements; i++) {
                        element_t *inside_element = hbox->elements[i];
                        switch(element->valign) {
                            case ALIGN_TOP: {
                                draw_element(&screen_hbox, inside_element, x_offset, 0);
                                break;
                            }
                            case ALIGN_MIDDLE: {
                                draw_element(&screen_hbox, inside_element, x_offset, (element->real_content_height / 2) - (inside_element->real_height / 2));
                                break;
                            }
                            case ALIGN_BOTTOM: {
                                draw_element(&screen_hbox, inside_element, x_offset, element->real_content_height - inside_element->real_height);
                                break;
                            }
                        }
                        x_offset += inside_element->real_width;
                    }
                }
                else {
                    // draw all elements from start
                    uint32_t x_offset = 0;
                    for(int i = 0; i < hbox->number_of_elements_from_start; i++) {
                        element_t *inside_element = hbox->elements[i];
                        switch(element->valign) {
                            case ALIGN_TOP: {
                                draw_element(&screen_hbox, inside_element, x_offset, 0);
                                break;
                            }
                            case ALIGN_MIDDLE: {
                                draw_element(&screen_hbox, inside_element, x_offset, (element->real_content_height / 2) - (inside_element->real_height / 2));
                                break;
                            }
                            case ALIGN_BOTTOM: {
                                draw_element(&screen_hbox, inside_element, x_offset, element->real_content_height - inside_element->real_height);
                                break;
                            }
                        }
                        x_offset += inside_element->real_width;
                    }

                    // draw all elements from end
                    x_offset = hbox->end_offset;
                    for(int i = (hbox->number_of_elements - 1); i >= hbox->number_of_elements_from_start; i--) {
                        element_t *inside_element = hbox->elements[i];
                        x_offset -= inside_element->real_width;
                        switch(element->valign) {
                            case ALIGN_TOP: {
                                draw_element(&screen_hbox, inside_element, x_offset, 0);
                                break;
                            }
                            case ALIGN_MIDDLE: {
                                draw_element(&screen_hbox, inside_element, x_offset, (element->real_content_height / 2) - (inside_element->real_height / 2));
                                break;
                            }
                            case ALIGN_BOTTOM: {
                                draw_element(&screen_hbox, inside_element, x_offset, element->real_content_height - inside_element->real_height);
                                break;
                            }
                        }
                    }
                }
            }
            else {
                uint32_t x_offset = 0;
                uint32_t y_offset = 0;
                uint32_t element_offset = 0;
                for(int i = 0; i < hbox->number_of_parts; i++) {
                    box_part_t *part = &hbox->parts[i];
                    switch(element->halign) {
                        case ALIGN_LEFT: {
                            x_offset = 0;
                            break;
                        }
                        case ALIGN_CENTER: {
                            x_offset = (element->real_content_width / 2) - (part->width / 2);
                            break;
                        }
                        case ALIGN_RIGHT: {
                            x_offset = (element->real_content_width - part->width);
                            break;
                        }
                    }
                    for(int j = 0; j < part->number_of_elements; j++) {
                        element_t *inside_element = hbox->elements[element_offset++];
                        uint32_t valign_offset = 0;
                        switch(element->valign) {
                            case ALIGN_TOP: {
                                valign_offset = 0;
                                break;
                            }
                            case ALIGN_MIDDLE: {
                                valign_offset = (part->height / 2) - (inside_element->real_height / 2);
                                break;
                            }
                            case ALIGN_BOTTOM: {
                                valign_offset = (part->height - inside_element->real_height);
                                break;
                            }
                        }
                        draw_element(&screen_hbox, inside_element, x_offset, y_offset + valign_offset);
                        x_offset += inside_element->real_width;
                    }
                    y_offset += part->height;
                }
            }

            break;
        }
        case ELEMENT_VBOX: {
            box_t *vbox = element->data;
            position_t pos = calculate_real_position_in_box(box, x, y, element->real_content_width, element->real_content_height);
            if(pos.drawable == false) {
                break;
            }
            buffer_box_t screen_vbox = {
                .buffer = box->buffer,
                .buffer_width = box->buffer_width,
                .buffer_height = box->buffer_height,
                .x = box->x + pos.left,
                .y = box->y + pos.top,
                .width = pos.width,
                .height = pos.height,
            };
            if(vbox->wrap == false || vbox->number_of_parts == 0) {
                if(element->full_content_width >= vbox->end_offset) {
                    uint32_t y_offset = 0;
                    for(int i = 0; i < vbox->number_of_elements; i++) {
                        element_t *inside_element = vbox->elements[i];
                        switch(element->halign) {
                            case ALIGN_LEFT: {
                                draw_element(&screen_vbox, inside_element, 0, y_offset);
                                break;
                            }
                            case ALIGN_CENTER: {
                                draw_element(&screen_vbox, inside_element, (element->real_content_width / 2) - (inside_element->real_width / 2), y_offset);
                                break;
                            }
                            case ALIGN_RIGHT: {
                                draw_element(&screen_vbox, inside_element, element->real_content_width - inside_element->real_width, y_offset);
                                break;
                            }
                        }
                        y_offset += inside_element->real_height;
                    }
                }
                else {
                    // draw all elements from start
                    uint32_t y_offset = 0;
                    for(int i = 0; i < vbox->number_of_elements_from_start; i++) {
                        element_t *inside_element = vbox->elements[i];
                        switch(element->halign) {
                            case ALIGN_LEFT: {
                                draw_element(&screen_vbox, inside_element, 0, y_offset);
                                break;
                            }
                            case ALIGN_CENTER: {
                                draw_element(&screen_vbox, inside_element, (element->real_content_width / 2) - (inside_element->real_width / 2), y_offset);
                                break;
                            }
                            case ALIGN_RIGHT: {
                                draw_element(&screen_vbox, inside_element, element->real_content_width - inside_element->real_width, y_offset);
                                break;
                            }
                        }
                        y_offset += inside_element->real_height;
                    }

                    // draw all elements from end
                    y_offset = vbox->end_offset;
                    for(int i = (vbox->number_of_elements - 1); i >= vbox->number_of_elements_from_start; i--) {
                        element_t *inside_element = vbox->elements[i];
                        y_offset -= inside_element->real_height;
                        switch(element->halign) {
                            case ALIGN_LEFT: {
                                draw_element(&screen_vbox, inside_element, 0, y_offset);
                                break;
                            }
                            case ALIGN_CENTER: {
                                draw_element(&screen_vbox, inside_element, (element->real_content_width / 2) - (inside_element->real_width / 2), y_offset);
                                break;
                            }
                            case ALIGN_RIGHT: {
                                draw_element(&screen_vbox, inside_element, element->real_content_width - inside_element->real_width, y_offset);
                                break;
                            }
                        }
                    }
                }
            }
            else {
                uint32_t x_offset = 0;
                uint32_t y_offset = 0;
                uint32_t element_offset = 0;
                for(int i = 0; i < vbox->number_of_parts; i++) {
                    box_part_t *part = &vbox->parts[i];
                    switch(element->valign) {
                        case ALIGN_TOP: {
                            y_offset = 0;
                            break;
                        }
                        case ALIGN_MIDDLE: {
                            y_offset = (element->real_content_height / 2) - (part->height / 2);
                            break;
                        }
                        case ALIGN_BOTTOM: {
                            y_offset = (element->real_content_height - part->height);
                            break;
                        }
                    }
                    for(int j = 0; j < part->number_of_elements; j++) {
                        element_t *inside_element = vbox->elements[element_offset++];
                        uint32_t halign_offset = 0;
                        switch(element->halign) {
                            case ALIGN_LEFT: {
                                halign_offset = 0;
                                break;
                            }
                            case ALIGN_CENTER: {
                                halign_offset = (part->width / 2) - (inside_element->real_width / 2);
                                break;
                            }
                            case ALIGN_RIGHT: {
                                halign_offset = (part->width - inside_element->real_width);
                                break;
                            }
                        }
                        draw_element(&screen_vbox, inside_element, x_offset + halign_offset, y_offset);
                        y_offset += inside_element->real_height;
                    }
                    x_offset += part->width;
                }
            }

            break;
        }
        case ELEMENT_TABLE_BOX: {
            table_box_t *table = element->data;
            position_t pos = calculate_real_position_in_box(box, x, y, element->real_content_width, element->real_content_height);
            if(pos.drawable == false) {
                break;
            }
            buffer_box_t screen_table_box = {
                .buffer = box->buffer,
                .buffer_width = box->buffer_width,
                .buffer_height = box->buffer_height,
                .x = box->x + pos.left,
                .y = box->y + pos.top,
                .width = pos.width,
                .height = pos.height,
            };
            uint32_t cell_x = 0;
            uint32_t cell_y = 0;
            for(int line = 0; line < table->lines; cell_x = x, cell_y += table->line_real_height[line], line++) {
                for(int column = 0; column < table->columns; cell_x += table->column_real_width[column], column++) {
                    uint32_t cell_offset = (line * table->lines) + column;
                    element_t *cell = table->elements[cell_offset];
                    cell_info_t *info = &table->cell_info[cell_offset];
                    if(cell == NULL || info->is_active == false) {
                        continue;
                    }

                    // calculate full width of cell
                    uint32_t cell_width = 0;
                    for(int i = 0; i < info->span_across_columns; i++) {
                        cell_width += table->column_real_width[column + i];
                    }

                    // calculate full height of cell
                    uint32_t cell_height = 0;
                    for(int i = 0; i < info->span_across_lines; i++) {
                        cell_height += table->line_real_height[line + i];
                    }

                    // calculate offsets to content according to alignment
                    uint32_t halign_offset = 0;
                    switch(info->halign) {
                        case ALIGN_LEFT: {
                            halign_offset = 0;
                            break;
                        }
                        case ALIGN_CENTER: {
                            halign_offset = (cell_width / 2) - (cell->real_width / 2);
                            break;
                        }
                        case ALIGN_RIGHT: {
                            halign_offset = cell_width - cell->real_width;
                            break;
                        }
                    }

                    uint32_t valign_offset = 0;
                    switch(info->valign) {
                        case ALIGN_TOP: {
                            valign_offset = 0;
                            break;
                        }
                        case ALIGN_MIDDLE: {
                            valign_offset = (cell_height / 2) - (cell->real_height / 2);
                            break;
                        }
                        case ALIGN_BOTTOM: {
                            valign_offset = cell_height - cell->real_height;
                            break;
                        }
                    }

                    // create drawing box specifically for the cell and draw inside content
                    position_t cell_pos = calculate_real_position_in_box(&screen_table_box, cell_x, cell_y, cell_width, cell_height);
                    if(cell_pos.drawable == false) {
                        break;
                    }
                    buffer_box_t cell_box = {
                        .buffer = box->buffer,
                        .buffer_width = box->buffer_width,
                        .buffer_height = box->buffer_height,
                        .x = box->x + cell_pos.left,
                        .y = box->y + cell_pos.top,
                        .width = cell_pos.width,
                        .height = cell_pos.height,
                    };
                    draw_element(&cell_box, cell, halign_offset, valign_offset);
                }
            }
            break;
        }
    }

    // draw scrollbars
    if(element->enable_scrollbars == true && element->real_content_width > 10 && element->real_content_height > 10) {
        uint32_t vertical_scrollbar = (element->full_content_height > element->real_content_height) ? true : false;
        uint32_t horizontal_scrollbar = (element->full_content_width > element->real_content_width) ? true : false;
        if(vertical_scrollbar == true) {
            horizontal_scrollbar = (element->full_content_width > (element->real_content_width - 10)) ? true : false;
        }
        if(horizontal_scrollbar == true) {
            vertical_scrollbar = (element->full_content_height > (element->real_content_height - 10)) ? true : false;
        }

        if(vertical_scrollbar == true && horizontal_scrollbar == true) {
            uint32_t vs_size = (element->real_content_height - 10);
            uint32_t vs_slider_size = ((element->real_content_height - 10) * vs_size / element->full_content_height);
            element->vertical_scrollbar_offset = (element->vertical_scrollbar_offset > (vs_size - vs_slider_size)) ? (vs_size - vs_slider_size) : element->vertical_scrollbar_offset;
            uint32_t hs_size = (element->real_content_width - 10);
            uint32_t hs_slider_size = ((element->real_content_width - 10) * hs_size / element->full_content_width);
            element->horizontal_scrollbar_offset = (element->horizontal_scrollbar_offset > (hs_size - hs_slider_size)) ? (hs_size - hs_slider_size) : element->horizontal_scrollbar_offset;
            draw_square(box, x + element->real_content_width - 10, y + element->real_content_height - 10, 10, 10, 0xFF666666);
            draw_vertical_scrollbar(box, x + element->real_content_width - 10, y, vs_size, vs_slider_size, element->vertical_scrollbar_offset);
            draw_horizontal_scrollbar(box, x, y + element->real_content_height - 10, hs_size, hs_slider_size, element->horizontal_scrollbar_offset);
        }
        else if(vertical_scrollbar == true) {
            uint32_t vs_size = element->real_content_height;
            uint32_t vs_slider_size = (element->real_content_height * vs_size / element->full_content_height);
            element->vertical_scrollbar_offset = (element->vertical_scrollbar_offset > (vs_size - vs_slider_size)) ? (vs_size - vs_slider_size) : element->vertical_scrollbar_offset;
            draw_vertical_scrollbar(box, x + element->real_content_width - 10, y, vs_size, vs_slider_size, element->vertical_scrollbar_offset);
        }
        else if(horizontal_scrollbar == true) {
            uint32_t hs_size = element->real_content_width;
            uint32_t hs_slider_size = (element->real_content_width * hs_size / element->full_content_width);
            element->horizontal_scrollbar_offset = (element->horizontal_scrollbar_offset > (hs_size - hs_slider_size)) ? (hs_size - hs_slider_size) : element->horizontal_scrollbar_offset;
            draw_horizontal_scrollbar(box, x, y + element->real_content_height - 10, hs_size, hs_slider_size, element->horizontal_scrollbar_offset);
        }
    }
}