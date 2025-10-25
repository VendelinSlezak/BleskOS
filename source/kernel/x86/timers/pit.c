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
#include <kernel/x86/interrupt_controllers/main.h>
#include <kernel/x86/cpu/idt.h>
#include <kernel/x86/scheduler/main.h>
#include <kernel/x86/entities/entities.h>

/* functions */
void initialize_pit(void) {
    pit_set_time_to_generate_interrupt(SCHEDULER_PERIOD); // every 10 ms
    connect_legacy_interrupt_to_handler(0, pit_handler);
    log("\n[PIT] PIT initialized");
}

void pit_set_time_to_generate_interrupt(dword_t microseconds) {
    // recount microseconds to number of ticks
    dword_t whole = (microseconds / 1000) * (PIT_FREQUENCY / 1000);
    dword_t frac  = (((microseconds % 1000) * PIT_FREQUENCY) / 1000000);
    word_t ticks = (((whole + frac) & 0xFFFE) + 2); // it is better to wait a little longer because PIT is not absolutely accurate

    // set kernel attribute
    kernel_attr->time_of_new_tick = microseconds;

    // send command: channel 0, access low+high, mode 3 (Square Wave Generator), 16-bit binary value
    outb(PIT_COMMAND, (0b00 << 6) | (0b11 << 4) | (0b011 << 1) | (0b0 << 0));

    // set ticks
    outb(PIT_CHANNEL0, (byte_t)(ticks & 0xFF));
    outb(PIT_CHANNEL0, (byte_t)((ticks >> 8) & 0xFF));
}

void pit_handler(void) {
    kernel_attr->kernel_time += kernel_attr->time_of_new_tick;

    switch_commands();
}