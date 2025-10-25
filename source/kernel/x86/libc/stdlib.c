/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Prevod int na string v desiatkovej sústave
void itoa(int value, char *str, size_t size) {
    if(size == 0) {
        return;
    }

    char buffer[20];
    int i = 0;
    int is_negative = 0;

    if(value == 0) {
        if(size > 1) {
            str[0] = '0';
            str[1] = '\0';
        }
        return;
    }

    if(value < 0) {
        is_negative = 1;
        value = -value;
    }

    while(value != 0 && i < (int)(sizeof(buffer) - 1)) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    if(i >= (int)size) {
        i = size - 1;
    }

    int j;
    for (j = 0; j < i; j++) {
        str[j] = buffer[i - j - 1];
    }
    str[i] = '\0';
}

int atoi(const char *str) {
    int result = 0;
    int sign = 1;

    while(*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    if(*str == '-') {
        sign = -1;
        str++;
    }
    else if(*str == '+') {
        str++;
    }

    while(*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

char *htoa(unsigned int value, char *str) {
    static const char hex_digits[] = "0123456789ABCDEF";
    char temp[9];
    int i = 0, j = 0;

    do {
        temp[i++] = hex_digits[value & 0xF];
        value >>= 4;
    } while(value && i < 8);

    while(i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
    return str;
}

char *htoan(unsigned int value, char *str, size_t n) {
    static const char hex_digits[] = "0123456789ABCDEF";

    str[0] = '0'; str[1] = 'x'; str[2 + n + 1] = '\0';
    do {
        str[2 + (--n)] = hex_digits[value & 0xF];
        value >>= 4;
    } while(n > 0);

    return str;
}