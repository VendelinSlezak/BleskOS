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
#include <kernel/hardware/main.h>
#include <kernel/software/syscall.h>

#include <libraries/time.h>
#include <libraries/gui.h>
#include <libraries/logging.h>
#include <libraries/bleskalloc.h>

/* global variables */
syslib_t *syslib = NULL;

/* local variables */
uint8_t hw_list[sizeof(hardware_list_t) + (sizeof(hardware_list_entry_t) * MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST)];

/* functions */
void syslib_initialize(syslib_t *syslib_ptr) {
    syslib = syslib_ptr;
    
    syscall_get_list_of_devices(hw_list, sizeof(hardware_list_t) + (sizeof(hardware_list_entry_t) * MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST));
    hardware_list_t *hardware_list = (hardware_list_t *) hw_list;
    for(int i = 0; i < hardware_list->number_of_entries; i++) {
        if(hardware_list->entries[i].type == HARDWARE_TYPE_TIMER) {
            timer = &hardware_list->entries[i];
        }
        if(hardware_list->entries[i].type == HARDWARE_TYPE_LOGGER) {
            logger = &hardware_list->entries[i];
        }
        if(hardware_list->entries[i].type == HARDWARE_TYPE_WINDOW) {
            window_subsystem = &hardware_list->entries[i];
        }
        if(hardware_list->entries[i].type == HARDWARE_TYPE_HUMAN_INPUT_DEVICE) {
            human_input = &hardware_list->entries[i];
        }
    }

    syslib_initialize_bleskalloc();
}