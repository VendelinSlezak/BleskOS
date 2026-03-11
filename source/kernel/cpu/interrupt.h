/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    // segments
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    // general purpose registers
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t old_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // interrupt stub
    uint32_t interrupt_number;
    uint32_t error_code;

    // interrupt data
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
}__attribute__((packed)) kernel_interrupt_stack_t;

typedef struct {
    // segments
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    // general purpose registers
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t old_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // interrupt stub
    uint32_t interrupt_number;
    uint32_t error_code;

    // interrupt data
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;

    // user space data (only for user threads, will be ignored for kernel threads)
    uint32_t user_esp;
    uint32_t ss;
}__attribute__((packed)) interrupt_stack_t;