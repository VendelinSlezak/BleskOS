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
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/kernel.h>

/* global variables */
uint32_t mouse_cursor_x = 0;
uint32_t mouse_cursor_y = 0;
const uint8_t *cursor_data = 
"B\n"
"BB\n"
"BWB\n"
"BWWB\n"
"BWWWB\n"
"BWWWWB\n"
"BWWWWWB\n"
"BWWWWWWB\n"
"BWWWWWWWB\n"
"BWWWWWWWWB\n"
"BWWWWWWWWB\n"
"BWWWWWWBB\n"
"BWWWWBB\n"
"BWWBB\n"
"BBB\n";

/* local variables */
uint32_t is_there_screen_subsystem = false;
void *global_double_buffer;
list_of_views_t *list_of_views;

/* functions */
void initialize_screen_subsystem(void) {
    if(very_unlikely(is_there_graphic_output_device == false)) {
        kernel_panic("[SCREEN] Can not initialize screen subsystem because there is no graphic output device");
    }

    global_double_buffer = kalloc(get_size_of_double_buffer());

    view_t *view = kalloc(sizeof(view_t));
    view->is_active = true;
    view->monitor = get_graphic_output_first_monitor_device();
    view->width = get_output_width();
    view->height = get_output_height();
    view->buffer = global_double_buffer;
    view->preview_buffer = NULL;

    screen_part_t *global_part = kalloc(sizeof(screen_part_t));
    global_part->x = 0;
    global_part->y = 0;
    global_part->width = view->width;
    global_part->height = view->height;
    global_part->view = view;
    global_part->parent = NULL;
    global_part->state = PART_STATE_MAIN_PANEL;
    global_part->split = 0;
    global_part->first_child = NULL;
    global_part->second_child = NULL;
    global_part->is_processed = true;

    view->global_part = global_part;

    list_of_views = kalloc(sizeof(list_of_views_t) + sizeof(view_t *) * 1);
    list_of_views->number_of_views = 1;
    list_of_views->views[0] = view;

    // split_part_vertically(global_part, view->width / 2);
    // split_part_horizontally(global_part->second_child, view->height / 2);
    // split_part_vertically(global_part->second_child->second_child, global_part->second_child->width / 2);
    // split_part_horizontally(global_part->second_child->first_child, global_part->second_child->first_child->height / 2);
    // dump_parts(global_part, 0);

    draw_view(view);
    redraw_screen();

    draw_mouse_cursor(global_part->width / 2, global_part->height / 2);
    is_there_screen_subsystem = true;
}

void draw_square(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    uint32_t *dst = (uint32_t *) ((uint32_t)global_double_buffer + (y * get_output_width() * 4) + (x * 4));
    for(uint32_t i = 0; i < height; i++) {
        for(uint32_t j = 0; j < width; j++) {
            dst[j] = color;
        }
        dst = (uint32_t *) ((uint32_t)dst + (get_output_width() * 4));
    }
}

void draw_main_panel(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    draw_square(x, y, width, height, 0x00C000);
}

void draw_program(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    draw_square(x, y, width, height, 0xFFFFFF);
}

void mark_all_parts_below_as_unprocessed(screen_part_t *part) {
    screen_part_t *stop_part = part->parent;
    while(part != stop_part) {
        if(part->first_child != NULL && part->first_child->is_processed == true) {
            part = part->first_child;
            continue;
        }
        if(part->second_child != NULL && part->second_child->is_processed == true) {
            part = part->second_child;
            continue;
        }
        part->is_processed = false;
        part = part->parent;
    }
}

void draw_view(view_t *view) {
    if(view->is_active == false) {
        return;
    }

    screen_part_t *part = view->global_part;
    mark_all_parts_below_as_unprocessed(part);
    while(part != NULL) {
        if(part->first_child != NULL && part->first_child->is_processed == false) {
            part = part->first_child;
            continue;
        }
        if(part->second_child != NULL && part->second_child->is_processed == false) {
            part = part->second_child;
            continue;
        }
        switch(part->state) {
            case PART_STATE_MAIN_PANEL: {
                log("\nMain panel %d %d %d %d", part->x, part->y, part->width, part->height);
                draw_main_panel(part->x, part->y, part->width, part->height);
                break;
            }
            case PART_STATE_PROGRAM: {
                log("\nProgram %d %d %d %d", part->x, part->y, part->width, part->height);
                draw_program(part->x, part->y, part->width, part->height);
                break;
            }
            case PART_STATE_HORIZONTAL_SPLIT: {
                log("\nHorizontal split %d %d", part->x, part->split);
                draw_square(part->x, part->y + part->split, part->width, 1, 0x000000);
                break;
            }
            case PART_STATE_VERTICAL_SPLIT: {
                log("\nVertical split %d %d", part->split, part->y);
                draw_square(part->x + part->split, part->y, 1, part->height, 0x000000);
                break;
            }
        }
        part->is_processed = true;
        part = part->parent;
    }
}

void redraw_screen(void) {
    for(uint32_t i = 0; i < list_of_views->number_of_views; i++) {
        if(list_of_views->views[i]->is_active == true) {
            redraw_full_screen(global_double_buffer);
        }
    }
}

void split_part_vertically(screen_part_t *part, uint32_t split) {
    part->first_child = kalloc(sizeof(screen_part_t));
    screen_part_t *first_child = part->first_child;
    first_child->x = part->x;
    first_child->y = part->y;
    first_child->width = split;
    first_child->height = part->height;
    first_child->view = part->view;
    first_child->parent = part;
    first_child->state = part->state;
    first_child->split = 0;
    first_child->first_child = NULL;
    first_child->second_child = NULL;
    first_child->is_processed = true;

    part->second_child = kalloc(sizeof(screen_part_t));
    screen_part_t *second_child = part->second_child;
    second_child->x = part->x + split + 1;
    second_child->y = part->y;
    second_child->width = part->width - split - 1;
    second_child->height = part->height;
    second_child->view = part->view;
    second_child->parent = part;
    second_child->state = PART_STATE_MAIN_PANEL;
    second_child->split = 0;
    second_child->first_child = NULL;
    second_child->second_child = NULL;
    second_child->is_processed = true;

    part->state = PART_STATE_VERTICAL_SPLIT;
    part->split = split;
    part->is_processed = true;
}

void split_part_horizontally(screen_part_t *part, uint32_t split) {
    part->first_child = kalloc(sizeof(screen_part_t));
    screen_part_t *first_child = part->first_child;
    first_child->x = part->x;
    first_child->y = part->y;
    first_child->width = part->width;
    first_child->height = split;
    first_child->view = part->view;
    first_child->parent = part;
    first_child->state = part->state;
    first_child->split = 0;
    first_child->first_child = NULL;
    first_child->second_child = NULL;
    first_child->is_processed = true;

    part->second_child = kalloc(sizeof(screen_part_t));
    screen_part_t *second_child = part->second_child;
    second_child->x = part->x;
    second_child->y = part->y + split + 1;
    second_child->width = part->width;
    second_child->height = part->height - split - 1;
    second_child->view = part->view;
    second_child->parent = part;
    second_child->state = PART_STATE_MAIN_PANEL;
    second_child->split = 0;
    second_child->first_child = NULL;
    second_child->second_child = NULL;
    second_child->is_processed = true;

    part->state = PART_STATE_HORIZONTAL_SPLIT;
    part->split = split;
    part->is_processed = true;
}

void remove_part(screen_part_t *part) {
    // replace parent with second child
    if(part->parent->first_child == part) {
        screen_part_t *old_parent = part->parent;
        screen_part_t *new_parent = part->parent->second_child;
        kfree(part);

        new_parent->x = old_parent->x;
        new_parent->y = old_parent->y;
        new_parent->width = old_parent->width;
        new_parent->height = old_parent->height;
        
        if(old_parent->parent == NULL) {
            ((view_t *)(old_parent->view))->global_part = new_parent;
        }
        else if(old_parent->parent->first_child == old_parent) {
            old_parent->parent->first_child = new_parent;
        }
        else {
            old_parent->parent->second_child = new_parent;
        }
        new_parent->parent = old_parent->parent;
        kfree(old_parent);

        part = new_parent;
    }
    else {
        screen_part_t *old_parent = part->parent;
        screen_part_t *new_parent = part->parent->first_child;
        kfree(part);

        new_parent->x = old_parent->x;
        new_parent->y = old_parent->y;
        new_parent->width = old_parent->width;
        new_parent->height = old_parent->height;

        if(old_parent->parent == NULL) {
            ((view_t *)(old_parent->view))->global_part = new_parent;
        }
        else if(old_parent->parent->first_child == old_parent) {
            old_parent->parent->first_child = new_parent;
        }
        else {
            old_parent->parent->second_child = new_parent;
        }
        new_parent->parent = old_parent->parent;
        kfree(old_parent);

        part = new_parent;
    }

    // update all parts in second child
    mark_all_parts_below_as_unprocessed(part);
    screen_part_t *stop_part = part->parent;
    while(part != stop_part) {
        if(part->first_child != NULL && part->first_child->is_processed == false) {
            if(part->state == PART_STATE_HORIZONTAL_SPLIT) {
                part->first_child->x = part->x;
                part->first_child->y = part->y;
                part->first_child->width = part->width;
                part->first_child->height = part->split;
            }
            else if(part->state == PART_STATE_VERTICAL_SPLIT) {
                part->first_child->x = part->x;
                part->first_child->y = part->y;
                part->first_child->width = part->split;
                part->first_child->height = part->height;
            }
            part->first_child->is_processed = false;
            part = part->first_child;
            continue;
        }
        if(part->second_child != NULL && part->second_child->is_processed == false) {
            if(part->state == PART_STATE_HORIZONTAL_SPLIT) {
                part->second_child->x = part->x;
                part->second_child->y = part->y + part->split + 1;
                part->second_child->width = part->width;
                part->second_child->height = part->height - part->split - 1;
            }
            else if(part->state == PART_STATE_VERTICAL_SPLIT) {
                part->second_child->x = part->x + part->split + 1;
                part->second_child->y = part->y;
                part->second_child->width = part->width - part->split - 1;
                part->second_child->height = part->height;
            }
            part->second_child->is_processed = false;
            part = part->second_child;
            continue;
        }
        part->is_processed = true;
        part = part->parent;
    }
}

void part_move_split(screen_part_t *part, uint32_t split) {
    mark_all_parts_below_as_unprocessed(part);
    part->is_processed = true;
    part->split = split;
    uint32_t splits_to_recalculate = part->state;
    screen_part_t *stop_part = part->parent;
    // every part is recalculated through those steps:
    // 1) check if split makes some child so small that it must be removed
    // 2) if child is also splitted, then update split position that ratio will be the same in new child size
    // 3) resize child
    while(part != stop_part) {
        if(part->first_child != NULL && part->first_child->is_processed == false) {
            if(part->state == PART_STATE_HORIZONTAL_SPLIT) {
                if(part->split < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->first_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                else if((part->height - part->split) < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->second_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                if(part->first_child->state == splits_to_recalculate) {
                    if(splits_to_recalculate == PART_STATE_HORIZONTAL_SPLIT) {
                        part->first_child->split = (part->first_child->split * part->split / part->first_child->height);
                    }
                    else if(splits_to_recalculate == PART_STATE_VERTICAL_SPLIT) {
                        part->first_child->split = (part->first_child->split * part->width / part->first_child->width);
                    }
                }
                part->first_child->x = part->x;
                part->first_child->y = part->y;
                part->first_child->width = part->width;
                part->first_child->height = part->split;
            }
            else if(part->state == PART_STATE_VERTICAL_SPLIT) {
                if(part->split < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->first_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                else if((part->width - part->split) < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->second_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                if(part->first_child->state == splits_to_recalculate) {
                    if(splits_to_recalculate == PART_STATE_HORIZONTAL_SPLIT) {
                        part->first_child->split = (part->first_child->split * part->height / part->first_child->height);
                    }
                    else if(splits_to_recalculate == PART_STATE_VERTICAL_SPLIT) {
                        part->first_child->split = (part->first_child->split * part->split / part->first_child->width);
                    }
                }
                part->first_child->x = part->x;
                part->first_child->y = part->y;
                part->first_child->width = part->split;
                part->first_child->height = part->height;
            }
            part->first_child->is_processed = true;
            part = part->first_child;
            continue;
        }
        if(part->second_child != NULL && part->second_child->is_processed == false) {
            if(part->state == PART_STATE_HORIZONTAL_SPLIT) {
                if(part->split < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->first_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                else if((part->height - part->split) < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->second_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                if(part->second_child->state == splits_to_recalculate) {
                    if(splits_to_recalculate == PART_STATE_HORIZONTAL_SPLIT) {
                        part->second_child->split = (part->second_child->split * part->split / part->second_child->height);
                    }
                    else if(splits_to_recalculate == PART_STATE_VERTICAL_SPLIT) {
                        part->second_child->split = (part->second_child->split * part->width / part->second_child->width);
                    }
                }
                part->second_child->x = part->x;
                part->second_child->y = part->y + part->split + 1;
                part->second_child->width = part->width;
                part->second_child->height = part->height - part->split - 1;
            }
            else if(part->state == PART_STATE_VERTICAL_SPLIT) {
                if(part->split < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->first_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                else if((part->width - part->split) < MINIMAL_PART_SIZE) {
                    screen_part_t *part_to_remove = part->second_child;
                    part = part->parent;
                    remove_part(part_to_remove);
                    continue;
                }
                if(part->second_child->state == splits_to_recalculate) {
                    if(splits_to_recalculate == PART_STATE_HORIZONTAL_SPLIT) {
                        part->second_child->split = (part->second_child->split * part->height / part->second_child->height);
                    }
                    else if(splits_to_recalculate == PART_STATE_VERTICAL_SPLIT) {
                        part->second_child->split = (part->second_child->split * part->split / part->second_child->width);
                    }
                }
                part->second_child->x = part->x + part->split + 1;
                part->second_child->y = part->y;
                part->second_child->width = part->width - part->split - 1;
                part->second_child->height = part->height;
            }
            part->second_child->is_processed = true;
            part = part->second_child;
            continue;
        }
        part = part->parent;
    }
}

void draw_mouse_cursor(uint32_t x, uint32_t y) {
    redraw_part_of_screen(mouse_cursor_x, mouse_cursor_y, global_double_buffer, get_output_width(), mouse_cursor_x, mouse_cursor_y, 16, 16);
    uint32_t bpp = get_output_bpp();
    void *dst = (void *) ((uint32_t)get_output_linear_frame_buffer() + (y * get_output_bytes_per_line()) + (x * (bpp / 8)));
    uint32_t i = 0;
    uint32_t line = y;
    uint32_t column = 0;
    while(cursor_data[i] != 0) {
        if(cursor_data[i] == 'B') {
            if((x + column) >= get_output_width()) {
                i++;
                continue;
            }
            if(bpp == 32) {
                ((uint32_t *)dst)[column] = 0x000000;
            }
            else if(bpp == 24) {
                ((uint8_t *)dst)[column * 3] = 0x00;
                ((uint8_t *)dst)[column * 3 + 1] = 0x00;
                ((uint8_t *)dst)[column * 3 + 2] = 0x00;
            }
            else if(bpp == 16 || bpp == 15) {
                ((uint16_t *)dst)[column] = 0x0000;
            }
            else if(bpp == 8) {
                ((uint8_t *)dst)[column] = 0x00;
            }
        }
        else if(cursor_data[i] == 'W') {
            if((x + column) >= get_output_width()) {
                i++;
                continue;
            }
            if(bpp == 32) {
                ((uint32_t *)dst)[column] = 0xFFFFFF;
            }
            else if(bpp == 24) {
                ((uint8_t *)dst)[column * 3] = 0xFF;
                ((uint8_t *)dst)[column * 3 + 1] = 0xFF;
                ((uint8_t *)dst)[column * 3 + 2] = 0xFF;
            }
            else if(bpp == 16 || bpp == 15) {
                ((uint16_t *)dst)[column] = 0xFFFF;
            }
            else if(bpp == 8) {
                ((uint8_t *)dst)[column] = 0xFF;
            }
        }
        else if(cursor_data[i] == '\n') {
            line++;
            if(line >= (get_output_height() - 1)) {
                break;
            }
            dst = (void *) ((uint32_t)dst + get_output_bytes_per_line());
            i++;
            column = 0;
            continue;
        }
        i++;
        column++;
    }
    mouse_cursor_x = x;
    mouse_cursor_y = y;
}

void move_mouse_cursor(uint32_t x_movement, uint32_t y_movement) {
    if(very_unlikely(is_there_screen_subsystem == false)) {
        return;
    }
    int new_x = mouse_cursor_x + x_movement;
    int new_y = mouse_cursor_y + y_movement;
    if(new_x < 0) {
        new_x = 0;
    }
    if(new_y < 0) {
        new_y = 0;
    }
    if(new_x >= get_output_width()) {
        new_x = get_output_width() - 1;
    }
    if(new_y >= (get_output_height() - 1)) {
        new_y = get_output_height() - 1;
    }

    draw_mouse_cursor(new_x, new_y);
}

void draw_dashed_column(screen_part_t *part, uint32_t x) {
    uint32_t line = part->y;
    while(line < (part->y + part->height)) {
        uint32_t line_height = 10;
        if(line + line_height > (part->y + part->height)) {
            line_height = part->y + part->height - line;
        }
        draw_square(part->x + x, line, 1, line_height, 0x000000);
        line += 20;
    }
}

void draw_solid_column(screen_part_t *part, uint32_t x) {
    draw_square(part->x + x, part->y, 1, part->height, 0x000000);
}

void draw_dashed_line(screen_part_t *part, uint32_t y) {
    uint32_t column = part->x;
    while(column < (part->x + part->width)) {
        uint32_t column_width = 10;
        if(column + column_width > (part->x + part->width)) {
            column_width = part->x + part->width - column;
        }
        draw_square(column, part->y + y, column_width, 1, 0x000000);
        column += 20;
    }
}

void draw_solid_line(screen_part_t *part, uint32_t y) {
    draw_square(part->x, part->y + y, part->width, 1, 0x000000);
}

// TODO: add column if previous split is not on those positions
void draw_part_in_vertical_fixed_editing_mode(screen_part_t *part) {
    draw_square(part->x, part->y, part->width, part->height, 0xFFFFFF); // TODO: this should be transparent

    fixed_editing_mode_area_t areas[5];
    areas[0].split_position = (part->width / 4);
    areas[1].split_position = (part->width / 3);
    areas[2].split_position = (part->width / 2);
    areas[3].split_position = (part->width / 3) * 2;
    areas[4].split_position = (part->width / 4) * 3;
    for(int i = 0; i < 5; i++) {
        if(i == 0) {
            areas[i].start_of_mouse_area = part->x;
            areas[i].end_of_mouse_area = (part->x + areas[i].split_position + ((areas[i + 1].split_position - areas[i].split_position) / 2));
        }
        else if(i == 4) {
            areas[i].start_of_mouse_area = (areas[i - 1].end_of_mouse_area + 1);
            areas[i].end_of_mouse_area = (part->x + part->width);
        }
        else {
            areas[i].start_of_mouse_area = (areas[i - 1].end_of_mouse_area + 1);
            areas[i].end_of_mouse_area = (part->x + areas[i].split_position + ((areas[i + 1].split_position - areas[i].split_position) / 2));
        }
    }

    for(int i = 0; i < 5; i++) {
        if(    mouse_cursor_x >= areas[i].start_of_mouse_area
            && mouse_cursor_x <= areas[i].end_of_mouse_area
            && mouse_cursor_y >= part->y
            && mouse_cursor_y < (part->y + part->height)) {
            draw_solid_column(part, areas[i].split_position);
        }
        else {
            draw_dashed_column(part, areas[i].split_position);
        }
    }
}

// TODO: add line if previous split is not on those positions
void draw_part_in_horizontal_fixed_editing_mode(screen_part_t *part) {
    draw_square(part->x, part->y, part->width, part->height, 0xFFFFFF); // TODO: this should be transparent

    fixed_editing_mode_area_t areas[5];
    areas[0].split_position = (part->height / 4);
    areas[1].split_position = (part->height / 3);
    areas[2].split_position = (part->height / 2);
    areas[3].split_position = (part->height / 3) * 2;
    areas[4].split_position = (part->height / 4) * 3;
    for(int i = 0; i < 5; i++) {
        if(i == 0) {
            areas[i].start_of_mouse_area = part->y;
            areas[i].end_of_mouse_area = (part->y + areas[i].split_position + ((areas[i + 1].split_position - areas[i].split_position) / 2));
        }
        else if(i == 4) {
            areas[i].start_of_mouse_area = (areas[i - 1].end_of_mouse_area + 1);
            areas[i].end_of_mouse_area = (part->y + part->height);
        }
        else {
            areas[i].start_of_mouse_area = (areas[i - 1].end_of_mouse_area + 1);
            areas[i].end_of_mouse_area = (part->y + areas[i].split_position + ((areas[i + 1].split_position - areas[i].split_position) / 2));
        }
    }

    for(int i = 0; i < 5; i++) {
        if(    mouse_cursor_y >= areas[i].start_of_mouse_area
            && mouse_cursor_y <= areas[i].end_of_mouse_area
            && mouse_cursor_x >= part->x
            && mouse_cursor_x < (part->x + part->width)) {
            draw_solid_line(part, areas[i].split_position);
        }
        else {
            draw_dashed_line(part, areas[i].split_position);
        }
    }
}

void draw_part_in_vertical_free_editing_mode(screen_part_t *part) {
    draw_square(part->x, part->y, part->width, part->height, 0xFFFFFF); // TODO: this should be transparent
    if(    mouse_cursor_x >= part->x
        && mouse_cursor_x < (part->x + part->width)
        && mouse_cursor_y >= part->y
        && mouse_cursor_y < (part->y + part->height)) {
        draw_solid_column(part, mouse_cursor_x - part->x);
    }
}

void draw_part_in_horizontal_free_editing_mode(screen_part_t *part) {
    draw_square(part->x, part->y, part->width, part->height, 0xFFFFFF); // TODO: this should be transparent
    if(    mouse_cursor_x >= part->x
        && mouse_cursor_x < (part->x + part->width)
        && mouse_cursor_y >= part->y
        && mouse_cursor_y < (part->y + part->height)) {
        draw_solid_line(part, mouse_cursor_y - part->y);
    }
}

void dump_parts(screen_part_t *part, int depth) {
    log("\n");
    for(int i = 0; i < depth; i++) {
        log("    ");
    }
    log("Part %d %d %d %d split %d is processed %d", part->x, part->y, part->width, part->height, part->split, part->is_processed);
    if(part->first_child == NULL && part->second_child == NULL) {
        return;
    }
    dump_parts(part->first_child, depth + 1);
    dump_parts(part->second_child, depth + 1);
}