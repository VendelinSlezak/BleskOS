/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* functions */
cpuid_t read_cpuid(uint32_t function_id, uint32_t subfunction_id) {
    cpuid_t result;

    // mov eax, function_id
    // mov ecx, subfunction_id
    // cpuid
    // mov result.eax, eax
    // mov result.ebx, ebx
    // mov result.ecx, ecx
    // mov result.edx, edx
    asm volatile("cpuid"
                 : "=a" (result.eax), "=b" (result.ebx), "=c" (result.ecx), "=d" (result.edx)
                 : "a" (function_id), "c" (subfunction_id));

    return result;
}

// TODO: safe read with catching GPF
uint64_t read_msr(uint32_t msr_register) {
    uint32_t high, low;

    // mov ecx, msr_register
    // rdmsr
    // mov high, edx
    // mov low, eax
    asm volatile("rdmsr" : "=d" (high), "=a" (low) : "c" (msr_register));

    uint64_t high_64_bit = high;
    return ((high_64_bit << 32) | low);
}

void write_msr(uint32_t msr_register, uint64_t value) {
    uint32_t low = (value & 0xFFFFFFFF);
    uint32_t high = (value >> 32);

    // mov ecx, msr_register
    // mov eax, low
    // mov edx, high
    // wrmsr
    asm volatile("wrmsr" : : "c" (msr_register), "a" (low), "d" (high));
}

uint32_t read_cr0(void) {
    uint32_t value;

    // mov (register), cr0
    // mov value, (register)
    asm volatile("mov %%cr0, %0" : "=r" (value));

    return value;
}

void write_cr0(uint32_t value) {
    // mov (register), value
    // mov cr0, (register)
    asm volatile ("mov %0, %%cr0" : : "r" (value));
}

uint32_t read_cr2(void) {
    uint32_t value;

    // mov (register), cr2
    // mov value, (register)
    asm volatile("mov %%cr2, %0" : "=r"(value));

    return value;
}

void write_cr2(uint32_t value) {
    // mov (register), value
    // mov cr2, (register)
    asm volatile("mov %0, %%cr2" : : "r"(value));
}

uint32_t read_cr3(void) {
    uint32_t value;

    // mov (register), cr3
    // mov value, (register)
    asm volatile("mov %%cr3, %0" : "=r"(value));

    return value;
}

void write_cr3(uint32_t value) {
    // mov (register), value
    // mov cr3, (register)
    asm volatile("mov %0, %%cr3" : : "r"(value));
}

uint32_t read_cr4(void) {
    uint32_t value;

    // mov (register), cr4
    // mov value, (register)
    asm volatile("mov %%cr4, %0" : "=r"(value));

    return value;
}

void write_cr4(uint32_t value) {
    // mov (register), value
    // mov cr4, (register)
    asm volatile("mov %0, %%cr4" : : "r"(value));
}

void invlpg(uint32_t page) {
    // invlpg page
    asm volatile("invlpg (%0)" : : "r"(page));
}

void lgdt(uint32_t gdt_wrap) {
    // lgdt gdt_wrap
    asm volatile("lgdt (%0)" : : "r"(gdt_wrap));
}

void lidt(uint32_t idt_wrap) {
    // lidt idt_wrap
    asm volatile("lidt (%0)" : : "r"(idt_wrap));
}

void ltr(uint16_t tss_selector) {
    // ltr tss_selector
    asm volatile("ltr %0" : : "m"(tss_selector) : "memory");
}

void fninit(void) {
    // finit
    asm volatile("fninit");
}

void cli(void) {
    // cli
    asm volatile("cli");
}

void sti(void) {
    // sti
    asm volatile("sti");
}

inline void mfence(void) {
    // mfence
    asm volatile("mfence" : : : "memory");
}

inline uint64_t rdtsc(void) {
    // rdtsc
    uint32_t low, high;
    asm volatile("rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

inline void outb(uint16_t port, uint8_t value) {
    // outb value, port
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t value) {
    // outw value, port
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

inline void outd(uint16_t port, uint32_t value) {
    // outl value, port
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t value;
    // inb value, port
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline uint16_t inw(uint16_t port) {
    uint16_t value;
    // inw value, port
    asm volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline uint32_t ind(uint16_t port) {
    uint32_t value;
    // inl value, port
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline void mmio_outb(uint32_t base, uint8_t value) {
    *(volatile uint8_t *) base = value;
}

inline void mmio_outw(uint32_t base, uint16_t value) {
    *(volatile uint16_t *) base = value;
}

inline void mmio_outd(uint32_t base, uint32_t value) {
    *(volatile uint32_t *) base = value;
}

inline uint8_t mmio_inb(uint32_t base) {
    return *(volatile uint8_t *) base;
}

inline uint16_t mmio_inw(uint32_t base) {
    return *(volatile uint16_t *) base;
}

inline uint32_t mmio_ind(uint32_t base) {
    return *(volatile uint32_t *) base;
}