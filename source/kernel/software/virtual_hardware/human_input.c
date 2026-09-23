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

/* functions */
void vh_human_input_doorbell(uint32_t demand) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    program_t *program = lpdata->current_program;
    running_executable_t *re = program->running_executable;
    virtual_hardware_t *virtual_hardware = re->template->virtual_hardware;

    switch(demand) {
        case VH_HUMAN_INPUT_DEMAND_ENABLE_STREAMING: {
            re->is_human_input_streaming_enabled = true;
            break;
        }
        case VH_HUMAN_INPUT_DEMAND_DISABLE_STREAMING: {
            re->is_human_input_streaming_enabled = false;
            break;
        }
    }
}

void vh_human_input_event(running_executable_t *re, uint32_t type, int argument1, int argument2, int argument3) {
    virtual_hardware_t *virtual_hardware = re->template->virtual_hardware;
    if(re->is_human_input_streaming_enabled == false) {
        return;
    }

    move_temporarily_to_virtual_space(re->page_directory_of_virtual_hardware);
    uint32_t next_producer = (virtual_hardware->producer + 1) % VH_HUMAN_INPUT_SIZE_OF_RING;
    if(virtual_hardware->consumer == next_producer) {
        move_back_to_previous_virtual_space();
        return;
    }
    vh_human_input_event_t *event = &virtual_hardware->events[virtual_hardware->producer];
    event->type = type;
    event->argument1 = argument1;
    event->argument2 = argument2;
    event->argument3 = argument3;
    virtual_hardware->producer = next_producer;
    move_back_to_previous_virtual_space();
}