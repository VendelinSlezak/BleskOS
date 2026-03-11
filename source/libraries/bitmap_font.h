/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <libraries/screen.h>

typedef struct {
    uint32_t type;
    uint8_t *data;
    uint32_t number_of_characters;
    uint32_t bytes_per_character;
    uint32_t character_width;
    uint32_t character_height;
} bitmap_font_t;

#define PSF1_FONT_MAGIC 0x0436
typedef struct {
    uint16_t magic;
    uint8_t font_mode;
    uint8_t character_size;
    uint8_t data[];
} psf1_header_t;

#define PSF2_FONT_MAGIC 0x864AB572
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t num_of_glyphs;
    uint32_t bytes_per_glyph;
    uint32_t height;
    uint32_t width;
    uint8_t data[];
} psf2_header_t;