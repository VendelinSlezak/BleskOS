/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* functions */
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
    uint8_t byte_val = (uint8_t) value;
    uint8_t *ptr = (uint8_t *) dest;

    while(((uint32_t)ptr % 4 != 0) && len > 0) {
        *ptr++ = byte_val;
        len--;
    }

    uint32_t word_val = ((uint32_t)byte_val << 24) |
                        ((uint32_t)byte_val << 16) |
                        ((uint32_t)byte_val << 8) |
                        ((uint32_t)byte_val);

    uint32_t *ptr32 = (uint32_t *) ptr;
    while(len >= 4) {
        *ptr32++ = word_val;
        len -= 4;
    }

    ptr = (uint8_t *) ptr32;
    while(len-- > 0) {
        *ptr++ = byte_val;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t len) {
    uint8_t *d = (uint8_t *) dest;
    const uint8_t *s = (const uint8_t *) src;

    while(len--) {
        *d++ = *s++;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

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

int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *p1 = (const unsigned char *)a;
    const unsigned char *p2 = (const unsigned char *)b;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i])
            return (int)p1[i] - (int)p2[i];
    }

    return 0;
}

char *strcpy(char *dest, const char *src) {
    char *original_dest = dest;

    while((*dest++ = *src++) != '\0') {}

    return original_dest;
}