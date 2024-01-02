//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PNG_COLOR_TYPE_GRAYSCALE 0
#define PNG_COLOR_TYPE_TRUECOLOR 2
#define PNG_COLOR_TYPE_PALETTE 3
#define PNG_COLOR_TYPE_GRAYSCALE_ALPHA 4
#define PNG_COLOR_TYPE_TRUECOLOR_ALPHA 6

#define PNG_BPP_PALETTE_8_BITS 5

#define PNG_FILTERING_NONE 0
#define PNG_FILTERING_SUB 1
#define PNG_FILTERING_UP 2
#define PNG_FILTERING_AVERAGE 3
#define PNG_FILTERING_PAETH 4

dword_t convert_png_to_image_data(dword_t png_memory, dword_t png_file_length);
