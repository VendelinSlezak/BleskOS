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
#include <syslib.h>

/* local variables */
syslib_t *syslib;

/* functions */
void main(syslib_t *syslib_ptr) {
    syslib = syslib_ptr;
    uint64_t start_time, end_time, time;
    uint32_t time_of_one;

    // uint8_t *ptr = syslib->aligned_malloc(4096, 4096 * 100000);
    // start_time = syslib->get_time_in_microseconds();
    // for(int i = 0; i < 100000; i++) {
    //     ptr[i] = 1;
    //     ptr += 4096;
    // }
    // end_time = syslib->get_time_in_microseconds();
    // time = end_time - start_time;
    // time_of_one = (uint32_t)(time & 0xFFFFFFFF) / 100;
    // syslib->log("ptr: 0x%x, time of 100000 page faults: %d us, time of one page fault: %d ns", ptr, (uint32_t)(time & 0xFFFFFFFF), time_of_one);

    start_time = syslib->get_time_in_microseconds();
    for(int i = 0; i < 100; i++) {
        syscall_virtual_hardware(VIRTUAL_HARDWARE_SCREEN_ID, VH_SCREEN_DEMAND_REDRAW);
    }
    end_time = syslib->get_time_in_microseconds();
    time = end_time - start_time;
    time_of_one = (uint32_t)(time & 0xFFFFFFFF) / 100;
    syslib->log("time of 100 program redraws: %d us, time of one redraw: %d us", (uint32_t)(time & 0xFFFFFFFF), time_of_one);
}