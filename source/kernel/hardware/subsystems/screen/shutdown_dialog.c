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
#include <kernel/firmware/acpi/services.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/subsystems/screen/simple_gui.h>
#include <kernel/hardware/subsystems/screen/draw.h>

/* functions */
void draw_shutdown_dialog(screen_part_t *part) {
    block_t *first_block = create_first_block();
    block_t *big_block = add_block(&first_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    big_block->min_width = part->width;
    big_block->min_height = part->height;
    big_block->background_color = 0xFF000000;
    big_block->vertical_alignment = ALIGN_CENTER;
    block_t *dialog_block = add_block(&big_block, ADD_FROM_START, VERTICAL_BLOCK, NULL);
    dialog_block->min_width = part->width;
    dialog_block->horizontal_alignment = ALIGN_CENTER;
    block_t *text_block = add_block(&dialog_block, ADD_FROM_START, TEXT_BLOCK, "Are you sure you want to shutdown computer?");
    text_block->text_color = 0xFFFFFFFF;
    text_block->padding_top = 10;
    text_block->padding_bottom = 10;
    text_block->padding_left = 10;
    text_block->padding_right = 10;
    text_block->margin_bottom = 10;
    block_t *button_block = add_block(&dialog_block, ADD_FROM_START, HORIZONTAL_BLOCK, NULL);
    block_t *yes_button = add_block(&button_block, ADD_FROM_START, TEXT_BLOCK, "Yes");
    yes_button->text_color = 0xFF000000;
    yes_button->background_color = 0xFFFFFFFF;
    yes_button->padding_top = 5;
    yes_button->padding_bottom = 5;
    yes_button->padding_left = 10;
    yes_button->padding_right = 10;
    yes_button->margin_right = 10;
    set_block_clickable(yes_button, shutdown_yes, 0);
    block_t *no_button = add_block(&button_block, ADD_FROM_START, TEXT_BLOCK, "No");
    no_button->text_color = 0xFF000000;
    no_button->background_color = 0xFFFFFFFF;
    no_button->padding_top = 5;
    no_button->padding_bottom = 5;
    no_button->padding_left = 10;
    no_button->padding_right = 10;
    set_block_clickable(no_button, shutdown_no, 0);

    draw_gui_blocks(part, first_block, 0, 0);
    free_gui_blocks(first_block);
}

void shutdown_yes(screen_part_t *part, uint32_t argument) {
    shutdown();
}

void shutdown_no(screen_part_t *part, uint32_t argument) {
    active_view->is_whole_screen_mode_active = false;
    draw_view(active_view);
    redraw_screen();
}