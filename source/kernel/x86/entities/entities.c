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
#include <kernel/x86/cpu/commands.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/interrupt_controllers/main.h>
#include <kernel/x86/scheduler/main.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>

/* global variables */
command_info_t *command_info;

/* functions */
void initialize_entities(void) {
    // initialize stack of free entities numbers
    vm_alloc_page(P_MEM_ENTITIES_METADATA, VM_FLAGS_KERNEL_RW);
    entities_metadata_t *e_metadata = (entities_metadata_t *) P_MEM_ENTITIES_METADATA;
    e_metadata->lifo_stack_ptr = 1; // pointer to first available entry
    for(int entry = 1; entry < MAX_NUMBER_OF_ENTITES; entry++) {
        e_metadata->stack[entry] = entry;
    }

    // initialize stack of free commands numbers
    vm_alloc_page(P_MEM_COMMANDS_METADATA, VM_FLAGS_KERNEL_RW);
    commands_metadata_t *c_metadata = (commands_metadata_t *) P_MEM_COMMANDS_METADATA;
    c_metadata->lifo_stack_ptr = 1; // pointer to first available entry
    for(int entry = 1; entry < MAX_NUMBER_OF_COMMANDS; entry++) {
        c_metadata->stack[entry] = entry;
    }

    // initialize command info
    vm_alloc_page(P_MEM_COMMAND_INFO, VM_FLAGS_KERNEL_RW);
    command_info = (command_info_t *) P_MEM_COMMAND_INFO;

    // initialize floating stack
    vm_alloc_page(P_MEM_FLOATING_STACK, VM_FLAGS_KERNEL_RW);

    // create kernel entity and boot command
    create_kernel_entity_and_boot_command();
}

void create_kernel_entity_and_boot_command(void) {
    // kernel entity has number 0
    // boot command has number 0

    // allocate kernel entity page
    vm_alloc_page(P_MEM_ENTITIES + (0 * PAGE_SIZE), VM_FLAGS_KERNEL_RW);

    // set global variable kernel attributes pointer
    kernel_attr = entity_get_attr_ptr(0);

    // set kernel entity data
    entity_t *kernel_entity = entity_get_ptr(0);
    strcpy(kernel_entity->name, "BleskOS kernel");
    kernel_entity->type = E_TYPE_KERNEL_RING;
    kernel_entity->pm_page_of_page_directory = PM_KERNEL_PAGE_DIRECTORY;

    // create kernel entity vma bitmap
    vm_alloc_page(P_MEM_KERNEL_VMA_BITMAP, VM_FLAGS_KERNEL_RW);
    kernel_entity->pm_page_of_vm_bitmap = (dword_t) get_pm_of_vm_page(P_MEM_KERNEL_VMA_BITMAP);
    virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) P_MEM_KERNEL_VMA_BITMAP;
    vma_bitmap->bitmap[0] = 0x01; // first block is for command specific structures
    vma_bitmap->free_block_start = SIZE_OF_VM_BLOCK;
    vma_bitmap->free_block_size = (END_OF_DATA_VM - SIZE_OF_VM_BLOCK);

    // allocate boot command page
    vm_alloc_page(P_MEM_COMMANDS + (0 * PAGE_SIZE), VM_FLAGS_KERNEL_RW);

    // set boot command data
    command_t *command = command_get_ptr(0);
    command->commanding_command = 0;
    command->time_of_creation = 0;
    command->time_of_running = 0;
    command->time_of_actual_run = 0;
    command->time_for_end_of_sleep = 0;
    command->signaling_command = 0;
    command->vm_of_communication_area = 0x2000;

    vm_alloc_page(0x00000000, VM_FLAGS_KERNEL_RW);
    command->vm_of_command_list = 0x00000000;

    command->vm_of_vma_bitmap = P_MEM_KERNEL_VMA_BITMAP;

    for(int i = 0; i < 17; i++) {
        vm_alloc_page(0x00001000 + i*PAGE_SIZE, VM_FLAGS_KERNEL_RW);
    }
    command->vm_of_vma_metadata = 0x00001000;

    command->vm_of_kernel_stack = 0x0001F000;

    command->pm_page_of_page_directory = PM_KERNEL_PAGE_DIRECTORY;
    command->esp = 0;

    // set command info
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    command_info->vm_of_communication_area = command->vm_of_communication_area;
    command_info->vm_of_command_list = command->vm_of_command_list;
    command_info->vm_of_vma_bitmap = command->vm_of_vma_bitmap;
    command_info->vm_of_vma_metadata = command->vm_of_vma_metadata;
    command_info->vm_of_user_stack = 0;
    command_info->number_of_pages_in_user_stack = 0;
    command_info->vm_of_kernel_stack = command->vm_of_kernel_stack;
    command_info->entity_attributes = entity_get_attr_ptr(0);

    // set basic kernel attributes
    kernel_attr->running_command = 0;
    kernel_attr->pm_of_loaded_page_directory = PM_KERNEL_PAGE_DIRECTORY;
    kernel_attr->time_of_new_tick = 0;
    kernel_attr->kernel_time = 0;
}

entity_t *entity_get_ptr(word_t entity_number) {
    return (void *) (P_MEM_ENTITIES + (entity_number * PAGE_SIZE));
}

command_t *command_get_ptr(word_t command_number) {
    return (void *) (P_MEM_COMMANDS + (command_number * PAGE_SIZE));
}

void *entity_get_attr_ptr(word_t entity_number) {
    return (void *) (P_MEM_ENTITIES + (entity_number * PAGE_SIZE) + 1024);
}

word_t create_entity(byte_t *name, dword_t type) {
    // check if there is free entity
    entities_metadata_t *metadata = (entities_metadata_t *) P_MEM_ENTITIES_METADATA;
    LOCK_MUTEX(&metadata->mutex);
    if(metadata->lifo_stack_ptr >= MAX_NUMBER_OF_ENTITES) {
        UNLOCK_MUTEX(&metadata->mutex);
        return FALSE;
    }

    // pop number from stack
    word_t entity_number = metadata->stack[metadata->lifo_stack_ptr];
    metadata->lifo_stack_ptr++;
    UNLOCK_MUTEX(&metadata->mutex);

    // allocate entity page
    vm_alloc_page(P_MEM_ENTITIES + (entity_number * PAGE_SIZE), VM_FLAGS_KERNEL_RW);

    // set entity data
    entity_t *entity = entity_get_ptr(entity_number);
    strcpy(entity->name, name);
    entity->type = type;
    if(type == E_TYPE_KERNEL_RING) {
        // copy data from kernel entity
        entity_t *kernel_entity = entity_get_ptr(KERNEL_ENTITY);
        entity->pm_page_of_vm_bitmap = kernel_entity->pm_page_of_vm_bitmap;
        entity->pm_page_of_page_directory = kernel_entity->pm_page_of_page_directory;
    }
    else if(type == E_TYPE_USER_RING) {
        // create virtual memory bitmap
        vm_alloc_page(P_MEM_CE_VM_BITMAP, VM_FLAGS_KERNEL_RW);
        entity->pm_page_of_vm_bitmap = (dword_t) get_pm_of_vm_page(P_MEM_CE_VM_BITMAP);
        virtual_memory_allocator_bitmap_t *vma = (virtual_memory_allocator_bitmap_t *) P_MEM_CE_VM_BITMAP;
        vma->free_block_size = END_OF_DATA_VM;

        // create new page directory
        vm_alloc_page(P_MEM_CE_PAGE_DIRECTORY, VM_FLAGS_KERNEL_RW);
        entity->pm_page_of_page_directory = (dword_t) get_pm_of_vm_page(P_MEM_CE_PAGE_DIRECTORY);
        dword_t *page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
        dword_t *entity_page_directory = (dword_t *) P_MEM_CE_PAGE_DIRECTORY;
        for(int i = 1020; i < 1023; i++) {
            entity_page_directory[i] = page_directory[i];
        }
        entity_page_directory[1023] = (entity->pm_page_of_page_directory | VM_FLAGS_PAGE_DIRECTORY); // recursive mapping of page directory
    }

    // debug log
    // log("\n[E%d] Entity %s was created", entity_number, name);

    // return entity number
    return entity_number;
}

void destroy_entity(word_t entity_number) {
    lock_cpu();

    // check if we should destroy this entity
    entities_metadata_t *metadata = (entities_metadata_t *) P_MEM_ENTITIES_METADATA;
    if(metadata->lifo_stack_ptr == 0) {
        kernel_panic("\n[ERROR] Destroying entity with full stack");
        return;
    }

    // load entity page directory
    entity_t *entity = entity_get_ptr(entity_number);
    if(kernel_attr->pm_of_loaded_page_directory != entity->pm_page_of_page_directory) {
        write_cr3(entity->pm_page_of_page_directory);
        kernel_attr->pm_of_loaded_page_directory = entity->pm_page_of_page_directory;
    }

    // destroy all commands
    volatile scheduler_running_ring_t *running_ring = (scheduler_running_ring_t *) P_MEM_SCHEDULER_RUNNING_COMMANDS;
    for(int i = 0; i < running_ring->number_of_commands; i++) {
        command_t *command = command_get_ptr(running_ring->commands[i]);
        if(command->entity == entity_number) {
            move_command_to_exit_state(running_ring->commands[i]);
        }
    }

    // free all allocated pages in virtual memory
    if(entity->type == E_TYPE_USER_RING) {
        dword_t *page_directory_entry = (dword_t *) P_MEM_PAGE_DIRECTORY;
        for(int i = 0; i < 1020; i++) {
            if((*page_directory_entry & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
                dword_t *page_table_entry = (dword_t *) (P_MEM_PAGE_TABLE + (PAGE_SIZE * i));
                for(int i = 0; i < 1024; i++) {
                    if((*page_table_entry & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
                        pm_free_page((void *)(*page_table_entry & 0xFFFFF000));
                    }
                }
                pm_free_page((void *)(*page_directory_entry & 0xFFFFF000));
            }
        }

        // free page directory
        pm_free_page((void *)entity->pm_page_of_page_directory);
    }

    // free vma bitmap
    pm_free_page((void *)entity->pm_page_of_vm_bitmap);

    // free page of entity
    pm_free_page(get_pm_of_vm_page((dword_t)entity));
    vm_unmap_page((dword_t)entity);

    // push entity number to stack
    metadata->lifo_stack_ptr--;
    metadata->stack[metadata->lifo_stack_ptr] = entity_number;

    // debug log
    // log("\n[E] Entity %d was destroyed", entity_number);

    // check if we destroyed current entity, if yes, switch commands
    command_t *running_command = command_get_ptr(kernel_attr->running_command);
    if(kernel_attr->running_command == running_command->entity) {
        // TODO: check if we are in interrupt
        SC_SWITCH();
    }
    unlock_cpu();
}

dword_t create_command(word_t entity_number, void function(void), void *communication_area, dword_t size_of_communication_area, void finalizing_method(void), dword_t is_independent) {
    // check if there is free entity
    commands_metadata_t *metadata = (commands_metadata_t *) P_MEM_COMMANDS_METADATA;
    LOCK_MUTEX(&metadata->mutex);
    if(metadata->lifo_stack_ptr >= MAX_NUMBER_OF_COMMANDS) {
        UNLOCK_MUTEX(&metadata->mutex);
        return INVALID;
    }

    // pop number from stack
    word_t command_number = metadata->stack[metadata->lifo_stack_ptr];
    metadata->lifo_stack_ptr++;
    UNLOCK_MUTEX(&metadata->mutex);

    // allocate command page
    vm_alloc_page(P_MEM_COMMANDS + (command_number * PAGE_SIZE), VM_FLAGS_KERNEL_RW);

    // set basic command data
    command_t *command = command_get_ptr(command_number);
    command->entity = entity_number;
    command->time_of_creation = kernel_attr->kernel_time;
    command->time_of_running = 0;
    command->time_of_actual_run = 0;
    command->time_for_end_of_sleep = 0;
    command->signaling_command = 0;
    command->finalizing_method = (dword_t) finalizing_method;
    if(size_of_communication_area == 0) {
        communication_area = (void *) INVALID;
    }

    // set rest of command data according to entity type
    entity_t *entity = entity_get_ptr(entity_number);
    if(entity->type == E_TYPE_KERNEL_RING) {
        // set page directory of kernel
        entity_t *kernel_entity = entity_get_ptr(KERNEL_ENTITY);
        command->pm_page_of_page_directory = kernel_entity->pm_page_of_page_directory;

        // create communication area
        // TODO: be sure that we are allocating from kernel VMA
        if(kernel_attr->pm_of_loaded_page_directory == kernel_entity->pm_page_of_page_directory) {
            command->vm_of_communication_area = (dword_t)communication_area;
        }
        else {
            // TODO: allocate communication area in kernel page directory
        }

        // create all other command structures
        // TODO: be sure that we are allocating from kernel VMA
        virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) P_MEM_KERNEL_VMA_BITMAP;
        if(vma_bitmap->free_block_size < (PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE)) {
            kernel_panic("Can not allocate command descriptor for new command");
        }
        void *command_descriptor = (void *) vma_bitmap->free_block_start;
        dword_t command_descriptor_block = ((dword_t)command_descriptor >> 17);
        vma_bitmap->free_block_start += SIZE_OF_VM_BLOCK;
        vma_bitmap->free_block_size -= SIZE_OF_VM_BLOCK;
        vma_bitmap->bitmap[(command_descriptor_block >> 3)] |= (1 << (command_descriptor_block & 0x7));
        vm_map_block((dword_t)command_descriptor, VM_FLAGS_KERNEL_RW, PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE);
        memset(command_descriptor, 0, PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE);
        command->vm_of_command_list = ((dword_t)command_descriptor + 0);
        command->vm_of_vma_bitmap = P_MEM_KERNEL_VMA_BITMAP; // use kernel vma bitmap
        command->vm_of_vma_metadata = ((dword_t)command_descriptor + PAGE_SIZE);
        command->vm_of_kernel_stack = ((dword_t)command_descriptor + PAGE_SIZE + PAGE_SIZE*17);

        // set kernel stack
        command->esp = (command->vm_of_kernel_stack + PAGE_SIZE - sizeof(interrupt_stack_t));
        interrupt_stack_t *stack = (interrupt_stack_t *) command->esp;

        stack->ds = 0x10 | 0b00;
        stack->es = 0x10 | 0b00;
        stack->fs = 0x10 | 0b00;
        stack->gs = 0x10 | 0b00;

        stack->edi = 0;
        stack->esi = 0;
        stack->ebp = 0;
        stack->old_esp = (command->esp + 32); // skip pushad data
        stack->ebx = 0;
        stack->edx = 0;
        stack->ecx = 0;
        stack->eax = 0;

        stack->eip = (dword_t) function;
        stack->eflags = 0x00000202; // interrupts enabled
        stack->cs = 0x08 | 0b00;

        extern void exit_command(void);
        stack->pointer_to_exit_method = (dword_t)&exit_command;
    }
    else if(entity->type == E_TYPE_USER_RING) {
        // create page directory of command (CPD)
        vm_alloc_page(P_MEM_CE_PAGE_DIRECTORY, VM_FLAGS_KERNEL_RW); // allocate new page directory
        command->pm_page_of_page_directory = (dword_t) get_pm_of_vm_page(P_MEM_CE_PAGE_DIRECTORY); // save pm of page directory
        dword_t *new_page_directory = (dword_t *) P_MEM_CE_PAGE_DIRECTORY;
        dword_t *old_page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
        for(int i = 1020; i < 1023; i++) {
            new_page_directory[i] = old_page_directory[i];
        }
        new_page_directory[1023] = (command->pm_page_of_page_directory | VM_FLAGS_PAGE_DIRECTORY); // recursive mapping of page directory
        
        // load all needed page directories
        dword_t *page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
        page_directory[1018] = (entity->pm_page_of_page_directory | VM_FLAGS_PAGE_TABLE); // entity page directory
        page_directory[1019] = (command->pm_page_of_page_directory | VM_FLAGS_PAGE_TABLE); // command page directory
        write_cr3(kernel_attr->pm_of_loaded_page_directory);

        // copy data from entity (code + code data)
        dword_t *command_page_directory = (dword_t *) P_MEM_SECOND_PAGE_DIRECTORY;
        dword_t *entity_page_directory = (dword_t *) P_MEM_THIRD_PAGE_DIRECTORY;
        for(int i = 0; i < 1018; i++) {
            if((entity_page_directory[i] & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
                log("\nEntity PD %d", i);

                // allocate page table
                command_page_directory[i] = ((dword_t)pm_alloc_page() | VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_USER | VM_FLAG_WRITE_BACK);

                // copy page table data
                dword_t *command_page_table = (dword_t *) (P_MEM_SECOND_PAGE_TABLE + i*PAGE_SIZE);
                dword_t *entity_page_table = (dword_t *) (P_MEM_THIRD_PAGE_TABLE + i*PAGE_SIZE);
                for(int j = 0; j < 1024; j++) {
                    if((entity_page_table[j] & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
                        log("\nCopy page PD %d PT %d ", i, j);
                        if((entity_page_table[j] & VM_FLAG_READ_WRITE) == VM_FLAG_READ_WRITE) {
                            command_page_table[j] = ((entity_page_table[j] & 0xFFFFF000) | VM_FLAG_TYPE_COPY_ON_WRITE | VM_FLAGS_USER_RO | VM_FLAG_PRESENT);
                        }
                        else {
                            command_page_table[j] = entity_page_table[j];
                        }
                    }
                }
            }
        }

        // allocate command descriptor structures
        dword_t size_of_command_descriptor = (PAGE_SIZE + PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE + PAGE_SIZE); // command list + vma bitmap + vma metadata + user stack + kernel stack
        void *vm_command_descriptor = kcalloc(size_of_command_descriptor);
        virtual_memory_allocator_bitmap_t *vm_vma_bitmap = (virtual_memory_allocator_bitmap_t *) ((dword_t)vm_command_descriptor + PAGE_SIZE);
        vm_map_page(P_MEM_CE_VM_BITMAP, entity->pm_page_of_vm_bitmap, VM_FLAGS_KERNEL_RO);
        memcpy((void *)vm_vma_bitmap, (void *)P_MEM_CE_VM_BITMAP, PAGE_SIZE); // copy vma bitmap from entity - it has mapped code and data

        // allocate space in command page directory
        void *command_descriptor = alloc_from_bitmap(command_page_directory, vm_vma_bitmap, sizeof(size_of_command_descriptor));
        if((dword_t)command_descriptor == INVALID) {
            kernel_panic("Can not allocate command descriptor for new command");
        }

        // copy command descriptor structures
        dword_t *page_table = (dword_t *) (P_MEM_PAGE_TABLE + ((dword_t)vm_command_descriptor >> 12)*4);
        dword_t *command_page_table = (dword_t *) (P_MEM_SECOND_PAGE_TABLE + ((dword_t)command_descriptor >> 12)*4);
        for(int i = 0; i < (size_of_command_descriptor / PAGE_SIZE); i++) {
            command_page_table[i] = page_table[i];
        }
        command->vm_of_command_list = ((dword_t)command_descriptor + 0);
        command->vm_of_vma_bitmap = ((dword_t)command_descriptor + PAGE_SIZE);
        command->vm_of_vma_metadata = ((dword_t)command_descriptor + PAGE_SIZE + PAGE_SIZE);
        command->vm_of_user_stack = ((dword_t)command_descriptor + PAGE_SIZE + PAGE_SIZE + PAGE_SIZE*17);
        command->number_of_pages_in_user_stack = 1;
        command->vm_of_kernel_stack = ((dword_t)command_descriptor + PAGE_SIZE + PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE);

        // set communication area
        if((dword_t)communication_area != INVALID) {
            // allocate space for communication area
            void *command_communication_area = alloc_from_bitmap(command_page_directory, vm_vma_bitmap, sizeof(size_of_communication_area));
            if((dword_t)command_communication_area == INVALID) {
                kernel_panic("Can not allocate communication area for new command");
            }

            // share pages of communication area
            page_table = (dword_t *) (P_MEM_PAGE_TABLE + ((dword_t)communication_area >> 12)*4);
            command_page_table = (dword_t *) (P_MEM_SECOND_PAGE_TABLE + ((dword_t)command_communication_area >> 12)*4);
            dword_t size_of_communication_area_in_pages = ((size_of_communication_area + PAGE_SIZE - 1) / PAGE_SIZE);
            for(int i = 0; i < size_of_communication_area_in_pages; i++) {
                command_page_table[i] = ((page_table[i] & 0xFFFFF000) | VM_FLAGS_USER_RW | VM_FLAG_PRESENT);
            }
        }

        // set kernel stack
        command->esp = (command->vm_of_kernel_stack + PAGE_SIZE - sizeof(interrupt_stack_t));
        interrupt_stack_t *stack = (interrupt_stack_t *) ((dword_t)vm_command_descriptor + PAGE_SIZE + PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE + PAGE_SIZE - sizeof(interrupt_stack_t));

        stack->ds = 0x20 | 0b11;
        stack->es = 0x20 | 0b11;
        stack->fs = 0x20 | 0b11;
        stack->gs = 0x20 | 0b11;

        stack->edi = 0;
        stack->esi = 0;
        stack->ebp = 0;
        stack->old_esp = (command->esp + 32); // skip pushad data
        stack->ebx = 0;
        stack->edx = 0;
        stack->ecx = 0;
        stack->eax = 0;

        stack->eip = (dword_t) function;
        stack->eflags = 0x00000202; // interrupts enabled
        stack->cs = 0x18 | 0b11;
        stack->esp = (command->vm_of_user_stack + command->number_of_pages_in_user_stack*PAGE_SIZE);
        stack->ss = 0x20 | 0b11;

        // remove command descriptor from current virtual memory
        unmap(vm_command_descriptor);
    }

    // save command to command list of running command
    if(is_independent == TRUE) {
        command->commanding_command = INVALID;
    }
    else {
        command->commanding_command = kernel_attr->running_command;
        command_info_t *running_command_info = (command_info_t *) P_MEM_COMMAND_INFO;
        command_list_t *running_command_list = (command_list_t *) running_command_info->vm_of_command_list;
        if(running_command_list->number_of_commands >= MAX_NUMBER_OF_COMMANDS) {
            kernel_panic("Too much running commands in command list");
        }
        running_command_list->commands[running_command_list->number_of_commands].number_of_command = command_number;
        running_command_list->commands[running_command_list->number_of_commands].vm_of_communication_area = (dword_t)communication_area;
        running_command_list->number_of_commands++;
    }

    // insert to scheduler
    if(insert_new_command(command_number) == ERROR) {
        kernel_panic("Can not insert command to running ring");
    }

    // debug log
    // entity_t *e = entity_get_ptr(entity_number);
    // log("\n[COM%d] Created command for entity %d %s jump at 0x%x", command_number, entity_number, e->name, function);

    return command_number;
}

/* functions for creating commands */
// ca = communication area
// fin = finalizing method
dword_t create_command_with_ca_with_fin(word_t entity_number, void function(void), dword_t size_of_communication_area, void *communication_area, void finalizing_method(void)) {
    return create_command(entity_number, function, communication_area, size_of_communication_area, finalizing_method, FALSE);
}
dword_t create_command_with_ca_without_fin(word_t entity_number, void function(void), dword_t size_of_communication_area, void *communication_area) {
    return create_command(entity_number, function, communication_area, size_of_communication_area, (void (*)()) INVALID, FALSE);
}
dword_t create_command_without_ca_with_fin(word_t entity_number, void function(void), void finalizing_method(void)) {
    return create_command(entity_number, function, (void *)INVALID, 0, finalizing_method, FALSE);
}
dword_t create_command_without_ca_without_fin(word_t entity_number, void function(void)) {
    return create_command(entity_number, function, (void *)INVALID, 0, (void (*)()) INVALID, FALSE);
}
dword_t create_independent_command_with_fin(word_t entity_number, void function(void), void finalizing_method(void)) {
    return create_command(entity_number, function, (void *)INVALID, 0, finalizing_method, TRUE);
}
dword_t create_independent_command_without_fin(word_t entity_number, void function(void)) {
    return create_command(entity_number, function, (void *)INVALID, 0, (void (*)()) INVALID, TRUE);
}

// this function is called from jump_to_kill_running_command which enables floating stack
void kill_running_command(void) {
    // save command number
    dword_t command_number = kernel_attr->running_command;

    // remove from scheduler
    remove_command(command_number);

    // remove from above command list
    command_t *running_command = command_get_ptr(command_number);
    if(running_command->commanding_command != INVALID) {
        command_t *commanding_command = command_get_ptr(running_command->commanding_command);
        vm_map_page(P_MEM_ABOVE_COMMAND_LIST, (dword_t)get_pm_of_vm_page(commanding_command->vm_of_command_list), VM_FLAGS_KERNEL_RW);
        command_list_t *command_list = (command_list_t *) P_MEM_ABOVE_COMMAND_LIST;
        for(int i = 0; i < command_list->number_of_commands; i++) {
            if(command_list->commands[i].number_of_command == command_number) {
                for(int j = (i + 1); j < command_list->number_of_commands; i++, j++) {
                    command_list->commands[i].number_of_command = command_list->commands[j].number_of_command;
                    command_list->commands[i].vm_of_communication_area = command_list->commands[j].vm_of_communication_area;
                }
                command_list->number_of_commands--;
            }
        }
    }

    // free command memory
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    virtual_memory_allocator_blocks_metadata_t *vma_metadata = (virtual_memory_allocator_blocks_metadata_t *) command_info->vm_of_vma_metadata;
    // for(int i = 0; i < NUMBER_OF_VM_BLOCKS; i++) {
    //     if(vma_metadata->allocated_blocks[i] != 0) {
    //         free((void *)(i*SIZE_OF_VM_BLOCK));
    //     }
    // }

    // free command structures
    // TODO: add destroying of user commands
    virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) P_MEM_KERNEL_VMA_BITMAP;
    dword_t command_structure_block = ((dword_t)command_info->vm_of_command_list >> 17);
    vma_bitmap->bitmap[(command_structure_block >> 3)] &= ~(1 << (command_structure_block & 0x7));
    dword_t command_descriptor_block = command_info->vm_of_command_list;
    for(int i = 0; i < (PAGE_SIZE + PAGE_SIZE*17 + PAGE_SIZE); i += PAGE_SIZE) {
        pd_unmap_page(command_descriptor_block);
        command_descriptor_block += PAGE_SIZE;
    }

    // free command page
    pm_free_page(get_pm_of_vm_page(P_MEM_COMMANDS + PAGE_SIZE*command_number));

    // flush TLB
    write_cr3(kernel_attr->pm_of_loaded_page_directory);

    // push command number to stack
    commands_metadata_t *metadata = (commands_metadata_t *) P_MEM_COMMANDS_METADATA;
    if(metadata->lifo_stack_ptr == 0) {
        kernel_panic("Freeing command to free stack");
    }
    else {
        metadata->lifo_stack_ptr--;
        metadata->stack[metadata->lifo_stack_ptr] = command_number;
    }

    // go to another command because actual command do not exist
    switch_commands();
}

void destroy_running_command(void) {
    // destroy all commands under this command
    command_t *command = command_get_ptr(kernel_attr->running_command);
    command_list_t *command_list = (command_list_t *) command->vm_of_command_list;
    for(int i = 0; i < command_list->number_of_commands; i++) {
        move_command_to_exit_state(command_list->commands[i].number_of_command);
        command_t *exit_command = command_get_ptr(command_list->commands[i].number_of_command);
        exit_command->commanding_command = INVALID;
    }

    // check if we should run finalizing method
    if(command->finalizing_method != INVALID) {
        interrupt_stack_t *stack = (interrupt_stack_t *) (command->vm_of_kernel_stack + PAGE_SIZE - sizeof(interrupt_stack_t));

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

        stack->eip = (dword_t) command->finalizing_method;
        stack->eflags = 0x00000202; // interrupts enabled
        stack->cs = 0x08 | 0b00;

        extern void exit_command(void);
        stack->pointer_to_exit_method = (dword_t)&exit_command;

        command->finalizing_method = INVALID;
        command->esp = (command->vm_of_kernel_stack + PAGE_SIZE - sizeof(interrupt_stack_t));
        extern interrupt_stack_t *stack_of_interrupt;
        stack_of_interrupt = (interrupt_stack_t *) command->esp;
        switch_commands();
    }

    // check commands waiting for this command to exit
    scheduler_waiting_ring_t *waiting_ring = (scheduler_waiting_ring_t *) P_MEM_SCHEDULER_WAITING_COMMANDS;
    for(int i = 0; i < waiting_ring->number_of_commands; i++) {
        command = command_get_ptr(waiting_ring->commands[i]);
        if(command->signaling_command == kernel_attr->running_command) {
            // insert command to running ring
            if(insert_new_command(waiting_ring->commands[i]) == ERROR) {
                kernel_panic("Can not insert waiting command to running commands");
            }

            // remove command from waiting ring
            waiting_ring_remove_command(waiting_ring->commands[i]);
            i--; // we moved next command to actual entry, so move pointer backward
        }
    }

    // kill command
    extern void jump_to_kill_running_command(void);
    jump_to_kill_running_command();
}

dword_t is_command_running(dword_t command_number) {
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    command_list_t *command_list = (command_list_t *) command_info->vm_of_command_list;

    for(int i = 0; i < command_list->number_of_commands; i++) {
        if((dword_t)command_list->commands[i].number_of_command == command_number) {
            return TRUE;
        }
    }

    return FALSE;
}