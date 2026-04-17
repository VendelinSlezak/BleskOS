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
#include <kernel/cpu/commands.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/kernel.h>

/* local variables */
uint8_t *buffer;
uint32_t line;
uint32_t column;
uint32_t num_of_lines;
uint32_t num_of_columns;
uint32_t is_cursor_visible;

/* functions */
uint32_t does_text_mode_vga_device_exist(void) {
    standardized_graphic_output_t *standardized_graphic_output = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    if(standardized_graphic_output->type == STANDARDIZED_GRAPHIC_OUTPUT_TYPE_TEXT_MODE) {
        return true;
    }
    else {
        return false;
    }
}

void initialize_text_mode_vga_device(void) {
    if(does_text_mode_vga_device_exist() == false) {
        return;
    }

    standardized_graphic_output_t *standardized_graphic_output = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    num_of_lines = standardized_graphic_output->height;
    num_of_columns = standardized_graphic_output->width;

    buffer = perm_phy_alloc(0xB8000, 0x4000, VM_MMIO);
    line = 0;
    column = 0;
    vga_set_cursor(0, 0);
    is_cursor_visible = false;
    vga_enable_cursor();

    add_logging_device(vga_show_char);
}

void vga_show_char(uint32_t character) {
    if(character == '\n') {
        if(line == num_of_lines - 1) {
            // move screen content
            for(int i = 0; i < num_of_lines; i++) {
                for(int j = 0; j < num_of_columns; j++) {
                    buffer[(i * num_of_columns + j) * 2] = buffer[((i + 1) * num_of_columns + j) * 2];
                }
            }
            // clear last line
            for(int i = 0; i < num_of_columns; i++) {
                buffer[(num_of_lines * num_of_columns + i) * 2] = 0x20;
            }
            line = num_of_lines - 1;
        }
        else {
            line++;
        }
        column = 0;
    }
    else {
        if(column < num_of_columns) {
            buffer[(line * num_of_columns + column) * 2] = character;
            column++;
        }
    }
    vga_set_cursor(column, line);
}

void vga_set_cursor(int x, int y) {
    if(x >= num_of_columns || y >= num_of_lines) {
        vga_disable_cursor();
        return;
    }
    else {
        vga_enable_cursor();
    }
    uint16_t pos = y * 80 + x;
    outb(VGA_CTRL_REGISTER, 0x0F);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REGISTER, 0x0E);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_disable_cursor(void) {
    if(is_cursor_visible == false) {
        return;
    }
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) | 0x20));
    is_cursor_visible = false;
}

void vga_enable_cursor(void) {
    if(is_cursor_visible == true) {
        return;
    }
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & ~0x20));
    is_cursor_visible = true;
}