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
#include <syslib.h>
#include <userspace_library/main.h>

/* functions */
void log(char *string, ...) {
    uint32_t length_of_string = 0;
    uint32_t number_of_args = 0;
    va_list args;
    va_start(args, string);
    while(length_of_string < 255 && *string != 0) {
        virtual_hardware->log_string[length_of_string++] = *string;
        if(string[0] == '%' && string[1] != '%') {
            number_of_args++;
            if(number_of_args >= 16) {
                break;
            }
        }
        string++;
    }
    virtual_hardware->log_string[length_of_string] = '\0';
    for(int i = 0; i < number_of_args; i++) {
        virtual_hardware->log_string_parameters[i] = va_arg(args, uint32_t);
    }
    va_end(args);
    syscall_virtual_hardware(VIRTUAL_HARDWARE_LOG_ID, VH_LOG_DEMAND_PRINTLNF);
}