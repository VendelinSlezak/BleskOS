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
#include <kernel/x86/interrupt_controllers/pic.h>
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/scheduler/lock.h>

#include <hardware/controllers/pci/device.h>

/* global functions */
void (*connect_legacy_interrupt_to_handler)(dword_t irq, void (*handler)(void));

/* functions */
void initialize_interrupt_controller(void) {
    // TODO: check if is APIC available
    // TODO: if yes, initialize APIC

    initialize_pic();
    enable_interrupts();
}

void connect_pci_device_interrupt_to_handler(pci_device_info_t pci_device, void (*handler)(void)) {
    // TODO: parse DSDT for GSI
    // TODO: use MSI if available

    connect_legacy_interrupt_to_handler(pci_device.interrupt_line, handler);
}

void enable_interrupts(void) {
    core_lock_level = 0;
    sti();
}

void disable_interrupts(void) {
    cli();
    core_lock_level = 0;
}