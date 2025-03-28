//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct hpet_info_t {
    dword_t port_type;
    dword_t base;
    dword_t number_of_timers;
    dword_t one_tick_nanoseconds;
    dword_t ticks_to_one_microseconds;
};

dword_t hpet_table_mem = 0, hpet_base = 0, hpet_num_of_timers = 0, hpet_one_tick_how_many_nanoseconds = 0, hpet_how_many_ticks_one_microsecond = 0;

dword_t hpet_read(dword_t reg);
void hpet_write(dword_t reg, dword_t value);
void initalize_hpet(void);
void hpet_reset_counter(void);
dword_t hpet_return_time_in_microseconds(void);