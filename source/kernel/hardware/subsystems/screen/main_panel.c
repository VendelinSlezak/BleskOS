/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/font.h>
#include <kernel/hardware/subsystems/screen/stb_image.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/subsystems/screen/draw.h>
#include <kernel/hardware/subsystems/screen/simple_gui.h>
#include <kernel/hardware/subsystems/screen/stb_image_implementation.h>
#include <kernel/software/ramdisk.h>
#include <kernel/libc/string.h>

/* local variables */
ramdisk_elf_program_list_t *program_list;
uint32_t number_of_running_programs = 0;

/* functions */
void initialize_main_panel(void) {
    program_list = get_ramdisk_elf_program_list();
    image_t *program_1_logo = load_image(get_ramdisk_file_ptr("default_elf_icon.png"), get_ramdisk_file_size("default_elf_icon.png"));
    for(int i = 0; i < program_list->number_of_programs; i++) {
        if(program_list->programs[i].icon == NULL) {
            program_list->programs[i].icon = program_1_logo;
        }
        log("\n[MAIN PANEL] Founded program: %s", program_list->programs[i].name);
    }
}

void draw_main_panel(screen_part_t *part) {
    block_t *first_block = create_first_block();
    block_t *big_block = add_block(&first_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    big_block->min_width = part->width;
    big_block->min_height = part->height;
    big_block->background_color = 0xFF00C000;
    big_block->padding_left = 10;
    big_block->padding_right = 10;
    big_block->padding_top = 10;
    big_block->padding_bottom = 10;
    block_t *software_block = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    software_block->min_width = (part->width - 20) / 2;
    software_block->max_width = (part->width - 20) / 2;
    software_block->min_height = (part->height - 20);
    block_t *hardware_block = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    hardware_block->min_width = (part->width - 20) / 2;
    hardware_block->max_width = (part->width - 20) / 2;
    hardware_block->min_height = (part->height - 20);

    block_t *tb1 = add_block(&software_block, ADD_FROM_START, TEXT_BLOCK, "Software");
    tb1->text_color = 0xFF000000;
    tb1->padding_bottom = 20;

    block_t *hb1 = add_block(&software_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    hb1->padding_bottom = 20;
    for(int i = 0; i < program_list->number_of_programs; i++) {
        block_t *p1 = add_block(&hb1, ADD_FROM_START, VERTICAL_BLOCK, NULL);
        p1->min_width = 96;
        p1->max_width = 96;
        p1->horizontal_alignment = ALIGN_CENTER;
        p1->padding_right = 10;
        p1->padding_bottom = 10;
        p1->background_color = (program_list->programs[i].is_loaded_into_memory == true) ? 0xFF00AA00 : 0x00000000;
        block_t *ib1 = add_block(&p1, ADD_FROM_START, IMAGE_BLOCK, program_list->programs[i].icon);
        ib1->padding_bottom = 10;
        block_t *tb2 = add_block(&p1, ADD_FROM_START, TEXT_BLOCK, program_list->programs[i].name);
        tb2->text_color = 0xFF000000;
        tb2->horizontal_alignment = ALIGN_CENTER;

        set_block_clickable(p1, start_program, i);
    }

    block_t *tb4 = add_block(&software_block, ADD_FROM_END, TEXT_BLOCK, "View 1");
    tb4->text_color = 0xFF000000;

    block_t *tb8 = add_block(&hardware_block, ADD_FROM_START, TEXT_BLOCK, "Keyboard layout");
    tb8->text_color = 0xFF000000;
    tb8->padding_bottom = 10;
    block_t *kl = add_block(&hardware_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    block_t *en = add_block(&kl, ADD_FROM_START, TEXT_BLOCK, "EN");
    en->margin_right = 10;
    en->border_top_size = 1; en->border_bottom_size = 1; en->border_left_size = 1; en->border_right_size = 1;
    en->border_color = 0xFF000000;
    en->background_color = 0xFF0000FF;
    en->padding_top = 5;
    en->padding_bottom = 5;
    en->padding_left = 5;
    en->padding_right = 5;
    block_t *sk = add_block(&kl, ADD_FROM_START, TEXT_BLOCK, "SK");
    sk->border_top_size = 1; sk->border_bottom_size = 1; sk->border_left_size = 1; sk->border_right_size = 1;
    sk->border_color = 0xFF000000;
    sk->padding_top = 5;
    sk->padding_bottom = 5;
    sk->padding_left = 5;
    sk->padding_right = 5;
    // TODO: make layouts changeable

    draw_gui_blocks(part, first_block, 0, 0);
    free_gui_blocks(first_block);
}

void start_program(screen_part_t *part, uint32_t index) {
    ramdisk_elf_program_t *program_in_part = NULL;
    for(int i = 0; i < program_list->number_of_programs; i++) {
        if(program_list->programs[i].part_where_program_is_running == part) {
            program_in_part = &program_list->programs[i];
            break;
        }
    }
    if(program_in_part != NULL) {
        program_in_part->part_where_program_is_running = NULL;
    }

    ramdisk_elf_program_t *new_program = &program_list->programs[index];
    screen_part_t *part_where_program_was_running = new_program->part_where_program_is_running;
    log("\nStarting program %s", new_program->name);
    if(new_program->is_loaded_into_memory == false) {
        // TODO: load program into memory here
        new_program->is_loaded_into_memory = true;
        number_of_running_programs++;
    }
    part->state = PART_STATE_PROGRAM;
    part->program_name = new_program->name;
    new_program->part_where_program_is_running = part;
    part_with_focus = part;

    if(part_where_program_was_running != NULL) {
        uint32_t is_part_set = false;
        for(int i = index + 1; i < program_list->number_of_programs; i++) {
            if(program_list->programs[i].is_loaded_into_memory == true && program_list->programs[i].part_where_program_is_running == NULL) {
                program_list->programs[i].part_where_program_is_running = part_where_program_was_running;

                part_where_program_was_running->program_name = program_list->programs[i].name; // TODO: set all variables here

                is_part_set = true;
                break;
            }
        }
        if(is_part_set == false) {
            for(int i = index - 1; i >= 0; i--) {
                if(program_list->programs[i].is_loaded_into_memory == true && program_list->programs[i].part_where_program_is_running == NULL) {
                    program_list->programs[i].part_where_program_is_running = part_where_program_was_running;

                    part_where_program_was_running->program_name = program_list->programs[i].name; // TODO: set all variables here

                    is_part_set = true;
                    break;
                }
            }
            if(is_part_set == false) {
                part_where_program_was_running->state = PART_STATE_MAIN_PANEL;
            }
        }
    }

    draw_view(active_view);
    redraw_screen();
}

void close_program(screen_part_t *part, uint32_t index) {
    log("\nClosing program %s", program_list->programs[index].name);
    // TODO: unload program from memory here
    program_list->programs[index].is_loaded_into_memory = false;
    number_of_running_programs--;
    if(program_list->programs[index].part_where_program_is_running == NULL) {
        return;
    }
    screen_part_t *part_with_program = program_list->programs[index].part_where_program_is_running;
    program_list->programs[index].part_where_program_is_running = NULL;
    for(int i = index + 1; i < program_list->number_of_programs; i++) {
        if(program_list->programs[i].is_loaded_into_memory == true && program_list->programs[i].part_where_program_is_running == NULL) {
            program_list->programs[i].part_where_program_is_running = part;
            part_with_focus = part;

            part->program_name = program_list->programs[i].name; // TODO: set all variables here

            draw_view(active_view);
            redraw_screen();
            return;
        }
    }
    for(int i = index - 1; i >= 0; i--) {
        if(program_list->programs[i].is_loaded_into_memory == true && program_list->programs[i].part_where_program_is_running == NULL) {
            program_list->programs[i].part_where_program_is_running = part;
            part_with_focus = part;

            part->program_name = program_list->programs[i].name; // TODO: set all variables here

            draw_view(active_view);
            redraw_screen();
            return;
        }
    }
    part_with_program->state = PART_STATE_MAIN_PANEL;
    draw_view(active_view);
    redraw_screen();
}

void draw_program(screen_part_t *part, uint32_t does_have_focus) {
    int program_index = -1;
    for(int i = 0; i < program_list->number_of_programs; i++) {
        if(program_list->programs[i].part_where_program_is_running == part) {
            program_index = i;
            break;
        }
    }
    log("\n[MAIN PANEL] Drawing program %d", program_index);

    block_t *first_block = create_first_block();
    block_t *big_block = add_block(&first_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    big_block->min_width = part->width;
    big_block->min_height = part->height;

    block_t *header = add_block(&big_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    header->vertical_alignment = ALIGN_CENTER;
    header->min_width = part->width;
    header->min_height = 30;
    header->border_bottom_size = 1;
    header->border_color = 0xFF000000;
    header->background_color = (does_have_focus == true) ? 0xFFFF0000 : 0xFF888888;
    block_t *program_name = add_block(&header, ADD_FROM_START, TEXT_BLOCK, part->program_name);
    program_name->padding_left = 10;
    program_name->text_color = 0xFF000000;

    block_t *program = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    program->min_width = part->width;
    program->min_height = part->height - (30 * 2);
    program->background_color = 0xFFFFFFFF;

    block_t *footer = add_block(&big_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    footer->min_width = part->width;
    footer->min_height = 30;
    footer->border_top_size = 1;
    footer->border_color = 0xFF000000;
    footer->background_color = (does_have_focus == true) ? 0xFFFF0000 : 0xFF888888;
    block_t *back_button = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, "Back");
    back_button->vertical_alignment = ALIGN_CENTER;
    back_button->min_height = 30;
    back_button->border_right_size = 1;
    back_button->border_color = 0xFF000000;
    back_button->padding_left = 10;
    back_button->padding_right = 10;
    back_button->text_color = 0xFF000000;
    back_button->background_color = (does_have_focus == true) ? 0xFFCC0000 : 0xFF666666;
    set_block_clickable(back_button, back_to_main_panel, 0);
    uint32_t size_of_one_program_bar = 150;
    uint32_t shown_programs = (number_of_running_programs * size_of_one_program_bar) > (part->width - 200) ? ((part->width - 200) / size_of_one_program_bar) : number_of_running_programs;
    for(int i = 0, d = 0; i < program_list->number_of_programs && d < shown_programs; i++) {
        if(program_list->programs[i].is_loaded_into_memory == false) {
            continue;
        }
        else {
            d++;
        }
        block_t *program_bar = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, program_list->programs[i].name);
        program_bar->vertical_alignment = ALIGN_CENTER;
        program_bar->min_width = size_of_one_program_bar;
        program_bar->min_height = 30;
        program_bar->border_right_size = 1;
        program_bar->padding_left = 10;
        program_bar->border_color = 0xFF000000;
        if(i == program_index) {
            program_bar->background_color = (does_have_focus == true) ? 0xFFFFFF00 : 0xFF666666;
        }
        set_block_clickable(program_bar, start_program, i);
    }
    if(number_of_running_programs > shown_programs) {
        block_t *more_bar = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, "...");
        more_bar->vertical_alignment = ALIGN_CENTER;
        more_bar->min_height = 30;
        more_bar->border_right_size = 1;
        more_bar->border_color = 0xFF000000;
        more_bar->padding_left = 10;
        more_bar->padding_right = 10;
        // TODO: add click
    }
    block_t *quit_button = add_block(&footer, ADD_FROM_END, TEXT_BLOCK, "Quit");
    quit_button->vertical_alignment = ALIGN_CENTER;
    quit_button->min_height = 30;
    quit_button->border_left_size = 1;
    quit_button->border_color = 0xFF000000;
    quit_button->padding_left = 10;
    quit_button->padding_right = 10;
    quit_button->text_color = 0xFF000000;
    quit_button->background_color = (does_have_focus == true) ? 0xFFCC0000 : 0xFF666666;
    set_block_clickable(quit_button, close_program, program_index);

    draw_gui_blocks(part, first_block, 0, 0);
    free_gui_blocks(first_block);

    // TODO: redraw inside of program / set flag for redraw
}

void back_to_main_panel(screen_part_t *part, uint32_t argument) {
    for(int i = 0; i < program_list->number_of_programs; i++) {
        if(program_list->programs[i].part_where_program_is_running == part) {
            program_list->programs[i].part_where_program_is_running = NULL;
        }
    }
    part->state = PART_STATE_MAIN_PANEL;
    draw_main_panel(part);
    redraw_screen();
}

void part_is_moving_to_part(screen_part_t *part, screen_part_t *new_part) {
    for(int i = 0; i < program_list->number_of_programs; i++) {
        if(program_list->programs[i].part_where_program_is_running == part) {
            program_list->programs[i].part_where_program_is_running = new_part;
        }
    }
}