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
#include <kernel/libc/string.h>
#include <kernel/software/starting_screen.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/cpu/initialize.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/cpu/lapic.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/memory/physical_memory.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/firmware/main.h>
#include <kernel/firmware/cmos/cmos.h>
#include <kernel/hardware/devices/interrupt_controllers/main.h>
#include <kernel/hardware/devices/timers/main.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/software/elf_loader.h>
#include <kernel/software/ramdisk.h>
#include <kernel/software/syscall.h>
#include <kernel/software/exceptions.h>
#include <kernel/software/syslib.h>

/* functions */
void initialize_kernel(void) {
    // initialize RAM memory
    initialize_physical_memory();
    initialize_virtual_memory();
    initialize_allocators();

    // initialize logging
    initialize_hardware_structs();
    initialize_logging();
    logging_enabled = true;

    // read all informations about hardware from firmware
    read_firmware_data();

    // initialize bootstrap processor
    initialize_cpu_structures();
    initialize_bootstrap_processor();

    // initialize interrupt controllers
    initialize_interrupt_controllers();

    // initialize timers for processors
    initialize_timers();
    start_counting_time();

    // initialize application processors
    initialize_all_application_processors();

    // initialize scheduler
    initialize_scheduler();

    // initialize hardware
    initialize_hardware_list();
    show_starting_screen();

    // initialize user space
    initialize_user_space_allocation();
    initialize_exceptions();
    initialize_syscalls();
    initialize_syslib();

    log("\nKernel initialized successfully");

    initialize_screen_subsystem();

    close_current_thread();
}

void kernel_panic(char *msg, interrupt_stack_t *stack_of_interrupt) {
    logging_enabled = true;
    log("\n[PANIC] %s", msg);

    log("\nCR2: %x", read_cr2());
    log("\nPT: %x", *((uint32_t *) (P_MEM_PAGE_DIRECTORY + ((read_cr2() >> 22) * 4))));
    log("\nPD: %x", read_cr3());
    log("\nEIP: %x", stack_of_interrupt->eip);

    while(true) {
        asm volatile("hlt");
    }
}