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
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/draw.h>
#include <kernel/libc/string.h>

/* local variables */
bitmap_font_t *system_font;

/* functions */
void load_system_bitmap_font(void *file, uint32_t size) {
    psf1_header_t *psf1_header = file;
    if(psf1_header->magic == PSF1_FONT_MAGIC) {
        system_font = load_psf1_font(psf1_header);
    }

    psf2_header_t *psf2_header = file;
    if(psf2_header->magic == PSF2_FONT_MAGIC) {
        system_font = load_psf2_font(psf2_header);
    }
}

bitmap_font_t *load_psf1_font(psf1_header_t *file) {
    log("\n--- Loading PSF1 font ---");
    log("\nNumber of characters: %d", (file->font_mode & 0x1) ? 512 : 256);
    log("\nSize of character: 8x%d", file->character_size);
    bitmap_font_t *font = kalloc(sizeof(bitmap_font_t));
    font->number_of_characters = (file->font_mode & 0x1) ? 512 : 256;
    font->bytes_per_character = file->character_size;
    font->character_width = 8;
    font->character_height = file->character_size;
    font->data = kalloc(font->number_of_characters * font->bytes_per_character);
    if((file->font_mode & 0x2) == 0) { // no unicode table
        memcpy(font->data, file->data, font->number_of_characters * font->bytes_per_character);
    }
    else {
        uint16_t *unicode_table = (uint16_t *) (file->data + (font->number_of_characters * font->bytes_per_character));
        uint8_t present_unicode_characters[512] = {0};
        uint32_t number_of_copied_characters = 0;
        while(number_of_copied_characters < font->number_of_characters) {
            uint16_t unicode_code = *unicode_table++;
            if(unicode_code == 0xFFFF) {
                number_of_copied_characters++;
                continue;
            }
            if(unicode_code >= font->number_of_characters) {
                continue;
            }
            present_unicode_characters[unicode_code] = 1;
            uint8_t *dst = font->data + (unicode_code * font->bytes_per_character);
            uint8_t *src = file->data + (number_of_copied_characters * font->bytes_per_character);
            for(uint32_t i = 0; i < font->bytes_per_character; i++) {
                *dst++ |= *src++;
            }
        }
        for(uint32_t i = 0; i < 512; i++) {
            if(present_unicode_characters[i] == 0) {
                memset(font->data + (i * font->bytes_per_character), 0xFF, font->bytes_per_character);
            }
        }
    }

    return font;
}

bitmap_font_t *load_psf2_font(psf2_header_t *file) {
    log("\n--- Loading PSF2 font ---");
    log("\nNumber of characters: %d", file->num_of_glyphs);
    log("\nSize of character: %dx%d", file->width, file->height);
    log("\nBytes per character: %d", file->bytes_per_glyph);
    bitmap_font_t *font = kalloc(sizeof(bitmap_font_t));
    font->number_of_characters = file->num_of_glyphs;
    font->bytes_per_character = file->bytes_per_glyph;
    font->character_width = file->width;
    font->character_height = file->height;
    font->data = kalloc(font->number_of_characters * font->bytes_per_character);
    memcpy(font->data, file->data, font->number_of_characters * font->bytes_per_character);
    return font;
}

void draw_bitmap_char(screen_part_t *part, int x, int y, uint32_t character, uint32_t color) {
    if(character >= system_font->number_of_characters) {
        log("\nCharacter %x is not in the font", character);
        character = '?';
    }

    calculated_area_t area = calculate_area_in_part(part, x, y, system_font->character_width, system_font->character_height);
    if(area.drawable == false) {
        return;
    }
    uint32_t top = area.top;
    uint32_t left = area.left;
    uint32_t showed_width = area.width;
    uint32_t showed_height = area.height;
    uint32_t first_line = area.first_line;
    uint32_t last_line = area.last_line;
    uint32_t first_column = area.first_column;
    uint32_t last_column = area.last_column;

    view_t *view = part->view;
    uint32_t view_width = view->width;
    uint32_t *buffer = (uint32_t *) view->buffer;
    top += part->y;
    left += part->x;
    uint32_t *dst = (uint32_t *) &buffer[top * view_width + left];
    uint8_t *src = system_font->data + (character * system_font->bytes_per_character) + (first_line * ((system_font->character_width + 7) / 8));
    for(uint32_t i = first_line; i < last_line; i++) {
        uint32_t *line_dst = dst;
        for(int j = first_column; j < ((last_column + 7) / 8); j++) {
            if(src[j] != 0) {
                int s = 7 - ((last_column + 7) % 8);
                for(int k = 7; k >= s; k--) {
                    if((src[j] >> k) & 0x1) {
                        *line_dst = color;
                    }
                    line_dst++;
                }
            }
            else {
                line_dst += 8;
            }
        }
        dst += view_width;
        src += (showed_width + 7) / 8;
    }
}

void draw_bitmap_string(screen_part_t *part, uint32_t x, uint32_t y, char *string, uint32_t color) {
    if(string == NULL) {
        return;
    }

    uint32_t original_x = x;
    uint32_t size_of_tabulator = system_font->character_width * 4;
    while(*string != 0) {
        // convert UTF8 to UTF32
        uint32_t character = *string;
        if((character & 0xF8) == 0xF0) {
            character = ((character & 0x7) << 18) | ((string[1] & 0x3F) << 12) | ((string[2] & 0x3F) << 6) | (string[3] & 0x3F);
            string += 4;
        }
        else if((character & 0xF0) == 0xE0) {
            character = ((character & 0xF) << 12) | ((string[1] & 0x3F) << 6) | (string[2] & 0x3F);
            string += 3;
        }
        else if((character & 0xE0) == 0xC0) {
            character = ((character & 0x1F) << 6) | (string[1] & 0x3F);
            string += 2;
        }
        else {
            character &= 0x7F;
            string++;
        }

        // handle character
        switch(character) {
            case '\n':
                y += system_font->character_height;
                x = original_x;
                break;
            case '\r':
                x = original_x;
                break;
            case '\t':
                x += (size_of_tabulator - ((x - original_x) % size_of_tabulator));
                break;
            default:
                draw_bitmap_char(part, x, y, character, color);
                x += system_font->character_width;
                break;
        }
    }
}

void destroy_bitmap_font(bitmap_font_t *font) {
    kfree(font->data);
    kfree(font);
}