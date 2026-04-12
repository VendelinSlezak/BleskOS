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
#include <libc/string.h>
#include <kernel/kernel.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/main.h>
#include <kernel/timers/main.h>
#include <kernel/hardware/subsystems/windows/windows.h>
#include <kernel/hardware/groups/human_input/human_input.h>

/* functions */
void initialize_syscalls(void) {
    register_interrupt_handler(0xD0, doorbell_interrupt_handler);
}

void *return_validated_pointer(void *ptr, uint32_t size) {
    if(ptr < (void *)PAGE_SIZE  || (ptr + size) > (void *)user_space_allocation_end || (ptr + size) < ptr) {
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
            void *entry_point = return_validated_pointer((void *) stack->ebx, 4);
            void *stack_pointer = return_validated_pointer((void *) stack->ecx, 4);
            uint8_t *end_of_thread_signal = return_validated_pointer((void *) stack->edx, 1);
            if(entry_point != NULL && stack_pointer != NULL && end_of_thread_signal != NULL) {
                create_user_thread(get_current_logical_processor_struct()->current_process, entry_point, stack_pointer, end_of_thread_signal);
            }
            break;
        }
        case DEMAND_TYPE_RESPAWN_PROCESS: {
            // not implemented yet
            break;
        }
        case DEMAND_TYPE_SEND_FILE_TO_APPLICATION: {
            // not implemented yet
            break;
        }
        case DEMAND_TYPE_SWITCH_THREADS: {
            scheduler_interrupt(stack);
            break;
        }
        case DEMAND_TYPE_SLEEP_FOR_THREAD: {
            uint32_t milliseconds = stack->ebx;
            if(milliseconds == 0) {
                break; // minimum sleep time is 1 ms
            }
            else if(milliseconds > 36000000) {
                stack->ebx = 36000000; // maximum sleep time is 36000000 ms (3600 seconds = 1 hour)
            }
            sleep_for_thread(stack, stack->ebx);
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
            map_physical_pages_to_userspace(stack->ebx, stack->ecx);
            break;
        }
        case DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE: {
            if(return_validated_pointer((void *) stack->ebx, stack->ecx) == NULL) {
                break;
            }
            unmap_physical_pages_from_userspace(stack->ebx, stack->ecx);
            break;
        }
        case DEMAND_TYPE_GET_LIST_OF_DEVICES: {
            void *hardware_list_in_userspace = return_validated_pointer((void *) stack->ebx, stack->ecx);
            if(hardware_list_in_userspace != NULL) {
                uint32_t size = stack->ecx;
                if(size > (sizeof(hardware_list_t) + (sizeof(hardware_list_entry_t) * MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST))) {
                    size = sizeof(hardware_list_t) + (sizeof(hardware_list_entry_t) * MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST);
                }
                memcpy(hardware_list_in_userspace, hardware_list, size);
            }
            break;
        }
        case DEMAND_TYPE_SEND_COMMAND_TO_DEVICE: {
            if(return_validated_pointer((void *) stack->ebx, sizeof(hardware_list_entry_t)) == NULL) {
                break;
            }
            hardware_list_entry_t *device = (hardware_list_entry_t *) stack->ebx;
            switch(device->type) {
                case HARDWARE_TYPE_TIMER: {
                    timer_group_process_userspace_command(device->id, (uint64_t *) stack->ecx);
                    break;
                }
                case HARDWARE_TYPE_LOGGER: {
                    logging_group_process_userspace_command(device->id, (logging_device_command_t *) stack->ecx);
                    break;
                }
                case HARDWARE_TYPE_WINDOW: {
                    window_subsystem_process_userspace_command(device->id, (windows_subsystem_command_t *) stack->ecx);
                    break;
                }
                case HARDWARE_TYPE_HUMAN_INPUT_DEVICE: {
                    human_input_group_process_userspace_command(device->id, (human_input_group_command_t *) stack->ecx);
                    break;
                }
            }
            break;
        }
    }
}