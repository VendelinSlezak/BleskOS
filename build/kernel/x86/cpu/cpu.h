#ifndef BUILD_KERNEL_X86_CPU_CPU_H
#define BUILD_KERNEL_X86_CPU_CPU_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define CPU_MSR_PAT 0x277
#define CPU_MSR_MTRR_REG_CAPABILITY 0xFE
#define CPU_MSR_MTRR_REG_DEFAULT_TYPE 0x2FF
#define CPU_MSR_MTRR_REG_BASE 0x200
#define CPU_MSR_MTRR_REG_MASK 0x201

#define CPU_MEMORY_TYPE_UNCACHEABLE         0x00  // UC
#define CPU_MEMORY_TYPE_WRITE_COMBINING     0x01  // WC
#define CPU_MEMORY_TYPE_WRITE_THROUGH       0x04  // WT
#define CPU_MEMORY_TYPE_WRITE_PROTECTED     0x05  // WP
#define CPU_MEMORY_TYPE_WRITE_BACK          0x06  // WB
#define CPU_MEMORY_TYPE_UNCACHEABLE_MINUS   0x07  // UC-
void initialize_cpu(void);
void cpu_initialize_all_cores(void);

#endif /* BUILD_KERNEL_X86_CPU_CPU_H */
