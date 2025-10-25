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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <hardware/controllers/graphic/default_graphic_card/default_graphic_card.h>

/* local variables */
vga_text_mode_mmio_t *vga_buffer;
dword_t vga_line;
dword_t vga_column;

/* functions */
dword_t is_vga_text_mode_present(void) {
    standardized_graphic_output_t *bootloader_graphic_info = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    if(bootloader_graphic_info->type == STANDARDIZED_GRAPHIC_OUTPUT_TYPE_TEXT_MODE) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

dword_t initialize_vga_text_mode(void) {
    standardized_graphic_output_t *bootloader_graphic_info = (standardized_graphic_output_t *) P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO;
    vga_buffer = kpalloc((dword_t)bootloader_graphic_info->linear_frame_buffer, bootloader_graphic_info->width*bootloader_graphic_info->height*2, VM_FLAG_UNCACHEABLE);
    vga_line = 0;
    vga_column = 0;

    return TRUE;
}

void vga_text_mode_send_character(dword_t character) {
    if(character > 127) {
        return;
    }

    if(character == 0xA) {
        vga_line++;
        vga_column = 0;
        if(vga_line >= 25) {
            for(int i = 0, j = 80; i < 24*80; i++, j++) {
                vga_buffer[i].character = vga_buffer[j].character;
                vga_buffer[i].character_color = vga_buffer[j].character_color;
                vga_buffer[i].background_color = vga_buffer[j].background_color;
            }
            for(int i = 24*80; i < 25*80; i++) {
                vga_buffer[i].character = 0;
            }
            vga_line = 24;
        }
        return;
    }

    if(vga_line >= 25 || vga_column >= 80) {
        return;
    }    

    vga_buffer[vga_line*80 + vga_column].character = character;
    vga_buffer[vga_line*80 + vga_column].character_color = 0x7;
    vga_buffer[vga_line*80 + vga_column].background_color = 0x0;
    vga_column++;
}