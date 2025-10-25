/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*

    Testing BleskOS system call specification

    eax = group of call
    ebx = call number
    rest = parameters

    GROUP 0 = Kernel Calls
        CALL 0 = TEST
            Logs "test kernel call"
        CALL 1 = EXIT COMMAND
            Quits command
        CALL 2 = SLEEP
            ecx = microseconds to sleep
        CALL 3 = SWITCH
            Yields execution
        CALL 4 = WAIT FOR COMMAND
            ecx = number of command
            Waits until this command is done

*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/scheduler/main.h>

/* functions */
// #define SC_DEBUG_LOGS
void system_call(void) {
    extern interrupt_stack_t *stack_of_interrupt;

    if(stack_of_interrupt->eax == 0) {
        if(stack_of_interrupt->ebx == 0) {
            log("\ntest kernel call");
        }
        else if(stack_of_interrupt->ebx == 1) {
            #ifdef SC_DEBUG_LOGS
            log("\ndestroy command %d", kernel_attr->running_command);
            #endif

            destroy_running_command();
        }
        else if(stack_of_interrupt->ebx == 2) {
            #ifdef SC_DEBUG_LOGS
            log("\ncommand %d sleep %d", kernel_attr->running_command, stack_of_interrupt->ecx);
            #endif

            move_command_to_sleeping_state(stack_of_interrupt->ecx);
        }
        else if(stack_of_interrupt->ebx == 3) {
            #ifdef SC_DEBUG_LOGS
            log("\nswitching commands");
            #endif

            command_t *command = command_get_ptr(kernel_attr->running_command);
            command->time_of_actual_run = SCHEDULER_PERIOD; // all time given for this command is gone
            switch_commands();
        }
        else if(stack_of_interrupt->ebx == 4) {
            #ifdef SC_DEBUG_LOGS
            log("\ncommand %d waiting for command %d", kernel_attr->running_command, stack_of_interrupt->ecx);
            #endif

            move_command_to_waiting_state(stack_of_interrupt->ecx);
        }
        else {
            log("\nunknown system call number %d", stack_of_interrupt->ebx);
            kernel_panic("System call can not process interrupt");
        }
    }
    else {
        log("\nunknown system call group %d", stack_of_interrupt->eax);
        kernel_panic("System call can not process interrupt");
    }

    switch_commands();
}