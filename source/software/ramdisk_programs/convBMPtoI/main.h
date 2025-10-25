/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CONVBMPTOI_MAIN_H
#define CONVBMPTOI_MAIN_H

#include <kernel/x86/libc/stdio.h>
#include <software/interfaces/memch/main.h>

typedef struct {
    word_t bfType;
    dword_t bfSize;
    word_t bfReserved1;
    word_t bfReserved2;
    dword_t bfOffBits;
}__attribute__((packed)) BMPHeader_t;

typedef struct {
    dword_t biSize;
    int biWidth;
    int biHeight;
    word_t biPlanes;
    word_t biBitCount;
    dword_t biCompression;
    dword_t biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    dword_t biClrUsed;
    dword_t biClrImportant;
}__attribute__((packed)) BMPInfoHeader_t;

enum {
    BMP_WRONG_SIGNATURE,
    BMP_UNSUPPORTED_BPP,
};

#endif