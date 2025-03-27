//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define BLESKOS_VERSION 2025
#define BLESKOS_UPDATE 9

#define STATUS_GOOD 1
#define STATUS_ERROR 0

#define STATUS_TRUE 1
#define STATUS_FALSE 0

#define BIG_ENDIAN_WORD(x) (((x & 0xFF)<<8) | ((x>>8) & 0xFF))
#define BIG_ENDIAN_DWORD(x) (((x & 0x000000FF)<<24) | ((x & 0x0000FF00)<<8) | ((x & 0x00FF0000)>>8) | ((x & 0xFF000000)>>24))

#define BOOT_OPTION_DEBUG_MESSAGES 0x1
#define BOOT_OPTION_DEEP_DEBUGGER 0x2

#define BLESKOS_NUMBER_OF_START_SCREEN_MESSAGES 10

typedef unsigned int size_t;
typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;
typedef unsigned long long qword_t;

byte_t value8;
dword_t value32, boot_options;
dword_t converted_file_memory, converted_file_size;
byte_t number_of_start_screen_messages;

void bleskos(dword_t bootloader_passed_value);