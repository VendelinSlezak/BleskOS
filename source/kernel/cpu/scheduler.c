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

/* global variables */
process_t *kernel_process;

/* local variables */
process_t *process_list = NULL;
uint32_t process_waiting_for_deletion = false;

/* functions */
void initialize_scheduler(void) {
    // create kernel process
    kernel_process = kalloc(sizeof(process_t) + (sizeof(thread_list_t) * number_of_logical_processors));
    kernel_process->next = NULL;
    kernel_process->pid = (uint32_t) kernel_process; // use address of process structure as PID
    kernel_process->delete_me = false;
    kernel_process->page_directory_physical_address = PM_KERNEL_PAGE_DIRECTORY;
    kernel_process->tid_bitmap = kalloc(sizeof(uint32_t) * number_of_logical_processors);

    // create main thread for kernel process
    thread_t *kernel_main_thread = kalloc(sizeof(thread_t));
    kernel_main_thread->next = NULL;
    kernel_main_thread->tid = 1; // main thread has TID 1
    kernel_main_thread->stack_pointer = 0; // stack pointer will be set when switching to this thread
    kernel_main_thread->delete_me = false;
    kernel_main_thread->kernel_stack = (void *) P_MEM_KERNEL_STACK;
    kernel_main_thread->end_of_thread_signal = NULL;

    // link main thread to kernel process
    uint32_t current_logical_processor_index = get_current_logical_processor_index();
    kernel_process->main_thread = kernel_main_thread;
    kernel_process->threads_on_logical_processor[current_logical_processor_index].number_of_running_threads = 1;
    kernel_process->threads_on_logical_processor[current_logical_processor_index].threads = kernel_main_thread;
    kernel_process->threads_on_logical_processor[current_logical_processor_index].current_thread = kernel_main_thread;

    // add kernel process to process list
    process_list = kernel_process;

    // update logical processor structures
    for(uint32_t i = 0; i < number_of_logical_processors; i++) {
        logical_processor_structs_ptr[i].current_process = kernel_process;
        if(logical_processors[i].hardware_id == bootstrap_processor_id) {
            logical_processor_structs_ptr[i].current_thread = kernel_main_thread;
        }
        else {
            logical_processor_structs_ptr[i].current_thread = NULL;
        }
    }
    mfence();

    // register scheduler interrupt handler
    register_interrupt_handler(INTERRUPT_SLEEP_FOR_THREAD, sleep_current_thread_interrupt);
    register_interrupt_handler(INTERRUPT_PREEMPETIVE_SCHEDULING, preemptive_scheduling_interrupt);
    register_interrupt_handler(INTERRUPT_VECTOR_EXIT_THREAD, close_current_thread_interrupt);
    register_interrupt_handler(INTERRUPT_VECTOR_SCHEDULER, scheduler_interrupt);

    // enable preemptive scheduling interrupt
    lapic_set_oneshot_interrupt(10, INTERRUPT_PREEMPETIVE_SCHEDULING); // 10 ms
}

/* create processes and threads */
void create_user_process_from_spawning_template(spawning_template_t *template) {
    lock_core();

    // load virtual space of template
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

    // move to new virtual space
    load_page_directory(new_page_directory);

    // create new process
    process_t *new_process = kalloc(sizeof(process_t) + (sizeof(thread_list_t) * number_of_logical_processors));
    new_process->pid = (uint32_t) new_process; // use address of process structure as PID
    new_process->delete_me = false;
    new_process->page_directory_physical_address = new_page_directory;
    new_process->tid_bitmap = kalloc(sizeof(uint32_t) * number_of_logical_processors);

    // create kernel stack
    uint32_t *kernel_stack = kalloc(KERNEL_STACK_SIZE);
    interrupt_stack_t *kernel_stack_pointer = (interrupt_stack_t *) ((uint32_t)kernel_stack + KERNEL_STACK_SIZE - sizeof(interrupt_stack_t));
    kernel_stack_pointer->eip = template->entry_point;
    kernel_stack_pointer->cs = 0x1B; // user code segment with RPL 3
    kernel_stack_pointer->eflags = 0x202; // interrupt enable flag
    kernel_stack_pointer->ds = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->es = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->fs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->gs = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->ss = 0x23; // user data segment with RPL 3
    kernel_stack_pointer->user_esp = (uint32_t) template->user_stack;

    // create main thread for new process
    thread_t *new_thread = kalloc(sizeof(thread_t));
    new_thread->next = NULL;
    new_thread->tid = 1;
    new_thread->stack_pointer = (uint32_t)kernel_stack_pointer;
    new_thread->delete_me = false;
    new_thread->kernel_stack = kernel_stack;
    uint32_t target_logical_processor_index = 0;
    log("\nCreating main thread %d in process %x on logical processor %d", new_thread->tid, new_process->pid, target_logical_processor_index);

    // add thread to process's thread list on target logical processor
    new_process->threads_on_logical_processor[target_logical_processor_index].threads = new_thread;

    // set number of threads in process
    new_process->threads_on_logical_processor[target_logical_processor_index].number_of_running_threads = 1;

    // update pointer to current thread
    new_process->threads_on_logical_processor[target_logical_processor_index].current_thread = new_thread;

    // add process to process list
    new_process->next = process_list;
    while (
        !__atomic_compare_exchange_n(
            &process_list,
            &new_process->next,
            new_process,
            false,
            __ATOMIC_RELEASE,
            __ATOMIC_RELAXED
        )
    ) {}

    unlock_core();

    // signal the logical processor to run the new process
    lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
}

uint32_t get_unique_tid(uint32_t target_logical_processor_index, process_t *process) {
    for(int i = 0; i < 32; i++) {
        uint32_t mask = (1 << i);
        if((process->tid_bitmap[target_logical_processor_index] & mask) == 0) {
            process->tid_bitmap[target_logical_processor_index] |= mask;
            return (target_logical_processor_index * 32 + i + 2); // TID 1 is for main thread, TIDs start from 2
        }
    }

    return 0; // no available TIDs
}

uint32_t create_kernel_thread(uint32_t function, uint32_t arguments[], uint32_t number_of_arguments) {
    // find logical processor with least number of threads
    uint32_t target_logical_processor_index = 0;
    uint32_t target_logical_processor_num_of_threads = kernel_process->threads_on_logical_processor[0].number_of_running_threads;
    for(uint32_t i = 1; i < number_of_logical_processors; i++) {
        if(kernel_process->threads_on_logical_processor[i].number_of_running_threads < target_logical_processor_num_of_threads) {
            target_logical_processor_index = i;
            target_logical_processor_num_of_threads = kernel_process->threads_on_logical_processor[i].number_of_running_threads;
        }
    }

    // get unique TID for new thread
    uint32_t tid = get_unique_tid(target_logical_processor_index, kernel_process);
    if(tid == 0) {
        return ERROR; // no available TIDs
    }

    // create new thread
    log("\nCreating thread %x in kernel process on logical processor %d", tid, target_logical_processor_index);
    thread_t *new_thread = kalloc(sizeof(thread_t));
    new_thread->next = NULL;
    new_thread->tid = tid;
    new_thread->delete_me = false;
    new_thread->kernel_stack = kalloc(KERNEL_STACK_SIZE);
    new_thread->end_of_thread_signal = NULL;

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
    new_thread->stack_pointer = (uint32_t) stack_pointer;

    // atomically add thread to process's thread list on target logical processor
    new_thread->next = kernel_process->threads_on_logical_processor[target_logical_processor_index].threads;
    while(
        !__atomic_compare_exchange_n(
            &kernel_process->threads_on_logical_processor[target_logical_processor_index].threads,
            &new_thread->next,
            new_thread,
            false, 
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
        )
    );

    // atomically increase number of threads in process
    __atomic_add_fetch(&kernel_process->threads_on_logical_processor[target_logical_processor_index].number_of_running_threads, 1, __ATOMIC_SEQ_CST);

    // update pointer to current thread if needed
    if(kernel_process->threads_on_logical_processor[target_logical_processor_index].current_thread == NULL) {
        kernel_process->threads_on_logical_processor[target_logical_processor_index].current_thread = new_thread;
        lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
    }

    return new_thread->tid;
}

uint32_t create_user_thread(process_t *process, void *entry_point, void *stack_pointer, uint8_t *end_of_thread_signal) {
    // find logical processor with least number of threads
    uint32_t target_logical_processor_index = 0;
    uint32_t target_logical_processor_num_of_threads = process->threads_on_logical_processor[0].number_of_running_threads;
    for(uint32_t i = 1; i < number_of_logical_processors; i++) {
        if(process->threads_on_logical_processor[i].number_of_running_threads < target_logical_processor_num_of_threads) {
            target_logical_processor_index = i;
            target_logical_processor_num_of_threads = process->threads_on_logical_processor[i].number_of_running_threads;
        }
    }

    // get unique TID for new thread
    uint32_t tid = get_unique_tid(target_logical_processor_index, process);
    if(tid == 0) {
        return ERROR; // no available TIDs
    }

    // create new thread
    log("\nCreating user thread %d in process %x on CPU%d", tid, process->pid, target_logical_processor_index);
    thread_t *new_thread = kalloc(sizeof(thread_t));
    new_thread->next = NULL;
    new_thread->tid = tid;
    new_thread->delete_me = false;
    new_thread->kernel_stack = kalloc(KERNEL_STACK_SIZE);
    new_thread->end_of_thread_signal = end_of_thread_signal;

    // create kernel stack as if interrupt happened
    interrupt_stack_t *kernel_interrupt_stack = (interrupt_stack_t *) (new_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(interrupt_stack_t));
    kernel_interrupt_stack->eip = (uint32_t) (entry_point);
    kernel_interrupt_stack->cs = 0x1B; // user code segment
    kernel_interrupt_stack->eflags = 0x202; // interrupt enable flag
    kernel_interrupt_stack->ds = 0x23; // user data segment with RPL 3
    kernel_interrupt_stack->es = 0x23; // user data segment with RPL 3
    kernel_interrupt_stack->fs = 0x23; // user data segment with RPL 3
    kernel_interrupt_stack->gs = 0x23; // user data segment with RPL 3
    kernel_interrupt_stack->ss = 0x23; // user data segment with RPL 3
    kernel_interrupt_stack->user_esp = (uint32_t) stack_pointer;
    new_thread->stack_pointer = (uint32_t) kernel_interrupt_stack;

    // atomically add thread to process's thread list on target logical processor
    new_thread->next = process->threads_on_logical_processor[target_logical_processor_index].threads;
    while(
        !__atomic_compare_exchange_n(
            &process->threads_on_logical_processor[target_logical_processor_index].threads,
            &new_thread->next,
            new_thread,
            false, 
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
        )
    );

    // atomically increase number of threads in process
    __atomic_add_fetch(&process->threads_on_logical_processor[target_logical_processor_index].number_of_running_threads, 1, __ATOMIC_SEQ_CST);

    // update pointer to current thread if needed
    if(process->threads_on_logical_processor[target_logical_processor_index].current_thread == NULL) {
        process->threads_on_logical_processor[target_logical_processor_index].current_thread = new_thread;
        if(target_logical_processor_index != get_current_logical_processor_struct()->index) {
            lapic_send_ipi(logical_processors[target_logical_processor_index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING);
        }
    }

    return new_thread->tid;
}

/* sleep for threads */
void sleep_for_thread(interrupt_stack_t *stack_of_interrupt, uint32_t milliseconds) {
    if(milliseconds == 0) {
        return;
    }

    logical_processor_t *lpdata = get_current_logical_processor_struct();
    thread_t *current_thread = lpdata->current_thread;

    // save stack pointer of current thread
    lpdata->current_thread->stack_pointer = (uint32_t) stack_of_interrupt;
    
    // set time for wakeup by value in eax
    current_thread->end_of_sleep = ((*get_time_in_microseconds)() + (milliseconds * 1000));

    // remove current thread from linked list - it is guaranteed that it will be here
    thread_list_t *current_thread_list = &lpdata->current_process->threads_on_logical_processor[lpdata->index];
    if(current_thread_list->threads == current_thread) {
        current_thread_list->threads = current_thread->next;
    }
    else {
        thread_t *previous_thread = current_thread_list->threads;
        thread_t *thread = current_thread_list->threads->next;
        while(thread != NULL) {
            if(thread == current_thread) {
                previous_thread->next = current_thread->next;
                break;
            }
            previous_thread = thread;
            thread = thread->next;
        }
    }
    current_thread_list->current_thread = NULL;
    current_thread_list->number_of_running_threads--;

    // remove current thread from processor data
    lpdata->current_thread = NULL;

    // add current thread to sleeping list
    current_thread->next = current_thread_list->sleeping_threads;
    current_thread_list->sleeping_threads = current_thread;

    log("\nThread %x is sleeping for %d ms", current_thread, milliseconds);

    // switch to next thread
    scheduler_interrupt(stack_of_interrupt);
}

/* destroy processes and threads */
void close_current_thread_interrupt(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    process_t *current_process = lpdata->current_process;
    thread_t *current_thread = lpdata->current_thread;

    log("\nClosing signal from thread %d in process %x on CPU%d", current_thread->tid, current_process->pid, lpdata->index);

    // if this is main thread, mark whole process for deletion
    if(current_thread->tid == 1) {
        log("\nMarking process %x for deletion", current_process->pid);
        current_process->delete_me = true;
        process_waiting_for_deletion = true;
    }

    // mark current thread for deletion
    current_thread->delete_me = true;

    // switch to next thread and clean up
    void move_to_floating_stack(uint32_t floating_stack_address, uint32_t function, uint32_t argument);
    move_to_floating_stack((uint32_t) lpdata->floating_stack + KERNEL_STACK_SIZE, (uint32_t) &scheduler_interrupt, (uint32_t) stack_of_interrupt);
}

/* signals */
uint32_t send_signal_to_current_user_thread(interrupt_stack_t *stack_of_interrupt, uint32_t signal_handler_address) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    thread_t *current_thread = lpdata->current_thread;
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

void wake_up_threads(thread_t *current_thread, thread_list_t *current_thread_list) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    thread_t *prev_thread = NULL;
    uint32_t actual_time = (*get_time_in_microseconds)();
    while(current_thread != NULL) {
        // check if this thread should be waken
        if((int)(actual_time - current_thread->end_of_sleep) >= 0) {
            // log("\nThread %x is waken up", current_thread);

            // remove thread from sleeping threads
            if(prev_thread == NULL) {
                current_thread_list->sleeping_threads = current_thread->next;
            }
            else {
                prev_thread->next = current_thread->next;
            }

            // save next thread in sleeping threads list
            thread_t *next_thread = current_thread->next;

            // insert thread to running threads
            current_thread->next = current_thread_list->threads;
            current_thread_list->threads = current_thread;
            current_thread_list->number_of_running_threads++;

            // move to next sleeping thread
            current_thread = next_thread;
        }
        else {
            // move to next sleeping thread
            prev_thread = current_thread;
            current_thread = current_thread->next;
        }
    }
}

void scheduler_interrupt(interrupt_stack_t *stack_of_interrupt) {
    // log("\nScheduler interrupt on CPU%d", get_current_logical_processor_index());
    
    // switch out of deleted process if needed
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->current_process->delete_me == true) {
        lpdata->current_process = kernel_process;
        lpdata->current_thread = NULL;
    }

    // save stack pointer of current thread
    if(lpdata->current_thread != NULL) {
        lpdata->current_thread->stack_pointer = (uint32_t) stack_of_interrupt;
    }

    // remove processes marked for deletion
    if(lpdata->index == 0 && process_waiting_for_deletion == true) { // only first logical processor cleans up deleted processes to prevent race conditions
        process_t *prev_process = NULL;
        process_t *current_process = process_list;
        while(current_process != NULL) {
            if(current_process->delete_me == true) {
                log("\nDeleting process %x", current_process->pid);

                // remove process from list
                if(prev_process == NULL) {
                    process_list = current_process->next;
                }
                else {
                    prev_process->next = current_process->next;
                }
                process_t *process_to_delete = current_process;
                current_process = current_process->next;

                // switch all logical processors running this process to another process
                logical_processor_t *lp = logical_processor_structs_ptr;
                for(uint32_t i = 0; i < number_of_logical_processors; i++) {
                    if(lp->current_process == process_to_delete) {
                        lapic_send_ipi(logical_processors[lp->index].hardware_id, INTERRUPT_PREEMPETIVE_SCHEDULING); // trigger scheduler interrupt to switch to another process
                    }
                    lp++;
                }

                // free all threads in process
                for(uint32_t i = 0; i < number_of_logical_processors; i++) {
                    thread_t *thread = process_to_delete->threads_on_logical_processor[i].threads;
                    while(thread != NULL) {
                        thread_t *thread_to_delete = thread;
                        thread = thread->next;
                        if(thread_to_delete->end_of_thread_signal != NULL) {
                            *(thread_to_delete->end_of_thread_signal) = 1; // signal end of thread if needed
                        }
                        kfree(thread_to_delete->kernel_stack);
                        kfree(thread_to_delete);
                    }

                    thread = process_to_delete->threads_on_logical_processor[i].sleeping_threads;
                    while(thread != NULL) {
                        thread_t *thread_to_delete = thread;
                        thread = thread->next;
                        if(thread_to_delete->end_of_thread_signal != NULL) {
                            *(thread_to_delete->end_of_thread_signal) = 1; // signal end of thread if needed
                        }
                        kfree(thread_to_delete->kernel_stack);
                        kfree(thread_to_delete);
                    }
                }

                // free process resources
                kfree(process_to_delete->tid_bitmap);
                kfree(process_to_delete);

                // free virtual memory of process
                free_virtual_space(process_to_delete->page_directory_physical_address);
            }
            else {
                prev_process = current_process;
                current_process = current_process->next;
            }
        }

        process_waiting_for_deletion = false;
        mfence();
    }

    // remove all threads marked for deletion in current process
    // TODO: mark all threads created by deleted threads for deletion
    thread_list_t *current_thread_list = &lpdata->current_process->threads_on_logical_processor[lpdata->index];
    thread_t *prev_thread = NULL;
    thread_t *current_thread = current_thread_list->threads;
    while(current_thread != NULL) {
        if(current_thread->delete_me == true) {
            // remove thread from list
            if(prev_thread == NULL) {
                current_thread_list->threads = current_thread->next;
            }
            else {
                prev_thread->next = current_thread->next;
            }
            thread_t *thread_to_delete = current_thread;
            current_thread = current_thread->next;

            // if it was the current thread, move current_thread pointer
            // TODO: prevent situation when other thread deleted current thread so stack can not be freed
            if(current_thread_list->current_thread == thread_to_delete) {
                current_thread_list->current_thread = current_thread;
                lpdata->current_thread = current_thread;
            }

            // signal end of thread if needed
            if(thread_to_delete->end_of_thread_signal != NULL) {
                *(thread_to_delete->end_of_thread_signal) = 1;
                mfence();
            }

            // free thread resources
            kfree(thread_to_delete->kernel_stack);
            kfree(thread_to_delete);

            // decrease number of running threads
            lpdata->current_process->threads_on_logical_processor[lpdata->index].number_of_running_threads--;
        }
        else {
            prev_thread = current_thread;
            current_thread = current_thread->next;
        }
    }

    // wake up all sleeping threads
    wake_up_threads(current_thread_list->sleeping_threads, current_thread_list);

    // move to next thread
    process_t *original_process = lpdata->current_process;
    thread_t *original_thread = lpdata->current_thread;
    thread_t *switch_to_thread = NULL;
    do {
        // move to next process
        if(lpdata->current_process->next != NULL) {
            lpdata->current_process = lpdata->current_process->next;
        }
        else {
            lpdata->current_process = process_list; // there will always be at least the kernel process
        }
        if(lpdata->current_process->delete_me == true) {
            continue; // skip processes marked for deletion
        }

        // find next runnable thread in the selected process
        thread_list_t *thread_list = &lpdata->current_process->threads_on_logical_processor[lpdata->index];
        if(thread_list->sleeping_threads != NULL) {
            wake_up_threads(thread_list->sleeping_threads, thread_list);
        }
        if(thread_list->number_of_running_threads == 0) {
            if(original_process == lpdata->current_process) {
                break;
            }
            else {
                continue;
            }
        }
        if(thread_list->current_thread == NULL) {
            thread_list->current_thread = thread_list->threads; // there will always be at least one thread if number_of_running_threads > 0
        }
        thread_t *original_thread = thread_list->current_thread;
        do {
            // move to next thread
            if(thread_list->current_thread->next != NULL) {
                thread_list->current_thread = thread_list->current_thread->next;
            }
            else {
                thread_list->current_thread = thread_list->threads;
            }

            // check if thread is runnable
            if(thread_list->current_thread->delete_me == false) {
                switch_to_thread = thread_list->current_thread;
                break;
            }

        } while(original_thread != thread_list->current_thread); // if this happens, there are no runnable threads in this process
        if(switch_to_thread != NULL) {
            break; // found a thread to switch to
        }
    } while(original_process != lpdata->current_process); // if this happens, there is only one process with threads

    // log("\nSwitching from process %x thread %x to process %x thread %x on CPU%d",
    //     original_process->pid,
    //     (original_thread != NULL) ? original_thread->tid : 0,
    //     lpdata->current_process->pid,
    //     (switch_to_thread != NULL) ? switch_to_thread->tid : 0,
    //     lpdata->index
    // );

    // if there is no other thread to switch to, continue running the current thread
    if(switch_to_thread == NULL) {
        switch_to_thread = lpdata->current_thread;
    }

    // prepare next preemptive scheduling interrupt
    lapic_set_oneshot_interrupt(10, INTERRUPT_PREEMPETIVE_SCHEDULING); // 10 ms

    // switch to the selected thread
    extern void exit_interrupt_to_thread(uint32_t stack_pointer);
    lpdata->current_thread = switch_to_thread;
    if(lpdata->current_thread == NULL) {
        // log("\nNo thread to run on CPU%d, switching to idle thread", lpdata->index);
        exit_interrupt_to_thread((uint32_t)lpdata->idle_thread_stack);
    }
    if(original_process->page_directory_physical_address != lpdata->current_process->page_directory_physical_address) {
        load_page_directory(lpdata->current_process->page_directory_physical_address);
    }
    get_current_logical_processor_struct()->tss->esp0 = (uint32_t) switch_to_thread->kernel_stack + KERNEL_STACK_SIZE; // set kernel stack for next thread
    exit_interrupt_to_thread(lpdata->current_thread->stack_pointer);
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