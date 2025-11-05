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

/* functions */
dword_t lapic_get_core_id(void) {
    return ((mmio_ind(P_MEM_LAPIC + LAPIC_ICR_ID) >> 24) & 0xFF);
}

void lapic_send_ipi(byte_t target_core_id, byte_t interrupt_type, byte_t vector) {
    // set core ID
    mmio_outd(P_MEM_LAPIC + LAPIC_ICR_HIGH, target_core_id << 24);

    // send interrupt
    dword_t icr_low = (0x00004000) | (interrupt_type << 8) | (vector << 0); // assert interrupt
    mmio_outd(P_MEM_LAPIC + LAPIC_ICR_LOW, icr_low);

    // wait for acknowledgement of interrupt
    // TODO: timeout
    while(mmio_ind(P_MEM_LAPIC + LAPIC_ICR_LOW) & LAPIC_ICR_SEND_PENDING) {}
}