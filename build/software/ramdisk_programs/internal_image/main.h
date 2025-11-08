#ifndef BUILD_SOFTWARE_RAMDISK_PROGRAMS_INTERNAL_IMAGE_MAIN_H
#define BUILD_SOFTWARE_RAMDISK_PROGRAMS_INTERNAL_IMAGE_MAIN_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef INTERNAL_IMAGE_H
#define INTERNAL_IMAGE_H

#include <software/interfaces/memch/main.h>

typedef struct {
    dword_t width;
    dword_t height;
    dword_t pixels[];
} output_image_t;

typedef struct {
    dword_t output_width;
    dword_t output_height;
    dword_t output_start_x;
    dword_t output_start_y;
} drawIonScreen_param_t;

#endif
void display_internal_image(memch_program_data_t *data);

#endif /* BUILD_SOFTWARE_RAMDISK_PROGRAMS_INTERNAL_IMAGE_MAIN_H */
