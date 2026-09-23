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
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/subsystems/screen/main_panel.h>
#include <kernel/libc/string.h>

/* global variables */
running_executable_list_t *running_executable_list;

/* functions */
void initialize_executables(void) {
    running_executable_list = (running_executable_list_t *) kalloc(sizeof(running_executable_list_t));
}

running_executable_t *add_running_executable(void) {
    running_executable_list = (running_executable_list_t *) krealloc(running_executable_list, sizeof(running_executable_list_t) + (sizeof(running_executable_t *) * (running_executable_list->number_of_running_executables + 1)));
    running_executable_t *running_executable = (running_executable_t *) kalloc(sizeof(running_executable_t));
    running_executable_list->running_executable[running_executable_list->number_of_running_executables] = running_executable;
    running_executable_list->number_of_running_executables++;
    return running_executable;
}

running_executable_t *get_running_executable_by_part(screen_part_t *part) {
    for(int i = 0; i < running_executable_list->number_of_running_executables; i++) {
        if(running_executable_list->running_executable[i]->part == part) {
            return running_executable_list->running_executable[i];
        }
    }
    return NULL;
}

running_executable_t *get_not_shown_running_executable_except(running_executable_t *running_executable) {
    for(int i = 0; i < running_executable_list->number_of_running_executables; i++) {
        if(running_executable_list->running_executable[i] != running_executable && running_executable_list->running_executable[i]->part != NULL) {
            return running_executable_list->running_executable[i];
        }
    }
    return NULL;
}

void stop_everything_on_page_directory(running_executable_t *running_executable, uint32_t page_directory) {
    if(running_executable->page_directory_of_virtual_hardware == page_directory) {
        running_executable->page_directory_of_virtual_hardware = NULL;
    }
}

void remove_running_executable(running_executable_t *running_executable) {
    for(int i = 0; i < running_executable_list->number_of_running_executables; i++) {
        if(running_executable_list->running_executable[i] == running_executable) {
            memmove(&running_executable_list->running_executable[i], &running_executable_list->running_executable[i + 1], sizeof(running_executable_t *) * (running_executable_list->number_of_running_executables - i - 1));
            running_executable_list->number_of_running_executables--;
            break;
        }
    }
    part_program_closed(running_executable->part);
    kfree(running_executable);
}