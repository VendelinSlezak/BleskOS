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
#include <kernel/firmware/cmos/cmos.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/groups/human_input/keyboard_layout.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/font.h>
#include <kernel/hardware/subsystems/screen/stb_image.h>
#include <kernel/hardware/subsystems/screen/draw.h>
#include <kernel/hardware/subsystems/screen/simple_gui.h>
#include <kernel/hardware/subsystems/screen/stb_image_implementation.h>
#include <kernel/hardware/subsystems/screen/shutdown_dialog.h>
#include <kernel/software/virtual_hardware/screen.h>
#include <kernel/software/ramdisk.h>
#include <kernel/software/running_executables.h>
#include <kernel/libc/string.h>
#include <syslib.h>
#include <kernel/software/virtual_hardware/human_input.h>

/* local variables */
ramdisk_elf_program_list_t *program_list;
uint32_t number_of_running_programs = 0;
main_panel_data_t *actual_processed_main_panel;

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
    main_panel_data_t *main_panel_data = part->main_panel_data;

    block_t *first_block = create_first_block();
    block_t *big_block;
    if(part->width >= BREAKING_POINT_FOR_LAYOUT) {
        big_block = add_block(&first_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    }
    else {
        big_block = add_block(&first_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    }
    big_block->min_width = (part->width - 20);
    big_block->min_height = (part->height - 20);
    big_block->background_color = 0xFF00C000;
    big_block->padding_left = 10;
    big_block->padding_right = 10;
    big_block->padding_top = 10;
    big_block->padding_bottom = 10;
    block_t *software_block = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    if(part->width >= BREAKING_POINT_FOR_LAYOUT) {
        software_block->min_width = (part->width - 20) / 2;
        software_block->max_width = (part->width - 20) / 2;
        software_block->min_height = (part->height - 20);
    }
    else {
        software_block->min_width = (part->width - 20);
        software_block->max_width = (part->width - 20);
    }
    block_t *hardware_block = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    if(part->width >= BREAKING_POINT_FOR_LAYOUT) {
        hardware_block->min_width = (part->width - 20) / 2;
        hardware_block->max_width = (part->width - 20) / 2;
        hardware_block->min_height = (part->height - 20);
    }
    else {
        hardware_block->min_width = (part->width - 20);
        hardware_block->max_width = (part->width - 20);
    }

    block_t *hb1 = add_block(&software_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    hb1->padding_bottom = 20;
    for(int i = 0; i < program_list->number_of_programs; i++) {
        block_t *p1 = add_block(&hb1, ADD_FROM_START, VERTICAL_BLOCK, NULL);
        p1->min_width = 96;
        p1->max_width = 96;
        p1->horizontal_alignment = ALIGN_CENTER;
        p1->padding_right = 10;
        p1->padding_bottom = 10;
        p1->background_color = (program_list->programs[i].running_executable != NULL) ? 0xFF00AA00 : 0x00000000;
        block_t *ib1 = add_block(&p1, ADD_FROM_START, IMAGE_BLOCK, program_list->programs[i].icon);
        ib1->padding_bottom = 10;
        block_t *tb2 = add_block(&p1, ADD_FROM_START, TEXT_BLOCK, program_list->programs[i].name);
        tb2->text_color = 0xFF000000;
        tb2->horizontal_alignment = ALIGN_CENTER;

        if(program_list->programs[i].running_executable == NULL) {
            set_block_clickable(p1, start_program, i);
        }
        else {
            set_block_clickable(p1, show_program, (uint32_t) program_list->programs[i].running_executable);
        }
    }

    if(part->width >= BREAKING_POINT_FOR_LAYOUT) {
        block_t *tb4 = add_block(&software_block, ADD_FROM_END, TEXT_BLOCK, "View 1");
        tb4->text_color = 0xFF000000;
    }

    block_t *tb8 = add_block(&hardware_block, ADD_FROM_START, TEXT_BLOCK, "Keyboard layout");
    tb8->text_color = 0xFF000000;
    tb8->padding_bottom = 10;
    block_t *kl = add_block(&hardware_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    block_t *en = add_block(&kl, ADD_FROM_START, TEXT_BLOCK, "EN");
    en->margin_right = 10; en->margin_bottom = 10;
    en->border_top_size = 1; en->border_bottom_size = 1; en->border_left_size = 1; en->border_right_size = 1;
    en->border_color = 0xFF000000;
    en->background_color = (strcmp(keyboard_layout, "EN") == 0) ? 0xFF0000FF : 0x00000000;
    en->padding_top = 5;
    en->padding_bottom = 5;
    en->padding_left = 5;
    en->padding_right = 5;
    set_block_clickable(en, select_keyboard_layout, (uint32_t)&"EN");
    block_t *sk = add_block(&kl, ADD_FROM_START, TEXT_BLOCK, "SK");
    sk->margin_right = 10; sk->margin_bottom = 10;
    sk->border_top_size = 1; sk->border_bottom_size = 1; sk->border_left_size = 1; sk->border_right_size = 1;
    sk->border_color = 0xFF000000;
    sk->background_color = (strcmp(keyboard_layout, "SK") == 0) ? 0xFF0000FF : 0x00000000;
    sk->padding_top = 5;
    sk->padding_bottom = 5;
    sk->padding_left = 5;
    sk->padding_right = 5;
    set_block_clickable(sk, select_keyboard_layout, (uint32_t)&"SK");

    block_t *tb9 = add_block(&hardware_block, ADD_FROM_START, TEXT_BLOCK, "Power options");
    tb9->text_color = 0xFF000000;
    tb9->padding_bottom = 10;
    block_t *shutdown_block = add_block(&hardware_block, ADD_FROM_START, TEXT_BLOCK, "Shutdown");
    shutdown_block->border_color = 0xFF000000;
    shutdown_block->border_top_size = 1; shutdown_block->border_bottom_size = 1; shutdown_block->border_left_size = 1; shutdown_block->border_right_size = 1;
    shutdown_block->background_color = 0xFFFFFF00;
    shutdown_block->padding_top = 5;
    shutdown_block->padding_bottom = 5;
    shutdown_block->padding_left = 5;
    shutdown_block->padding_right = 5;
    set_block_clickable(shutdown_block, send_shutdown_signal, 0);

    block_t *footer;
    if(part->width >= BREAKING_POINT_FOR_LAYOUT) {
        footer = add_block(&hardware_block, ADD_FROM_END, HORIZONTAL_BLOCK, NULL);
    }
    else {
        footer = add_block(&big_block, ADD_FROM_END, HORIZONTAL_BLOCK, NULL);
    }
    footer->min_width = hardware_block->min_width;
    footer->margin_top = 10;
    if(part->width < BREAKING_POINT_FOR_LAYOUT) {
        block_t *tb4 = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, "View 1");
        tb4->text_color = 0xFF000000;
    }
    uint8_t time_str[30] = {0};
    sprintf(time_str, "%02d:%02d:%02d %d/%02d/%02d", current_time.hour, current_time.minute, current_time.second, current_time.year, current_time.month, current_time.day);
    block_t *time = add_block(&footer, ADD_FROM_END, TEXT_BLOCK, time_str);

    draw_gui_blocks(part, first_block, 0, main_panel_data->y_offset);
    if(big_block->actual_height > part->height) {
        main_panel_data->is_there_vertical_scrollbar = true;
        main_panel_data->y_offset_range = (big_block->actual_height - part->height);
        main_panel_data->vertical_scrollbar_area_size = part->height;
        main_panel_data->vertical_scrollbar_size = (part->height * part->height) / big_block->actual_height;
        main_panel_data->vertical_scrollbar_range = (part->height - main_panel_data->vertical_scrollbar_size);
        draw_square(part->x + part->width - 10, part->y, 10, part->height, 0xFF888888);
        uint32_t size_of_scrollbar = (part->height * part->height) / big_block->actual_height;
        draw_square(part->x + part->width - 10, part->y + main_panel_data->vertical_scrollbar_position, 10, size_of_scrollbar, 0xFF444444);
        gui_add_left_click_event(part, part->width - 10, 0, 10, part->height, grab_scrollbar, 0);
    }
    else {
        main_panel_data->is_there_vertical_scrollbar = false;
        main_panel_data->is_vertical_scrollbar_dragged = false;
    }
    free_gui_blocks(first_block);
}

// TODO: this whole scrollbar thing code is the ugliest thing possible, rewrite it later
void grab_scrollbar(screen_part_t *part, uint32_t argument) {
    actual_processed_main_panel = (main_panel_data_t *) part->main_panel_data;
    actual_processed_main_panel->is_vertical_scrollbar_dragged = true;
    actual_processed_main_panel->initial_grab_y_position = mouse_cursor_y;
    part_with_focus = part;
    redraw_part(part);
}
void main_panel_process_mouse_movement(screen_part_t *part) {
    if(actual_processed_main_panel != NULL) {
        if(actual_processed_main_panel->is_vertical_scrollbar_dragged == true) {
            int offset = actual_processed_main_panel->vertical_scrollbar_position + (int)(mouse_cursor_y - actual_processed_main_panel->initial_grab_y_position);
            if(offset < 0) {
                offset = 0;
            }
            else if(offset > (actual_processed_main_panel->vertical_scrollbar_area_size - actual_processed_main_panel->vertical_scrollbar_size)) {
                offset = actual_processed_main_panel->vertical_scrollbar_area_size - actual_processed_main_panel->vertical_scrollbar_size;
            }
            if(offset != actual_processed_main_panel->vertical_scrollbar_position) {
                actual_processed_main_panel->vertical_scrollbar_position = offset;
                actual_processed_main_panel->y_offset = (actual_processed_main_panel->vertical_scrollbar_position * actual_processed_main_panel->y_offset_range) / actual_processed_main_panel->vertical_scrollbar_range;
                redraw_part(part);
            }
        }
    }
}
void main_panel_process_mouse_left_click_release(screen_part_t *part) {
    if(actual_processed_main_panel != NULL) {
        actual_processed_main_panel->is_vertical_scrollbar_dragged = false;
        actual_processed_main_panel = NULL;
    }
}
// I am reading this again and like seriously what is even that code why is it redrawing whole view??

void start_program(screen_part_t *part, uint32_t index) {
    ramdisk_elf_program_t *new_program = &program_list->programs[index];
    log("\nStarting program %s", new_program->name);

    // if this part already shows program, close it
    running_executable_t *program_in_part = part->running_executable;
    if(program_in_part != NULL) {
        program_in_part->part = NULL;
    }

    main_panel_data_t *data = (main_panel_data_t *) part->main_panel_data;
    data->show_remaining_programs = false;

    // show program in this part
    running_executable_t *re = new_program->running_executable;
    screen_part_t *part_where_program_was_running = NULL;
    re = create_running_program_from_spawning_template(&new_program->spawning_template);
    re->name = new_program->name;
    re->part = part;
    new_program->running_executable = re;
    vh_screen_update_window_resolution(re, part->width, part->height - 60);

    part->program_name = re->name;
    part->running_executable = re;
    part->state = PART_STATE_PROGRAM;
    part_with_focus = part;

    redraw_part(part);
    redraw_main_panels_under_part(active_view->global_part);
}

void show_program(screen_part_t *part, uint32_t running_executable) {
    running_executable_t *re = (running_executable_t *) (running_executable);

    log("\nShow program %s", re->name);

    // if program is already shown at other part, then change what is shown in that part to other program or to main panel
    if(re->part != part && re->part != NULL) {
        running_executable_t *other_re = get_not_shown_running_executable_except(re);
        if(other_re == NULL) {
            re->part->running_executable = NULL;
            re->part->state = PART_STATE_MAIN_PANEL;
            redraw_part(re->part);
        }
        else {
            re->part->program_name = other_re->name;
            re->part->running_executable = other_re;
            re->part->state = PART_STATE_PROGRAM;
            other_re->part = re->part;
            vh_screen_demand_redraw_from_program(other_re);
            redraw_part(other_re->part);
        }
    }
    re->part = part;

    // show program in this part
    part->program_name = re->name;
    part->running_executable = re;
    part->state = PART_STATE_PROGRAM;
    screen_part_t *old_part_with_focus = part_with_focus;
    part_with_focus = part;
    vh_screen_demand_redraw_from_program(re);

    redraw_part(part);
    if(old_part_with_focus != NULL && old_part_with_focus != part) {
        redraw_part(old_part_with_focus);
    }
}

void send_shutdown_signal(screen_part_t *part, uint32_t argument) {
    active_view->is_whole_screen_mode_active = true;
    active_view->draw_whole_screen = draw_shutdown_dialog;
    draw_view(active_view);
    redraw_screen();
}

void select_keyboard_layout(screen_part_t *part, uint32_t layout) {
    if(strcmp(keyboard_layout, (char *) layout) == 0) {
        return;
    }
    set_keyboard_layout((uint8_t *) layout);
    redraw_main_panels_under_part(active_view->global_part);
}

void draw_program(screen_part_t *part, uint32_t does_have_focus) {
    main_panel_data_t *data = (main_panel_data_t *) part->main_panel_data;

    log("\ndraw program");

    running_executable_t *re = part->running_executable;

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
    set_block_clickable(header, start_dragging_program_between_parts, 0);
    block_t *program_name = add_block(&header, ADD_FROM_START, TEXT_BLOCK, part->program_name);
    program_name->padding_left = 10;
    program_name->text_color = 0xFF000000;

    block_t *program = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    program->min_width = part->width;
    program->min_height = part->height - (30 * 2);
    program->background_color = 0x00000000;

    block_t *footer = add_block(&big_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    footer->min_width = part->width;
    footer->min_height = 30;
    footer->border_top_size = 1;
    footer->border_color = 0xFF000000;
    footer->background_color = (does_have_focus == true) ? 0xFFFF0000 : 0xFF888888;
    set_block_clickable(footer, start_dragging_program_between_parts, 0);
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
    uint32_t last_not_checked_program = 0;
    for(int i = 0; i < running_executable_list->number_of_running_executables; i++) {
        last_not_checked_program = i;
        block_t *program_bar = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, running_executable_list->running_executable[i]->name);
        program_bar->vertical_alignment = ALIGN_CENTER;
        program_bar->min_width = size_of_one_program_bar;
        program_bar->min_height = 30;
        program_bar->border_right_size = 1;
        program_bar->padding_left = 10;
        program_bar->border_color = 0xFF000000;
        if(running_executable_list->running_executable[i] == re) {
            program_bar->background_color = (does_have_focus == true) ? 0xFFFFFF00 : 0xFF666666;
        }
        set_block_clickable(program_bar, show_program, (uint32_t) running_executable_list->running_executable[i]);
    }
    if(last_not_checked_program != 0) {
        last_not_checked_program++;
    }
    if(number_of_running_programs > shown_programs) {
        block_t *more_bar = add_block(&footer, ADD_FROM_START, TEXT_BLOCK, "...");
        more_bar->vertical_alignment = ALIGN_CENTER;
        more_bar->min_height = 30;
        more_bar->border_right_size = 1;
        more_bar->border_color = 0xFF000000;
        more_bar->padding_left = 10;
        more_bar->padding_right = 10;
        set_block_clickable(more_bar, show_remaining_programs, 0);
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
    set_block_clickable(quit_button, close_program, (uint32_t) re);

    draw_gui_blocks(part, first_block, 0, 0);
    if(data->show_remaining_programs == true) {
        uint32_t x = (53 + (shown_programs * (size_of_one_program_bar + 1)) - 1);
        uint32_t y = (part->height - 29);
        uint32_t background_color = 0xFFFF0000;
        for(int i = last_not_checked_program; i < running_executable_list->number_of_running_executables; i++) {
            y -= 30;
            if(running_executable_list->running_executable[i] == re) {
                draw_square_in_part(part, x, y, size_of_one_program_bar, 30, (does_have_focus == true) ? 0xFFFFFF00 : 0xFF666666);
            }
            else {
                draw_square_in_part(part, x, y, size_of_one_program_bar, 30, background_color);
            }
            background_color = (background_color == 0xFFFF0000) ? 0xFFEE0000 : 0xFFFF0000;
            draw_bitmap_string(part, x + 10, y + 7, running_executable_list->running_executable[i]->name, 0xFF000000);
            gui_add_left_click_event(part, x, y, size_of_one_program_bar, 30, show_program, (uint32_t) running_executable_list->running_executable[i]);
        }
    }
    free_gui_blocks(first_block);

    // set flag for redraw of program
    if(is_view_edited == false && is_program_dragged == false) {
        vh_screen_update_window_resolution(re, part->width, part->height - 60);
    }
}

void start_dragging_program_between_parts(screen_part_t *part, uint32_t argument) {
    start_dragging_part(part);
    draw_view(active_view);
    redraw_screen();
}

void back_to_main_panel(screen_part_t *part, uint32_t argument) {
    running_executable_t *re = part->running_executable;
    re->part = NULL;
    part->state = PART_STATE_MAIN_PANEL;
    part->running_executable = NULL;
    part_with_focus = part;
    redraw_part(part);
}

void close_program(screen_part_t *part, uint32_t running_executable) {
    running_executable_t *re = (running_executable_t *) (running_executable);
    log("\nClosing program %s", re->name);
    send_closing_signal_to_program(re->program);
    // TODO: set timer, and if program is not closed, show "Program had not completed closing signal so far, do you want to kill it?"
}

void part_program_closed(screen_part_t *part) {
    running_executable_t *running_executable = part->running_executable;
    if(running_executable != NULL) {
        for(int i = 0; i < program_list->number_of_programs; i++) {
            if(program_list->programs[i].running_executable == running_executable) {
                program_list->programs[i].running_executable = NULL;
            }
        }
    }
    part->running_executable = NULL;
    part->state = PART_STATE_MAIN_PANEL;
    draw_view(active_view); // we need to update everything because closing program will affect main panel and also program parts
    redraw_screen();
}

void process_mouse_button_click_for_program(screen_part_t *part, uint32_t button, uint32_t x, uint32_t y) {
    if(    x < part->x
        || x >= (part->x + part->width)
        || y < (part->y + 30)
        || y >= (part->y + part->height - 30)) {
        return;
    }
    main_panel_data_t *data = (main_panel_data_t *) part->main_panel_data;
    if(data->show_remaining_programs == true) {
        data->show_remaining_programs = false;
        redraw_part(part);
    }
    vh_human_input_event(part->running_executable, VH_HUMAN_INPUT_EVENT_BUTTON_PRESSED, button, x - part->x, y - part->y - 30);
}

void show_remaining_programs(screen_part_t *part, uint32_t argument) {
    main_panel_data_t *data = (main_panel_data_t *) part->main_panel_data;
    if(data->show_remaining_programs == true) {
        data->show_remaining_programs = false;
    }
    else {
        data->show_remaining_programs = true;
    }
    redraw_part(part);
}

void part_is_moving_to_part(screen_part_t *part, screen_part_t *new_part) {
    running_executable_t *re = part->running_executable;
    if(re == NULL) {
        return;
    }
    re->part->running_executable = NULL;
    re->part = new_part;
    new_part->running_executable = re;
}