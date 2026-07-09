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

char *strchr(const char *string, int c) {
    while(*string) {
        if(*string == c) {
            return (char *) string;
        }
        string++;
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