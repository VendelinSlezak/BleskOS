//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct cpu_info_t {
    byte_t cpu_id[13];
    byte_t family;
    byte_t model;
    byte_t stepping;
    byte_t type;
    byte_t bits;
    dword_t number_of_logical_processors;

    dword_t running_in_emulator;

    dword_t p_mtrr;
    dword_t mtrr_number_of_entries;
    dword_t mtrr_default_memory_type;
    dword_t mtrr_write_combining_available;
};

byte_t *processor_type_string[] = {
    "OEM Processor",
    "Intel Overdrive Processor",
    "Dual Processor",
    "Reserved"
};

byte_t *processor_features_edx[] = {
    "FPU",
    "Virtual 8086 Mode",
    "Debugging extensions",
    "4 MB Page Size",
    "Time Stamp Counter",
    "Model-Specific Registers",
    "Physical Address Extension",
    "Machine Check Extension",
    "CMPXCHG8B",
    "APIC",
    "Reserved",
    "SYSENTER/SYSEXIT",
    "Memory-Type Range Registers",
    "Page Global Enable",
    "Machine Check Architecture",
    "CMOV/FCMOV/FCOMI",
    "Page Attribute Table",
    "36-bit Page Size Extension",
    "Processor Serial Number",
    "CLFLUSH",
    "Reserved",
    "Debug store",
    "ACPI Onboard Thermal Control",
    "MMX",
    "FXSAVE/FXRSTOR",
    "SSE",
    "SSE2",
    "CPU Cache with self-snoop",
    "Logical Processors",
    "Automatic Thermal Monitor",
    "IA64 emulation for x86",
    "Pending Break Enable"
};

byte_t *processor_features_ecx[] = {
    "SSE3",
    "PCLMULQDQ",
    "64-bit Debug Store",
    "MONITOR/MWAIT",
    "CPL Qualified Debug Store",
    "Virtual Machine eXtensions",
    "Safer Mode Extensions",
    "Enhanced SpeedStep",
    "Thermal Monitor 2",
    "Supplemental SSE3",
    "L1 Context ID",
    "Silicon Debug Interface",
    "Fused multiply-add",
    "CMPXCHG16B",
    "Possible to disable sending task priority messages",
    "Performance and Debug Capability",
    "Reserved",
    "Process Context Identifiers",
    "Direct Cache Access for DMA writes",
    "SSE4.1",
    "SSE4.2",
    "x2APIC",
    "MOVBE",
    "POPCNT",
    "APIC one-shot use TCP deadline value",
    "AES",
    "XSAVE/XRSTOR/XSETBV/XGETBV",
    "XSAVE enabled",
    "Advanced Vector Extensions",
    "Convertion for FP16",
    "RDRAND",
    "Hypervisor present"
};

void read_cpu_info(void);