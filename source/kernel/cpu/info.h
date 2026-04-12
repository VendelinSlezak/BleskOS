/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/scheduler.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/cpu/initialize.h>

#define PROCESSOR_IS_NOT_RUNNING 0
#define PROCESSOR_IS_RUNNING 1
#define PROCESSOR_IS_UNUSABLE 2

typedef struct {
    uint32_t hardware_id;
} logical_processor_info_t;
typedef struct {
    uint32_t index;
    uint32_t is_running;

    uint32_t is_fpu_supported;
    uint32_t is_mmx_supported;
    uint32_t is_sse_supported;
    uint32_t is_sse2_supported;
    uint32_t is_sse3_supported;
    uint32_t is_ssse3_supported;
    uint32_t is_sse4_1_supported;
    uint32_t is_sse4_2_supported;
    uint32_t is_avx_supported;


    tss_t *tss;

    uint32_t depth_of_lock;

    interrupt_stack_t *idle_thread_stack;
    void *floating_stack;

    process_t *current_process;
    thread_t *current_thread;

    void *copy_on_write_page;

    uint32_t lapic_ticks_per_millisecond;
} logical_processor_t;

extern uint32_t number_of_logical_processors;
extern logical_processor_info_t *logical_processors;
extern uint32_t *logical_processor_index_number;
extern logical_processor_t *logical_processor_structs_ptr;
extern uint32_t bootstrap_processor_id;