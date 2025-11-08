#ifndef BUILD_KERNEL_X86_CPU_COMMANDS_H
#define BUILD_KERNEL_X86_CPU_COMMANDS_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    dword_t eax;
    dword_t ebx;
    dword_t ecx;
    dword_t edx;
} cpuid_t;

extern cpuid_t cpuid;

#define outb_set_bits(port, value) outb(port, inb(port) | value)
#define outw_set_bits(port, value) outw(port, inw(port) | value)
#define outd_set_bits(port, value) outd(port, ind(port) | value)

#define outb_clear_bits(port, value) outb(port, inb(port) & ~value)
#define outw_clear_bits(port, value) outw(port, inw(port) & ~value)
#define outd_clear_bits(port, value) outd(port, ind(port) & ~value)

#define mmio_outb_set_bits(port, value) mmio_outb(port, mmio_inb(port) | value)
#define mmio_outw_set_bits(port, value) mmio_outw(port, mmio_inw(port) | value)
#define mmio_outd_set_bits(port, value) mmio_outd(port, mmio_ind(port) | value)

#define mmio_outb_clear_bits(port, value) mmio_outb(port, mmio_inb(port) & ~value)
#define mmio_outw_clear_bits(port, value) mmio_outw(port, mmio_inw(port) & ~value)
#define mmio_outd_clear_bits(port, value) mmio_outd(port, mmio_ind(port) & ~value)
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
void invlpg(dword_t page);
void lgdt(dword_t gdt_wrap);
void lidt(dword_t idt_wrap);
void cli(void);
void sti(void);
void outb(word_t port, byte_t value);
void outw(word_t port, word_t value);
void outd(word_t port, dword_t value);
byte_t inb(word_t port);
word_t inw(word_t port);
dword_t ind(word_t port);
void mmio_outb(dword_t base, byte_t value);
void mmio_outw(dword_t base, word_t value);
void mmio_outd(dword_t base, dword_t value);
byte_t mmio_inb(dword_t base);
word_t mmio_inw(dword_t base);
dword_t mmio_ind(dword_t base);

#endif /* BUILD_KERNEL_X86_CPU_COMMANDS_H */
