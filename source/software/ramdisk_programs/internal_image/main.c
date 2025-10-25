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
#include <kernel/x86/libc/string.h>
#include <kernel/x86/system_call.h>

/* functions */
void display_internal_image(memch_program_data_t *data) {
    drawIonScreen_param_t *param = data->parameters;
    output_image_t *src = (output_image_t *) memch_input_get_ptr(data);
    dword_t *buff = memch_output_get_ptr(data);
    dword_t copy_width  = (src->width < param->output_width) ? src->width : param->output_width;
    dword_t copy_height = (src->height < param->output_height) ? src->height : param->output_height;

    for(dword_t y = 0; y < copy_height; y++) {
        const dword_t *src_row = &src->pixels[y * src->width];
        dword_t *dst_row = &buff[(param->output_start_y + y) * param->output_width + param->output_start_x];
        memcpy(dst_row, src_row, copy_width * sizeof(dword_t));
    }
}