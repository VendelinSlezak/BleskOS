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
#include <kernel/libc/string.h>
#include <kernel/kernel.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/timers/main.h>
#include <kernel/hardware/groups/human_input/human_input.h>
#include <kernel/software/virtual_hardware/screen.h>
#include <kernel/software/virtual_hardware/log.h>
#include <kernel/software/virtual_hardware/timer.h>
#include <kernel/software/virtual_hardware/human_input.h>

/* functions */
void initialize_syscalls(void) {
    register_interrupt_handler(0xD0, doorbell_interrupt_handler);
}

void *return_validated_pointer(void *ptr, uint32_t size) {
    if(ptr < (void *)PAGE_SIZE  || (ptr + size) > (void *)VM_KERNEL_SPACE_START || (ptr + size) < ptr) {
        return NULL;
    }
    else {
        return ptr;
    }
}

void doorbell_interrupt_handler(interrupt_stack_t *stack) {
    // log("\nDoorbell interrupt received on CPU%d with EAX: %x", get_current_logical_processor_index(), stack->eax);

    switch(stack->eax) {
        case DEMAND_TYPE_CREATE_THREAD: {
            void *entry_point = return_validated_pointer((void *) stack->ebx, PAGE_SIZE);
            void *stack_pointer = return_validated_pointer((void *) stack->ecx, 4);
            uint32_t delete_signal_handler = NULL;
            if(stack->edx != 0) {
                delete_signal_handler = (uint32_t) return_validated_pointer((void *) stack->edx, PAGE_SIZE);
            }
            if(entry_point != NULL && stack_pointer != NULL) {
                stack->eax = create_user_thread(get_current_logical_processor_struct()->current_program, entry_point, stack_pointer, delete_signal_handler);
            }
            else {
                stack->eax = 0;
            }
            break;
        }
        case DEMAND_TYPE_SPAWN_THREAD: {
            void *entry_point = return_validated_pointer((void *) stack->ebx, PAGE_SIZE);
            uint32_t delete_signal_handler = NULL;
            if(stack->ecx != 0) {
                delete_signal_handler = (uint32_t) return_validated_pointer((void *) stack->ecx, PAGE_SIZE);
            }
            if(entry_point != NULL) {
                stack->eax = spawn_user_thread(get_current_logical_processor_struct()->current_program, entry_point, delete_signal_handler);
            }
            else {
                stack->eax = 0;
            }
            break;
        }
        case DEMAND_TYPE_SWITCH_THREADS: {
            scheduler_interrupt(stack);
            break;
        }
        case DEMAND_TYPE_SLEEP_FOR_THREAD: {
            uint32_t microseconds = stack->ebx;
            if(microseconds == 0) {
                break;
            }
            else if(microseconds > 60000000) {
                microseconds = 60000000; // maximum sleep time is 60000000 microseconds (60 seconds = 1 minute)
            }
            sleep_for_thread(stack, microseconds);
            break;
        }
        case DEMAND_TYPE_CLOSE_THREAD: {
            close_current_thread_interrupt(stack);
            break;
        }
        case DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE: {
            if(return_validated_pointer((void *) stack->ebx, stack->ecx) == NULL) {
                break;
            }
            // log("\nMapping from %x to %x", stack->ebx,(stack->ebx + stack->ecx + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
            stack->eax = map_physical_pages_to_userspace(stack->ebx, stack->ecx);
            break;
        }
        case DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE: {
            if(return_validated_pointer((void *) stack->ebx, stack->ecx) == NULL) {
                break;
            }
            // log("\nUnmapping from %x to %x", stack->ebx, (stack->ebx + stack->ecx + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
            stack->eax = unmap_physical_pages_from_userspace(stack->ebx, stack->ecx);
            break;
        }
        case DEMAND_TYPE_SEND_DOORBELL_TO_VIRTUAL_DEVICE: {
            switch(stack->ebx) {
                case VIRTUAL_HARDWARE_SCREEN_ID:
                    // log("\n[SYSCALL] Doorbell sended to screen virtual device");
                    vh_screen_doorbell(stack->ecx);
                    stack->eax = SUCCESS;
                    break;
                case VIRTUAL_HARDWARE_LOG_ID:
                    // log("\n[SYSCALL] Doorbell sended to log virtual device");
                    vh_log_doorbell(stack->ecx);
                    stack->eax = SUCCESS;
                    break;
                case VIRTUAL_HARDWARE_TIMER_ID:
                    // log("\n[SYSCALL] Doorbell sended to timer virtual device");
                    vh_timer_doorbell(stack->ecx);
                    stack->eax = SUCCESS;
                    break;
                case VIRTUAL_HARDWARE_HUMAN_INPUT_ID:
                    // log("\n[SYSCALL] Doorbell sended to human input virtual device");
                    vh_human_input_doorbell(stack->ecx);
                    stack->eax = SUCCESS;
                    break;
                default:
                    log("\n[SYSCALL] Doorbell sended to unknown virtual device %d", stack->ebx);
                    stack->eax = ERROR;
                    break;
            }
            break;
        }
    }
}