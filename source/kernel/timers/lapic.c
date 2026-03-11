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
#include <kernel/cpu/commands.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/lapic.h>
#include <kernel/timers/main.h>

/* functions */
void lapic_calibrate_timer(void) {
    // check frequency with different dividers and choose the best one
    uint32_t lapic_timer_frequency_hz = get_lapic_timer_frequency_hz(0xA); // 128 ticks
    if(lapic_timer_frequency_hz < 1000) {
        lapic_timer_frequency_hz = get_lapic_timer_frequency_hz(0x3); // 16 ticks
        if(lapic_timer_frequency_hz < 1000) {
            lapic_timer_frequency_hz = get_lapic_timer_frequency_hz(0xB); // 1 tick
            if(lapic_timer_frequency_hz < 1000) {
                log("\n[CPU%d] Warning: LAPIC timer frequency is below 1 Hz", get_current_logical_processor_struct()->index);
                lapic_timer_frequency_hz = 1000;
            }
        }
    }

    // calculate ticks per microsecond
    get_current_logical_processor_struct()->lapic_ticks_per_millisecond = lapic_timer_frequency_hz / 1000;

    // log LAPIC timer frequency
    log("\n[CPU%d] LAPIC timer frequency: %d Hz", get_current_logical_processor_struct()->index, lapic_timer_frequency_hz);
}

uint32_t get_lapic_timer_frequency_hz(uint32_t divider) {
    // mask timer
    mmio_outd(lapic_base + 0x320, (1 << 16));

    // set timer divider
    mmio_outd(lapic_base + 0x3E0, divider);

    // set timer to one-shot mode and masked
    mmio_outd(lapic_base + 0x320, (0 << 17) | (1 << 16));

    // set initial count to maximum value
    mmio_outd(lapic_base + 0x380, 0xFFFFFFFF);

    // wait for 10 ms
    wait_microseconds(10000);

    // read current count
    uint32_t current_count = mmio_ind(lapic_base + 0x390);

    // calculate elapsed ticks
    uint32_t elapsed_ticks = 0xFFFFFFFF - current_count;

    // calculate LAPIC timer frequency
    return (elapsed_ticks * 100);
}

void lapic_set_oneshot_interrupt(uint32_t milliseconds, uint8_t vector) {
    // mask timer
    mmio_outd(lapic_base + 0x320, (1 << 16));

    // set initial count to calculated ticks
    mmio_outd(lapic_base + 0x380, get_current_logical_processor_struct()->lapic_ticks_per_millisecond * milliseconds);

    // set timer to one-shot mode and unmasked with given vector
    mmio_outd(lapic_base + 0x320, vector);
}