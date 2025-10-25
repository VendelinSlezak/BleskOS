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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/cpu/isr.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/system_call.h>
#include <kernel/x86/scheduler/lock.h>

/* global variables */
void (*idt_functions[NUMBER_OF_IDT_ENTRIES])(void);

/* functions */
void initialize_idt(void) {
    // allocate page for IDT
    vm_alloc_page(P_MEM_IDT, VM_FLAGS_KERNEL_RW); // TODO: remove

    // set functions for entries
    memset(idt_functions, 0, sizeof(idt_functions));
    idt_functions[0]  = isr0_handler_divide_by_zero;
    idt_functions[1]  = isr1_handler_debug;
    idt_functions[2]  = isr2_handler_nmi;
    idt_functions[3]  = isr3_handler_breakpoint;
    idt_functions[4]  = isr4_handler_overflow;
    idt_functions[5]  = isr5_handler_bound_range_exceeded;
    idt_functions[6]  = isr6_handler_invalid_opcode;
    idt_functions[7]  = isr7_handler_device_not_available;
    idt_functions[8]  = isr8_handler_double_fault;
    idt_functions[9]  = isr9_handler_coprocessor_segment_overrun;
    idt_functions[10] = isr10_handler_invalid_tss;
    idt_functions[11] = isr11_handler_segment_not_present;
    idt_functions[12] = isr12_handler_stack_segment_fault;
    idt_functions[13] = isr13_handler_general_protection_fault;
    idt_functions[14] = isr14_handler_page_fault;
    idt_functions[16] = isr16_handler_x87_fpu_error;
    idt_functions[17] = isr17_handler_alignment_check;
    idt_functions[18] = isr18_handler_machine_check;
    idt_functions[19] = isr19_handler_simd_fpu_exception;
    idt_functions[160] = system_call;

    // set all entries in IDT table
    extern idt_entry_t protected_mode_idt[];
    extern void setup_idt(void);
    setup_idt();
    protected_mode_idt[0xA0].gate_type = INTERRUPT_GATE_32_BIT_USER; // system call interrupt is available also in user mode

    // load IDT table
    extern idt_wrap_t protected_mode_idt_wrap;
    protected_mode_idt_wrap.size = NUMBER_OF_IDT_ENTRIES*8-1;
    protected_mode_idt_wrap.idt_address = (dword_t) &protected_mode_idt;
    lidt((dword_t)&protected_mode_idt_wrap);

    // log
    log("\n[IDT] IDT was loaded");
}

void global_interrupt_handler(dword_t interrupt_number) {
    // handle spurious interrupts
    if(interrupt_number == 32+7) {
        outb(0x20, 0x0B);
        if((inb(0x20) & 0x80) == 0x00) {
            return;
        }
    }
    if(interrupt_number == 32+15) {
        outb(0xA0, 0x0B);
        if((inb(0xA0) & 0x80) == 0x00) {
            outb(0xA0, 0x20);
            return;
        }
    }

    // send End Of Interrupt to PIC if scheduler interrupt is raised, because it will not return here
    if(interrupt_number == 32) {
        outb(0x20, 0x20);
    }

    // call interrupt method
    if(idt_functions[interrupt_number] != 0) {
        is_interrupt = TRUE;
        idt_functions[interrupt_number]();
        is_interrupt = FALSE;
    }
    else {
        log("\nUnknown interrupt %d fired", interrupt_number);
    }

    // send End Of Interrupt to PIC
    if(interrupt_number > 32 && interrupt_number <= 32+8) {
        outb(0x20, 0x20);
    }
    else if(interrupt_number >= 32+8 && interrupt_number <= 32+8+8) {
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }
}