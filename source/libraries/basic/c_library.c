//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

int strcmp(const char* str1, const char* str2) {
 while(*str1==*str2) {
  if(*str1==0) {
   return 0; //equal
  }
  str1++;
  str2++;
 }

 return 1; //non equal
}

int strncmp(const char *str1, const char *str2, size_t n) {
 for(dword_t i=0; i<n; i++) {
  if(*str1==0 && *str2!=0) {
   return -1; //first is less
  }
  else if(*str1==0 && *str2==0) {
   return 0; //equal
  }
  else if(*str1!=0 && *str2==0) {
   return 1; //first is less
  }
  str1++;
  str2++;
 }

 return 0; //equal
}

long int strtol(const char *str, char **endptr, int base) {
    if (base < 2 || base > 36) {
        // Invalid base
        if (endptr) *endptr = (char *)str;
        return 0;
    }

    // Skip leading whitespace
    while (*str==' ') {
        str++;
    }

    // Handle optional sign
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    else if (*str == '+') {
        str++;
    }

    long int result = 0;
    int any = 0;
    
    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            break;
        }

        if (digit >= base) {
            break;
        }

        any = 1;
        result = result * base + digit;
        str++;
    }

    if (endptr) {
        *endptr = any ? (char *)str : (char *)str - 1;
    }

    return sign * result;
}