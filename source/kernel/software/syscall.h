/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/interrupt.h>

enum {
    DEMAND_TYPE_CREATE_THREAD = 1, // ebx = entry point, ecx = stack pointer, edx = delete signal handler
    DEMAND_TYPE_SPAWN_THREAD, // ebx = entry point, ecx = delete signal handler
    DEMAND_TYPE_SWITCH_THREADS, // nothing
    DEMAND_TYPE_SLEEP_FOR_THREAD, // ebx = milliseconds to sleep
    DEMAND_TYPE_CLOSE_THREAD, // nothing
    DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE, // ebx = address, ecx = size
    DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE, // ebx = address, ecx = size
    DEMAND_TYPE_DOES_VIRTUAL_DEVICE_EXIST, // ebx = device type
    DEMAND_TYPE_SEND_COMMAND_TO_VIRTUAL_DEVICE // ebx = device id, ecx = pointer to command
};

#define syscall_create_thread(entry_point, stack_pointer, delete_signal_handler) ({ \
    int _retval;                                             \
    asm volatile (                                           \
        "int %1"                                             \
        : "=a" (_retval)                                     \
        : "i" (0xD0),                                        \
          "a" (DEMAND_TYPE_CREATE_THREAD),                   \
          "b" (entry_point),                                 \
          "c" (stack_pointer),                               \
          "d" (delete_signal_handler)                        \
        : "memory"                                           \
    );                                                       \
    _retval;                                                 \
})
#define syscall_spawn_thread(entry_point, delete_signal_handler) ({ \
    int _retval;                                                    \
    asm volatile (                                                  \
        "int %1"                                                    \
        : "=a" (_retval)                                            \
        : "i" (0xD0),                                               \
          "a" (DEMAND_TYPE_SPAWN_THREAD),                           \
          "b" (entry_point),                                        \
          "c" (delete_signal_handler)                               \
        : "memory"                                                  \
    );                                                              \
    _retval;                                                        \
})
#define syscall_switch() asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_SWITCH_THREADS))
#define syscall_sleep(microseconds) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_SLEEP_FOR_THREAD), "b" (microseconds))
#define syscall_close() asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_CLOSE_THREAD))
#define syscall_map_pages(address, size) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE), "b" (address), "c" (size))
#define syscall_unmap_pages(address, size) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE), "b" (address), "c" (size))
#define syscall_does_virtual_device_exist(type) ({      \
    int _retval;                                        \
    asm volatile (                                      \
        "int %1"                                        \
        : "=a" (_retval)                                \
        : "i" (0xD0),                                   \
          "a" (DEMAND_TYPE_DOES_VIRTUAL_DEVICE_EXIST),  \
          "b" (type)                                    \
        : "memory"                                      \
    );                                                  \
    _retval;                                            \
})
#define syscall_send_command_to_virtual_device(type, pointer) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_SEND_COMMAND_TO_VIRTUAL_DEVICE), "b" (type), "c" (pointer))