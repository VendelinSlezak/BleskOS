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
#include <kernel/hardware/main.h>
#include <kernel/software/syscall.h>

#include <userspace_library/time.h>
#include <userspace_library/logging.h>
#include <userspace_library/bleskalloc.h>

/* global variables */
virtual_hardware_t *virtual_hardware;

/* functions */
void initialize(virtual_hardware_t *virtual_hardware_ptr) {
    virtual_hardware = virtual_hardware_ptr;
    virtual_hardware_ptr->consumer = 0;
    virtual_hardware_ptr->producer = 0;
    syscall_virtual_hardware(VIRTUAL_HARDWARE_HUMAN_INPUT_ID, VH_HUMAN_INPUT_DEMAND_ENABLE_STREAMING);
}