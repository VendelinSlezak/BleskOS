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
#include <userspace_library/main.h>
#include <userspace_library/bitmap_font.h>
#include <userspace_library/bleskalloc.h>
#include <userspace_library/logging.h>
#include <userspace_library/libc/string.h>
#include <userspace_library/screen.h>

/* local variables */
bitmap_font_t *default_font = NULL;

/* functions */
void initialize_bitmap_font(void) {
    default_font = load_bitmap_font((void *)0xB0000000); // TODO: after storage virtual hardware load font from file
    if(default_font == ERROR) {
        log("Failed to load default font");
        return;
    }
}

bitmap_font_t *load_bitmap_font(void *file) {
    psf1_header_t *psf1_header = file;
    if(psf1_header->magic == PSF1_FONT_MAGIC) {
        return load_psf1_font(psf1_header);
    }

    psf2_header_t *psf2_header = file;
    if(psf2_header->magic == PSF2_FONT_MAGIC) {
        return load_psf2_font(psf2_header);
    }

    return ERROR;
}

bitmap_font_t *load_psf1_font(psf1_header_t *file) {
    bitmap_font_t *font = malloc(sizeof(bitmap_font_t));
    font->number_of_characters = (file->font_mode & 0x1) ? 512 : 256;
    font->bytes_per_character = file->character_size;
    font->character_width = 8;
    font->character_height = file->character_size;
    font->data = malloc(font->number_of_characters * font->bytes_per_character);
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
    bitmap_font_t *font = malloc(sizeof(bitmap_font_t));
    font->number_of_characters = file->num_of_glyphs;
    font->bytes_per_character = file->bytes_per_glyph;
    font->character_width = file->width;
    font->character_height = file->height;
    font->data = malloc(font->number_of_characters * font->bytes_per_character);
    memcpy(font->data, file->data, font->number_of_characters * font->bytes_per_character);
    return font;
}

void draw_bitmap_char(buffer_box_t *box, int x, int y, uint32_t character, uint32_t color) {
    bitmap_font_t *font = default_font;
    if(character >= font->number_of_characters) {
        log("\nCharacter %x is not in the font", character);
        character = '?';
    }
    
    position_t pos = calculate_real_position_in_box(box, x, y, font->character_width, font->character_height);
    if(pos.drawable == false) {
        return;
    }

    int full_width = font->character_width;
    int full_height = font->character_height;
    int bytes_per_row = (full_width + 7) / 8;
    int start_x = pos.first_column;
    int start_y = pos.first_line;
    uint8_t *src = font->data + character * font->bytes_per_character;
    src += start_y * bytes_per_row;

    for(int i = 0; i < pos.height; i++) {
        for(int j = 0; j < pos.width; j++) {
            int current_x = start_x + j;
            int byte_idx = current_x / 8;
            int bit_idx = 7 - (current_x % 8);
            if((src[byte_idx] >> bit_idx) & 0x1) {
                draw_pixel(box, pos.left + j, pos.top + i, color);
            }
        }
        src += bytes_per_row;
    }
}

void draw_bitmap_string(buffer_box_t *box, uint32_t x, uint32_t y, char *string, uint32_t color) {
    bitmap_font_t *font = default_font;
    uint32_t original_x = x;
    uint32_t size_of_tabulator = font->character_width * 4;
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
            string++;
        }

        // handle character
        switch(character) {
            case '\n':
                y += font->character_height;
                x = original_x;
                break;
            case '\r':
                x = original_x;
                break;
            case '\t':
                x += (size_of_tabulator - ((x - original_x) % size_of_tabulator));
                break;
            default:
                draw_bitmap_char(box, x, y, character, color);
                x += font->character_width;
                break;
        }
    }
}

void destroy_bitmap_font(bitmap_font_t *font) {
    free(font->data);
    free(font);
}