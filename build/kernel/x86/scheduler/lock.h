#ifndef BUILD_KERNEL_X86_SCHEDULER_LOCK_H
#define BUILD_KERNEL_X86_SCHEDULER_LOCK_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/x86/kernel.h>
#include <kernel/x86/system_call.h>

#define MUTEX_UNLOCKED_VALUE 0
#define MUTEX_LOCKED_VALUE 1
typedef struct {
    dword_t lock;
    dword_t locking_command;
}__attribute__((packed)) mutex_t;

// TODO: interrupts mutex
#define LOCK_MUTEX(memory)                \
    do {                                           \
        int expected;                             \
        for (;;) {                                 \
            expected = MUTEX_UNLOCKED_VALUE;                         \
            asm volatile (                         \
                "lock cmpxchgl %2, %1"            \
                : "=a" (expected), "+m" (((mutex_t *)memory)->lock) \
                : "r"(MUTEX_LOCKED_VALUE), "0" (expected)        \
                : "memory"                        \
            );                                     \
            if (expected == MUTEX_UNLOCKED_VALUE) { \
                ((mutex_t *)memory)->locking_command = kernel_attr->running_command; \
                break; \
            } \
            SC_SWITCH(); \
        }                                          \
    } while(0)

#define UNLOCK_MUTEX(memory) \
    do {                                   \
        ((mutex_t*)(memory))->locking_command = INVALID;                      \
        asm volatile ("mfence" ::: "memory");                           \
        asm volatile (                                                  \
            "movl %1, %0"                                               \
            : "=m"(((mutex_t*)(memory))->lock)                          \
            : "r"(MUTEX_UNLOCKED_VALUE)                                 \
            : "memory"                                                  \
        );                                                              \
    } while(0)

extern dword_t core_lock_level;
extern dword_t is_interrupt;
void lock_cpu(void);
void unlock_cpu(void);
void lock_core(void);
void unlock_core(void);

#endif /* BUILD_KERNEL_X86_SCHEDULER_LOCK_H */
