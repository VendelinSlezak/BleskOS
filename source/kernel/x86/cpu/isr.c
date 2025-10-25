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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/memory/vm_allocator.h>

/* functions */
void isr0_handler_divide_by_zero(void) {
    kernel_panic("ISR0: Division by zero");
}

void isr1_handler_debug(void) {
    kernel_panic("ISR1: Debug (single step)");
}

void isr2_handler_nmi(void) {
    kernel_panic("ISR2: Non-maskable interrupt");
}

void isr3_handler_breakpoint(void) {
    kernel_panic("ISR3: Breakpoint");
}

void isr4_handler_overflow(void) {
    kernel_panic("ISR4: Overflow");
}

void isr5_handler_bound_range_exceeded(void) {
    kernel_panic("ISR5: Bound range exceeded");
}

void isr6_handler_invalid_opcode(void) {
    kernel_panic("ISR6: Invalid opcode");
}

void isr7_handler_device_not_available(void) {
    kernel_panic("ISR7: Device not available (No math coprocessor)");
}

void isr8_handler_double_fault(void) {
    kernel_panic("ISR8: Double fault");
}

void isr9_handler_coprocessor_segment_overrun(void) {
    kernel_panic("ISR9: Coprocessor segment overrun");
}

void isr10_handler_invalid_tss(void) {
    kernel_panic("ISR10: Invalid TSS");
}

void isr11_handler_segment_not_present(void) {
    kernel_panic("ISR11: Segment not present");
}

void isr12_handler_stack_segment_fault(void) {
    kernel_panic("ISR12: Stack segment fault");
}

void isr13_handler_general_protection_fault(void) {
    kernel_panic("ISR13: General protection fault");
}

void isr14_handler_page_fault(void) {
    dword_t page_fault = read_cr2();
        log("\n[PAGE FAULT] At 0x%x", page_fault);
    dword_t *page_directory_entry = (dword_t *) (P_MEM_PAGE_DIRECTORY + (page_fault >> 22));
    dword_t *page_table_entry = (dword_t *) (P_MEM_PAGE_TABLE + (page_fault >> 12));
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        *page_directory_entry = ((dword_t)pm_alloc_page() | VM_FLAGS_PAGE_TABLE);
    }
    if((*page_table_entry & (0x7 << 9)) == VM_FLAG_TYPE_ERROR_BY_ACCESS) {
        log("\n[COMMAND] Unallowed memory access by command %d", kernel_attr->running_command);
        kernel_panic("Command tried to manipulate with error memory area");
        // TODO: do not allocate, kill command
    }
    *page_table_entry = ((dword_t)pm_alloc_page() | VM_FLAGS_USER_RW | VM_FLAG_PRESENT);
    invlpg(page_fault & 0xFFFFF000); // TODO: is this needed?

        // DEBUG
        kernel_panic("Page fault");
}

// ISR15 je rezervované – neimplementuje sa

void isr16_handler_x87_fpu_error(void) {
    kernel_panic("ISR16: x87 FPU floating-point error");
}

void isr17_handler_alignment_check(void) {
    kernel_panic("ISR17: Alignment check");
}

void isr18_handler_machine_check(void) {
    kernel_panic("ISR18: Machine check");
}

void isr19_handler_simd_fpu_exception(void) {
    kernel_panic("ISR19: SIMD floating-point exception");
}

// ISR20 až ISR31 sú rezervované – neimplementujú sa