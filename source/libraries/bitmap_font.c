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
#include <libraries/main.h>
#include <libraries/bitmap_font.h>
#include <libraries/string.h>
#include <libraries/screen.h>

/* local variables */
uint32_t number_of_characters;
uint32_t bytes_per_character;
uint32_t character_width;
uint32_t character_height;
uint8_t *data;

/* functions */
bitmap_font_t *syslib_load_bitmap_font(void *file, uint32_t size) {
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
    syslib->logf("\n--- Loading PSF1 font ---");
    syslib->logf("\nNumber of characters: %d", (file->font_mode & 0x1) ? 512 : 256);
    syslib->logf("\nSize of character: 8x%d", file->character_size);
    bitmap_font_t *font = syslib->malloc(sizeof(bitmap_font_t));
    font->number_of_characters = (file->font_mode & 0x1) ? 512 : 256;
    font->bytes_per_character = file->character_size;
    font->character_width = 8;
    font->character_height = file->character_size;
    font->data = syslib->malloc(font->number_of_characters * font->bytes_per_character);
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
    syslib->logf("\n--- Loading PSF2 font ---");
    syslib->logf("\nNumber of characters: %d", file->num_of_glyphs);
    syslib->logf("\nSize of character: %dx%d", file->width, file->height);
    syslib->logf("\nBytes per character: %d", file->bytes_per_glyph);
    bitmap_font_t *font = syslib->malloc(sizeof(bitmap_font_t));
    font->number_of_characters = file->num_of_glyphs;
    font->bytes_per_character = file->bytes_per_glyph;
    font->character_width = file->width;
    font->character_height = file->height;
    font->data = syslib->malloc(font->number_of_characters * font->bytes_per_character);
    memcpy(font->data, file->data, font->number_of_characters * font->bytes_per_character);
    return font;
}

void syslib_draw_bitmap_char(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, uint32_t character, uint32_t color) {
    if(x >= buffer->width || y >= buffer->height) {
        return;
    }
    if(character >= font->number_of_characters) {
        syslib->logf("\nCharacter %x is not in the font", character);
        character = '?';
    }

    // TODO: negative offset

    uint32_t showed_width = font->character_width;
    if(showed_width > buffer->width - x) {
        showed_width = buffer->width - x;
    }
    uint32_t showed_height = font->character_height;
    if(showed_height > buffer->height - y) {
        showed_height = buffer->height - y;
    }

    uint32_t *dst = (uint32_t *) &buffer->buffer[y * buffer->width + x];
    uint8_t *src = font->data + character * font->bytes_per_character;
    for(uint32_t i = 0; i < showed_height; i++) {
        uint32_t *line_dst = dst;
        for(int j = 0; j < ((showed_width + 7) / 8); j++) {
            if(src[j] != 0) {
                int s = 7 - ((showed_width + 7) % 8);
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
        dst += buffer->width;
        src += (showed_width + 7) / 8;
    }
}

void syslib_draw_bitmap_string(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, char *string, uint32_t color) {
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
                syslib_draw_bitmap_char(buffer, x, y, font, character, color);
                x += font->character_width;
                break;
        }
    }
}

void syslib_destroy_bitmap_font(bitmap_font_t *font) {
    syslib->free(font->data);
    syslib->free(font);
}