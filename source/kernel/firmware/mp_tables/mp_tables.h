/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/interrupt_controller/main.h>
#include <kernel/interrupt_controller/apic.h>
#include <kernel/cpu/info.h>

typedef struct {
    uint32_t signature;
    uint32_t mp_configuration_table_pm;
    uint8_t length;
    uint8_t specification_revision;
    uint8_t checksum;
    uint8_t mp_system_configuration_type;
    uint8_t : 7;
    uint8_t imcr_present : 1;
    uint8_t reserved_feature_bytes[3];
}__attribute__((packed)) mp_floating_pointer_table_t;

typedef struct {
    uint32_t signature;
    uint16_t base_table_length;
    uint8_t specification_revision;
    uint8_t checksum;
    uint8_t oem_id[8];
    uint8_t product_id[12];
    uint32_t oem_table_pointer;
    uint16_t oem_table_size;
    uint16_t entry_count;
    uint32_t mmio_lapic_address;
    uint16_t extended_table_length;
    uint8_t extended_table_checksum;
    uint8_t reserved;
}__attribute__((packed)) mp_configuration_table_header_t;

typedef struct {
    uint8_t entry_type;
    uint8_t lapic_id;
    uint8_t lapic_version;
    uint8_t cpu_enabled : 1;
    uint8_t cpu_bootstrap : 1;
    uint8_t : 6;
    uint32_t cpu_stepping : 4;
    uint32_t cpu_model : 4;
    uint32_t cpu_family : 4;
    uint32_t : 20;
    uint32_t fpu_present : 1;
    uint32_t : 6;
    uint32_t machine_check_exception_present : 1;
    uint32_t cmpxchg8b_present : 1;
    uint32_t apic_present : 1;
    uint32_t : 22;
    uint32_t reserved1;
    uint32_t reserved2;
}__attribute__((packed)) mp_processor_entry_t;

typedef struct {
    uint8_t entry_type;
    uint8_t bus_id;
    uint8_t bus_type_string[6];
}__attribute__((packed)) mp_bus_entry_t;

typedef struct {
    uint8_t entry_type;
    uint8_t io_apic_id;
    uint8_t io_apic_version;
    uint8_t io_apic_usable : 1;
    uint8_t : 7;
    uint32_t io_apic_address;
}__attribute__((packed)) mp_io_apic_entry_t;

typedef struct {
    uint8_t entry_type;
    uint8_t interrupt_type;
    uint16_t interrupt_polarity : 2;
    uint16_t interrupt_trigger_mode : 2;
    uint16_t : 12;
    uint8_t source_bus_id;
    uint8_t source_bus_irq;
    uint8_t destination_io_apic_id;
    uint8_t destination_io_apic_intin;
}__attribute__((packed)) mp_interrupt_assignment_entry_t;

typedef struct {
    uint8_t entry_type;
    uint8_t interrupt_type;
    uint16_t interrupt_polarity : 2;
    uint16_t interrupt_trigger_mode : 2;
    uint16_t : 12;
    uint8_t source_bus_id;
    uint8_t source_bus_irq;
    uint8_t destination_lapic_id;
    uint8_t destination_lapic_intin;
}__attribute__((packed)) mp_local_interrupt_assignment_entry_t;