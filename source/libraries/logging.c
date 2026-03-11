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
#include <kernel/hardware/main.h>
#include <kernel/software/syscall.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <libraries/main.h>

/* global variables */
hardware_list_entry_t *logger = NULL;

/* functions */
void syslib_log(char *string) {
    if(logger == NULL) {
        return;
    }
    static logging_device_command_t command = {
        .type = LOGGING_GROUP_COMMAND_SEND_STRING,
        .argument = {
            0
        }
    };
    command.argument[0] = (uint32_t) string;
    syscall_send_command_to_device(logger, &command);
}

void syslib_logf(char *string, ...) {
    if(logger == NULL) {
        return;
    }
    static uint32_t free_memory[2 + 1000];
    logging_device_command_t *command = (logging_device_command_t *) &free_memory[0];
    command->type = LOGGING_GROUP_COMMAND_SEND_FORMATTED_STRING;
    command->argument[0] = (uint32_t) string;
    uint32_t *argument = (uint32_t *) &command->argument[1];
    uint32_t number_of_args = 0;
    va_list args;
    va_start(args, string);
    while(*string != 0) {
        if(*string != '%') {
            string++;
        }
        else {
            string++;
            if(*string == 0) {
                break;
            }
            else if(*string == '%') {
                string++;
            }
            else if(number_of_args < 1000) {
                number_of_args++;
                *argument++ = va_arg(args, uint32_t);
                string++;
            }
        }
    }
    va_end(args);
    syscall_send_command_to_device(logger, command);
}