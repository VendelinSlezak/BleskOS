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
#include <kernel/cpu/commands.h>
#include <kernel/cpu/info.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/timers/main.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
uint32_t lapic_base = NULL;

/* functions */
void enable_lapic(void) {
    // enable LAPIC in MSR
    uint64_t lapic_base_msr = read_msr(CPU_MSR_APIC_BASE);
    lapic_base_msr |= (1 << 11); // set bit 11 to enable LAPIC
    write_msr(CPU_MSR_APIC_BASE, lapic_base_msr);

    // initialize Spurious Interrupt Vector Register
    uint32_t sivr = mmio_ind(lapic_base + 0xF0);
    sivr |= 0xFF; // set spurious interrupt vector to 0xFF
    sivr |= (1 << 8); // enable LAPIC
    mmio_outd(lapic_base + 0xF0, sivr);
}

uint32_t lapic_get_processor_id(void) {
    if(lapic_base == NULL) {
        return 0x00000000;
    }
    else {
        return (mmio_ind(lapic_base + 0x20) >> 24) & 0xFF;
    }
}

void lapic_send_eoi(void) {
    if(lapic_base != NULL) {
        mmio_outd(lapic_base + 0xB0, 0x00000000);
    }
}

void lapic_assert_init_ipi(uint8_t apic_id) {
    // write to ICR high dword
    mmio_outd(lapic_base + 0x310, ((uint32_t)apic_id) << 24);

    // write to ICR low dword
    mmio_outd(lapic_base + 0x300, 0x00000500); // INIT delivery mode, level triggered, assert
}

void lapic_deassert_init_ipi(uint8_t apic_id) {
    // write to ICR high dword
    mmio_outd(lapic_base + 0x310, ((uint32_t)apic_id) << 24);

    // write to ICR low dword
    mmio_outd(lapic_base + 0x300, 0x00000400); // INIT delivery mode, level triggered, de-assert
}

void lapic_send_startup_ipi(uint8_t apic_id, uint8_t vector) {
    // write to ICR high dword
    mmio_outd(lapic_base + 0x310, ((uint32_t)apic_id) << 24);

    // write to ICR low dword
    mmio_outd(lapic_base + 0x300, 0x00000600 | vector); // STARTUP delivery mode
}

void lapic_send_ipi(uint8_t apic_id, uint8_t vector) {
    // write to ICR high dword
    mmio_outd(lapic_base + 0x310, ((uint32_t)apic_id) << 24);

    // prepare ICR low dword
    uint32_t icr_low =  (uint32_t)vector  |   // vector
                        (0b000 << 8)      |   // delivery mode: FIXED
                        (1 << 14)         |   // level: assert
                        (0 << 15)         |   // trigger: edge
                        (0 << 18);            // destination shorthand: none

    // write to ICR low dword
    mmio_outd(lapic_base + 0x300, icr_low);

    // wait until delivery is complete
    while(mmio_ind(lapic_base + 0x300) & (1 << 12)) { asm volatile("pause"); }
}