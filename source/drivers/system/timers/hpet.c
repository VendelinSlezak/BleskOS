//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t hpet_read(dword_t reg) {
    if(components->hpet.port_type == ACPI_GAS_ADDRESS_ID_MEMORY) {
        return mmio_ind(components->hpet.base+reg);
    }
    else {
        return ind(components->hpet.base+reg);
    }
}

void hpet_write(dword_t reg, dword_t value) {
    if(components->hpet.port_type == ACPI_GAS_ADDRESS_ID_MEMORY) {
        mmio_outd(components->hpet.base+reg, value);
    }
    else {
        outd(components->hpet.base+reg, value);
    }
}

void initalize_hpet(void) {
    // read basic info
    components->hpet.number_of_timers = ((hpet_read(0x00) >> 8) & 0xF);
    components->hpet.one_tick_nanoseconds = (hpet_read(0x04)/1000000);
    if(components->hpet.one_tick_nanoseconds>100 || components->hpet.one_tick_nanoseconds==0) {
        logf("\n\nHPET: invalid nanoseconds value %d", components->hpet.one_tick_nanoseconds);
        components->p_hpet = STATUS_FALSE;
        return;
    }
    components->hpet.ticks_to_one_microseconds = (1000/components->hpet.one_tick_nanoseconds);

    // reset HPET counter and timers
    hpet_write(0x10, 0);
    hpet_write(0xF0, 0);
    hpet_write(0xF4, 0);
    for(dword_t i = 0; i < hpet_num_of_timers; i++) {
        hpet_write(0x100+i*0x20+0, 0);
    }

    // start counting
    hpet_write(0x10, 1);

    // log
    logf("\n\nHPET INFO\n Number of timers: %d \n Nanoseconds per tick %d", components->hpet.number_of_timers, components->hpet.one_tick_nanoseconds);
}

void hpet_reset_counter(void) {
    hpet_write(0x10, 0);
    hpet_write(0xF0, 0);
    hpet_write(0xF4, 0);
    hpet_write(0x10, 1);
}

dword_t hpet_return_time_in_microseconds(void) {
    return (hpet_read(0xF0)/components->hpet.ticks_to_one_microseconds);
}