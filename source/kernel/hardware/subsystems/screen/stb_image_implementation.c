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
#define STB_IMAGE_IMPLEMENTATION
#include <kernel/hardware/subsystems/screen/stb_image.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/subsystems/screen/draw.h>

/* functions */
image_t *load_image(const void *data, uint32_t size) {
    if(very_unlikely(data == NULL || data == (void *)INVALID)) {
        return NULL;
    }
    image_t *image = (image_t *) kalloc(sizeof(image_t));
    image->data = stbi_load_from_memory((uint8_t *)data, size, &image->width, &image->height, &image->channels, 4);
    return image;
}

void draw_image(image_t *image, screen_part_t *part, uint32_t x, uint32_t y) {
    draw_bitmap(part, x, y, (uint32_t *) image->data, image->width, image->height);
}

void free_image(image_t *image) {
    stbi_image_free(image->data);
    kfree(image);
}