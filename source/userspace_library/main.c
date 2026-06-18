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
#include <kernel/hardware/main.h>
#include <kernel/software/syslib.h>
#include <kernel/software/syscall.h>

#include <userspace_library/time.h>
#include <userspace_library/gui.h>
#include <userspace_library/logging.h>
#include <userspace_library/bleskalloc.h>

/* global variables */
syslib_t *syslib = NULL;

/* functions */
void syslib_initialize(syslib_t *syslib_ptr) {
    syslib = syslib_ptr;

    does_timer_exist = syscall_does_virtual_device_exist(VIRTUAL_HARDWARE_TIMER);
    does_logger_exist = syscall_does_virtual_device_exist(VIRTUAL_HARDWARE_LOGGER);
    does_window_subsystem_exist = syscall_does_virtual_device_exist(VIRTUAL_HARDWARE_WINDOW);
    does_human_input_exist = syscall_does_virtual_device_exist(VIRTUAL_HARDWARE_HUMAN_INPUT_DEVICE);

    syslib_initialize_bleskalloc();
}