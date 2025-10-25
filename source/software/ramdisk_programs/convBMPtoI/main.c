/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <hardware/groups/logging/logging.h>
#include <software/interfaces/memch/main.h>
#include <kernel/x86/system_call.h>
#include <software/ramdisk_programs/internal_image/main.h>

/* functions */
void conv_bmp_to_i(memch_program_data_t *data) {
    // load BMP header
    BMPHeader_t header;
    memch_input_read(data, &header, sizeof(header));

    // check signature
    if(header.bfType != 0x4D42)  {
        memch_set_state(data, MEMCH_STATE_ERROR, BMP_WRONG_SIGNATURE);
        return;
    }

    // read BMP info
    BMPInfoHeader_t info;
    memch_input_read(data, &info, sizeof(info));

    // check bpp
    int bpp = info.biBitCount;
    if(bpp != 24 && bpp != 32) {
        memch_set_state(data, MEMCH_STATE_ERROR, BMP_UNSUPPORTED_BPP);
        return;
    }

    // read size
    int width = info.biWidth;
    int height = info.biHeight;

    // allocate output image
    memch_output_prepare_size(data, sizeof(output_image_t) + width * height * sizeof(dword_t));
    output_image_t *img = memch_output_get_ptr(data);
    img->width = width;
    img->height = height;

    // pointer to start of pixel data
    memch_input_seek(data, header.bfOffBits, SEEK_SET);
    const byte_t *bmp_pixels = get_memch_input_ptr(data);

    // size of one line in BMP (rounded to 4 bytes)
    int row_padded = ((bpp / 8 * width + 3) / 4) * 4;

    // BMP is saved bottom-up
    for(int y = 0; y < height; y++) {
        const byte_t *src_row = bmp_pixels + (height - 1 - y) * row_padded;
        dword_t *dst_row = &img->pixels[y * width];

        for(int x = 0; x < width; x++) {
            const byte_t *p = src_row + x * (bpp / 8);
            dword_t pixel;

            if(bpp == 24) {
                // BMP 24-bit = BGR, add alpha
                pixel = 0xFF000000 | (p[2] << 16) | (p[1] << 8) | p[0];
            }
            else {
                // BMP 32-bit = BGRA
                pixel = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
            }

            dst_row[x] = pixel;
        }

        // update processed percents of image
        memch_set_state(data, MEMCH_STATE_PROCESSING, 100 * y / height);
    }

    // update state
    memch_set_state(data, MEMCH_STATE_SUCCESS, 100);
}