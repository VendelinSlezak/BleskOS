//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
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