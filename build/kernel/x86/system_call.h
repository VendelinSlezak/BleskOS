#ifndef BUILD_KERNEL_X86_SYSTEM_CALL_H
#define BUILD_KERNEL_X86_SYSTEM_CALL_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SC_EXIT() \
    asm volatile ( \
        "xor %%eax, %%eax\n\t" \
        "mov $1, %%ebx\n\t" \
        "int $0xA0\n\t" \
        : \
        : \
        : "%eax", "%ebx" \
    )

#define SC_SLEEP(x) \
    asm volatile ( \
        "xor %%eax, %%eax\n\t" \
        "mov $2, %%ebx\n\t" \
        "mov %0, %%ecx\n\t" \
        "int $0xA0\n\t" \
        : \
        : "r"(x) \
        : "%eax", "%ebx", "%ecx" \
    )

#define SC_SWITCH() \
    asm volatile ( \
        "xor %%eax, %%eax\n\t" \
        "mov $3, %%ebx\n\t" \
        "int $0xA0\n\t" \
        : \
        : \
        : "%eax", "%ebx" \
    )

#define SC_WAIT_FOR_COMMAND(x) \
    asm volatile ( \
        "xor %%eax, %%eax\n\t" \
        "mov $4, %%ebx\n\t" \
        "mov %0, %%ecx\n\t" \
        "int $0xA0\n\t" \
        : \
        : "r"(x) \
        : "%eax", "%ebx", "%ecx" \
    )
void system_call(void);

#endif /* BUILD_KERNEL_X86_SYSTEM_CALL_H */
