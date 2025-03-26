//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_cpuid(dword_t code) {
    // mov eax, code
    // cpuid
    // mov cpuid.eax, eax
    // mov cpuid.ebx, ebx
    // mov cpuid.ecx, ecx
    // mov cpuid.edx, edx
    asm volatile("cpuid" : "=a" (cpuid.eax), "=b" (cpuid.ebx), "=c" (cpuid.ecx), "=d" (cpuid.edx) : "a" (code));
}

qword_t read_msr(dword_t msr_register) {
    dword_t high, low;

    // mov ecx, msr_register
    // rdmsr
    // mov high, edx
    // mov low, eax
    asm volatile("rdmsr" : "=d" (high), "=a" (low) : "c" (msr_register));

    qword_t high_64_bit = high;
    return ((high_64_bit << 32) | low);
}

void write_msr(dword_t msr_register, qword_t value) {
    dword_t low = (value & 0xFFFFFFFF);
    dword_t high = (value >> 32);

    // mov ecx, msr_register
    // mov eax, low
    // mov edx, high
    // wrmsr
    asm volatile("wrmsr" : : "c" (msr_register), "a" (low), "d" (high));
}

dword_t read_cr0(void) {
    dword_t value;

    // mov (register), cr0
    // mov value, (register)
    asm volatile("mov %%cr0, %0" : "=r" (value));

    return value;
}

void write_cr0(dword_t value) {
    // mov (register), value
    // mov cr0, (register)
    asm volatile ("mov %0, %%cr0" : : "r" (value));
}

dword_t read_cr2(void) {
    dword_t value;

    // mov (register), cr2
    // mov value, (register)
    asm volatile("mov %%cr2, %0" : "=r"(value));

    return value;
}

void write_cr2(dword_t value) {
    // mov (register), value
    // mov cr2, (register)
    asm volatile("mov %0, %%cr2" : : "r"(value));
}

dword_t read_cr3(void) {
    dword_t value;

    // mov (register), cr3
    // mov value, (register)
    asm volatile("mov %%cr3, %0" : "=r"(value));

    return value;
}

void write_cr3(dword_t value) {
    // mov (register), value
    // mov cr3, (register)
    asm volatile("mov %0, %%cr3" : : "r"(value));
}

dword_t read_cr4(void) {
    dword_t value;

    // mov (register), cr4
    // mov value, (register)
    asm volatile("mov %%cr4, %0" : "=r"(value));

    return value;
}

void write_cr4(dword_t value) {
    // mov (register), value
    // mov cr4, (register)
    asm volatile("mov %0, %%cr4" : : "r"(value));
}

static inline void outb(word_t port, byte_t value) {
    // outb value, port
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(word_t port, word_t value) {
    // outw value, port
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outd(word_t port, dword_t value) {
    // outl value, port
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline byte_t inb(word_t port) {
    byte_t value;
    // inb value, port
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline word_t inw(word_t port) {
    word_t value;
    // inw value, port
    asm volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline dword_t ind(word_t port) {
    dword_t value;
    // inl value, port
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void mmio_outb(dword_t base, byte_t value) {
    *(volatile byte_t *) base = value;
}

static inline void mmio_outw(dword_t base, word_t value) {
    *(volatile word_t *) base = value;
}

static inline void mmio_outd(dword_t base, dword_t value) {
    *(volatile dword_t *) base = value;
}

static inline byte_t mmio_inb(dword_t base) {
    return *(volatile byte_t *) base;
}

static inline word_t mmio_inw(dword_t base) {
    return *(volatile word_t *) base;
}

static inline dword_t mmio_ind(dword_t base) {
    return *(volatile dword_t *) base;
}