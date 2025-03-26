//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct cpuid_t {
 dword_t eax;
 dword_t ebx;
 dword_t ecx;
 dword_t edx;
} cpuid;

void read_cpuid(dword_t code);
qword_t read_msr(dword_t msr_register);
void write_msr(dword_t msr_register, qword_t value);
dword_t read_cr0(void);
void write_cr0(dword_t value);
dword_t read_cr2(void);
void write_cr2(dword_t value);
dword_t read_cr3(void);
void write_cr3(dword_t value);
dword_t read_cr4(void);
void write_cr4(dword_t value);
static inline void outb(word_t port, byte_t value);
static inline void outw(word_t port, word_t value);
static inline void outd(word_t port, dword_t value);
static inline byte_t inb(word_t port);
static inline word_t inw(word_t port);
static inline dword_t ind(word_t port);
static inline void mmio_outb(dword_t base, byte_t value);
static inline void mmio_outw(dword_t base, word_t value);
static inline void mmio_outd(dword_t base, dword_t value);
static inline byte_t mmio_inb(dword_t base);
static inline word_t mmio_inw(dword_t base);
static inline dword_t mmio_ind(dword_t base);