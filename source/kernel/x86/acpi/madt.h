/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/x86/acpi/main.h>

typedef struct {
    acpi_table_header_t header;
    dword_t local_apic_address;
    dword_t flags;
    byte_t  entries[];
} __attribute__((packed)) madt_table_t;

typedef struct {
    byte_t type;
    byte_t length;
} __attribute__((packed)) madt_entry_header_t;

#define MADT_TYPE_LOCAL_APIC             0
#define MADT_TYPE_IO_APIC                1
#define MADT_TYPE_INTERRUPT_OVERRIDE     2
#define MADT_TYPE_NMI_SOURCE             3
#define MADT_TYPE_LOCAL_APIC_NMI         4
#define MADT_TYPE_LOCAL_APIC_OVERRIDE    5
#define MADT_TYPE_IO_SAPIC               6
#define MADT_TYPE_LOCAL_SAPIC            7
#define MADT_TYPE_PLATFORM_INTERRUPT     8
#define MADT_TYPE_LOCAL_X2APIC           9
#define MADT_TYPE_LOCAL_X2APIC_NMI       10
#define MADT_TYPE_GICC                   11
#define MADT_TYPE_GICD                   12
#define MADT_TYPE_GIC_MSI_FRAME          13
#define MADT_TYPE_GICR                   14
#define MADT_TYPE_GIC_ITS                15

typedef struct {
    madt_entry_header_t header;
    byte_t acpi_processor_id;
    byte_t apic_id;
    dword_t flags;
} __attribute__((packed)) madt_local_apic_t;

typedef struct {
    madt_entry_header_t header;
    byte_t io_apic_id;
    byte_t reserved;
    dword_t io_apic_address;
    dword_t global_system_interrupt_base;
} __attribute__((packed)) madt_io_apic_t;

typedef struct {
    madt_entry_header_t header;
    byte_t bus_source;
    byte_t irq_source;
    dword_t global_system_interrupt;
    word_t flags;
} __attribute__((packed)) madt_interrupt_override_t;

typedef struct {
    madt_entry_header_t header;
    word_t flags;
    dword_t global_system_interrupt;
} __attribute__((packed)) madt_nmi_source_t;

typedef struct {
    madt_entry_header_t header;
    byte_t acpi_processor_id;
    word_t flags;
    byte_t local_apic_lint;
} __attribute__((packed)) madt_local_apic_nmi_t;

typedef struct {
    madt_entry_header_t header;
    word_t reserved;
    qword_t local_apic_address;
} __attribute__((packed)) madt_local_apic_override_t;

typedef struct {
    madt_entry_header_t header;
    word_t reserved;
    dword_t x2apic_id;
    dword_t flags;
    dword_t acpi_processor_uid;
} __attribute__((packed)) madt_local_x2apic_t;

typedef struct {
    madt_entry_header_t header;
    word_t flags;
    dword_t acpi_processor_uid;
    byte_t local_x2apic_lint;
    byte_t reserved[3];
} __attribute__((packed)) madt_local_x2apic_nmi_t;
