/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/interrupt_controllers/main.h>
#include <kernel/x86/cpu/idt.h>
#include <kernel/x86/cpu/tss.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/system_call.h>

/* functions */
void initialize_scheduler(void) {
    vm_alloc_page(P_MEM_SCHEDULER_RUNNING_COMMANDS, VM_FLAGS_KERNEL_RW);
    vm_alloc_page(P_MEM_SCHEDULER_SLEEPING_COMMANDS, VM_FLAGS_KERNEL_RW);
    vm_alloc_page(P_MEM_SCHEDULER_WAITING_COMMANDS, VM_FLAGS_KERNEL_RW);

    // insert boot command to running ring
    scheduler_running_ring_t *running_ring = (scheduler_running_ring_t *) P_MEM_SCHEDULER_RUNNING_COMMANDS;
    running_ring->number_of_commands = 1;
    running_ring->running_command = 0;
    running_ring->commands[0] = 0; // boot command

    // create idle command stack
    vm_alloc_page(P_MEM_IDLE_COMMAND_STACK, VM_FLAGS_KERNEL_RW);
    interrupt_stack_t *stack = (interrupt_stack_t *) (P_MEM_IDLE_COMMAND_STACK + PAGE_SIZE - sizeof(interrupt_stack_t));

    stack->ds = 0x10 | 0b00;
    stack->es = 0x10 | 0b00;
    stack->fs = 0x10 | 0b00;
    stack->gs = 0x10 | 0b00;

    stack->edi = 0;
    stack->esi = 0;
    stack->ebp = 0;
    stack->old_esp = ((dword_t)stack + 32); // skip pushad data
    stack->ebx = 0;
    stack->edx = 0;
    stack->ecx = 0;
    stack->eax = 0;

    extern void idle_command(void);
    stack->eip = (dword_t) idle_command;
    stack->eflags = 0x00000202; // interrupts enabled
    stack->cs = 0x08 | 0b00;
}

/* work with rings */
dword_t running_ring_insert_command(scheduler_running_ring_t *running_ring, dword_t command_number) {
    LOCK_MUTEX(&running_ring->mutex);

    // check if there is free space
    if(running_ring->number_of_commands >= MAX_NUMBER_OF_COMMANDS_IN_RUNNING_RING) {
        UNLOCK_MUTEX(&running_ring->mutex);
        return ERROR;
    }

    // insert command
    running_ring->commands[running_ring->number_of_commands] = command_number;
    running_ring->number_of_commands++;

    UNLOCK_MUTEX(&running_ring->mutex);
    return SUCCESS;
}

void running_ring_remove_command(scheduler_running_ring_t *running_ring, dword_t command_number) {
    LOCK_MUTEX(&running_ring->mutex);
    if(running_ring->number_of_commands == 0) {
        UNLOCK_MUTEX(&running_ring->mutex);
        return;
    }

    for(int i = 0; i < running_ring->number_of_commands; i++) {
        if(running_ring->commands[i] == command_number) {
            // remove command from array
            if (i < (running_ring->number_of_commands - 1)) {
                memmove(&running_ring->commands[i],
                        &running_ring->commands[i + 1],
                        (running_ring->number_of_commands - i - 1) * sizeof(dword_t));
            }

            running_ring->number_of_commands--;

            // set new running command
            if(running_ring->running_command >= running_ring->number_of_commands) {
                running_ring->running_command = 0;
            }
            UNLOCK_MUTEX(&running_ring->mutex);
            return;
        }
    }

    UNLOCK_MUTEX(&running_ring->mutex);
}

dword_t sleeping_ring_insert_command(dword_t command_number) {
    // check if there is free space
    scheduler_sleeping_ring_t *sleeping_ring = (scheduler_sleeping_ring_t *) P_MEM_SCHEDULER_SLEEPING_COMMANDS;
    LOCK_MUTEX(&sleeping_ring->mutex);
    if(sleeping_ring->number_of_commands >= MAX_NUMBER_OF_COMMANDS_IN_SLEEPING_RING) {
        UNLOCK_MUTEX(&sleeping_ring->mutex);
        return ERROR;
    }

    // insert command
    sleeping_ring->commands[sleeping_ring->number_of_commands] = command_number;
    sleeping_ring->number_of_commands++;

    UNLOCK_MUTEX(&sleeping_ring->mutex);
    return SUCCESS;
}

void sleeping_ring_remove_command(dword_t command_number) {
    scheduler_sleeping_ring_t *sleeping_ring = (scheduler_sleeping_ring_t *) P_MEM_SCHEDULER_SLEEPING_COMMANDS;
    LOCK_MUTEX(&sleeping_ring->mutex);
    if(sleeping_ring->number_of_commands == 0) {
        UNLOCK_MUTEX(&sleeping_ring->mutex);
        return;
    }

    for(dword_t i = 0; i < sleeping_ring->number_of_commands; i++) {
        if(sleeping_ring->commands[i] == command_number) {
            if(i < (sleeping_ring->number_of_commands - 1)) {
                memmove(&sleeping_ring->commands[i],
                        &sleeping_ring->commands[i + 1],
                        (sleeping_ring->number_of_commands - i - 1) * sizeof(dword_t));
            }

            sleeping_ring->number_of_commands--;
            UNLOCK_MUTEX(&sleeping_ring->mutex);
            return;
        }
    }

    UNLOCK_MUTEX(&sleeping_ring->mutex);
}

dword_t waiting_ring_insert_command(dword_t command_number) {
    // check if there is free space
    scheduler_waiting_ring_t *waiting_ring = (scheduler_waiting_ring_t *) P_MEM_SCHEDULER_WAITING_COMMANDS;
    LOCK_MUTEX(&waiting_ring->mutex);
    if(waiting_ring->number_of_commands >= MAX_NUMBER_OF_COMMANDS_IN_WAITING_RING) {
        UNLOCK_MUTEX(&waiting_ring->mutex);
        return ERROR;
    }

    // insert command
    waiting_ring->commands[waiting_ring->number_of_commands] = command_number;
    waiting_ring->number_of_commands++;

    UNLOCK_MUTEX(&waiting_ring->mutex);
    return SUCCESS;
}

void waiting_ring_remove_command(dword_t command_number) {
    scheduler_waiting_ring_t *waiting_ring = (scheduler_waiting_ring_t *) P_MEM_SCHEDULER_WAITING_COMMANDS;
    LOCK_MUTEX(&waiting_ring->mutex);
    if(waiting_ring->number_of_commands == 0) {
        UNLOCK_MUTEX(&waiting_ring->mutex);
        return;
    }

    for(dword_t i = 0; i < waiting_ring->number_of_commands; i++) {
        if(waiting_ring->commands[i] == command_number) {
            if(i < (waiting_ring->number_of_commands - 1)) {
                memmove(&waiting_ring->commands[i],
                        &waiting_ring->commands[i + 1],
                        (waiting_ring->number_of_commands - i - 1) * sizeof(dword_t));
            }

            waiting_ring->number_of_commands--;
            UNLOCK_MUTEX(&waiting_ring->mutex);
            return;
        }
    }

    UNLOCK_MUTEX(&waiting_ring->mutex);
}

dword_t insert_new_command(dword_t command_number) {
    scheduler_running_ring_t *running_ring = (scheduler_running_ring_t *) P_MEM_SCHEDULER_RUNNING_COMMANDS;
    if(running_ring->number_of_commands >= MAX_NUMBER_OF_COMMANDS_IN_RUNNING_RING) {
        return ERROR;
    }
    running_ring->commands[running_ring->number_of_commands] = command_number;
    running_ring->number_of_commands++;

    return SUCCESS;

    // TODO: add command right after actual command

    // TODO: support for multiple CPU core rings
}

void remove_command(dword_t command_number) {
    // TODO: support for multiple CPU core rings
    scheduler_running_ring_t *running_ring = (scheduler_running_ring_t *) P_MEM_SCHEDULER_RUNNING_COMMANDS;
    LOCK_MUTEX(&running_ring->mutex);
    for(int i = 0; i < running_ring->number_of_commands; i++) {
        if(running_ring->commands[i] == command_number) {
            // remove command from array TODO: do it by memmove
            for(int j = i; j < (running_ring->number_of_commands-1); j++) {
                running_ring->commands[j] = running_ring->commands[j+1];
            }
            running_ring->number_of_commands--;

            // set new running command
            if(running_ring->running_command >= running_ring->number_of_commands) {
                running_ring->running_command = 0;
            }

            // update kernel attributes
            if(kernel_attr->running_command == command_number) {
                command_t *running_command = command_get_ptr(running_ring->running_command);
                extern interrupt_stack_t *stack_of_interrupt;
                stack_of_interrupt = (interrupt_stack_t *) running_command->esp; // set value as if we went here from this command
            }
            kernel_attr->running_command = running_ring->running_command;
            UNLOCK_MUTEX(&running_ring->mutex);
            return;
        }
    }

    UNLOCK_MUTEX(&running_ring->mutex);
}

void move_command_to_sleeping_state(dword_t microseconds) {
    // update time for sleep
    command_t *running_command = command_get_ptr(kernel_attr->running_command);
    running_command->time_for_end_of_sleep = (kernel_attr->kernel_time + microseconds);

    // update esp
    extern interrupt_stack_t *stack_of_interrupt;
    running_command->esp = (dword_t) stack_of_interrupt;

    // move command to sleeping ring
    if(sleeping_ring_insert_command(kernel_attr->running_command) == ERROR) {
        kernel_panic("Can not insert command to sleeping ring");
    }

    // remove command from running ring
    remove_command(kernel_attr->running_command);
}

void move_command_to_waiting_state(dword_t signaling_command) {
    // check if signaling command still runs under this command
    if(is_command_running(signaling_command) == FALSE) {
        return;
    }

    // set values
    command_t *running_command = command_get_ptr(kernel_attr->running_command);
    running_command->time_of_actual_run = 0;
    running_command->signaling_command = signaling_command;

    // update esp
    extern interrupt_stack_t *stack_of_interrupt;
    running_command->esp = (dword_t) stack_of_interrupt;

    // move command to waiting ring
    if(waiting_ring_insert_command(kernel_attr->running_command) == ERROR) {
        kernel_panic("Can not insert command to waiting ring");
    }

    // remove command from running ring
    remove_command(kernel_attr->running_command);
}

void move_command_to_exit_state(dword_t command_number) {
    if(kernel_attr->running_command == command_number) {
        log("\n[WARNING] moving running command to exit state");
        SC_EXIT();
    }
    
    command_t *command = command_get_ptr(command_number);
    vm_map_page(P_MEM_COMMAND_KERNEL_STACK, (dword_t)get_pm_of_vm_page(command->vm_of_kernel_stack), VM_FLAGS_KERNEL_RW);
    interrupt_stack_t *stack = (interrupt_stack_t *) (P_MEM_COMMAND_KERNEL_STACK + PAGE_SIZE - sizeof(interrupt_stack_t));
    
    stack->ds = 0x10 | 0b00;
    stack->es = 0x10 | 0b00;
    stack->fs = 0x10 | 0b00;
    stack->gs = 0x10 | 0b00;

    stack->edi = 0;
    stack->esi = 0;
    stack->ebp = 0;
    stack->old_esp = ((dword_t)stack + 32); // skip pushad data
    stack->ebx = 0;
    stack->edx = 0;
    stack->ecx = 0;
    stack->eax = 0;

    extern void exit_command(void);
    stack->eip = (dword_t) &exit_command;
    stack->eflags = 0x00000202; // interrupts enabled
    stack->cs = 0x08 | 0b00;

    command->esp = (dword_t) (command->vm_of_kernel_stack + PAGE_SIZE - sizeof(interrupt_stack_t));
}

void switch_commands(void) {
    extern void change_pd_esp_and_jump_to_new_command(dword_t esp, dword_t page_directory);
    extern void change_esp_and_jump_to_new_command(dword_t esp);

    // check if there is any command running
    scheduler_running_ring_t *running_ring = (scheduler_running_ring_t *) P_MEM_SCHEDULER_RUNNING_COMMANDS;
    LOCK_MUTEX(&running_ring->mutex);
    if(running_ring->number_of_commands == 0) {
        // check if some sleeping command should wake up
        dword_t go_to_idle_state = TRUE;
        scheduler_sleeping_ring_t *sleeping_ring = (scheduler_sleeping_ring_t *) P_MEM_SCHEDULER_SLEEPING_COMMANDS;
        for(int i = 0; i < sleeping_ring->number_of_commands; i++) {
            command_t *sleeping_command = command_get_ptr(sleeping_ring->commands[i]);
            if(sleeping_command->time_for_end_of_sleep <= kernel_attr->kernel_time) { 
                go_to_idle_state = FALSE;       
                break;
            }
        }

        // go to idle command
        if(go_to_idle_state == TRUE) {
            is_interrupt = FALSE;
            UNLOCK_MUTEX(&running_ring->mutex);
            change_esp_and_jump_to_new_command(P_MEM_IDLE_COMMAND_STACK + PAGE_SIZE - sizeof(interrupt_stack_t)); // go to idle command
        }
    }
    else {
        // save esp
        extern interrupt_stack_t *stack_of_interrupt;
        command_t *running_command = command_get_ptr(kernel_attr->running_command);
        running_command->esp = (dword_t) stack_of_interrupt;

        // update time
        running_command->time_of_running += kernel_attr->time_of_new_tick; // how much time command runs from its creation
        running_command->time_of_actual_run += kernel_attr->time_of_new_tick; // how much time command runs continuously

        // check if actual command should still run
        if(running_command->time_of_actual_run < SCHEDULER_PERIOD) {
            is_interrupt = FALSE;
            UNLOCK_MUTEX(&running_ring->mutex);
            change_esp_and_jump_to_new_command(running_command->esp); // continue to run actual command because it did not run for all period
        }
        else {
            running_command->time_of_actual_run = 0; // reset time of running
        }
    }

    // check sleeping commands
    scheduler_sleeping_ring_t *sleeping_ring = (scheduler_sleeping_ring_t *) P_MEM_SCHEDULER_SLEEPING_COMMANDS;
    for(int i = 0; i < sleeping_ring->number_of_commands; i++) {
        command_t *sleeping_command = command_get_ptr(sleeping_ring->commands[i]);
        if(sleeping_command->time_for_end_of_sleep <= kernel_attr->kernel_time) {            
            // insert command to running ring
            if(insert_new_command(sleeping_ring->commands[i]) == ERROR) {
                break; // now there are too much commands running
            }

            // remove command from sleeping ring
            sleeping_ring_remove_command(sleeping_ring->commands[i]);
            i--; // we moved next command to actual entry, so move pointer backward
        }
    }

    // go to next running command
    running_ring->running_command++;
    if(running_ring->running_command == running_ring->number_of_commands) {
        running_ring->running_command = 0;
    }
    dword_t new_command_number = running_ring->commands[running_ring->running_command];
    UNLOCK_MUTEX(&running_ring->mutex);
    command_t *new_command = command_get_ptr(new_command_number);

    // debug log
    // log("\n[SCHEDULER] Switch command %d to command %d", kernel_attr->running_command, new_command_number);

    // update kernel attributes
    kernel_attr->running_command = new_command_number;

    // set command info
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    command_info->vm_of_communication_area = new_command->vm_of_communication_area;
    command_info->vm_of_command_list = new_command->vm_of_command_list;
    command_info->vm_of_vma_bitmap = new_command->vm_of_vma_bitmap;
    command_info->vm_of_vma_metadata = new_command->vm_of_vma_metadata;
    command_info->vm_of_user_stack = new_command->vm_of_user_stack;
    command_info->number_of_pages_in_user_stack = new_command->number_of_pages_in_user_stack;
    command_info->vm_of_kernel_stack = new_command->vm_of_kernel_stack;
    command_info->entity_number = new_command->entity;
    command_info->entity_attributes = entity_get_attr_ptr(new_command->entity);
    command_info->command_number = new_command_number;

    // set kernel stack address in TSS
    tss_t *tss = (tss_t *) P_MEM_TSS;
    tss->esp0 = (command_info->vm_of_kernel_stack + PAGE_SIZE);

    // jump to new command
    if(new_command->pm_page_of_page_directory != kernel_attr->pm_of_loaded_page_directory) {
        kernel_attr->pm_of_loaded_page_directory = new_command->pm_page_of_page_directory;
        is_interrupt = FALSE;
        change_pd_esp_and_jump_to_new_command(new_command->esp, new_command->pm_page_of_page_directory);
    }
    else {
        is_interrupt = FALSE;
        change_esp_and_jump_to_new_command(new_command->esp);
    }
}