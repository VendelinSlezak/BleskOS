/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/devices/cpu/interrupt.h>

enum {
    DEMAND_TYPE_CREATE_THREAD = 1, // ebx = entry point, ecx = stack pointer, edx = delete signal handler
    DEMAND_TYPE_SPAWN_THREAD, // ebx = entry point, ecx = delete signal handler
    DEMAND_TYPE_SWITCH_THREADS, // nothing
    DEMAND_TYPE_SLEEP_FOR_THREAD, // ebx = milliseconds to sleep
    DEMAND_TYPE_CLOSE_THREAD, // nothing
    DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE, // ebx = address, ecx = size
    DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE, // ebx = address, ecx = size
    DEMAND_TYPE_SEND_DOORBELL_TO_VIRTUAL_DEVICE // ebx = device id, ecx = demand
};