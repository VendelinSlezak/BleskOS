//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

word_t bga_read(byte_t index) {
 outw(BGA_REGISTER_INDEX, index);
 return inw(BGA_REGISTER_DATA);
}

void bga_write(byte_t index, word_t value) {
 outw(BGA_REGISTER_INDEX, index);
 outw(BGA_REGISTER_DATA, value);
}

byte_t is_bga_present(void) {
 //read ID
 word_t id = bga_read(BGA_INDEX_ID);

 //check if BGA is present
 if(id < 0xB0C0 || id > 0xB0C5) {
  return STATUS_FALSE;
 }

 //log
 logf("\n\nBochs Graphic Adapter version %d", (id & 0xF));

 //BGA is present
 return STATUS_TRUE;
}

void bga_set_graphic_mode(void) {
 //disable BGA
 bga_write(BGA_INDEX_ENABLE, 0x0000);

 //set graphic mode 1024x768x32
 bga_write(BGA_INDEX_WIDTH, 1024);
 bga_write(BGA_INDEX_HEIGHT, 768);
 bga_write(BGA_INDEX_BPP, 32);

 //update variables
 monitor_screen_bytes_per_line = 1024*4;
 screen_width = 1024;
 screen_height = 768;
 screen_bpp = 32;

 //enable BGA with Linear Frame Buffer
 bga_write(BGA_INDEX_ENABLE, 0x0001 | 0x0040);
}