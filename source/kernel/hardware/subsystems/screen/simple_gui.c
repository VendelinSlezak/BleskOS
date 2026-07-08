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
#include <kernel/kernel.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/font.h>
#include <kernel/hardware/subsystems/screen/draw.h>
#include <kernel/hardware/subsystems/screen/stb_image_implementation.h>
#include <kernel/libc/string.h>

/* functions */
block_t *create_first_block(void) {
    block_t *first_block = kalloc(sizeof(block_t));
    first_block->type = VERTICAL_BLOCK;
    first_block->horizontal_alignment = ALIGN_LEFT;
    first_block->vertical_alignment = ALIGN_TOP;
    first_block->max_width = 0x7FFFFFFF;
    first_block->max_height = 0x7FFFFFFF;
    return first_block;
}

block_t *add_block(block_t **parent_block_ptr, uint32_t placement, block_type_t type, void *data) {
    block_t *original_parent_block = *parent_block_ptr;
    *parent_block_ptr = krealloc(original_parent_block, sizeof(block_t) + ((original_parent_block->number_of_blocks + 1) * sizeof(block_t *)));
    block_t *parent_block = *parent_block_ptr;

    if(parent_block->parent != NULL) {
        for(int i = 0; i < parent_block->parent->number_of_blocks; i++) {
            if(parent_block->parent->blocks[i] == original_parent_block) {
                parent_block->parent->blocks[i] = parent_block;
            }
        }
    }

    block_t *block = kalloc(sizeof(block_t));
    if(placement == ADD_FROM_START) {
        memmove(&parent_block->blocks[parent_block->number_of_blocks], &parent_block->blocks[parent_block->number_of_blocks_from_start], sizeof(block_t *) * parent_block->number_of_blocks_from_end);
        parent_block->blocks[parent_block->number_of_blocks_from_start] = block;
        parent_block->number_of_blocks_from_start++;
    }
    else {
        parent_block->blocks[parent_block->number_of_blocks] = block;
        parent_block->number_of_blocks_from_end++;
    }
    parent_block->number_of_blocks++;

    for(int i = 0; i < parent_block->number_of_blocks; i++) {
        parent_block->blocks[i]->parent = parent_block;
    }

    block->type = type;
    block->data = data;

    block->horizontal_alignment = ALIGN_LEFT;
    block->vertical_alignment = ALIGN_TOP;
    block->max_width = 0x7FFFFFFF;
    block->max_height = 0x7FFFFFFF;

    block->draw_from_two_ends = true;

    return block;
}

void draw_gui_blocks(screen_part_t *part, block_t *block, uint32_t x_offset, uint32_t y_offset) {
    // set all max dimensions by inheriting max dimensions from blocks above
    set_block_max_dimensions(block, 0x7FFFFFFF, 0x7FFFFFFF);

    // calculate all widths and heights
    calculate_block_dimensions(block);

    // draw blocks
    if(part->event_list != NULL) {
        kfree(part->event_list);
    }
    part->event_list = kalloc(sizeof(click_area_list_t));
    draw_gui_block(part, block, (click_area_list_t **) &part->event_list, x_offset * -1, y_offset * -1);
}

void set_block_max_dimensions(block_t *block, int max_width, int max_height) {
    block->actual_max_width = (block->max_width < max_width) ? block->max_width : max_width;
    block->actual_max_height = (block->max_height < max_height) ? block->max_height : max_height;

    for(int i = 0; i < block->number_of_blocks; i++) {
        set_block_max_dimensions(block->blocks[i], block->actual_max_width, block->actual_max_height);
    }
}

void calculate_block_dimensions(block_t *block) {
    if(block->type == VERTICAL_BLOCK || block->type == HORIZONTAL_BLOCK) {
        for(int i = 0; i < block->number_of_blocks; i++) {
            calculate_block_dimensions(block->blocks[i]);
        }
    }

    switch(block->type) {
        case VERTICAL_BLOCK: {
            uint32_t width = 0, height = 0, line_width = 0, line_height = 0;

            for(int i = 0; i < block->number_of_blocks; i++) {
                if((line_height + block->blocks[i]->actual_height) > block->actual_max_height) {
                    width += line_width;
                    height = (height < line_height) ? line_height : height;
                    line_width = 0;
                    line_height = 0;
                }
                line_width = (line_width < block->blocks[i]->actual_width) ? block->blocks[i]->actual_width : line_width;
                line_height += block->blocks[i]->actual_height;
            }
            width += line_width;
            height = (height < line_height) ? line_height : height;

            block->inside_width = width;
            if(width < block->min_width) {
                width = block->min_width;
            }
            block->content_width = width;
            block->inside_height = height;
            if(height < block->min_height) {
                height = block->min_height;
            }
            else {
                block->draw_from_two_ends = false;
            }
            block->content_height = height;
            width += block->margin_left + block->border_left_size + block->padding_left + block->padding_right + block->border_right_size + block->margin_right;
            height += block->margin_top + block->border_top_size + block->padding_top + block->padding_bottom + block->border_bottom_size + block->margin_bottom;
            block->actual_width = width;
            block->actual_height = height;
            break;
        }
        case HORIZONTAL_BLOCK: {
            uint32_t width = 0, height = 0, line_width = 0, line_height = 0;

            for(int i = 0; i < block->number_of_blocks; i++) {
                if((line_width + block->blocks[i]->actual_width) > block->actual_max_width) {
                    width = (width < line_width) ? line_width : width;
                    height += line_height;
                    line_width = 0;
                    line_height = 0;
                }
                line_width += block->blocks[i]->actual_width;
                line_height = (line_height < block->blocks[i]->actual_height) ? block->blocks[i]->actual_height : line_height;
            }
            width = (width < line_width) ? line_width : width;
            height += line_height;

            block->inside_width = width;
            if(width < block->min_width) {
                width = block->min_width;
            }
            else {
                block->draw_from_two_ends = false;
            }
            block->content_width = width;
            block->inside_height = height;
            if(height < block->min_height) {
                height = block->min_height;
            }
            block->content_height = height;
            width += block->margin_left + block->border_left_size + block->padding_left + block->padding_right + block->border_right_size + block->margin_right;
            height += block->margin_top + block->border_top_size + block->padding_top + block->padding_bottom + block->border_bottom_size + block->margin_bottom;
            block->actual_width = width;
            block->actual_height = height;
            break;
        }
        case TEXT_BLOCK:
        case DYNAMIC_TEXT_BLOCK: {
            uint8_t *string = block->data;
            if(block->type == DYNAMIC_TEXT_BLOCK) {
                uint8_t *(*get_string)(void) = block->data;
                string = (*get_string)();
            }

            if(block->text != NULL) {
                kfree(block->text);
            }
            text_t *text = kalloc(sizeof(text_t));

            uint32_t width = 0, height = 0;
            uint8_t *start_of_line = string;
            uint32_t actual_line_width = 0, actual_line_height = 0;
            uint8_t *end_of_line = start_of_line;
            uint32_t line_width = 0, line_height = 0, in_word = false;
            uint8_t *scanning_ptr = start_of_line;
            while(true) {
                // TODO: process UTF-8 string characters
                uint32_t character = *scanning_ptr, character_bytes = 1, char_width = 8, char_height = 16;

                if((line_width + char_width) > block->actual_max_width || character == '\0') {
                    if(very_unlikely(scanning_ptr == start_of_line)) {
                        break;
                    }
                    if(in_word == true && character == '\0') {
                        end_of_line = scanning_ptr;
                        actual_line_width = line_width;
                        actual_line_height = line_height;
                        in_word = false;
                    }
                    if(start_of_line == end_of_line) { // one word is so long that it is on whole line
                        // set end of actual line
                        end_of_line = scanning_ptr;

                        // process new line
                        text = krealloc(text, sizeof(text_t) + (text->number_of_lines * sizeof(line_of_text_t)) + (sizeof(line_of_text_t)));
                        text->lines[text->number_of_lines].start = start_of_line;
                        text->lines[text->number_of_lines].end = end_of_line;
                        text->lines[text->number_of_lines].width = line_width;
                        text->lines[text->number_of_lines].height = line_height;
                        text->number_of_lines++;
                        width = (width < line_width) ? line_width : width;
                        height += line_height;

                        // move to next line
                        while(*end_of_line == ' ') {
                            end_of_line++;
                        }
                        start_of_line = end_of_line;
                        actual_line_width = 0;
                        actual_line_height = 0;
                        line_width = 0;
                        line_height = 0;
                    }
                    else {
                        // process new line
                        text = krealloc(text, sizeof(text_t) + (text->number_of_lines * sizeof(line_of_text_t)) + (sizeof(line_of_text_t)));
                        text->lines[text->number_of_lines].start = start_of_line;
                        text->lines[text->number_of_lines].end = end_of_line;
                        text->lines[text->number_of_lines].width = actual_line_width;
                        text->lines[text->number_of_lines].height = actual_line_height;
                        text->number_of_lines++;
                        width = (width < actual_line_width) ? actual_line_width : width;
                        height += actual_line_height;

                        // move to next line
                        while(*end_of_line == ' ') {
                            end_of_line++;
                        }
                        start_of_line = end_of_line;
                        scanning_ptr = start_of_line;
                        actual_line_width = 0;
                        actual_line_height = 0;
                        line_width = 0;
                        line_height = 0;
                    }
                    if(*scanning_ptr == '\0') {
                        break;
                    }
                    continue;
                }

                if(character != ' ') {
                    in_word = true;
                }
                else if(in_word == true) {
                    end_of_line = scanning_ptr;
                    actual_line_width = line_width;
                    actual_line_height = line_height;
                    in_word = false;
                }

                line_width += char_width;
                line_height = (line_height < char_height) ? char_height : line_height;
                scanning_ptr += character_bytes;
            }
            text->width = width;
            text->height = height;
            block->text = text;

            block->inside_width = width;
            if(width < block->min_width) {
                width = block->min_width;
            }
            block->content_width = width;
            block->inside_height = height;
            if(height < block->min_height) {
                height = block->min_height;
            }
            block->content_height = height;
            width += block->margin_left + block->border_left_size + block->padding_left + block->padding_right + block->border_right_size + block->margin_right;
            height += block->margin_top + block->border_top_size + block->padding_top + block->padding_bottom + block->border_bottom_size + block->margin_bottom;
            block->actual_width = width;
            block->actual_height = height;
            break;
        }
        case IMAGE_BLOCK: {
            uint32_t width = 0, height = 0;

            image_t *image = block->data;
            width = image->width;
            height = image->height;

            block->inside_width = width;
            if(width < block->min_width) {
                width = block->min_width;
            }
            block->content_width = width;
            block->inside_height = height;
            if(height < block->min_height) {
                height = block->min_height;
            }
            block->content_height = height;
            width += block->margin_left + block->border_left_size + block->padding_left + block->padding_right + block->border_right_size + block->margin_right;
            height += block->margin_top + block->border_top_size + block->padding_top + block->padding_bottom + block->border_bottom_size + block->margin_bottom;
            block->actual_width = width;
            block->actual_height = height;
            break;
        }
        case INTERACTIVE_DRAW: {
            break;
        }
    }
}

void draw_gui_block(screen_part_t *part, block_t *block, click_area_list_t **click_area_list, uint32_t x, uint32_t y) {
    if(block->process_click != NULL) {
        calculated_area_t area = calculate_area_in_part(part, x, y, block->actual_width, block->actual_height);
        if(area.drawable == true) {
            click_area_list_t *list = *click_area_list;
            *click_area_list = krealloc(list, sizeof(click_area_list_t) + (sizeof(click_area_t) * (list->number_of_click_areas + 1)));
            list = *click_area_list;
            uint32_t index = list->number_of_click_areas;
            list->click_areas[index].x = part->x + area.left;
            list->click_areas[index].y = part->y + area.top;
            list->click_areas[index].width = area.width;
            list->click_areas[index].height = area.height;
            list->click_areas[index].process_click = block->process_click;
            list->click_areas[index].argument = block->argument;
            list->number_of_click_areas++;
        }
    }

    if(block->border_top_size != 0) {
        draw_square_in_part(part,
                            x + block->margin_left,
                            y + block->margin_top, 
                            block->border_left_size + block->padding_left + block->content_width + block->padding_right + block->border_right_size,
                            block->border_top_size,
                            block->border_color);
    }
    if(block->border_bottom_size != 0) {
        draw_square_in_part(part,
                            x + block->margin_left,
                            y + block->margin_top + block->border_top_size + block->padding_top + block->content_height + block->padding_bottom,
                            block->border_left_size + block->padding_left + block->content_width + block->padding_right + block->border_right_size,
                            block->border_bottom_size,
                            block->border_color);
    }
    if(block->border_left_size != 0) {
        draw_square_in_part(part,
                            x + block->margin_left,
                            y + block->margin_top,
                            block->border_left_size,
                            block->border_top_size + block->padding_top + block->content_height + block->padding_bottom + block->border_bottom_size,
                            block->border_color);
    }
    if(block->border_right_size != 0) {
        draw_square_in_part(part,
                            x + block->margin_left + block->border_left_size + block->padding_left + block->content_width + block->padding_right,
                            y + block->margin_top,
                            block->border_right_size,
                            block->border_top_size + block->padding_top + block->content_height + block->padding_bottom + block->border_bottom_size,
                            block->border_color);
    }

    if(block->background_color != 0x00000000) {
        draw_square_in_part(part, 
                            x + block->margin_left + block->border_left_size,
                            y + block->margin_top + block->border_top_size,
                            block->padding_left + block->content_width + block->padding_right,
                            block->padding_top + block->content_height + block->padding_bottom,
                            block->background_color);
    }

    x += block->margin_left + block->border_left_size + block->padding_left;
    y += block->margin_top + block->border_top_size + block->padding_top;

    switch(block->type) {
        case VERTICAL_BLOCK: {
            if(block->number_of_blocks == 0) {
                break;
            }
            if(block->draw_from_two_ends == true) { // draw on one line, so draw from two ends
                uint32_t line_y = y;
                for(int b = 0; b < block->number_of_blocks_from_start; b++) {
                    switch(block->horizontal_alignment) {
                        case ALIGN_CENTER: {
                            draw_gui_block(part, block->blocks[b], click_area_list, x + (block->content_width / 2) - (block->blocks[b]->actual_width / 2), line_y);
                            line_y += block->blocks[b]->actual_height;
                            break;
                        }
                        case ALIGN_RIGHT: {
                            draw_gui_block(part, block->blocks[b], click_area_list, x + block->content_width - block->blocks[b]->actual_width, line_y);
                            line_y += block->blocks[b]->actual_height;
                            break;
                        }
                        case ALIGN_LEFT:
                        default: {
                            draw_gui_block(part, block->blocks[b], click_area_list, x, line_y);
                            line_y += block->blocks[b]->actual_height;
                            break;
                        }
                    }
                }

                line_y = (y + block->content_height);
                for(int b = block->number_of_blocks - 1; b >= block->number_of_blocks_from_start; b--) {
                    switch(block->horizontal_alignment) {
                        case ALIGN_CENTER: {
                            line_y -= block->blocks[b]->actual_height;
                            draw_gui_block(part, block->blocks[b], click_area_list, x + (block->content_width / 2) - (block->blocks[b]->actual_width / 2), line_y);
                            break;
                        }
                        case ALIGN_RIGHT: {
                            line_y -= block->blocks[b]->actual_height;
                            draw_gui_block(part, block->blocks[b], click_area_list, x + block->content_width - block->blocks[b]->actual_width, line_y);
                            break;
                        }
                        case ALIGN_LEFT:
                        default: {
                            line_y -= block->blocks[b]->actual_height;
                            draw_gui_block(part, block->blocks[b], click_area_list, x, line_y);
                            break;
                        }
                    }
                }
            }
            else { // draw multiple lines, so do not care about drawing from two ends
                switch(block->horizontal_alignment) {
                    case ALIGN_CENTER: {
                        x = x + (block->actual_width / 2) - (block->inside_width / 2);
                        break;
                    }
                    case ALIGN_RIGHT: {
                        x = x + block->actual_width - block->inside_width;
                        break;
                    }
                    case ALIGN_LEFT:
                    default: {
                        break;
                    }
                }

                uint32_t first_block_in_line = 0, last_block_in_line = 0, line_width = 0, line_height = 0;
                for(int b = 0; b < block->number_of_blocks; b++) {
                    if((line_height + block->blocks[b]->actual_height) > block->content_height) {
                        uint32_t line_y = y;
                        for(int i = first_block_in_line; i <= last_block_in_line; i++) {
                            switch(block->horizontal_alignment) {
                                case ALIGN_CENTER: {
                                    draw_gui_block(part, block->blocks[i], click_area_list, x + (line_width / 2) - (block->blocks[i]->actual_width / 2), line_y);
                                    line_y += block->blocks[i]->actual_height;
                                    break;
                                }
                                case ALIGN_RIGHT: {
                                    draw_gui_block(part, block->blocks[i], click_area_list, x + line_width - block->blocks[i]->actual_width, line_y);
                                    line_y += block->blocks[i]->actual_height;
                                    break;
                                }
                                case ALIGN_LEFT:
                                default: {
                                    draw_gui_block(part, block->blocks[i], click_area_list, x, line_y);
                                    line_y += block->blocks[i]->actual_height;
                                    break;
                                }
                            }
                        }
                        x += line_width;
                        first_block_in_line = b;
                        last_block_in_line = b;
                        line_width = 0;
                        line_height = 0;
                    }

                    line_width = (line_width > block->blocks[b]->actual_width) ? line_width : block->blocks[b]->actual_width;
                    line_height += block->blocks[b]->actual_height;
                    last_block_in_line = b;
                }

                uint32_t line_y = y;
                for(int i = first_block_in_line; i <= last_block_in_line; i++) {
                    switch(block->horizontal_alignment) {
                        case ALIGN_CENTER: {
                            draw_gui_block(part, block->blocks[i], click_area_list, x + (line_width / 2) - (block->blocks[i]->actual_width / 2), line_y);
                            line_y += block->blocks[i]->actual_height;
                            break;
                        }
                        case ALIGN_RIGHT: {
                            draw_gui_block(part, block->blocks[i], click_area_list, x + line_width - block->blocks[i]->actual_width, line_y);
                            line_y += block->blocks[i]->actual_height;
                            break;
                        }
                        case ALIGN_LEFT:
                        default: {
                            draw_gui_block(part, block->blocks[i], click_area_list, x, line_y);
                            line_y += block->blocks[i]->actual_height;
                            break;
                        }
                    }
                }
            }
            break;
        }
        case HORIZONTAL_BLOCK: {
            if(block->number_of_blocks == 0) {
                break;
            }
            if(block->draw_from_two_ends == true) { // draw on one line, so draw from two ends
                uint32_t line_x = x;
                for(int b = 0; b < block->number_of_blocks_from_start; b++) {
                    switch(block->vertical_alignment) {
                        case ALIGN_CENTER: {
                            draw_gui_block(part, block->blocks[b], click_area_list, line_x, y + (block->content_height / 2) - (block->blocks[b]->actual_height / 2));
                            line_x += block->blocks[b]->actual_width;
                            break;
                        }
                        case ALIGN_BOTTOM: {
                            draw_gui_block(part, block->blocks[b], click_area_list, line_x, y + block->content_height - block->blocks[b]->actual_height);
                            line_x += block->blocks[b]->actual_width;
                            break;
                        }
                        case ALIGN_TOP:
                        default: {
                            draw_gui_block(part, block->blocks[b], click_area_list, line_x, y);
                            line_x += block->blocks[b]->actual_width;
                            break;
                        }
                    }
                }

                line_x = (x + block->content_width);
                for(int b = block->number_of_blocks - 1; b >= block->number_of_blocks_from_start; b--) {
                    switch(block->vertical_alignment) {
                        case ALIGN_CENTER: {
                            line_x -= block->blocks[b]->actual_width;
                            draw_gui_block(part, block->blocks[b], click_area_list, line_x, y + (block->content_height / 2) - (block->blocks[b]->actual_height / 2));
                            break;
                        }
                        case ALIGN_BOTTOM: {
                            line_x -= block->blocks[b]->actual_width;
                            draw_gui_block(part, block->blocks[b], click_area_list, line_x, y + block->content_height - block->blocks[b]->actual_height);
                            break;
                        }
                        case ALIGN_TOP:
                        default: {
                            line_x -= block->blocks[b]->actual_width;
                            draw_gui_block(part, block->blocks[b], click_area_list, line_x, y);
                            break;
                        }
                    }
                }
            }
            else { // draw multiple lines, so do not care about drawing from two ends
                switch(block->vertical_alignment) {
                    case ALIGN_CENTER: {
                        y = y + (block->actual_height / 2) - (block->inside_height / 2);
                        break;
                    }
                    case ALIGN_BOTTOM: {
                        y = y + block->content_height - block->inside_height;
                        break;
                    }
                    case ALIGN_TOP:
                    default: {
                        break;
                    }
                }

                uint32_t first_block_in_line = 0, last_block_in_line = 0, line_width = 0, line_height = 0;
                for(int b = 0; b < block->number_of_blocks; b++) {
                    if((line_width + block->blocks[b]->actual_width) > block->content_width) {
                        uint32_t line_x = x;
                        for(int i = first_block_in_line; i <= last_block_in_line; i++) {
                            switch(block->vertical_alignment) {
                                case ALIGN_CENTER: {
                                    draw_gui_block(part, block->blocks[i], click_area_list, line_x, y + (line_height / 2) - (block->blocks[i]->actual_height / 2));
                                    line_x += block->blocks[i]->actual_width;
                                    break;
                                }
                                case ALIGN_BOTTOM: {
                                    draw_gui_block(part, block->blocks[i], click_area_list, line_x, y + line_height - block->blocks[i]->actual_height);
                                    line_x += block->blocks[i]->actual_width;
                                    break;
                                }
                                case ALIGN_TOP:
                                default: {
                                    draw_gui_block(part, block->blocks[i], click_area_list, line_x, y);
                                    line_x += block->blocks[i]->actual_width;
                                    break;
                                }
                            }
                        }
                        y += line_height;
                        first_block_in_line = b;
                        last_block_in_line = b;
                        line_width = 0;
                        line_height = 0;
                    }

                    line_width += block->blocks[b]->actual_width;
                    line_height = (line_height > block->blocks[b]->actual_height) ? line_height : block->blocks[b]->actual_height;
                    last_block_in_line = b;
                }

                uint32_t line_x = x;
                for(int i = first_block_in_line; i <= last_block_in_line; i++) {
                    switch(block->vertical_alignment) {
                        case ALIGN_CENTER: {
                            draw_gui_block(part, block->blocks[i], click_area_list, line_x, y + (line_height / 2) - (block->blocks[i]->actual_height / 2));
                            line_x += block->blocks[i]->actual_width;
                            break;
                        }
                        case ALIGN_BOTTOM: {
                            draw_gui_block(part, block->blocks[i], click_area_list, line_x, y + line_height - block->blocks[i]->actual_height);
                            line_x += block->blocks[i]->actual_width;
                            break;
                        }
                        case ALIGN_TOP:
                        default: {
                            draw_gui_block(part, block->blocks[i], click_area_list, line_x, y);
                            line_x += block->blocks[i]->actual_width;
                            break;
                        }
                    }
                }
            }
            break;

        }
        case TEXT_BLOCK:
        case DYNAMIC_TEXT_BLOCK: {
            text_t *text = block->text;
            if(block->content_height > text->height) {
                if(block->vertical_alignment == ALIGN_CENTER) {
                    y = (y + (block->content_height / 2) - (text->height / 2));
                }
                else if(block->vertical_alignment == ALIGN_BOTTOM) {
                    y = (y + block->content_height - text->height);
                }
            }
            for(int i = 0; i < text->number_of_lines; i++) {
                uint32_t line_x = x;
                if(block->actual_width > text->lines[i].width) {
                    if(block->horizontal_alignment == ALIGN_CENTER) {
                        line_x = (x + (block->actual_width / 2) - (text->lines[i].width / 2));
                    }
                    else if(block->horizontal_alignment == ALIGN_RIGHT) {
                        line_x = (x + block->actual_width - text->lines[i].width);
                    }
                }

                uint8_t *ptr = text->lines[i].start;
                while(ptr < text->lines[i].end) {
                    uint32_t character = *ptr, character_bytes = 1, char_width = 8, char_height = 16;
                    draw_bitmap_char(part, line_x, y, character, block->text_color);
                    line_x += char_width;
                    ptr += character_bytes;
                }

                y += text->lines[i].height;
            }
            break;
        }
        case IMAGE_BLOCK: {
            image_t *image = block->data;
            if(block->content_width > image->width) {
                if(block->horizontal_alignment == ALIGN_CENTER) {
                    x = (x + (block->content_width / 2) - (image->width / 2));
                }
                else if(block->horizontal_alignment == ALIGN_RIGHT) {
                    x = (x + block->content_width - image->width);
                }
            }
            if(block->actual_height > image->height) {
                if(block->vertical_alignment == ALIGN_CENTER) {
                    y = (y + (block->actual_height / 2) - (image->height / 2));
                }
                else if(block->vertical_alignment == ALIGN_BOTTOM) {
                    y = (y + block->actual_height - image->height);
                }
            }
            draw_image(block->data, part, x, y);
            break;
        }
        case INTERACTIVE_DRAW: {
            break;
        }
    }
}

void set_block_clickable(block_t *block, void (*process_click)(screen_part_t *part, uint32_t argument), uint32_t argument) {
    block->process_click = process_click;
    block->argument = argument;
}

void dump_gui_block(block_t *block, int depth) {
    if (block == NULL) return;

    #define INDENT() for(int i = 0; i < depth; i++) { log("    "); }

    log("\n");
    INDENT(); log("=== BLOCK DUMP (Address: 0x%x) ===\n", (void*)block);
    INDENT(); log("Type: %d\n", block->type);

    INDENT(); log("Horizontal alignment: %d, Vertical alignment: %d\n", 
                  block->horizontal_alignment, block->vertical_alignment);
    INDENT(); log("Margin  - T: %d, B: %d, L: %d, R: %d\n", 
                  block->margin_top, block->margin_bottom, block->margin_left, block->margin_right);
    INDENT(); log("Padding - T: %d, B: %d, L: %d, R: %d\n", 
                  block->padding_top, block->padding_bottom, block->padding_left, block->padding_right);
    INDENT(); log("Border size: %d %d %d %d, Border color: 0x%08X\n",
        block->border_top_size,
        block->border_bottom_size,
        block->border_left_size,
        block->border_right_size,
        block->border_color);
    INDENT(); log("Background color: 0x%08X\n", block->background_color);
    INDENT(); log("Text color: 0x%08X\n", block->text_color);
    INDENT(); log("Width  - Min: %d, Max: %d, ActMax: %d, Actual: %d\n", 
                  block->min_width, block->max_width, block->actual_max_width, block->actual_width);
    INDENT(); log("Height - Min: %d, Max: %d, ActMax: %d, Actual: %d\n", 
                  block->min_height, block->max_height, block->actual_max_height, block->actual_height);
    INDENT(); log("Number of child blocks: %u", block->number_of_blocks);

    #undef INDENT

    for (uint32_t i = 0; i < block->number_of_blocks; i++) {
        if (block->blocks[i] != NULL) {
            dump_gui_block(block->blocks[i], depth + 1);
        }
    }
}

void free_gui_blocks(block_t *block) {
    for(int i = 0; i < block->number_of_blocks; i++) {
        free_gui_blocks(block->blocks[i]);
    }
    kfree(block);
}

void process_left_click_event(screen_part_t *part, uint32_t x, uint32_t y) {
    click_area_list_t *list = part->event_list;
    for(int i = (list->number_of_click_areas - 1); i >= 0; i--) {
        if(    x >= list->click_areas[i].x
            && x < (list->click_areas[i].x + list->click_areas[i].width)
            && y >= list->click_areas[i].y
            && y < (list->click_areas[i].y + list->click_areas[i].height)) {
            list->click_areas[i].process_click(part, list->click_areas[i].argument);
            return;
        }
    }
}