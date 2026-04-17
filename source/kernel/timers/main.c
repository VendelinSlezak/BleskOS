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
#include <kernel/timers/pit.h>
#include <kernel/timers/hpet.h>
#include <kernel/timers/tsc.h>
#include <kernel/timers/lapic.h>
#include <kernel/hardware/main.h>
#include <kernel/software/syscall.h>

/* global variables */
uint32_t timer_group_id;
uint64_t (*get_time_in_microseconds)(void);

/* functions */
void initialize_timers(void) {
    // initialize basic timer
    if(does_hpet_exist() == true) {
        if(initialize_hpet() == ERROR) {
            initialize_pit();
        }
    }
    else {
        initialize_pit();
    }

    // use TSC timer instead of basic timer if it exists
    if(does_invariable_tsc_exist() == true) {
        initialize_tsc();
    }

    // initialize LAPIC timer on bootstrap processor
    lapic_calibrate_timer();

    // add timer to hardware list
    timer_group_id = get_unique_hardware_id();
    add_device_to_hardware_list(timer_group_id, HARDWARE_TYPE_TIMER);
}

void wait_microseconds(uint32_t microseconds) {
    uint64_t wakeup_time = (*get_time_in_microseconds)() + microseconds;
    while((int)(wakeup_time - (*get_time_in_microseconds)()) > 0) {
        asm volatile("pause");
    }
}

/* userspace functions */
void timer_group_process_userspace_command(uint32_t id, uint64_t *pointer) {
    if(id != timer_group_id) {
        return;
    }
    if(return_validated_pointer(pointer, sizeof(uint64_t)) == NULL) {
        return;
    }
    *pointer = (*get_time_in_microseconds)();
}