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
#include <syslib.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/software/running_executables.h>
#include <kernel/software/syscall.h>

/* functions */
void vh_log_doorbell(uint32_t demand) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    program_t *program = lpdata->current_program;
    running_executable_t *re = program->running_executable;
    virtual_hardware_t *virtual_hardware = re->template->virtual_hardware;

    switch(demand) {
        case VH_LOG_DEMAND_PRINTLNF: {
            log("\n[%s] ", re->name);
            uint32_t *parameters = virtual_hardware->log_string_parameters;
            uint32_t parameters_count = 0;
            for(int i = 0; i < 256; i++) {
                if(virtual_hardware->log_string[i] == '\0') {
                    break;
                }
                else if(virtual_hardware->log_string[i] == '%') {
                    i++;
                    switch(virtual_hardware->log_string[i]) {
                        case '%': {
                            log_char('%');
                            break;
                        }
                        case 'c': {
                            log_char(parameters[parameters_count++]);
                            break;
                        }
                        case 'd': {
                            log("%d", parameters[parameters_count++]);
                            break;
                        }
                        case 'x': {
                            log("%x", parameters[parameters_count++]);
                            break;
                        }
                        case 's': {
                            char *string = (char *)parameters[parameters_count++];
                            while(return_validated_pointer(string, 1) != NULL && *string != 0) {
                                log_char(*string++);
                            }
                            break;
                        }
                    }
                }
                else {
                    log_char(virtual_hardware->log_string[i]);
                }
            }
        }
    }
}