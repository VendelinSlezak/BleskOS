//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void vga_text_mode_clear_screen(byte_t color) {
    byte_t *vga_mem = (byte_t *) (0xB8000);

    for(dword_t i=0; i<(80*25); i++) {
        vga_mem[0] = 0;
        vga_mem[1] = color;
        vga_mem += 2;
    }
}

void vga_text_mode_put_char(byte_t line, byte_t column, byte_t character) {
    byte_t *vga_mem = (byte_t *) (0xB8000 + (line*80*2) + (column*2));
    vga_mem[0] = character;
}

void vga_text_mode_print(byte_t line, byte_t column, byte_t *string) {
    byte_t *vga_mem = (byte_t *) (0xB8000 + (line*80*2) + (column*2));

    while(*string != 0 && (dword_t)vga_mem < (0xB8000 + 80*25*2)) {
        vga_mem[0] = *string;
        vga_mem += 2;
        string++;
    }
}

void vga_text_mode_draw_square(byte_t line, byte_t column, byte_t width, byte_t height, byte_t color) {
    if((column+width) >= 80 || (line+height) >= 25) {
        return;
    }

    byte_t *vga_mem = (byte_t *) (0xB8000 + (line*80*2) + (column*2));

    for(dword_t i=0; i<height; i++) {
        for(dword_t j=0; j<width; j++) {
            vga_mem[0] = 0;
            vga_mem[1] = color;
            vga_mem += 2;
        }

        line++;
        vga_mem = (byte_t *) (0xB8000 + (line*80*2) + (column*2));
    }
}

void vga_text_mode_move_cursor(byte_t line, byte_t column) {
	word_t position = ((line*80) + column);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (byte_t)(position & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (byte_t)(position >> 8));
}