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
#include <kernel/kernel.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/cpu/scheduler.h>
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void initialize_exceptions(void) {
    set_isr_interrupt_handler(0x00, exception_division_by_zero);
    set_isr_interrupt_handler(0x06, exception_unknown_opcode);
    set_isr_interrupt_handler(0x0D, exception_general_protection_fault);
}

void kill_running_thread(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->current_process == kernel_process) {
        kernel_panic("Exception in kernel space");
    }
    close_current_thread_interrupt(stack_of_interrupt);
}

void exception_division_by_zero(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] Division by zero in process %x thread %d on CPU%d", lpdata->current_process->pid, lpdata->current_thread->tid, lpdata->index);
    kill_running_thread(stack_of_interrupt);
}

void exception_unknown_opcode(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] Unknown opcode in process %x thread %d on CPU%d", lpdata->current_process->pid, lpdata->current_thread->tid, lpdata->index);
    kill_running_thread(stack_of_interrupt);
}

void exception_general_protection_fault(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] General protection fault in process %x thread %d on CPU%d", lpdata->current_process->pid, lpdata->current_thread->tid, lpdata->index);
    kill_running_thread(stack_of_interrupt);
}