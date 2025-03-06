//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_cpu_info(void) {
    logf("\n\nCPU INFO");

    // read register size of processor
    components->cpu.bits = 32;
    read_cpuid(0x80000001);
    if((cpuid.edx & (1 << 29)) == (1 << 29)) {
        components->cpu.bits = 64;
    }
    logf("\n %d-bit processor", components->cpu.bits);

    // read Manufacturer ID
    read_cpuid(0x00);
    copy_memory((dword_t)(&cpuid.ebx), (dword_t)(&components->cpu.cpu_id[0]), 4);
    copy_memory((dword_t)(&cpuid.edx), (dword_t)(&components->cpu.cpu_id[4]), 4);
    copy_memory((dword_t)(&cpuid.ecx), (dword_t)(&components->cpu.cpu_id[8]), 4);
    logf("\n CPU ID: %s", components->cpu.cpu_id);

    // read basic informations
    read_cpuid(0x01);
    components->cpu.stepping = (cpuid.eax & 0xF);
    components->cpu.model = ((cpuid.eax >> 4) & 0xF);
    components->cpu.family = ((cpuid.eax >> 8) & 0xF);
    components->cpu.type = ((cpuid.eax >> 12) & 0x3);
    if(components->cpu.family == 0x6 || components->cpu.family == 0xF) {
        components->cpu.family += ((cpuid.eax >> 20) & 0xFF);
    }
    if(components->cpu.family >= 0x6) {
        components->cpu.model += (((cpuid.eax >> 16) & 0xF) << 4);
    }
    logf("\n Family: %d Model: %d Stepping: %d", components->cpu.family, components->cpu.model, components->cpu.stepping);
    logf("\n Processor Type: %s", processor_type_string[components->cpu.type]);
    
    // read number of logical processors
    if((cpuid.edx & (1 << 28)) == (1 << 28)) {
        components->cpu.number_of_logical_processors = ((cpuid.ebx >> 16) & 0xFF);
        logf("\n Number of Logical Processors: %d", components->cpu.number_of_logical_processors);
    }

    // read useful features
    if((cpuid.edx & (1 << 9)) == (1 << 9)) {
        components->p_apic = 1;
    }
    if((cpuid.edx & (1 << 12)) == (1 << 12)) {
        components->cpu.p_mtrr = STATUS_TRUE;
    }
    if((cpuid.ecx & (1 << 31)) == (1 << 31)) {
        components->cpu.running_in_emulator = STATUS_TRUE;
    }
    
    // print features
    logf("\nFeatures:");
    for(dword_t i = 0; i < 32; i++) {
        if((cpuid.edx & (1 << i)) == (1 << i)) {
            logf("\n %s", processor_features_edx[i]);
        }
    }
    for(dword_t i = 0; i < 32; i++) {
        if((cpuid.ecx & (1 << i)) == (1 << i)) {
            logf("\n %s", processor_features_ecx[i]);
        }
    }
}