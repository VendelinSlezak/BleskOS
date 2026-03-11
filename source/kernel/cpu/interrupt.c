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
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/interrupt_controller/main.h>
#include <kernel/interrupt_controller/pic.h>
#include <kernel/interrupt_controller/apic.h>
#include <kernel/cpu/lapic.h>
#include <kernel/cpu/info.h>
#include <kernel/kernel.h>

/* local variables */
void (*interrupt_handlers[256])(interrupt_stack_t *stack_of_interrupt);

/* functions */
void global_interrupt_handler(uint32_t interrupt_number, interrupt_stack_t *stack_of_interrupt) {
    // log("\n[INTERRUPT] Interrupt number: %x %x handler: %x", interrupt_number, stack_of_interrupt, interrupt_handlers[interrupt_number]);

    // call specific interrupt handler
    if(interrupt_handlers[interrupt_number] != NULL) {
        interrupt_handlers[interrupt_number](stack_of_interrupt);
    }
    else {
        log("\n[INTERRUPT] No handler for interrupt number %x with error code %x on CPU%d", interrupt_number, stack_of_interrupt->error_code, get_current_logical_processor_index());
    }

    // send End of Interrupt (EOI) signal to PIC or APIC
    if(interrupt_mode == INTERRUPTS_THROUGH_PIC && interrupt_number >= 0x20 && interrupt_number <= 0x2F) {
        pic_send_eoi(interrupt_number - 0x20);
    }
    else if(interrupt_mode == INTERRUPTS_THROUGH_APIC && ((interrupt_number >= 0x20 && interrupt_number <= 0x80) || interrupt_number == 0xD0)) {
        lapic_send_eoi();
    }
}

void register_interrupt_handler(uint32_t interrupt_number, void (*handler)(interrupt_stack_t *stack_of_interrupt)) {
    interrupt_handlers[interrupt_number] = handler;
    asm volatile("mfence" : : : "memory");
}

void set_isr_interrupt_handler(uint32_t isr_number, void (*handler)(interrupt_stack_t *stack_of_interrupt)) {
    interrupt_handlers[isr_number] = handler;
}

void set_isa_interrupt_handler(uint32_t isa_interrupt_number, void (*handler)(interrupt_stack_t *stack_of_interrupt)) {
    interrupt_handlers[isa_interrupt_number + 0x20] = handler;
    interrupt_controller_enable_isa_interrupt(isa_interrupt_number);
}

// TODO: add locking for interrupts
void lock_core(void) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->depth_of_lock == 0) {
        asm volatile("cli");
    }
    lpdata->depth_of_lock++;
}

void unlock_core(void) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    if(lpdata->depth_of_lock > 0) {
        lpdata->depth_of_lock--;
        if(lpdata->depth_of_lock == 0) {
            asm volatile("sti");
        }
    }
}