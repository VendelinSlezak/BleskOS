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
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void initialize_exceptions(void) {
    set_isr_interrupt_handler(0x00, exception_division_by_zero);
    set_isr_interrupt_handler(0x06, exception_unknown_opcode);
    set_isr_interrupt_handler(0x08, exception_double_fault);
    set_isr_interrupt_handler(0x0D, exception_general_protection_fault);
}

void close_running_thread(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->running_thread_state == SCHEDULER_STATE_KERNEL) {
        kernel_panic("Exception in kernel space", stack_of_interrupt);
    }
    close_current_thread_interrupt(stack_of_interrupt);
}

void kill_running_thread(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->running_thread_state == SCHEDULER_STATE_KERNEL) {
        kernel_panic("Exception in kernel space", stack_of_interrupt);
    }
    kill_current_thread_interrupt(stack_of_interrupt);
}

void exception_division_by_zero(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] Division by zero in process %x thread %d on CPU%d", lpdata->current_program, lpdata->current_user_thread->id, lpdata->index);
    close_running_thread(stack_of_interrupt);
}

void exception_unknown_opcode(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] Unknown opcode in process %x thread %d on CPU%d", lpdata->current_program, lpdata->current_user_thread->id, lpdata->index);
    close_running_thread(stack_of_interrupt);
}

void exception_double_fault(interrupt_stack_t *stack_of_interrupt) {
    kernel_panic("Double fault", stack_of_interrupt);
}

void exception_general_protection_fault(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] General protection fault in process %x thread %d on CPU%d", lpdata->current_program, lpdata->current_user_thread->id, lpdata->index);
    kill_running_thread(stack_of_interrupt);
}

void null_page_fault_handler(interrupt_stack_t *stack_of_interrupt) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    log("\n[EXCEPTION] Null page fault in process %x thread %d on CPU%d by EIP 0x%x", lpdata->current_program, lpdata->current_user_thread->id, lpdata->index, stack_of_interrupt->eip);
    if(lpdata->running_thread_state == SCHEDULER_STATE_KERNEL) {
        kernel_panic("Null page fault in kernel space", stack_of_interrupt);
    }
    else {
        send_closing_signal_to_program(lpdata->current_program);
        close_running_thread(stack_of_interrupt);
    }
}