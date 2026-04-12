/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/firmware/acpi/acpi.h>
#include <kernel/interrupt_controller/main.h>
#include <kernel/interrupt_controller/apic.h>
#include <kernel/cpu/info.h>

typedef struct {
    acpi_table_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
    uint8_t  entries[];
}__attribute__((packed)) madt_table_t;

#define MADT_TYPE_LOCAL_APIC             0
#define MADT_TYPE_IO_APIC                1
#define MADT_TYPE_INTERRUPT_OVERRIDE     2
#define MADT_TYPE_NMI_SOURCE             3
#define MADT_TYPE_LOCAL_APIC_NMI         4
#define MADT_TYPE_LOCAL_APIC_OVERRIDE    5
typedef struct {
    uint8_t type;
    uint8_t length;
}__attribute__((packed)) madt_entry_header_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t enabled : 1;
    uint32_t online_capable : 1;
    uint32_t : 30;
}__attribute__((packed)) madt_local_apic_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address;
    uint32_t global_system_interrupt_base;
}__attribute__((packed)) madt_io_apic_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
}__attribute__((packed)) madt_interrupt_override_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t irq_source;
    uint8_t reserved;
    uint16_t flags;
    uint32_t global_system_interrupt;
}__attribute__((packed)) madt_nmi_source_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t acpi_processor_id;
    uint16_t flags;
    uint8_t local_apic_lint;
}__attribute__((packed)) madt_local_apic_nmi_t;

typedef struct {
    madt_entry_header_t header;
    uint16_t reserved;
    uint64_t local_apic_address;
}__attribute__((packed)) madt_local_apic_override_t;