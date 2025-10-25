/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/acpi/madt.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/cpu/gdt.h>
#include <kernel/x86/cpu/idt.h>
#include <kernel/x86/cpu/tss.h>

/* local variables */
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

byte_t *mtrr_memory_types[] = {
    "Uncacheable",
    "Write Combining",
    "Reserved (2)",
    "Reserved (3)",
    "Write Through",
    "Write Protected", 
    "Write Back",
    "Uncacheable Minus"
};

const dword_t mxcsr_default = 0x1F80;

/* functions */
void initialize_cpu(void) {
    // read register size of processor
    kernel_attr->cpu_bits = 32;
    read_cpuid(0x80000001);
    if((cpuid.edx & (1 << 29)) == (1 << 29)) {
        kernel_attr->cpu_bits = 64;
    }

    // read Manufacturer ID
    read_cpuid(0x00);
    memcpy(&kernel_attr->cpu_id[0], &cpuid.ebx, 4);
    memcpy(&kernel_attr->cpu_id[4], &cpuid.edx, 4);
    memcpy(&kernel_attr->cpu_id[8], &cpuid.ecx, 4);

    // read basic informations
    read_cpuid(0x01);
    kernel_attr->cpu_stepping = (cpuid.eax & 0xF);
    kernel_attr->cpu_model = ((cpuid.eax >> 4) & 0xF);
    kernel_attr->cpu_family = ((cpuid.eax >> 8) & 0xF);
    kernel_attr->cpu_type = ((cpuid.eax >> 12) & 0x3);
    if(kernel_attr->cpu_family == 0x6 || kernel_attr->cpu_family == 0xF) {
        kernel_attr->cpu_family += ((cpuid.eax >> 20) & 0xFF);
    }
    if(kernel_attr->cpu_family >= 0x6) {
        kernel_attr->cpu_model += (((cpuid.eax >> 16) & 0xF) << 4);
    }
    
    // read number of logical processors
    if((cpuid.edx & (1 << 28)) == (1 << 28)) {
        kernel_attr->cpu_number_of_logical_processors = ((cpuid.ebx >> 16) & 0xFF);
    }
    else {
        kernel_attr->cpu_number_of_logical_processors = 1;
    }

    // read useful features
    if((cpuid.edx & (1 << 9)) == (1 << 9)) {
        kernel_attr->lapic_present = 1;
    }
    if((cpuid.edx & (1 << 12)) == (1 << 12)) {
        kernel_attr->cpu_p_mtrr = TRUE;
    }
    if((cpuid.ecx & (1 << 31)) == (1 << 31)) {
        kernel_attr->cpu_running_in_emulator = TRUE;
    }
    // TODO: check if MSR are supported

    // set write-protect
    write_cr0(read_cr0() | (1 << 16));

    // set Page Attribute Table
    if((cpuid.edx & (1 << 16)) == (1 << 16)) {
        write_msr(CPU_MSR_PAT, (((qword_t)CPU_MEMORY_TYPE_WRITE_BACK)            |   // index 0 = WB
                                ((qword_t)CPU_MEMORY_TYPE_WRITE_THROUGH << 8)    |   // index 1 = WT
                                ((qword_t)CPU_MEMORY_TYPE_UNCACHEABLE << 16)     |   // index 2 = UC
                                ((qword_t)CPU_MEMORY_TYPE_UNCACHEABLE << 24)     |   // index 3 = UC
                                ((qword_t)CPU_MEMORY_TYPE_WRITE_COMBINING << 32) |   // index 4 = WC
                                ((qword_t)CPU_MEMORY_TYPE_WRITE_THROUGH << 40)   |   // index 5 = WT
                                ((qword_t)CPU_MEMORY_TYPE_UNCACHEABLE << 48)     |   // index 6 = UC
                                ((qword_t)CPU_MEMORY_TYPE_UNCACHEABLE << 56)));      // index 7 = UC
        write_cr3(kernel_attr->pm_of_loaded_page_directory);
    }

    // log
    log("\n[CPU] %d-bit processor", kernel_attr->cpu_bits);
    log("\n[CPU] ID: %s", kernel_attr->cpu_id);
    log("\n[CPU] Family: %d Model: %d Stepping: %d", kernel_attr->cpu_family, kernel_attr->cpu_model, kernel_attr->cpu_stepping);
    log("\n[CPU] Processor Type: %s", processor_type_string[kernel_attr->cpu_type]);
    log("\n[CPU] Number of Logical Processors: %d", kernel_attr->cpu_number_of_logical_processors);
    for(dword_t i = 0; i < 32; i++) {
        if((cpuid.edx & (1 << i)) == (1 << i)) {
            log("\n[CPU] Feature: %s", processor_features_edx[i]);
        }
    }
    for(dword_t i = 0; i < 32; i++) {
        if((cpuid.ecx & (1 << i)) == (1 << i)) {
            log("\n[CPU] Feature: %s", processor_features_ecx[i]);
        }
    }

    // log MTRR
    if(kernel_attr->cpu_p_mtrr == TRUE) {
        log("\n[CPU] Memory-type range registers supported");

        qword_t capability = read_msr(CPU_MSR_MTRR_REG_CAPABILITY);
        log("\n[CPU] Default memory type: %s", mtrr_memory_types[read_msr(CPU_MSR_MTRR_REG_DEFAULT_TYPE) & 0xFF]);
        if(((capability >> 10) & 0x1) == 0x1) {
            log("\n[CPU] MTRR: Write-combined supported");
        }

        dword_t mtrr_number_of_entries = (capability & 0xFF);
        for(int i = 0; i < mtrr_number_of_entries; i++) {
            log("\n[CPU] MTRR entry %d: ", i);

            if((read_msr(CPU_MSR_MTRR_REG_MASK + i*2) & (1 << 11)) == (1 << 11)) {
                // read entry data
                dword_t entry_type = (read_msr(CPU_MSR_MTRR_REG_BASE + i*2) & 0xFF);
                qword_t entry_start = read_msr(CPU_MSR_MTRR_REG_BASE + i*2);
                qword_t entry_size = (~read_msr(CPU_MSR_MTRR_REG_MASK + i*2) & 0xFFFFFFFFFFFFF000) + 0x1000;
                qword_t entry_end = (entry_start + entry_size);

                // log memory type
                log("%s ", mtrr_memory_types[entry_type]);

                // log memory position and size
                log("0x%x%x - 0x%x%x", (dword_t)(entry_start >> 32),
                                     (dword_t)(entry_start & 0xFFFFF000),
                                     (dword_t)(entry_end >> 32),
                                     (dword_t)(entry_end & 0xFFFFF000));
            }
            else {
                log("Unused");
            }
        }
    }

    // set up processor specific structures
    initialize_gdt();
    initialize_idt();
    initialize_tss();

    // TODO: initialize FPU

    // enable SSE instructions
    if((cpuid.edx & (1 << 25)) == (1 << 25)) {
        // CR0: EM = 0 (disable FPU emulation), MP = 1 (monitor coprocessor)
        dword_t cr0 = read_cr0();
        cr0 &= ~0x4;
        cr0 |= 0x2;
        write_cr0(cr0);

        // CR4: OSFXSR = 1 (SSE enabled), OSXMMEXCPT = 1 (SSE exceptions)
        dword_t cr4 = read_cr4();
        cr4 |= 0x600;
        write_cr4(cr4);
    }

    // initialize processor cores
    kernel_attr->number_of_cores = 0;
    if(kernel_attr->lapic_present == 1 && kernel_attr->pm_madt != INVALID) {
        void *vm_of_madt_table = kpalloc(kernel_attr->pm_madt, kernel_attr->size_of_madt, VM_FLAG_WRITE_BACK);
        madt_table_t *madt = (madt_table_t *) (vm_of_madt_table + (kernel_attr->pm_madt & 0xFFF));

        log("\n[MADT] LAPIC base: 0x%08X, flags=0x%08X", madt->local_apic_address, madt->flags);
        kernel_attr->lapic_base = madt->local_apic_address;

        byte_t *ptr = madt->entries;
        byte_t *end = ((byte_t*)madt) + madt->header.length;

        while (ptr < end) {
            madt_entry_header_t *entry = (madt_entry_header_t *) ptr;

            if (entry->length == 0) {
                log("\n[MADT] invalid entry length = 0, aborting parsing");
                break;
            }

            log("\n[MADT] ");

            switch (entry->type) {
                case MADT_TYPE_LOCAL_APIC: {
                    madt_local_apic_t *e = (madt_local_apic_t *) entry;
                    log("Local APIC: CPU ID=%u, APIC ID=%u, flags=0x%08X",
                            e->acpi_processor_id, e->apic_id, e->flags);
                    kernel_attr->core_id[kernel_attr->number_of_cores++] = e->apic_id;
                    break;
                }
                case MADT_TYPE_IO_APIC: {
                    madt_io_apic_t *e = (madt_io_apic_t *) entry;
                    log("I/O APIC: ID=%u, addr=0x%08X, GSI base=%u",
                            e->io_apic_id, e->io_apic_address, e->global_system_interrupt_base);
                    break;
                }
                case MADT_TYPE_INTERRUPT_OVERRIDE: {
                    madt_interrupt_override_t *e = (madt_interrupt_override_t *) entry;
                    log("Interrupt Override: bus=%u, IRQ=%u, GSI=%u, flags=0x%04X",
                            e->bus_source, e->irq_source, e->global_system_interrupt, e->flags);
                    break;
                }
                case MADT_TYPE_NMI_SOURCE: {
                    madt_nmi_source_t *e = (madt_nmi_source_t *) entry;
                    log("NMI Source: GSI=%u, flags=0x%04X",
                            e->global_system_interrupt, e->flags);
                    break;
                }
                case MADT_TYPE_LOCAL_APIC_NMI: {
                    madt_local_apic_nmi_t *e = (madt_local_apic_nmi_t *) entry;
                    log("Local APIC NMI: CPU=%u, LINT=%u, flags=0x%04X",
                            e->acpi_processor_id, e->local_apic_lint, e->flags);
                    break;
                }
                case MADT_TYPE_LOCAL_APIC_OVERRIDE: {
                    madt_local_apic_override_t *e = (madt_local_apic_override_t *) entry;
                    log("Local APIC Override: addr=0x%016llX", 
                            (unsigned long long)e->local_apic_address);
                    break;
                }
                case MADT_TYPE_LOCAL_X2APIC: {
                    madt_local_x2apic_t *e = (madt_local_x2apic_t *) entry;
                    log("Local x2APIC: CPU UID=%u, x2APIC ID=%u, flags=0x%08X",
                            e->acpi_processor_uid, e->x2apic_id, e->flags);
                    kernel_attr->core_id[kernel_attr->number_of_cores] = e->x2apic_id;
                    kernel_attr->number_of_cores++;
                    break;
                }
                case MADT_TYPE_LOCAL_X2APIC_NMI: {
                    madt_local_x2apic_nmi_t *e = (madt_local_x2apic_nmi_t *) entry;
                    log("Local x2APIC NMI: CPU UID=%u, LINT=%u, flags=0x%04X",
                            e->acpi_processor_uid, e->local_x2apic_lint, e->flags);
                    break;
                }
                default: {
                    log("Unknown MADT entry type %u (length=%u)", 
                            entry->type, entry->length);
                    byte_t *raw = (byte_t *) entry;
                    for(int i = 0; i < entry->length; i++) {
                        log("\n [%02d] = 0x%02X", i, raw[i]);
                    }
                    break;
                }
            }

            ptr += entry->length;
        }

        unmap(vm_of_madt_table);
    }
    else {
        log("\n[CPU] Can not enable multiple cores");
        kernel_attr->number_of_cores = 1;
        kernel_attr->core_id[0] = 0x00000000; // default value
        return;
    }
    if(kernel_attr->number_of_cores == 0) {
        log("\n[CPU] MADT does not have core entry, using default values");
        kernel_attr->number_of_cores = 1;
        kernel_attr->core_id[0] = 0x00000000; // default value
        kernel_attr->lapic_base = 0xFEE00000; // default fallback value
    }

    log("\n[CPU] Number of cores: %d", kernel_attr->number_of_cores);
    for(int i = 0; i < kernel_attr->number_of_cores; i++) {
        log("\n[CPU] Core %d ID: 0x%02x", i, kernel_attr->core_id[i]);
    }
}