/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MUTEX_UNLOCKED_VALUE 0
#define MUTEX_LOCKED_VALUE 1
typedef struct {
    uint32_t lock;
}__attribute__((packed)) mutex_t;

#define LOCK_MUTEX(memory)                                              \
    do {                                                                \
        int expected;                                                   \
        for (;;) {                                                      \
            expected = MUTEX_UNLOCKED_VALUE;                            \
            asm volatile (                                              \
                "lock cmpxchgl %2, %1"                                  \
                : "=a" (expected), "+m" (((mutex_t *)memory)->lock)     \
                : "r"(MUTEX_LOCKED_VALUE), "0" (expected)               \
                : "memory"                                              \
            );                                                          \
            if (expected == MUTEX_UNLOCKED_VALUE) {                     \
                break;                                                  \
            }                                                           \
            asm volatile("pause");                                        \
        }                                                               \
    } while(0)

#define UNLOCK_MUTEX(memory)                    \
    do {                                        \
        asm volatile ("mfence" ::: "memory");   \
        asm volatile (                          \
            "movl %1, %0"                       \
            : "=m"(((mutex_t *)memory)->lock)   \
            : "r"(MUTEX_UNLOCKED_VALUE)         \
            : "memory"                          \
        );                                      \
    } while(0)