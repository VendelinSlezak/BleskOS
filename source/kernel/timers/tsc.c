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
#include <kernel/cpu/info.h>
#include <kernel/cpu/commands.h>
#include <kernel/timers/main.h>

/* local variables */
uint64_t tsc_ticks_per_microsecond;

/* functions */
uint32_t does_invariable_tsc_exist(void) {
    if(does_extended_cpuid_exist() == false) {
        return false;
    }
    if(maximal_exended_cpuid_value() < 0x80000007) {
        return false;
    }
    cpuid_t cpuid = read_cpuid(0x80000007, 0);
    if((cpuid.edx & (1 << 8)) == (1 << 8)) {
        return true;
    }
    else {
        return false;
    }
}

// TODO: use better calibration
void initialize_tsc(void) {
    if(maximal_cpuid_value() >= 0x15) {
        cpuid_t cpuid = read_cpuid(0x15, 0);
        if(cpuid.eax != 0 && cpuid.ebx != 0 && cpuid.ecx != 0) {
            tsc_ticks_per_microsecond = cpuid.ecx * cpuid.ebx / cpuid.eax / 1000000;
        }
    }
    else if(maximal_cpuid_value() >= 0x16) {
        cpuid_t cpuid = read_cpuid(0x16, 0);
        tsc_ticks_per_microsecond = cpuid.eax;
    }
    else {
        // some timer is already initialized, so we can use it to calibrate TSC
        uint64_t tsc = rdtsc();
        wait_microseconds(10000);
        tsc = rdtsc() - tsc;

        // calculate ticks per microsecond
        tsc_ticks_per_microsecond = tsc / 10000;
    }

    get_time_in_microseconds = get_tsc_time_in_microsecond;

    log("\n[TSC] Ticks per microsecond: %d", tsc_ticks_per_microsecond);
}

// TODO: use multiplication and shift
uint64_t get_tsc_time_in_microsecond(void) {
    return rdtsc() / tsc_ticks_per_microsecond;
}