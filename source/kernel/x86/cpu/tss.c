/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/vm_allocator.h>

/* functions */
void initialize_tss(void) {
    // allocate TSS page
    vm_alloc_page(P_MEM_TSS, VM_FLAGS_KERNEL_RW);

    // set TSS
    tss_t *tss = (tss_t *) P_MEM_TSS;
    tss->esp0 = P_MEM_KERNEL_STACK + PAGE_SIZE;
    tss->ss0 = 0x10;
    tss->iomap_base = sizeof(tss_t);

    // load TSS
    word_t tss_selector = 0x28;
    asm volatile("ltr %0" :: "r"(tss_selector));

    // log
    log("\n[TSS] TSS was loaded");
}