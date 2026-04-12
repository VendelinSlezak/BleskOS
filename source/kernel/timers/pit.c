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
#include <kernel/cpu/interrupt.h>
#include <kernel/timers/main.h>

/* local variables */
uint64_t pit_interrupts = 0;

/* functions */
void initialize_pit(void) {
    // stop interrupts to avoid breaking initialization
    lock_core();

    // connect interrupt handler
    set_isa_interrupt_handler(0, pit_periodic_interrupt);

    // set interrupt every 65536 ticks = 54925 microseconds
    outb(0x43, 0x34);
    outb(0x40, 0xFF);
    outb(0x40, 0xFF);

    // re-enable interrupts
    unlock_core();

    // set time function
    get_time_in_microseconds = get_pit_time_in_microseconds;

    log("\n[PIT] PIT timer initialized");
}

void pit_periodic_interrupt(interrupt_stack_t *stack_of_interrupt) {
    pit_interrupts++;
}

// TODO: implement absolutely correct couting of elapsed time
uint64_t get_pit_time_in_microseconds(void) {
    uint64_t interrupts;
    uint16_t ticks_after_interrupt;
    while(true) {
        // 32 bit system reads 64 bit value in 2 steps, so locking is required
        lock_core();
        interrupts = pit_interrupts;
        unlock_core();

        // get PIT ticks count after last interrupt
        outb(0x43, 0x00);
        uint8_t low = inb(0x40);
        uint8_t high = inb(0x40);
        ticks_after_interrupt = (0xFFFF - ((high << 8) | low));

        // 32 bit system reads 64 bit value in 2 steps, so locking is required
        lock_core();
        if(interrupts == pit_interrupts) {
            unlock_core();
            break;
        }
        unlock_core();
    }

    // calculate microseconds
    uint64_t microseconds = interrupts * 54925; // 54925 microseconds per interrupt
    microseconds += (((uint32_t)ticks_after_interrupt * 54925) >> 16); // calculate microseconds after interrupt

    return microseconds;
}