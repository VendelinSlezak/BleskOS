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
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/cpu/cpu.h>
#include <kernel/x86/cpu/lapic.h>
#include <kernel/x86/cpu/isr.h>
#include <kernel/x86/cmos/cmos.h>
#include <kernel/x86/acpi/main.h>
#include <kernel/x86/interrupt_controllers/main.h>
#include <kernel/x86/timers/main.h>
#include <kernel/x86/scheduler/main.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/scheduler/elf_loader.h>
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/ramdisk/ramdisk.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/stdio.h>
#include <kernel/x86/system_call.h>
#include <kernel/x86/hardware/initialization.h>

#include <hardware/groups/logging/logging.h>

#include <software/ramdisk_programs/main_window/main_window.h>

/* global variables */
e_kernel_attr_t *kernel_attr;

/* functions */
void initialize_kernel(void) {
    // INITALIZATION OF FUNDAMENTAL INTERFACE
    core_lock_level = 1; // interrupts are disabled from bootloader
    initialize_physical_memory();
    initialize_virtual_memory();
    initialize_entities();
    initialize_logging_group();
    log("\nBleskOS 2025 kernel");    
    log_physical_memory_map();
    read_acpi();
    initialize_cpu();
    initialize_scheduler();
    read_cmos();
    initialize_interrupt_controller();
    initialize_timer();
    cpu_initialize_all_cores();

    // INITIALIZATION OF CONNECTED HARDWARE
    initialize_hardware();

    // INIALIZATION OF SOFTWARE INTERFACES
    // TODO:
    log("\n[KERNEL] Booting is complete");
    enable_interrupts();

    // START MAIN WINDOW
    main_window();
}

void kernel_panic(byte_t *str) {
    command_t *command = command_get_ptr(kernel_attr->running_command);
    entity_t *entity = entity_get_ptr(command->entity);
    log("\nKERNEL PANIC\n Entity %d: %s\n Command: %d\n Cause: %s", command->entity, entity->name, kernel_attr->running_command, str);

    while(1) {
        asm volatile ("hlt");
    }
}

void kernel_core_entry(void) {
    log("\n[CPU] Message from core 0x%02x: Entry point", lapic_get_core_id());
}