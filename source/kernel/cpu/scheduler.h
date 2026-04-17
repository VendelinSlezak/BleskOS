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

typedef struct thread_t {
    struct thread_t *next;
    uint32_t tid;
    uint32_t stack_pointer;
    uint32_t delete_me;
    void *kernel_stack;
    uint8_t *end_of_thread_signal;
    uint32_t end_of_sleep;
} thread_t;

typedef struct {
    uint32_t number_of_running_threads;
    thread_t *current_thread;
    thread_t *threads;
    thread_t *sleeping_threads;
} thread_list_t;

typedef struct process_t {
    struct process_t *next;
    uint32_t pid;
    void *window;
    void *human_input_event_stack;
    uint32_t delete_me;
    uint32_t page_directory_physical_address;
    mutex_t virtual_memory_mutex;
    uint32_t *tid_bitmap;
    thread_t *main_thread;
    thread_list_t threads_on_logical_processor[];
} process_t;

extern process_t *kernel_process;