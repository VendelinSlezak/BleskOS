/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes*/
#include <kernel/cpu/lapic.h>

/* functions */
void core_mutex_lock(core_mutex_t *mutex) {
    int current_core_id = lapic_get_processor_id();
    if(__atomic_load_n(mutex, __ATOMIC_RELAXED) == current_core_id) {
        return; 
    }

    int expected;
    do {
        expected = CORE_MUTEX_UNLOCKED;
    } while(!__atomic_compare_exchange_n(
                mutex,
                &expected,
                current_core_id, 
                false,
                __ATOMIC_ACQUIRE,
                __ATOMIC_RELAXED
            ));
}

void core_mutex_unlock(core_mutex_t *mutex) {
    int current_core_id = lapic_get_processor_id();
    if(__atomic_load_n(mutex, __ATOMIC_RELAXED) == current_core_id) {
        __atomic_store_n(mutex, CORE_MUTEX_UNLOCKED, __ATOMIC_RELEASE);
    }
}