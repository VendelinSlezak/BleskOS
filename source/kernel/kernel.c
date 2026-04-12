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
#include <libc/string.h>
#include <kernel/starting_screen.h>
#include <kernel/cpu/commands.h>
#include <kernel/cpu/initialize.h>
#include <kernel/cpu/scheduler.h>
#include <kernel/cpu/lapic.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/memory/physical_memory.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/firmware/main.h>
#include <kernel/interrupt_controller/main.h>
#include <kernel/timers/main.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/software/elf_loader.h>
#include <kernel/software/ramdisk.h>
#include <kernel/software/syscall.h>
#include <kernel/software/exceptions.h>

/* functions */
void initialize_kernel(void) {
    // initialize RAM memory
    initialize_physical_memory();
    initialize_virtual_memory();
    initialize_allocators();

    // initialize logging
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

    // initialize application processors
    initialize_all_application_processors();

    // initialize scheduler
    initialize_scheduler();

    // initialize hardware
    initialize_hardware_list();
    show_starting_screen();

    log("\nKernel initialized successfully");
    
    // initialize user space
    initialize_user_space_allocation();
    initialize_exceptions();
    initialize_syscalls();
    initialize_syslib();

    // create_kernel_thread((uint32_t)kthread_test, (uint32_t []) { 1000, 3000 }, 2);
    // create_kernel_thread((uint32_t)kthread_test, (uint32_t []) { 300, 3000 }, 2);
    // create_kernel_thread((uint32_t)kthread_test, (uint32_t []) { 100, 3000 }, 2);

    // spawning_template_t template = load_elf32_to_spawning_template(get_ramdisk_file_ptr("test.elf"));
    // create_user_process_from_spawning_template(&template);

    spawning_template_t template = load_elf32_to_spawning_template(get_ramdisk_file_ptr("dump_log.elf"), prepare_memory_for_dump_log);
    create_user_process_from_spawning_template(&template);

    while(true) {
        asm volatile("pause");
    }
}

void prepare_memory_for_dump_log(void) {
    memcpy((void *) 0xE8000000, get_ramdisk_file_ptr("ter-v16n.psf"), get_ramdisk_file_size("ter-v16n.psf"));
    copy_log_to_userspace((uint32_t *)0x80000000);
}

void kernel_panic(char *msg) {
    logging_enabled = true;
    log("\n[PANIC] %s", msg);

    log("\nCR2: %x", read_cr2());
    log("\nPD: %x", *((uint32_t *) (P_MEM_PAGE_DIRECTORY + ((read_cr2() >> 22) * 4))));

    while(true) {
        asm volatile("hlt");
    }
}

void kthread_test(uint32_t how_much_sleep, uint32_t until) {
    uint32_t sleep = 0;
    while(sleep < until) {
        log("\nHere: %d", sleep);
        sleep_current_thread(how_much_sleep);
        sleep += how_much_sleep;
    }
}