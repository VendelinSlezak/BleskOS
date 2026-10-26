/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// right now we just use primitive bump allocation

/* includes */
#include <userspace_library/libc/string.h>

/* local variables */
void *bleskalloc_ptr;
uint32_t bleskalloc_free_size;

/* functions */
void initialize_bleskalloc(void) {
    bleskalloc_ptr = (void *) (1024 * 1024 * 256); // start at 256 MB in virtual memory
    bleskalloc_free_size = 1024 * 1024 * 768; // start with 768 MB of free space
}

void *malloc(uint32_t size) {
    size = (size + 3) / 4 * 4;
    if(bleskalloc_free_size < size) {
        return NULL;
    }
    void *ptr = bleskalloc_ptr;
    bleskalloc_ptr += size;
    bleskalloc_free_size -= size;
    return ptr;
}

void *aligned_malloc(uint32_t alignment, uint32_t size) {
    size = (size + 3) / 4 * 4;
    uint32_t bytes_to_alignment_in_free_space = (alignment - ((uint32_t) bleskalloc_ptr & (alignment - 1)));
    if(bytes_to_alignment_in_free_space != alignment) {
        if(bleskalloc_free_size < bytes_to_alignment_in_free_space) {
            return NULL;
        }
        else {
            bleskalloc_ptr += bytes_to_alignment_in_free_space;
            bleskalloc_free_size -= bytes_to_alignment_in_free_space;
        }
    }
    if(bleskalloc_free_size < size) {
        return NULL;
    }
    void *ptr = bleskalloc_ptr;
    bleskalloc_ptr += size;
    bleskalloc_free_size -= size;
    return ptr; 
}

void *calloc(uint32_t count, uint32_t size) {
    void *ptr = malloc(count * size);
    if(ptr != NULL) {
        memset(ptr, 0, count * size);
    }
    return ptr;
}

void *realloc(void *ptr, uint32_t size) {
    if(ptr == NULL) {
        return malloc(size);
    }
    void *new_ptr = malloc(size);
    if(new_ptr == NULL) {
        return NULL;
    }
    memcpy(new_ptr, ptr, size);
    free(ptr);
    return new_ptr;
}

void free(void *ptr) {
    return;
}