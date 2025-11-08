#ifndef BUILD_KERNEL_X86_SCHEDULER_MAIN_H
#define BUILD_KERNEL_X86_SCHEDULER_MAIN_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef SCHEDULER_MAIN_H
#define SCHEDULER_MAIN_H

#include <kernel/x86/scheduler/lock.h>

#define SCHEDULER_PERIOD 10000 // in microseconds

typedef struct {
    // segments
    dword_t gs;
    dword_t fs;
    dword_t es;
    dword_t ds;

    // general purpose registers
    dword_t edi;
    dword_t esi;
    dword_t ebp;
    dword_t old_esp;
    dword_t ebx;
    dword_t edx;
    dword_t ecx;
    dword_t eax;

    // interrupt stub
    dword_t interrupt_number;

    // interrupt data
    dword_t eip;
    dword_t cs;
    dword_t eflags;

    union {
        dword_t esp;
        dword_t pointer_to_exit_method;
    };
    dword_t ss;
}__attribute__((packed)) interrupt_stack_t;

#define MAX_NUMBER_OF_COMMANDS_IN_RUNNING_RING 128
typedef struct {
    mutex_t mutex;
    dword_t number_of_commands;
    dword_t running_command;
    dword_t commands[MAX_NUMBER_OF_COMMANDS_IN_RUNNING_RING];
}__attribute__((packed)) scheduler_running_ring_t;

#define MAX_NUMBER_OF_COMMANDS_IN_SLEEPING_RING 128
typedef struct {
    mutex_t mutex;
    dword_t number_of_commands;
    dword_t commands[MAX_NUMBER_OF_COMMANDS_IN_SLEEPING_RING];
}__attribute__((packed)) scheduler_sleeping_ring_t;

#define MAX_NUMBER_OF_COMMANDS_IN_WAITING_RING 128
typedef struct {
    mutex_t mutex;
    dword_t number_of_commands;
    dword_t commands[MAX_NUMBER_OF_COMMANDS_IN_WAITING_RING];
}__attribute__((packed)) scheduler_waiting_ring_t;

#endif
void initialize_scheduler(void);
dword_t running_ring_insert_command(scheduler_running_ring_t *running_ring, dword_t command_number);
void running_ring_remove_command(scheduler_running_ring_t *running_ring, dword_t command_number);
dword_t sleeping_ring_insert_command(dword_t command_number);
void sleeping_ring_remove_command(dword_t command_number);
dword_t waiting_ring_insert_command(dword_t command_number);
void waiting_ring_remove_command(dword_t command_number);
dword_t insert_new_command(dword_t command_number);
void remove_command(dword_t command_number);
void move_command_to_sleeping_state(dword_t microseconds);
void move_command_to_waiting_state(dword_t signaling_command);
void move_command_to_exit_state(dword_t command_number);
void switch_commands(void);

#endif /* BUILD_KERNEL_X86_SCHEDULER_MAIN_H */
