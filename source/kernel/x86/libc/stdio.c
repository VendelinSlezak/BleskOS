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
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/string.h>

/* functions */
STREAM_t open_rwstream(void) {
    STREAM_t s;

    s.buffer = kmalloc(SIZE_OF_VM_BLOCK);
    s.size = 0;
    s.capacity = SIZE_OF_VM_BLOCK;
    s.read_pointer = 0;
    s.write_pointer = 0;

    return s;
}

void s_grow(STREAM_t *s, dword_t extra) {
    if((s->write_pointer + extra) > s->capacity) {
        dword_t new_capacity = ((s->capacity + extra) * 2);
        s->buffer = krealloc(s->buffer, new_capacity);
        s->capacity = new_capacity;
    }
}

int s_has_data(STREAM_t *s, dword_t size) {
    if((s->read_pointer + size) <= s->capacity) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

int sputc(int c, STREAM_t *s) {
    s_grow(s, 1);
    s->buffer[s->write_pointer++] = (byte_t) c;
    s->size = s->write_pointer;
    return c;
}

size_t swrite(const void *ptr, size_t size, size_t nmemb, STREAM_t *s) {
    size_t total = size * nmemb;
    s_grow(s, total);
    memcpy(s->buffer + s->write_pointer, ptr, total);
    s->write_pointer += total;
    s->size = s->write_pointer;
    return nmemb;
}

int sgetc(STREAM_t *s) {
    if(s->read_pointer >= s->size) {
        return EOF;
    }
    return (byte_t) s->buffer[s->read_pointer++];
}

size_t sread(void *ptr, size_t size, size_t nmemb, STREAM_t *s) {
    size_t total = size * nmemb;
    if(s->read_pointer + total > s->size) {
        total = s->size - s->read_pointer;
    }
    memcpy(ptr, s->buffer + s->read_pointer, total);
    s->read_pointer += total;
    return total / size;
}

char *sgets(char *str, int n, STREAM_t *s) {
    if(s->read_pointer >= s->size) {
        return NULL;
    }
    int i = 0;
    while(i < (n - 1) && s->read_pointer < s->size) {
        char c = s->buffer[s->read_pointer++];
        str[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    str[i] = '\0';
    return str;
}

int sputs(const char *str, STREAM_t *s) {
    return swrite(str, 1, strlen(str), s) == strlen(str) ? 0 : EOF;
}

int sseek(STREAM_t *s, long offset, int whence) {
    dword_t new_pos;
    switch(whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = s->read_pointer + offset;
            break;
        case SEEK_END:
            new_pos = s->size + offset;
            break;
        default:
            return -1;
    }
    if(new_pos > s->size) {
        return -1;
    }
    s->read_pointer = new_pos;
    return 0;
}

long stell(STREAM_t *s) {
    return s->read_pointer;
}

int seof(STREAM_t *s) {
    return (s->read_pointer >= s->size);
}

void sclose(STREAM_t *s) {
    free(s->buffer);
    s->buffer = NULL;
    s->size = 0;
    s->capacity = 0;
    s->read_pointer = 0;
    s->write_pointer = 0;
}