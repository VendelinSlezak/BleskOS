//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define STATUS_GOOD 1
#define STATUS_ERROR 0
#define STATUS_TRUE 1
#define STATUS_FALSE 0
#define BIG_ENDIAN(x) (((x & 0xFF)<<8) | ((x>>8) & 0xFF))

#define BOOT_OPTION_DEBUG_MESSAGES 0x1

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;
typedef unsigned long qword_t;

byte_t value8;
word_t value16;
dword_t value32, boot_options;
dword_t converted_file_memory, converted_file_size;

void bleskos(dword_t bootloader_passed_value);
void bleskos_redraw_starting_screen(char *string, dword_t percent);
void bleskos_boot_debug_top_screen_color(dword_t color);
void bleskos_boot_debug_log_message(void);