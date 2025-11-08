#ifndef BUILD_HARDWARE_GROUPS_LOGGING_LOGGING_H
#define BUILD_HARDWARE_GROUPS_LOGGING_LOGGING_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/x86/libc/stdio.h>
#include <kernel/x86/scheduler/lock.h>

#define MAX_NUMBER_OF_LOGGING_DEVICES 8
typedef struct {
    dword_t number_of_devices;
    void (*send_character[MAX_NUMBER_OF_LOGGING_DEVICES])(dword_t character);
    STREAM_t logs;
    mutex_t logging_to_output;
} e_logging_group_t;
void initialize_logging_group(void);
void *get_start_of_log_stream(void);
void log_char(dword_t character);
void log(byte_t *string, ...);

#endif /* BUILD_HARDWARE_GROUPS_LOGGING_LOGGING_H */
