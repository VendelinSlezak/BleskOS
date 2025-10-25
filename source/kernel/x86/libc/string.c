/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

size_t strlen(const char *str) {
    size_t length = 0;
    
    while(str[length] != '\0') {
        length++;
    }
    
    return length;
}

int strcmp(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;

    while(i < n && s1[i] && (s1[i] == s2[i])) {
        i++;
    }

    if(i == n) {
        return 0;
    }
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

void *memset(void *dest, int value, size_t len) {
    byte_t byte_val = (byte_t) value;
    byte_t *ptr = (byte_t *) dest;

    while(((dword_t)ptr % 4 != 0) && len > 0) {
        *ptr++ = byte_val;
        len--;
    }

    dword_t word_val = ((dword_t)byte_val << 24) |
                        ((dword_t)byte_val << 16) |
                        ((dword_t)byte_val << 8) |
                        ((dword_t)byte_val);

    dword_t *ptr32 = (dword_t *) ptr;
    while(len >= 4) {
        *ptr32++ = word_val;
        len -= 4;
    }

    ptr = (byte_t *) ptr32;
    while(len-- > 0) {
        *ptr++ = byte_val;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t len) {
    byte_t *d = (byte_t *) dest;
    const byte_t *s = (const byte_t *) src;

    while(len--) {
        *d++ = *s++;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t len) {
    byte_t *d = (byte_t *)dest;
    const byte_t *s = (const byte_t *)src;

    if(d == s || len == 0)
        return dest;

    if(d < s) {
        while (len--) {
            *d++ = *s++;
        }
    }
    else {
        d += len;
        s += len;
        while(len--) {
            *--d = *--s;
        }
    }

    return dest;
}

char *strcpy(char *dest, const char *src) {
    char *original_dest = dest;

    while((*dest++ = *src++) != '\0') {}

    return original_dest;
}