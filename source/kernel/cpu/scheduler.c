/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// TODO: add priority switch to interactive thread
// TODO: add waking up sleeping threads immediately

/* includes */
#include <kernel/kernel.h>
#include <libc/string.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/memory/physical_memory.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/commands.h>
#include <kernel/cpu/mutex.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/cpu/lapic.h>
#include <kernel/timers/main.h>
#include <kernel/timers/lapic.h>
#include <kernel/software/elf_loader.h>
#include <kernel/software/syscall.h>
#include <kernel/software/spawning_template.h>

/* local variables */
kernel_thread_list_t *kernel_threads = NULL;
program_t *programs = NULL;

/* functions */
void initialize_scheduler(void) {
    // create kernel thread list
    kernel_threads = kalloc(sizeof(kernel_thread_list_t) * number_of_logical_processors);

    // create main kernel thread
    kernel_thread_t *kernel_main_thread = kalloc(sizeof(kernel_thread_t));
    kernel_main_thread->next = NULL;
    kernel_main_thread->id = get_unique_kernel_tid(get_current_logical_processor_index());
    kernel_main_thread->page_directory_physical_address = NULL; // this thread is in all page directories
    kernel_main_thread->kernel_stack_pointer = 0; // stack pointer will be set when switching to this thread
    kernel_main_thread->kill_me = false;
    kernel_main_thread->sleeping = false;
    kernel_main_thread->time_end_of_sleep = NULL;

    // add main kernel thread to kernel thread list
    kernel_threads[get_current_logical_processor_index()].number_of_running_threads = 1;
    kernel_threads[get_current_logical_processor_index()].threads = kernel_main_thread;
    kernel_threads[get_current_logical_processor_index()].current_thread = kernel_main_thread;

    // update logical processor structures
    for(uint32_t i = 0; i < number_of_logical_processors; i++) {
        logical_processor_structs_ptr[i].scheduler_state = (logical_processors[i].hardware_id == bootstrap_processor_id) ? SCHEDULER_STATE_KERNEL : SCHEDULER_STATE_IDLE;
        logical_processor_structs_ptr[i].current_kernel_thread = NULL;
        logical_processor_structs_ptr[i].current_program = NULL;
        logical_processor_structs_ptr[i].current_user_thread = NULL;
    }
    logical_processor_structs_ptr[get_current_logical_processor_index()].current_kernel_thread = kernel_main_thread;

    // register scheduler interrupt handler
    register_interrupt_handler(INTERRUPT_SLEEP_FOR_THREAD, sleep_current_thread_interrupt);
    register_interrupt_handler(INTERRUPT_PREEMPETIVE_SCHEDULING, preemptive_scheduling_interrupt);
    register_interrupt_handler(INTERRUPT_VECTOR_EXIT_THREAD, close_current_thread_interrupt);
    register_interrupt_handler(INTERRUPT_VECTOR_SCHEDULER, scheduler_interrupt);
}

uint32_t get_number_of_running_programs_on_processor(uint32_t logical_processor_index) {
    uint32_t num = 0;
    program_t *program = programs;
    while(program != NULL) {
        if(program->thread_list_on_logical_processor[logical_processor_index].number_of_running_threads > 0) {
            num++;
        }
        program = program->next;
    }
    return num;
}

/* create processes and threads */
void create_user_process_from_spawning_template(spawning_template_t *template) {
    // load virtual space of template
    uint32_t original_page_directory = read_cr3();
    load_page_directory(template->page_directory);

    // create new page directory
    LOCK_MUTEX(&creating_page_directory_mutex);
    uint32_t new_page_directory = (uint32_t) pm_alloc_page();
    vm_map_page(P_MEM_NEW_PAGE_DIRECTORY, new_page_directory, VM_KERNEL);
    invlpg(P_MEM_NEW_PAGE_DIRECTORY);
    uint32_t *template_page_directory = (uint32_t *) P_MEM_PAGE_DIRECTORY;
    uint32_t *page_directory = (uint32_t *) P_MEM_NEW_PAGE_DIRECTORY;
    for(int i = 0; i < (1024 - number_of_shared_page_tables); i++) {
        if((template_page_directory[i] & VM_FLAG_PRESENT) != 0) {
            // copy pages into new page tables
            uint32_t new_page_table = (uint32_t) pm_alloc_page();
            page_directory[i] = (new_page_table | VM_PAGE_TABLE);
            vm_map_page(P_MEM_NEW_PAGE_TABLE, new_page_table, VM_KERNEL);
            invlpg(P_MEM_NEW_PAGE_TABLE);
            memcpy((void *)P_MEM_NEW_PAGE_TABLE, (void *) (P_MEM_PAGE_TABLE + (i * PAGE_SIZE)), PAGE_SIZE);
        }
    }
    for(int i = (1024 - number_of_shared_page_tables); i < 1023; i++) {
        page_directory[i] = template_page_directory[i]; // copy kernel page tables
    }
    page_directory[1023] = (new_page_directory | VM_KERNEL); // map page directory to itself
    UNLOCK_MUTEX(&creating_page_directory_mutex);

    // find processor where program will run fastest
    uint32_t target_logical_processor_index = 0;
    uint32_t target_logical_processor_num_of_running_programs = get_number_of_running_programs_on_processor(0);
    for(int i = 1; i < number_of_logical_processors; i++) {
        uint32_t number_of_running_programs_on_logical_processor = get_number_of_running_programs_on_processor(i);
        if(number_of_running_programs_on_logical_processor < target_logical_processor_num_of_running_programs) {
            target_logical_processor_index = i;
            target_logical_processor_num_of_running_programs = number_of_running_programs_on_logical_processor;
        }
    }

    // create new program
    program_t *program = kalloc(sizeof(program_t) + (sizeof(user_thread_list_t) * number_of_logical_processors));
    program->template = template;
    program->window = NULL;
    program->page_directory_for_human_input_event_stack = NULL;
    program->human_input_event_stack = NULL;

    // create new user thread
    user_thread_t *user_main_thread = kalloc(sizeof(user_thread_t));
    user_main_thread->next = NULL;
    user_main_thread->page_directory_physical_address = new_page_directory;
    user_main_thread->number_of_threads_in_page_directory = kalloc(sizeof(uint32_t));
    *user_main_thread->number_of_threads_in_page_directory = 1;
    user_main_thread->creation_thread_id = 0; // this is main thread so it is not connected to any other thread
    user_main_thread->id = get_unique_user_program_tid(target_logical_processor_index, program);
    user_main_thread->kernel_stack = kalloc(KERNEL_STACK_SIZE);
    interrupt_stack_t *kernel_stack_pointer = (interrupt_stack_t *) ((uint32_t)user_main_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(interrupt_stack_t));
    kernel_stack_pointer->eip = template->entry_point;
    kernel_stack_pointer->cs = 0x1B; // user code segment with RPL 3
    kernel_stack_pointer->eflags = 0x202; // interrupt enable flag
    kernel_stack_pointer->ds = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->es = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->fs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->gs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->ss = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->user_esp = (uint32_t) template->user_stack;
    user_main_thread->kernel_stack_pointer = (uint32_t) kernel_stack_pointer;
    user_main_thread->delete_me = false;
    user_main_thread->delete_signal_running = false;
    user_main_thread->kill_me = false;
    user_main_thread->sleeping = false;
    user_main_thread->delete_signal_handler_address = NULL;
    user_main_thread->time_end_of_sleep = NULL;

    // update stats
    program->number_of_threads = 1;
    program->thread_list_on_logical_processor[target_logical_processor_index].number_of_running_threads = 1;
    program->thread_list_on_logical_processor[target_logical_processor_index].threads = user_main_thread;
    program->thread_list_on_logical_processor[target_logical_processor_index].current_thread = user_main_thread;

    // add program to program list
    program->next = programs;
    while (
        !__atomic_compare_exchange_n(
            &programs,
            &program->next,
            program,
            false,
            __ATOMIC_RELEASE,
            __ATOMIC_RELAXED
        )
    ) {}

    // move back to original page directory
    load_page_directory(original_page_directory);

    // signal the logical processor to run the new process
    lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
}

uint32_t get_unique_kernel_tid(uint32_t target_logical_processor_index) {
    for(int i = 0; i < 32; i++) {
        uint32_t mask = (1 << i);
        if((kernel_threads[target_logical_processor_index].tid_bitmap & mask) == 0) {
            kernel_threads[target_logical_processor_index].tid_bitmap |= mask;
            return (target_logical_processor_index * 32 + i + 1); // TID 0 is reserved
        }
    }

    return 0; // no available TIDs
}

uint32_t get_unique_user_program_tid(uint32_t target_logical_processor_index, program_t *program) {
    uint32_t tid_bitmap = program->thread_list_on_logical_processor[target_logical_processor_index].tid_bitmap;
    for(int i = 0; i < 32; i++) {
        uint32_t mask = (1 << i);
        if((tid_bitmap & mask) == 0) {
            program->thread_list_on_logical_processor[target_logical_processor_index].tid_bitmap |= mask;
            return (target_logical_processor_index * 32 + i + 1); // TID 0 is reserved
        }
    }

    return 0; // no available TIDs
}

uint32_t create_kernel_thread(uint32_t function, uint32_t arguments[], uint32_t number_of_arguments) {
    // find logical processor with least number of threads
    uint32_t target_logical_processor_index = 0;
    uint32_t target_logical_processor_num_of_threads = kernel_threads[0].number_of_running_threads;
    for(uint32_t i = 1; i < number_of_logical_processors; i++) {
        if(kernel_threads[i].number_of_running_threads < target_logical_processor_num_of_threads) {
            target_logical_processor_index = i;
            target_logical_processor_num_of_threads = kernel_threads[i].number_of_running_threads;
        }
    }

    // get unique TID for new thread
    uint32_t tid = get_unique_kernel_tid(target_logical_processor_index);
    if(tid == 0) {
        return ERROR; // no available TIDs
    }

    // create new thread
    log("\nCreating thread %d in kernel process on CPU%d", tid, target_logical_processor_index);
    kernel_thread_t *new_thread = kalloc(sizeof(kernel_thread_t));
    new_thread->next = NULL;
    new_thread->id = tid;
    new_thread->page_directory_physical_address = NULL;
    new_thread->kernel_stack = kalloc(KERNEL_STACK_SIZE);
    new_thread->kill_me = false;
    new_thread->sleeping = false;
    new_thread->time_end_of_sleep = 0;

    // push arguments onto stack
    uint32_t *stack_pointer = (uint32_t *)(new_thread->kernel_stack + KERNEL_STACK_SIZE);
    if(number_of_arguments > 0) {
        for(int i = (number_of_arguments - 1); i >= 0; i--) {
            stack_pointer--;
            *stack_pointer = arguments[i];
        }
    }
    stack_pointer--;
    *stack_pointer = (uint32_t) &close_current_thread; // return address

    // create stack as if interrupt happened
    stack_pointer -= (sizeof(kernel_interrupt_stack_t) / sizeof(uint32_t));
    kernel_interrupt_stack_t *interrupt_stack = (kernel_interrupt_stack_t *) stack_pointer;
    interrupt_stack->eip = function;
    interrupt_stack->cs = 0x08; // kernel code segment
    interrupt_stack->eflags = 0x202; // interrupt enable flag
    interrupt_stack->ds = 0x10; // kernel data segment
    interrupt_stack->es = 0x10; // kernel data segment
    interrupt_stack->fs = 0x10; // kernel data segment
    interrupt_stack->gs = 0x10; // kernel data segment
    new_thread->kernel_stack_pointer = (uint32_t) stack_pointer;

    // atomically increase number of threads
    __atomic_add_fetch(&kernel_threads[target_logical_processor_index].number_of_running_threads, 1, __ATOMIC_SEQ_CST);

    // atomically add thread to thread list on target logical processor
    new_thread->next = kernel_threads[target_logical_processor_index].threads;
    while(
        !__atomic_compare_exchange_n(
            &kernel_threads[target_logical_processor_index].threads,
            &new_thread->next,
            new_thread,
            false, 
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
        )
    );

    // update pointer to current thread if needed
    if(kernel_threads[target_logical_processor_index].current_thread == NULL) {
        kernel_threads[target_logical_processor_index].current_thread = new_thread;
        lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
    }

    return new_thread->id;
}

uint32_t create_user_thread(program_t *program, void *entry_point, void *stack_pointer, uint32_t delete_signal_handler_address) {
    // find logical processor where will thread run fastest
    uint32_t target_logical_processor_index = 0xFFFFFFFF;
    uint32_t target_logical_processor_switches_to_execute = 0xFFFFFFFF;
    for(uint32_t i = 0; i < number_of_logical_processors; i++) {
        if(program->thread_list_on_logical_processor[i].number_of_running_threads >= 32) {
            continue;
        }

        // calculate how many switches it takes to get on processor, 1 = thread is running 100% of time, 2 = thread is running 50% of time, 4 = thread is running 25% of time...
        uint32_t logical_processor_number_of_running_programs = get_number_of_running_programs_on_processor(i);
        if(program->thread_list_on_logical_processor[target_logical_processor_index].number_of_running_threads == 0) {
            logical_processor_number_of_running_programs++; // after adding thread to this processor, number of running programs will increase by 1
        }
        uint32_t logical_processor_switches_to_execute = logical_processor_number_of_running_programs * (program->thread_list_on_logical_processor[i].number_of_running_threads + 1);

        if(logical_processor_switches_to_execute < target_logical_processor_switches_to_execute) {
            target_logical_processor_index = i;
            target_logical_processor_switches_to_execute = logical_processor_switches_to_execute;
        }
    }
    if(target_logical_processor_index == 0xFFFFFFFF) {
        return 0; // no available logical processor
    }

    // get unique TID for new thread
    uint32_t tid = get_unique_user_program_tid(target_logical_processor_index, program);
    if(tid == 0) {
        return 0; // no available TIDs
    }

    // create new user thread
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    user_thread_t *user_thread = kalloc(sizeof(user_thread_t));
    user_thread->next = NULL;
    user_thread->page_directory_physical_address = get_current_logical_processor_struct()->current_user_thread->page_directory_physical_address;
    user_thread->creation_thread_id = lpdata->current_user_thread->id;
    user_thread->id = tid;
    user_thread->kernel_stack = kalloc(KERNEL_STACK_SIZE);
    interrupt_stack_t *kernel_stack_pointer = (interrupt_stack_t *) ((uint32_t)user_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(interrupt_stack_t));
    kernel_stack_pointer->eip = (uint32_t) entry_point;
    kernel_stack_pointer->cs = 0x1B; // user code segment with RPL 3
    kernel_stack_pointer->eflags = 0x202; // interrupt enable flag
    kernel_stack_pointer->ds = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->es = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->fs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->gs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->ss = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->user_esp = (uint32_t) stack_pointer;
    user_thread->kernel_stack_pointer = (uint32_t) kernel_stack_pointer;
    user_thread->delete_me = false;
    user_thread->delete_signal_running = false;
    user_thread->kill_me = false;
    user_thread->sleeping = false;
    user_thread->delete_signal_handler_address = delete_signal_handler_address;
    user_thread->time_end_of_sleep = NULL;

    // update stats
    __atomic_add_fetch(&program->thread_list_on_logical_processor[target_logical_processor_index].number_of_running_threads, 1, __ATOMIC_SEQ_CST);

    // atomically add thread to thread list on target logical processor
    user_thread->next = program->thread_list_on_logical_processor[target_logical_processor_index].threads;
    while(
        !__atomic_compare_exchange_n(
            &program->thread_list_on_logical_processor[target_logical_processor_index].threads,
            &user_thread->next,
            user_thread,
            false, 
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
        )
    );

    // execute thread
    if(program->thread_list_on_logical_processor[target_logical_processor_index].current_thread == NULL) {
        program->thread_list_on_logical_processor[target_logical_processor_index].current_thread = user_thread;
        if(target_logical_processor_index != lpdata->index) {
            lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
        }
    }

    log("\nCreated user thread %d in process %x on CPU%d", tid, program, target_logical_processor_index);

    return tid;
}

uint32_t spawn_user_thread(program_t *program, void *entry_point, uint32_t delete_signal_handler_address) {
    // find logical processor where will thread run fastest
    uint32_t target_logical_processor_index = 0xFFFFFFFF;
    uint32_t target_logical_processor_switches_to_execute = 0xFFFFFFFF;
    for(uint32_t i = 0; i < number_of_logical_processors; i++) {
        if(program->thread_list_on_logical_processor[i].number_of_running_threads >= 32) {
            continue;
        }

        // calculate how many switches it takes to get on processor, 1 = thread is running 100% of time, 2 = thread is running 50% of time, 4 = thread is running 25% of time...
        uint32_t logical_processor_number_of_running_programs = get_number_of_running_programs_on_processor(i);
        if(program->thread_list_on_logical_processor[target_logical_processor_index].number_of_running_threads == 0) {
            logical_processor_number_of_running_programs++; // after adding thread to this processor, number of running programs will increase by 1
        }
        uint32_t logical_processor_switches_to_execute = logical_processor_number_of_running_programs * (program->thread_list_on_logical_processor[i].number_of_running_threads + 1);

        if(logical_processor_switches_to_execute < target_logical_processor_switches_to_execute) {
            target_logical_processor_index = i;
            target_logical_processor_switches_to_execute = logical_processor_switches_to_execute;
        }
    }
    if(target_logical_processor_index == 0xFFFFFFFF) {
        return 0; // no available logical processor
    }

    // get unique TID
    uint32_t tid = get_unique_user_program_tid(target_logical_processor_index, program);
    if(tid == 0) {
        return 0; // no available TIDs
    }

    // load virtual space of template
    uint32_t original_page_directory = read_cr3();
    spawning_template_t *template = (spawning_template_t *) program->template;
    load_page_directory(template->page_directory);

    // create new page directory
    LOCK_MUTEX(&creating_page_directory_mutex);
    uint32_t new_page_directory = (uint32_t) pm_alloc_page();
    vm_map_page(P_MEM_NEW_PAGE_DIRECTORY, new_page_directory, VM_KERNEL);
    invlpg(P_MEM_NEW_PAGE_DIRECTORY);
    uint32_t *template_page_directory = (uint32_t *) P_MEM_PAGE_DIRECTORY;
    uint32_t *page_directory = (uint32_t *) P_MEM_NEW_PAGE_DIRECTORY;
    for(int i = 0; i < (1024 - number_of_shared_page_tables); i++) {
        if((template_page_directory[i] & VM_FLAG_PRESENT) != 0) {
            // copy pages into new page tables
            uint32_t new_page_table = (uint32_t) pm_alloc_page();
            page_directory[i] = (new_page_table | VM_PAGE_TABLE);
            vm_map_page(P_MEM_NEW_PAGE_TABLE, new_page_table, VM_KERNEL);
            invlpg(P_MEM_NEW_PAGE_TABLE);
            memcpy((void *)P_MEM_NEW_PAGE_TABLE, (void *) (P_MEM_PAGE_TABLE + (i * PAGE_SIZE)), PAGE_SIZE);
        }
    }
    for(int i = (1024 - number_of_shared_page_tables); i < 1023; i++) {
        page_directory[i] = template_page_directory[i]; // copy kernel page tables
    }
    page_directory[1023] = (new_page_directory | VM_KERNEL); // map page directory to itself
    // TODO: add shared page tables
    UNLOCK_MUTEX(&creating_page_directory_mutex);

    // create new user thread
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    user_thread_t *user_thread = kalloc(sizeof(user_thread_t));
    user_thread->next = NULL;
    user_thread->page_directory_physical_address = new_page_directory;
    user_thread->number_of_threads_in_page_directory = kalloc(sizeof(uint32_t));
    *user_thread->number_of_threads_in_page_directory = 1;
    user_thread->creation_thread_id = lpdata->current_user_thread->id;
    user_thread->id = tid;
    user_thread->kernel_stack = kalloc(KERNEL_STACK_SIZE);
    interrupt_stack_t *kernel_stack_pointer = (interrupt_stack_t *) ((uint32_t)user_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(interrupt_stack_t));
    kernel_stack_pointer->eip = template->entry_point;
    kernel_stack_pointer->cs = 0x1B; // user code segment with RPL 3
    kernel_stack_pointer->eflags = 0x202; // interrupt enable flag
    kernel_stack_pointer->ds = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->es = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->fs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->gs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->ss = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->user_esp = (uint32_t) template->user_stack;
    user_thread->kernel_stack_pointer = (uint32_t) kernel_stack_pointer;
    user_thread->delete_me = false;
    user_thread->delete_signal_running = false;
    user_thread->kill_me = false;
    user_thread->sleeping = false;
    user_thread->delete_signal_handler_address = delete_signal_handler_address;
    user_thread->time_end_of_sleep = NULL;

    // update stats
    __atomic_add_fetch(&program->number_of_threads, 1, __ATOMIC_RELAXED);
    __atomic_add_fetch(&program->thread_list_on_logical_processor[target_logical_processor_index].number_of_running_threads, 1, __ATOMIC_RELAXED);
    program->thread_list_on_logical_processor[target_logical_processor_index].threads = user_thread;
    program->thread_list_on_logical_processor[target_logical_processor_index].current_thread = user_thread;

    // atomically add thread to thread list on target logical processor
    user_thread->next = program->thread_list_on_logical_processor[target_logical_processor_index].threads;
    while(
        !__atomic_compare_exchange_n(
            &program->thread_list_on_logical_processor[target_logical_processor_index].threads,
            &user_thread->next,
            user_thread,
            false, 
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
        )
    );

    // execute thread
    if(program->thread_list_on_logical_processor[target_logical_processor_index].current_thread == NULL) {
        program->thread_list_on_logical_processor[target_logical_processor_index].current_thread = user_thread;
        if(target_logical_processor_index != lpdata->index) {
            lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
        }
    }

    // move back to original page directory
    load_page_directory(original_page_directory);

    return tid;
}

/* sleep for threads */
void sleep_for_thread(interrupt_stack_t *stack_of_interrupt, uint32_t microseconds) {
    if(microseconds == 0) {
        return;
    }

    logical_processor_t *lpdata = get_current_logical_processor_struct();
    scheduler_state_t scheduler_state = lpdata->scheduler_state;
    if(scheduler_state == SCHEDULER_STATE_KERNEL) {
        // save stack pointer of current thread
        kernel_thread_t *current_thread = lpdata->current_kernel_thread;
        current_thread->kernel_stack_pointer = (uint32_t) stack_of_interrupt;

        // set time for wakeup by value in eax
        current_thread->time_end_of_sleep = ((*get_time_in_microseconds)() + microseconds);

        // set sleeping flag
        current_thread->sleeping = true;

        // atomically decrease number of running threads
        __atomic_sub_fetch(&kernel_threads[lpdata->index].number_of_running_threads, 1, __ATOMIC_SEQ_CST);

        log("\nKernel thread %d is sleeping for %d us", current_thread->id, microseconds);
    }
    else { // scheduler_state is SCHEDULER_STATE_USER
        // save stack pointer of current thread
        user_thread_t *current_thread = lpdata->current_user_thread;
        current_thread->kernel_stack_pointer = (uint32_t) stack_of_interrupt;

        // set time for wakeup by value in eax
        current_thread->time_end_of_sleep = ((*get_time_in_microseconds)() + microseconds);

        // set sleeping flag
        current_thread->sleeping = true;

        // atomically decrease number of running threads
        __atomic_sub_fetch(&lpdata->current_program->thread_list_on_logical_processor[lpdata->index].number_of_running_threads, 1, __ATOMIC_SEQ_CST);
        
        log("\nUser thread %d is sleeping for %d us", current_thread->id, microseconds);
    }

    // switch to next thread
    scheduler_interrupt(stack_of_interrupt);
}

/* destroy processes and threads */
void close_current_thread_interrupt(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->scheduler_state == SCHEDULER_STATE_KERNEL) {
        kernel_thread_t *current_thread = lpdata->current_kernel_thread;
        log("\nClosing signal from kernel thread %d on CPU%d", current_thread->id, lpdata->index);
        current_thread->kill_me = true;
        extern void move_to_floating_stack(uint32_t new_esp, uint32_t new_eip, uint32_t argument);
        move_to_floating_stack((uint32_t) lpdata->floating_stack + KERNEL_STACK_SIZE, (uint32_t) &scheduler_interrupt, (uint32_t) stack_of_interrupt);
    }
    else {
        user_thread_t *current_thread = lpdata->current_user_thread;
        log("\nClosing signal from user thread %d in program %x on CPU%d", current_thread->id, lpdata->current_program, lpdata->index);
        current_thread->delete_me = true;
        if(current_thread->delete_signal_handler_address == NULL ||current_thread->delete_signal_running == true) {
            current_thread->kill_me = true;
        }
        extern void move_to_floating_stack(uint32_t new_esp, uint32_t new_eip, uint32_t argument);
        move_to_floating_stack((uint32_t) lpdata->floating_stack + KERNEL_STACK_SIZE, (uint32_t) &scheduler_interrupt, (uint32_t) stack_of_interrupt);
    }
}

/* signals */
uint32_t send_signal_to_current_user_thread(interrupt_stack_t *stack_of_interrupt, uint32_t signal_handler_address) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    user_thread_t *current_thread = lpdata->current_user_thread;
    uint32_t *user_stack = (uint32_t *) stack_of_interrupt->user_esp;
    user_stack--;
    if(return_validated_pointer(user_stack, sizeof(uint32_t)) == NULL) {
        return ERROR;
    }
    *user_stack = stack_of_interrupt->eip; // ret from signal handler will return to previously executed code
    stack_of_interrupt->user_esp = (uint32_t) user_stack;
    stack_of_interrupt->eip = signal_handler_address;
    return SUCCESS;
}

/* switch between threads */
void preemptive_scheduling_interrupt(interrupt_stack_t *stack_of_interrupt) {
    // this interrupt can be invoked only by LAPIC timer, or IPI so acknowledge interrupt on LAPIC
    lapic_send_eoi();

    // invoke scheduler
    scheduler_interrupt(stack_of_interrupt);
}

void scheduler_interrupt(interrupt_stack_t *stack_of_interrupt) {
    uint32_t actual_time = (*get_time_in_microseconds)();
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    // log("\nScheduler invoked on CPU%d", lpdata->index);

    // save stack pointer
    switch(lpdata->scheduler_state) {
        case SCHEDULER_STATE_USER: {
            if(lpdata->current_user_thread != NULL) {
                user_thread_t *current_thread = lpdata->current_user_thread;
                current_thread->kernel_stack_pointer = (uint32_t) stack_of_interrupt;
            }
            break;
        }
        case SCHEDULER_STATE_KERNEL: {
            if(lpdata->current_kernel_thread != NULL) {
                kernel_thread_t *current_thread = lpdata->current_kernel_thread;
                current_thread->kernel_stack_pointer = (uint32_t) stack_of_interrupt;
            }
            break;
        }
        case SCHEDULER_STATE_IDLE: {
            lpdata->scheduler_state = SCHEDULER_STATE_USER;
            break;
        }
    }

    // three passes before concluding that there are no running threads, it guarantees that everything was checked at least once
    for(int i = 0; i < 3; i++) {
        switch(lpdata->scheduler_state) {
            case SCHEDULER_STATE_USER: {
                // move to next program
                program_t *previous_program = lpdata->current_program;
                if(previous_program == NULL) {
                    lpdata->current_program = programs;
                }
                else {
                    // log("\nProgram list current program %x on CPU%d", previous_program, lpdata->index);
                    lpdata->current_program = previous_program->next;
                }
                program_t *current_program = lpdata->current_program;
                if(current_program == NULL) {
                    // log("\nEnd of programs list on CPU%d", lpdata->index);
                    lpdata->scheduler_state = SCHEDULER_STATE_KERNEL;
                    break;
                }

                // remove program if it has no threads
                // this must be done only from one logical processor to prevent ABA problem
                if(lpdata->index == 0 && current_program->number_of_threads == 0) {
                    log("\nRemoving program %x from program list", current_program);

                    // unlink program from list
                    if(previous_program == NULL) {
                        while(!__atomic_compare_exchange_n(
                            &programs,
                            &current_program,
                            current_program->next,
                            false,
                            __ATOMIC_SEQ_CST,
                            __ATOMIC_SEQ_CST
                        )) {}
                    }
                    else {
                        previous_program->next = current_program->next;
                    }

                    // free process resources
                    kfree(current_program);

                    log("\nProgram %x removed from program list", current_program);

                    continue;
                }

                // find next runnable thread in program
                user_thread_list_t *thread_list = &current_program->thread_list_on_logical_processor[lpdata->index];
                while(true) {
                    // move to next thread
                    user_thread_t *previous_thread = thread_list->current_thread;
                    if(thread_list->current_thread == NULL) {
                        thread_list->current_thread = thread_list->threads;
                    }
                    else {
                        // log("\nUser thread list current thread %d on CPU%d", previous_thread->id, lpdata->index);
                        thread_list->current_thread = thread_list->current_thread->next;
                    }
                    user_thread_t *current_thread = thread_list->current_thread;
                    if(current_thread == NULL) {
                        // log("\nEnd of user threads list on CPU%d", lpdata->index);
                        lpdata->scheduler_state = SCHEDULER_STATE_KERNEL;
                        break;
                    }

                    // process flags
                    if(current_thread->kill_me == true) {
                        uint32_t tid = current_thread->id;
                        log("\nKilling user thread %d", tid);

                        // unlink from list
                        if(previous_thread == NULL) {
                            while(!__atomic_compare_exchange_n(
                                &thread_list->threads,
                                &current_thread,
                                current_thread->next,
                                false,
                                __ATOMIC_SEQ_CST,
                                __ATOMIC_SEQ_CST
                            )) {}
                        }
                        else {
                            previous_thread->next = current_thread->next;
                        }

                        // decrease number of running threads
                        if(current_thread->sleeping == false) {
                            __atomic_sub_fetch(&thread_list->number_of_running_threads, 1, __ATOMIC_SEQ_CST);
                        }
                        __atomic_sub_fetch(&current_program->number_of_threads, 1, __ATOMIC_SEQ_CST);

                        // free thread ID
                        thread_list->tid_bitmap &= ~(1 << ((tid - 1) & 0x1F));

                        // remove thread from pointers if it was currently executed
                        if(lpdata->current_user_thread == current_thread) {
                            lpdata->current_user_thread = NULL;
                        }
                        if(thread_list->current_thread == current_thread) {
                            thread_list->current_thread = NULL;
                        }

                        // atomically decrease number of threads in page directory
                        __atomic_sub_fetch(&(current_thread->number_of_threads_in_page_directory), 1, __ATOMIC_SEQ_CST);

                        // free page directory if necessary
                        if(current_thread->number_of_threads_in_page_directory == 0) {
                            free_virtual_space(current_thread->page_directory_physical_address);
                            kfree(current_thread->number_of_threads_in_page_directory);
                        }

                        // free stack
                        kfree(current_thread->kernel_stack);
                        kfree(current_thread);

                        log("\nUser thread %d is killed", tid);

                        continue;
                    }
                    if(current_thread->delete_me == true) {
                        if(current_thread->delete_signal_running == false) {
                            // move execution to signal handler
                            stack_of_interrupt->eip = current_thread->delete_signal_handler_address;

                            // set all threads that were created by this thread to be deleted
                            // we can do this because this thread can not now create more threads
                            uint32_t current_thread_id = current_thread->id;
                            for(int i = 0; i < number_of_logical_processors; i++) {
                                user_thread_list_t *local_thread_list = &current_program->thread_list_on_logical_processor[i];
                                user_thread_t *local_current_thread = local_thread_list->threads;
                                while(local_current_thread != NULL) {
                                    if(local_current_thread->creation_thread_id == current_thread_id) {
                                        local_current_thread->delete_me = true;
                                    }
                                    local_current_thread = local_current_thread->next;
                                }
                            }

                            // set flag that signal is running
                            current_thread->delete_signal_running = true;
                        }
                    }
                    if(current_thread->sleeping == true) {
                        if((int)(actual_time - current_thread->time_end_of_sleep) > 0) {
                            log("\nUser thread %d is waken up", current_thread->id);
                            current_thread->sleeping = false;

                            // increase number of running threads
                            __atomic_add_fetch(&thread_list->number_of_running_threads, 1, __ATOMIC_SEQ_CST);
                        }
                        else {
                            continue;
                        }
                    }

                    // next time check kernel threads
                    lpdata->scheduler_state = SCHEDULER_STATE_KERNEL;

                    // switch to selected thread
                    // log("\nSwitching to program %x to user thread %d on CPU%d", current_program, current_thread->id, lpdata->index);
                    lpdata->current_program = current_program;
                    lpdata->current_user_thread = current_thread;
                    extern void exit_interrupt_to_thread(uint32_t stack_pointer);
                    if(current_thread->page_directory_physical_address != read_cr3()) {
                        load_page_directory(current_thread->page_directory_physical_address);
                    }
                    lpdata->tss->esp0 = (uint32_t) current_thread->kernel_stack + KERNEL_STACK_SIZE; // set kernel stack for next thread
                    lapic_set_oneshot_interrupt(10, INTERRUPT_PREEMPETIVE_SCHEDULING); // set preempetive scheduler interrupt
                    exit_interrupt_to_thread(current_thread->kernel_stack_pointer);
                }

                break;
            }
            case SCHEDULER_STATE_KERNEL: {
                kernel_thread_list_t *thread_list = &kernel_threads[lpdata->index];
                while(true) {
                    // move to next thread
                    kernel_thread_t *previous_thread = thread_list->current_thread;
                    if(thread_list->current_thread == NULL) {
                        thread_list->current_thread = thread_list->threads;
                    }
                    else {
                        // log("\nKernel thread list current thread %d on CPU%d", previous_thread->id, lpdata->index);
                        thread_list->current_thread = thread_list->current_thread->next;
                    }
                    kernel_thread_t *current_thread = thread_list->current_thread;
                    if(current_thread == NULL) {
                        // log("\nEnd of kernel threads list on CPU%d", lpdata->index);
                        lpdata->scheduler_state = SCHEDULER_STATE_USER;
                        break;
                    }

                    // process flags
                    if(current_thread->kill_me == true) {
                        uint32_t tid = current_thread->id;
                        log("\nKilling kernel thread %d", tid);

                        // unlink from list
                        if(previous_thread == NULL) {
                            while(!__atomic_compare_exchange_n(
                                &thread_list->threads,
                                &current_thread,
                                current_thread->next,
                                false,
                                __ATOMIC_SEQ_CST,
                                __ATOMIC_SEQ_CST
                            )) {}
                        }
                        else {
                            previous_thread->next = current_thread->next;
                        }

                        // decrease number of running threads
                        if(current_thread->sleeping == false) {
                            __atomic_sub_fetch(&thread_list->number_of_running_threads, 1, __ATOMIC_SEQ_CST);
                        }

                        // free thread ID
                        thread_list->tid_bitmap &= ~(1 << ((tid - 1) & 0x1F));

                        // remove thread if it was currently executed
                        if(lpdata->current_kernel_thread == current_thread) {
                            lpdata->current_kernel_thread = NULL;
                        }
                        if(thread_list->current_thread == current_thread) {
                            thread_list->current_thread = NULL;
                        }

                        // free stack
                        kfree(current_thread->kernel_stack);
                        kfree(current_thread);

                        log("\nKernel thread %d is killed", tid);

                        continue;
                    }
                    if(current_thread->sleeping == true) {
                        if((int)(actual_time - current_thread->time_end_of_sleep) > 0) {
                            log("\nKernel thread %d is waken up", current_thread->id);
                            current_thread->sleeping = false;

                            // increase number of running threads
                            __atomic_add_fetch(&thread_list->number_of_running_threads, 1, __ATOMIC_SEQ_CST);
                        }
                        else {
                            continue;
                        }
                    }

                    // switch to this thread
                    // log("\nSwitching to kernel thread %d on CPU%d", current_thread->id, lpdata->index);
                    lpdata->current_kernel_thread = current_thread;
                    extern void exit_interrupt_to_thread(uint32_t stack_pointer);
                    if(current_thread->page_directory_physical_address != NULL && current_thread->page_directory_physical_address != read_cr3()) {
                        load_page_directory(current_thread->page_directory_physical_address);
                    }
                    exit_interrupt_to_thread(current_thread->kernel_stack_pointer);
                }

                break;
            }
            case SCHEDULER_STATE_IDLE: {
                break;
            }
        }
    }

    // there are no running threads, go to idle state
    // log("\nCPU%d has no running kernel threads, going to idle state on %x", lpdata->index, lpdata->idle_thread_stack);
    lpdata->scheduler_state = SCHEDULER_STATE_IDLE;
    lapic_set_oneshot_interrupt(10, INTERRUPT_PREEMPETIVE_SCHEDULING); // TODO: wake from sleep exactly when it is needed for next sleeping thread
    extern void exit_interrupt_to_thread(uint32_t stack_pointer);
    exit_interrupt_to_thread((uint32_t)lpdata->idle_thread_stack);
}

/* calls */
void sleep_current_thread_interrupt(interrupt_stack_t *stack_of_interrupt) {
    sleep_for_thread(stack_of_interrupt, stack_of_interrupt->eax);
}

void sleep_current_thread(uint32_t milliseconds) {
    asm volatile ("int %0" : : "i" (INTERRUPT_SLEEP_FOR_THREAD), "a" (milliseconds));
}

void switch_to_another_thread(void) {
    asm volatile ("int %0" : : "i" (INTERRUPT_VECTOR_SCHEDULER));
}

void close_current_thread(void) {
    asm volatile ("int %0" : : "i" (INTERRUPT_VECTOR_EXIT_THREAD));
}