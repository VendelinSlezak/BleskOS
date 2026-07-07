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
placeholder_running_program_list_t *running_programs_list;

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
    running_programs_list = kalloc(sizeof(placeholder_running_program_list_t));
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
        block_t *ib1 = add_block(&p1, ADD_FROM_START, IMAGE_BLOCK, program_list->programs[i].icon);
        ib1->padding_bottom = 10;
        block_t *tb2 = add_block(&p1, ADD_FROM_START, TEXT_BLOCK, program_list->programs[i].name);
        tb2->text_color = 0xFF000000;
        tb2->horizontal_alignment = ALIGN_CENTER;

        set_block_clickable(p1, start_program, i);
    }

    if(running_programs_list->number_of_programs > 0) {
        block_t *tb4 = add_block(&software_block, ADD_FROM_END, TEXT_BLOCK, "Running software");
        tb4->text_color = 0xFF000000;
        tb4->padding_bottom = 10;
    
        block_t *hb2 = add_block(&software_block, ADD_FROM_END, HORIZONTAL_BLOCK, NULL);
        hb2->padding_bottom = 10;

        for(int i = 0; i < running_programs_list->number_of_programs; i++) {
            block_t *rp = add_block(&hb2, ADD_FROM_START, VERTICAL_BLOCK, NULL);
            rp->min_width = 150;
            rp->min_height = 100;
            rp->margin_right = 10;
            rp->border_size = 1;
            rp->border_color = 0xFF000000;
            rp->background_color = 0xFFFFFFFF;
            rp->margin_right = 10;
            rp->margin_bottom = 10;
            set_block_clickable(rp, open_running_program, i);
            block_t *rph1 = add_block(&rp, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
            rph1->min_width = 150;
            rph1->min_height = 25;
            rph1->padding_left = 8;
            rph1->padding_right = 8;
            rph1->vertical_alignment = ALIGN_CENTER;
            rph1->background_color = 0xFFFF0000;
            block_t *tb5 = add_block(&rph1, ADD_FROM_START, TEXT_BLOCK, running_programs_list->programs[i].name);
            block_t *tb6 = add_block(&rph1, ADD_FROM_END, TEXT_BLOCK, "X");
            tb6->padding_left = 4;
            tb6->padding_right = 4;
            tb6->border_size = 1;
            tb6->border_color = 0xFF000000;
            set_block_clickable(tb6, close_running_program, i);
        }
    }

    block_t *tb7 = add_block(&hardware_block, ADD_FROM_START, TEXT_BLOCK, "Hardware");
    tb7->text_color = 0xFF000000;
    tb7->padding_bottom = 10;

    block_t *tb8 = add_block(&hardware_block, ADD_FROM_START, TEXT_BLOCK, "Keyboard layout");
    tb8->text_color = 0xFF000000;
    tb8->padding_bottom = 10;
    block_t *kl = add_block(&hardware_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    block_t *en = add_block(&kl, ADD_FROM_START, TEXT_BLOCK, "EN");
    en->border_size = 1;
    en->border_color = 0xFF000000;
    en->background_color = 0xFF0000FF;
    en->padding_top = 5;
    en->padding_bottom = 5;
    en->padding_left = 5;
    en->padding_right = 5;

    draw_gui_blocks(part, first_block, 0, 0);
    free_gui_blocks(first_block);
}

void start_program(screen_part_t *part, uint32_t index) {
    log("\nStarting program %s", program_list->programs[index].name);
    ramdisk_elf_program_t *program = &program_list->programs[index];
    part->state = PART_STATE_PROGRAM;
    part->program_name = program->name;
    part_with_focus = part;
    running_programs_list = krealloc(running_programs_list, sizeof(placeholder_running_program_list_t) + sizeof(placeholder_running_program_t) * (running_programs_list->number_of_programs + 1));
    uint32_t i = running_programs_list->number_of_programs;
    running_programs_list->programs[i].name = program->name;
    running_programs_list->programs[i].part_where_program_is_running = part;
    running_programs_list->number_of_programs++;
    draw_view(active_view);
    redraw_screen();
}

void open_running_program(screen_part_t *part, uint32_t index) {
    log("\nOpening running program %s", running_programs_list->programs[index].name);
    if(running_programs_list->programs[index].part_where_program_is_running != NULL) {
        running_programs_list->programs[index].part_where_program_is_running->state = PART_STATE_MAIN_PANEL;
        draw_main_panel(running_programs_list->programs[index].part_where_program_is_running);
        part_with_focus = NULL;
    }
    part->state = PART_STATE_PROGRAM;
    part->program_name = running_programs_list->programs[index].name;
    running_programs_list->programs[index].part_where_program_is_running = part;
    if(part_with_focus != NULL && part_with_focus->state == PART_STATE_PROGRAM) {
        draw_program(part_with_focus, false); // TODO: without need to request internal program redraw
    }
    part_with_focus = part;
    draw_program(part, true);
    redraw_screen();
}

void close_running_program(screen_part_t *part, uint32_t index) {
    log("\nClosing running program %s", running_programs_list->programs[index].name);
    if(running_programs_list->programs[index].part_where_program_is_running != NULL) {
        screen_part_t *part_with_program = running_programs_list->programs[index].part_where_program_is_running;
        part_with_program->state = PART_STATE_MAIN_PANEL;
        running_programs_list->programs[index].part_where_program_is_running = NULL;
    }
    memmove(&running_programs_list->programs[index], &running_programs_list->programs[index + 1], sizeof(placeholder_running_program_t) * (running_programs_list->number_of_programs - index - 1));
    running_programs_list->number_of_programs--;
    running_programs_list = krealloc(running_programs_list, sizeof(placeholder_running_program_list_t) + sizeof(placeholder_running_program_t) * running_programs_list->number_of_programs);
    draw_view(active_view);
    redraw_screen();
}

void draw_program(screen_part_t *part, uint32_t does_have_focus) {
    block_t *first_block = create_first_block();
    block_t *big_block = add_block(&first_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    big_block->min_width = part->width;
    big_block->min_height = part->height;

    block_t *header = add_block(&big_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    header->vertical_alignment = ALIGN_CENTER;
    header->min_width = part->width;
    header->min_height = 30;
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
    footer->background_color = (does_have_focus == true) ? 0xFFFF0000 : 0xFF888888;
    block_t *back_button = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, "Back");
    back_button->vertical_alignment = ALIGN_CENTER;
    back_button->min_height = 30;
    back_button->padding_left = 10;
    back_button->padding_right = 10;
    back_button->text_color = 0xFF000000;
    back_button->background_color = (does_have_focus == true) ? 0xFFCC0000 : 0xFF666666;
    set_block_clickable(back_button, back_to_main_panel, 0);

    draw_gui_blocks(part, first_block, 0, 0);
    free_gui_blocks(first_block);

    // TODO: redraw inside of program / set flag for redraw
}

void back_to_main_panel(screen_part_t *part, uint32_t argument) {
    for(int i = 0; i < running_programs_list->number_of_programs; i++) {
        if(running_programs_list->programs[i].part_where_program_is_running == part) {
            running_programs_list->programs[i].part_where_program_is_running = NULL;
        }
    }
    part->state = PART_STATE_MAIN_PANEL;
    draw_main_panel(part);
    redraw_screen();
}

void part_is_moving_to_part(screen_part_t *part, screen_part_t *new_part) {
    for(int i = 0; i < running_programs_list->number_of_programs; i++) {
        if(running_programs_list->programs[i].part_where_program_is_running == part) {
            running_programs_list->programs[i].part_where_program_is_running = new_part;
        }
    }
}