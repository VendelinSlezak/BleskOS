/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/interrupt.h>

#define MAX_NUMBER_OF_LOGICAL_PROCESSORS_ON_SYSTEM 256

#define GDT_ENTRY_TYPE_DATA 0
#define GDT_ENTRY_TYPE_CODE 1
#define GDT_ENTRY_TYPE_32_BIT_AVAILABLE_TSS 0x9
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    union {
        uint8_t access_byte;
        struct {
            uint8_t accessed : 1;
            uint8_t read_write : 1;
            uint8_t direction_or_conforming_bit : 1;
            uint8_t executable : 1;
            uint8_t type : 1;
            uint8_t privilege_level : 2;
            uint8_t present : 1;
        }__attribute__((packed));
    };
    uint8_t limit_high : 4;
    uint8_t : 1;
    uint8_t long_mode : 1;
    uint8_t size : 1;
    uint8_t granularity : 1;
    uint8_t base_high;
}__attribute__((packed)) gdt_entry_t;
typedef struct {
    uint16_t size;
    uint32_t address;
}__attribute__((packed)) gdt_wrap_t;

#define NUMBER_OF_IDT_ENTRIES 256
#define IDT_GATE_TYPE_INTERRUPT_32_BIT 0xE
typedef struct {
    uint16_t handler_address_lower;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t gate_type;
    uint16_t handler_address_higher;
}__attribute__((packed)) idt_entry_t;
typedef struct {
    uint16_t size;
    uint32_t address;
}__attribute__((packed)) idt_wrap_t;

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
}__attribute__((packed)) tss_t;

#define CPU_MSR_PAT 0x277
#define CPU_MSR_MTRR_REG_CAPABILITY 0xFE
#define CPU_MSR_MTRR_REG_DEFAULT_TYPE 0x2FF
#define CPU_MSR_MTRR_REG_BASE 0x200
#define CPU_MSR_MTRR_REG_MASK 0x201

#define CPU_MEMORY_TYPE_UNCACHEABLE         0x00  // UC
#define CPU_MEMORY_TYPE_WRITE_COMBINING     0x01  // WC
#define CPU_MEMORY_TYPE_WRITE_THROUGH       0x04  // WT
#define CPU_MEMORY_TYPE_WRITE_PROTECTED     0x05  // WP
#define CPU_MEMORY_TYPE_WRITE_BACK          0x06  // WB
#define CPU_MEMORY_TYPE_UNCACHEABLE_MINUS   0x07  // UC-

extern uint32_t bootstrap_processor_id;
extern uint32_t invalid_opcode_return_address;
extern uint32_t general_protection_fault_return_address;