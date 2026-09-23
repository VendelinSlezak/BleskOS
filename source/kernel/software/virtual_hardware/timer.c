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
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/software/running_executables.h>
#include <kernel/software/syscall.h>
#include <kernel/firmware/cmos/cmos.h>
#include <kernel/hardware/devices/timers/main.h>

/* functions */
void vh_timer_doorbell(uint32_t demand) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    program_t *program = lpdata->current_program;
    running_executable_t *re = program->running_executable;
    virtual_hardware_t *virtual_hardware = re->template->virtual_hardware;

    switch(demand) {
        case VH_TIMER_DEMAND_MICROSECONDS: {
            move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
            virtual_hardware->microseconds = get_time_in_microseconds();
            move_back_to_previous_virtual_space();
            break;
        }
        case VH_TIMER_DEMAND_GET_TIME: {
            read_time();
            move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
            virtual_hardware->year = cmos_datetime.year;
            virtual_hardware->month = cmos_datetime.month;
            virtual_hardware->day = cmos_datetime.day;
            virtual_hardware->hour = cmos_datetime.hour;
            virtual_hardware->minute = cmos_datetime.minute;
            virtual_hardware->second = cmos_datetime.second;
            move_back_to_previous_virtual_space();
            break;
        }
    }
}