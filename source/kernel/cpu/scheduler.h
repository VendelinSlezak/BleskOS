/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/interrupt.h>
#include <kernel/cpu/mutex.h>
#include <kernel/software/elf_loader.h>

#define KERNEL_STACK_SIZE 4096

#define INTERRUPT_SLEEP_FOR_THREAD 0xFB
#define INTERRUPT_PREEMPETIVE_SCHEDULING 0xFC
#define INTERRUPT_VECTOR_EXIT_THREAD 0xFD
#define INTERRUPT_VECTOR_SCHEDULER 0xFE

typedef enum {
    SCHEDULER_STATE_KERNEL = 0,
    SCHEDULER_STATE_USER = 1,
    SCHEDULER_STATE_IDLE = 2
} scheduler_state_t;

typedef struct kernel_thread_t {
    struct kernel_thread_t *next;
    uint32_t id;

    uint32_t page_directory_physical_address;
    void *kernel_stack;
    uint32_t kernel_stack_pointer;

    uint8_t kill_me;
    uint8_t sleeping;

    uint32_t time_end_of_sleep;
} kernel_thread_t;

typedef struct {
    uint32_t number_of_running_threads;
    uint32_t tid_bitmap;
    kernel_thread_t *current_thread;
    kernel_thread_t *threads;
} kernel_thread_list_t;

typedef struct user_thread_t {
    struct user_thread_t *next;
    uint32_t page_directory_physical_address;
    uint32_t *number_of_threads_in_page_directory;
    uint32_t creation_thread_id;
    uint32_t id;
    
    void *kernel_stack;
    uint32_t kernel_stack_pointer;

    uint8_t delete_me;
    uint8_t delete_signal_running;
    uint8_t kill_me;
    uint8_t sleeping;

    uint32_t delete_signal_handler_address;

    uint32_t time_end_of_sleep;
} user_thread_t;

typedef struct {
    uint32_t number_of_running_threads;
    uint32_t tid_bitmap;
    user_thread_t *current_thread;
    user_thread_t *threads;
} user_thread_list_t;

typedef struct program_t {
    struct program_t *next;

    void *template;
    void *window;
    uint32_t page_directory_for_human_input_event_stack;
    void *human_input_event_stack;

    uint32_t number_of_threads;
    user_thread_list_t thread_list_on_logical_processor[];
} program_t;