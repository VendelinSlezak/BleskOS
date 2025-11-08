#ifndef BUILD_KERNEL_X86_LIBC_STDIO_H
#define BUILD_KERNEL_X86_LIBC_STDIO_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef struct {
    byte_t *buffer;
    dword_t size;
    dword_t capacity;
    dword_t read_pointer;
    dword_t write_pointer;
} STREAM_t;
STREAM_t open_rwstream(void);
void s_grow(STREAM_t *s, dword_t extra);
int s_has_data(STREAM_t *s, dword_t size);
int sputc(int c, STREAM_t *s);
size_t swrite(const void *ptr, size_t size, size_t nmemb, STREAM_t *s);
int sgetc(STREAM_t *s);
size_t sread(void *ptr, size_t size, size_t nmemb, STREAM_t *s);
char *sgets(char *str, int n, STREAM_t *s);
int sputs(const char *str, STREAM_t *s);
int sseek(STREAM_t *s, long offset, int whence);
long stell(STREAM_t *s);
int seof(STREAM_t *s);
void sclose(STREAM_t *s);

#endif /* BUILD_KERNEL_X86_LIBC_STDIO_H */
