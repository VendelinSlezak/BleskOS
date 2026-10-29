/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <stdarg.h>
#include <kernel/libc/stdlib.h>
#include <kernel/libc/ctype.h>
#include <kernel/hardware/devices/cpu/commands.h>

/* global variables */
void *(*memcpy)(void *dst, void *src, size_t len) = memcpy_i386;
void *(*memcpynt)(void *dst, void *src, size_t len) = memcpy_i386;
void (*memcpynt_finish)(void) = memcpynt_finish_i386;

/* functions */
void *memcpy_i386(void *dst, void *src, size_t len) {
    void *original_dest = dst;
    extern void memcpy1(void *dst, void *src, uint32_t len);
    extern void memcpy4(void *dst, void *src, uint32_t len);
    uint32_t blocks = len / 4 * 4;
    memcpy4(dst, src, blocks);
    memcpy1(dst + blocks, src + blocks, len - blocks);
    return original_dest;
}

void memcpynt_finish_i386(void) {
    return;
}

void *memcpy_sse(void *dst, void *src, size_t len) {
    extern void memcpy1(void *dst, void *src, uint32_t len);
    extern void memcpy128a(void *dst, void *src, uint32_t len);
    extern void memcpy128u(void *dst, void *src, uint32_t len);
    extern void memcpynt128a(void *dst, void *src, uint32_t len);
    extern void memcpynt128u(void *dst, void *src, uint32_t len);

    // small block are copied faster by good old rep movsd
    if(len < 2048) {
        return memcpy_i386(dst, src, len);
    }

    // align destination to 16 bytes
    void *original_dest = dst;
    if(((uint32_t)dst % 16) != 0) {
        uint32_t bytes_to_align_dst = 16 - ((uint32_t)dst % 16);
        memcpy1(dst, src, bytes_to_align_dst);
        dst += bytes_to_align_dst;
        src += bytes_to_align_dst;
        len -= bytes_to_align_dst;
    }

    // copy by 128b blocks
    uint32_t bytes_to_copy_by_blocks = len / 128 * 128;
    if(len < (1024 * 1024)) { // TODO: set treshold by half size of L3 cache
        if(((uint32_t)src % 16) == 0) {
            memcpy128a(dst, src, bytes_to_copy_by_blocks);
        }
        else {
            memcpy128u(dst, src, bytes_to_copy_by_blocks);
        }
        len -= bytes_to_copy_by_blocks;
    }
    else {
        if(((uint32_t)src % 16) == 0) {
            memcpynt128a(dst, src, bytes_to_copy_by_blocks);
        }
        else {
            memcpynt128u(dst, src, bytes_to_copy_by_blocks);
        }
        sfence();
    }
    len -= bytes_to_copy_by_blocks;

    // copy rest of bytes
    uint32_t bytes_to_align = ((uint32_t)len % 128);
    if(bytes_to_align != 128) {
        dst += bytes_to_copy_by_blocks;
        src += bytes_to_copy_by_blocks;
        memcpy_i386(dst, src, bytes_to_align);
    }

    return original_dest;
}

void *memcpynt_sse(void *dst, void *src, size_t len) {
    extern void memcpy1(void *dst, void *src, uint32_t len);
    extern void memcpynt128a(void *dst, void *src, uint32_t len);
    extern void memcpynt128u(void *dst, void *src, uint32_t len);

    // align destination to 16 bytes
    void *original_dest = dst;
    if(((uint32_t)dst % 16) != 0) {
        uint32_t bytes_to_align_dst = 16 - ((uint32_t)dst % 16);
        memcpy1(dst, src, bytes_to_align_dst);
        dst += bytes_to_align_dst;
        src += bytes_to_align_dst;
        len -= bytes_to_align_dst;
    }

    // copy by 128b blocks
    uint32_t bytes_to_copy_by_blocks = len / 128 * 128;
    if(((uint32_t)src % 16) == 0) {
        memcpynt128a(dst, src, bytes_to_copy_by_blocks);
    }
    else {
        memcpynt128u(dst, src, bytes_to_copy_by_blocks);
    }
    len -= bytes_to_copy_by_blocks;

    // copy rest of bytes
    uint32_t bytes_to_align = ((uint32_t)len % 128);
    if(bytes_to_align != 128) {
        dst += bytes_to_copy_by_blocks;
        src += bytes_to_copy_by_blocks;
        memcpy_i386(dst, src, bytes_to_align);
    }

    return original_dest;
}

void memcpynt_finish_sse(void) {
    sfence();
}

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

void strncpy(char* dest, const char *src, unsigned int n) {
    int i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    if (i < n) {
        dest[i] = 0;
    }
}

char *strcat(char *dest, const char *src) {
    char *ptr = dest;
    while (*ptr != '\0') {
        ptr++;
    }
    while (*src != '\0') {
        *ptr = *src;
        ptr++;
        src++;
    }
    *ptr = '\0';
    return dest;
}

char *strchr(const char *string, int c) {
    while(*string) {
        if(*string == c) {
            return (char *) string;
        }
        string++;
    }
    return NULL;
}

char *strstr(const char *haystack, const char *needle) {
    if (*needle == '\0') {
        return (char *)haystack;
    }
    for (int i = 0; haystack[i] != '\0'; i++) {
        int j = 0;
        while (haystack[i + j] != '\0' && needle[j] != '\0' && haystack[i + j] == needle[j]) {
            j++;
        }
        if (needle[j] == '\0') {
            return (char *)(haystack + i);
        }
    }
    return NULL;
}

int sprintf(char *str, const char *format, ...) {
    char *start = str;

    va_list args;
    va_start(args, format);

    while(*format != 0) {
        // print character from string
        if(*format != '%') {
            *str++ = *format;
            format++;
        }
        else {
            // skip '%' char
            format++;

            // print % character
            if(*format == '%') {
                *str++ = '%';
                format++;
                continue;
            }
            // print character from argument
            else if(*format == 'c') {
                // load character
                uint16_t character = va_arg(args, uint32_t);
                format++;

                // print character
                *str++ = character;
                continue;
            }
            // get number of characters to print
            uint32_t number_of_chars_in_parameter = 0;
            if(*format == '0') {
                format++;
                number_of_chars_in_parameter = atoi(format);

                // skip numbers in string
                while(isdigit(*format) != false) {
                    format++;
                }
            }

            // print string from argument
            if(*format == 's') {
                // load argument
                uint8_t *parameter_string = va_arg(args, uint8_t *);

                // print full string
                if(number_of_chars_in_parameter == 0) {
                    while(*parameter_string != 0) {
                        *str++ = *parameter_string;
                        parameter_string++;
                    }
                }
                // print part of string
                else {
                    for(uint32_t i = 0; i < number_of_chars_in_parameter; i++) {
                        *str++ = *parameter_string;
                        parameter_string++;
                    }
                }

                // skip 's' / 'S' char
                format++;
            }
            // print unicode string from argument
            else if(*format == 'S') {
                // load argument
                uint16_t *parameter_string = va_arg(args, uint16_t *);

                // print full string
                if(number_of_chars_in_parameter == 0) {
                    while(*parameter_string != 0) {
                        *str++ = *parameter_string;
                        parameter_string++;
                    }
                }
                // print part of string
                else {
                    for(uint32_t i = 0; i < number_of_chars_in_parameter; i++) {
                        *str++ = *parameter_string;
                        parameter_string++;
                    }
                }

                // skip 'S' char
                format++;
            }
            // print decadic number from argument
            else if(*format == 'd' || *format == 'u') {
                // buffer na číslo ako string
                char number_string[12]; // big enough for sign + whole number + zero ending

                // convert number to string
                itoa(va_arg(args, uint32_t), number_string, 11);

                // add zeros if needed
                uint32_t number_of_chars_in_number = strlen(number_string);
                for(uint32_t i = number_of_chars_in_number; i < number_of_chars_in_parameter; i++) {
                    *str++ = '0';
                }

                // print number string
                char *p = number_string;
                while(*p != '\0') {
                    *str++ = *p;
                    p++;
                }

                // skip 'd' in string
                format++;
            }
            // print hexadecimal number from argument
            else if(*format == 'x' || *format == 'X') {
                // load argument
                uint32_t number = va_arg(args, uint32_t);

                // set number of digits on output
                if(number_of_chars_in_parameter == 0) {
                    number_of_chars_in_parameter = 8;
                }

                // print digits
                for(uint32_t i = 0, digit = 0, shift = (number_of_chars_in_parameter*4 - 4); i < number_of_chars_in_parameter; i++, shift -= 4) {
                    digit = ((number >> shift) & 0xF);
                    *str++ = (digit < 10) ? (digit + '0') : (digit + 'A' - 10);
                }

                // skip 'x' char
                format++;
            }
        }
    }

    va_end(args);

    return (str - start);
}

void switch_16_endian(void *buffer, uint32_t size) {
    uint8_t *buffer8 = buffer;
    for(uint32_t i = 0; i < (size & ~1); i += 2) {
        uint8_t temp = buffer8[i];
        buffer8[i] = buffer8[i + 1];
        buffer8[i + 1] = temp;
    }
}

void switch_32_endian(void *buffer, uint32_t size) {
    uint8_t *buffer8 = buffer;
    for(uint32_t i = 0; i < (size & ~3); i += 4) {
        uint8_t temp = buffer8[i];
        buffer8[i] = buffer8[i + 3];
        buffer8[i + 3] = temp;
        temp = buffer8[i + 1];
        buffer8[i + 1] = buffer8[i + 2];
        buffer8[i + 2] = temp;
    }
}

void switch_64_endian(void *buffer, uint32_t size) {
    uint8_t *buffer8 = buffer;
    for(uint32_t i = 0; i < (size & ~7); i += 8) {
        uint8_t temp = buffer8[i];
        buffer8[i] = buffer8[i + 7];
        buffer8[i + 7] = temp;
        temp = buffer8[i + 1];
        buffer8[i + 1] = buffer8[i + 6];
        buffer8[i + 6] = temp;
        temp = buffer8[i + 2];
        buffer8[i + 2] = buffer8[i + 5];
        buffer8[i + 5] = temp;
        temp = buffer8[i + 3];
        buffer8[i + 3] = buffer8[i + 4];
        buffer8[i + 4] = temp;
    }
}