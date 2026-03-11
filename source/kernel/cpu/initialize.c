/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <libc/string.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/commands.h>
#include <kernel/cpu/lapic.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/cpu/scheduler.h>
#include <kernel/timers/main.h>
#include <kernel/timers/lapic.h>
#include <kernel/firmware/acpi/madt.h>
#include <kernel/firmware/main.h>
#include <kernel/firmware/mp_tables/mp_tables.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* local variables */
uint32_t bootstrap_processor_lapic_address;
void *tss;
gdt_entry_t *gdt;
gdt_wrap_t *gdt_wrap;
idt_entry_t *idt;
idt_wrap_t *idt_wrap;
#define I(n) extern void interrupt_stub##n(void);
I(0)   I(1)   I(2)   I(3)   I(4)   I(5)   I(6)   I(7)   I(8)   I(9)
I(10)  I(11)  I(12)  I(13)  I(14)  I(15)  I(16)  I(17)  I(18)  I(19)
I(20)  I(21)  I(22)  I(23)  I(24)  I(25)  I(26)  I(27)  I(28)  I(29)
I(30)  I(31)  I(32)  I(33)  I(34)  I(35)  I(36)  I(37)  I(38)  I(39)
I(40)  I(41)  I(42)  I(43)  I(44)  I(45)  I(46)  I(47)  I(48)  I(49)
I(50)  I(51)  I(52)  I(53)  I(54)  I(55)  I(56)  I(57)  I(58)  I(59)
I(60)  I(61)  I(62)  I(63)  I(64)  I(65)  I(66)  I(67)  I(68)  I(69)
I(70)  I(71)  I(72)  I(73)  I(74)  I(75)  I(76)  I(77)  I(78)  I(79)
I(80)  I(81)  I(82)  I(83)  I(84)  I(85)  I(86)  I(87)  I(88)  I(89)
I(90)  I(91)  I(92)  I(93)  I(94)  I(95)  I(96)  I(97)  I(98)  I(99)
I(100) I(101) I(102) I(103) I(104) I(105) I(106) I(107) I(108) I(109)
I(110) I(111) I(112) I(113) I(114) I(115) I(116) I(117) I(118) I(119)
I(120) I(121) I(122) I(123) I(124) I(125) I(126) I(127) I(128) I(129)
I(130) I(131) I(132) I(133) I(134) I(135) I(136) I(137) I(138) I(139)
I(140) I(141) I(142) I(143) I(144) I(145) I(146) I(147) I(148) I(149)
I(150) I(151) I(152) I(153) I(154) I(155) I(156) I(157) I(158) I(159)
I(160) I(161) I(162) I(163) I(164) I(165) I(166) I(167) I(168) I(169)
I(170) I(171) I(172) I(173) I(174) I(175) I(176) I(177) I(178) I(179)
I(180) I(181) I(182) I(183) I(184) I(185) I(186) I(187) I(188) I(189)
I(190) I(191) I(192) I(193) I(194) I(195) I(196) I(197) I(198) I(199)
I(200) I(201) I(202) I(203) I(204) I(205) I(206) I(207) I(208) I(209)
I(210) I(211) I(212) I(213) I(214) I(215) I(216) I(217) I(218) I(219)
I(220) I(221) I(222) I(223) I(224) I(225) I(226) I(227) I(228) I(229)
I(230) I(231) I(232) I(233) I(234) I(235) I(236) I(237) I(238) I(239)
I(240) I(241) I(242) I(243) I(244) I(245) I(246) I(247) I(248) I(249)
I(250) I(251) I(252) I(253) I(254) I(255)
#undef I
static void (*const interrupt_stubs[256])(void) = {
    #define I(n) interrupt_stub##n,
    I(0)   I(1)   I(2)   I(3)   I(4)   I(5)   I(6)   I(7)   I(8)   I(9)
    I(10)  I(11)  I(12)  I(13)  I(14)  I(15)  I(16)  I(17)  I(18)  I(19)
    I(20)  I(21)  I(22)  I(23)  I(24)  I(25)  I(26)  I(27)  I(28)  I(29)
    I(30)  I(31)  I(32)  I(33)  I(34)  I(35)  I(36)  I(37)  I(38)  I(39)
    I(40)  I(41)  I(42)  I(43)  I(44)  I(45)  I(46)  I(47)  I(48)  I(49)
    I(50)  I(51)  I(52)  I(53)  I(54)  I(55)  I(56)  I(57)  I(58)  I(59)
    I(60)  I(61)  I(62)  I(63)  I(64)  I(65)  I(66)  I(67)  I(68)  I(69)
    I(70)  I(71)  I(72)  I(73)  I(74)  I(75)  I(76)  I(77)  I(78)  I(79)
    I(80)  I(81)  I(82)  I(83)  I(84)  I(85)  I(86)  I(87)  I(88)  I(89)
    I(90)  I(91)  I(92)  I(93)  I(94)  I(95)  I(96)  I(97)  I(98)  I(99)
    I(100) I(101) I(102) I(103) I(104) I(105) I(106) I(107) I(108) I(109)
    I(110) I(111) I(112) I(113) I(114) I(115) I(116) I(117) I(118) I(119)
    I(120) I(121) I(122) I(123) I(124) I(125) I(126) I(127) I(128) I(129)
    I(130) I(131) I(132) I(133) I(134) I(135) I(136) I(137) I(138) I(139)
    I(140) I(141) I(142) I(143) I(144) I(145) I(146) I(147) I(148) I(149)
    I(150) I(151) I(152) I(153) I(154) I(155) I(156) I(157) I(158) I(159)
    I(160) I(161) I(162) I(163) I(164) I(165) I(166) I(167) I(168) I(169)
    I(170) I(171) I(172) I(173) I(174) I(175) I(176) I(177) I(178) I(179)
    I(180) I(181) I(182) I(183) I(184) I(185) I(186) I(187) I(188) I(189)
    I(190) I(191) I(192) I(193) I(194) I(195) I(196) I(197) I(198) I(199)
    I(200) I(201) I(202) I(203) I(204) I(205) I(206) I(207) I(208) I(209)
    I(210) I(211) I(212) I(213) I(214) I(215) I(216) I(217) I(218) I(219)
    I(220) I(221) I(222) I(223) I(224) I(225) I(226) I(227) I(228) I(229)
    I(230) I(231) I(232) I(233) I(234) I(235) I(236) I(237) I(238) I(239)
    I(240) I(241) I(242) I(243) I(244) I(245) I(246) I(247) I(248) I(249)
    I(250) I(251) I(252) I(253) I(254) I(255)
    #undef I
};

uint32_t invalid_opcode_return_address = NULL;
uint32_t general_protection_fault_return_address = NULL;

/* functions */
void initialize_cpu_structures(void) {
    // read number of logical processors
    logical_processors = kalloc(sizeof(logical_processor_info_t) * MAX_NUMBER_OF_LOGICAL_PROCESSORS_ON_SYSTEM);
    if(firmware_info.madt != NULL) {
        madt_read_info_about_logical_processors(&number_of_logical_processors, logical_processors);
    }
    else if(firmware_info.mpct != NULL) {
        mp_tables_read_info_about_logical_processors(&number_of_logical_processors, logical_processors);
    }
    else {
        log("\n[CPU] MADT and MPCT not found");
        number_of_logical_processors = 1;
        logical_processors[0].hardware_id = 0x00000000;
    }
    logical_processors = krealloc(logical_processors, sizeof(logical_processor_info_t) * number_of_logical_processors);

    for(int i = 0; i < number_of_logical_processors; i++) {
        log("\n[CPU] Logical processor %d | Hardware ID: 0x%02x", 
            i, 
            logical_processors[i].hardware_id
        );
    }

    // TSS
    tss = perm_alloc(PAGE_SIZE * number_of_logical_processors);
    memset(tss, 0, PAGE_SIZE * number_of_logical_processors);

    // GDT
    gdt = (gdt_entry_t *) perm_alloc(sizeof(gdt_entry_t) * 5 + sizeof(gdt_entry_t) * number_of_logical_processors);
    set_gdt_entry(&gdt[1], 0x00000000, 0xFFFFFFFF, GDT_ENTRY_TYPE_CODE, 0); // kernel code segment
    set_gdt_entry(&gdt[2], 0x00000000, 0xFFFFFFFF, GDT_ENTRY_TYPE_DATA, 0); // kernel data segment
    set_gdt_entry(&gdt[3], 0x00000000, 0xFFFFFFFF, GDT_ENTRY_TYPE_CODE, 3); // user code segment
    set_gdt_entry(&gdt[4], 0x00000000, 0xFFFFFFFF, GDT_ENTRY_TYPE_DATA, 3); // user data segment
    for(int i = 0; i < number_of_logical_processors; i++) {
        set_gdt_system_entry(&gdt[5 + i], (uint32_t)tss + (PAGE_SIZE * i), sizeof(tss_t), GDT_ENTRY_TYPE_32_BIT_AVAILABLE_TSS, 0); // TSS segment
    }
    gdt_wrap = perm_alloc(sizeof(gdt_wrap_t));
    gdt_wrap->address = (uint32_t) gdt;
    gdt_wrap->size = (sizeof(gdt_entry_t) * 5 + sizeof(gdt_entry_t) * number_of_logical_processors) - 1;

    // IDT
    idt = (idt_entry_t *) perm_alloc(sizeof(idt_entry_t) * NUMBER_OF_IDT_ENTRIES);
    for(int i = 0; i < NUMBER_OF_IDT_ENTRIES; i++) {
        set_idt_entry(&idt[i], (uint32_t) interrupt_stubs[i], false);
    }
    extern void page_fault_handler_i686(void);
    set_idt_entry(&idt[0x0E], (uint32_t) page_fault_handler_i686, false); // set page fault handler directly to increase performance
    set_idt_entry(&idt[0xD0], (uint32_t) interrupt_stubs[0xD0], true); // syscall doorbell interrupt
    idt_wrap = perm_alloc(sizeof(idt_wrap_t));
    idt_wrap->address = (uint32_t) idt;
    idt_wrap->size = (sizeof(idt_entry_t) * NUMBER_OF_IDT_ENTRIES) - 1;

    // LAPIC is not enabled yet
    lapic_base = NULL;

    // allocate specific structures of logical processors
    uint32_t highest_processor_id = 0;
    for(int i = 0; i < number_of_logical_processors; i++) {
        if(logical_processors[i].hardware_id > highest_processor_id) {
            highest_processor_id = logical_processors[i].hardware_id;
        }
    }
    logical_processor_index_number = perm_alloc(sizeof(uint32_t) * (highest_processor_id + 1));
    for(int i = 0; i < number_of_logical_processors; i++) {
        logical_processor_index_number[logical_processors[i].hardware_id] = i;
    }
    logical_processor_structs_ptr = perm_alloc(sizeof(logical_processor_t) * number_of_logical_processors);
    memset(logical_processor_structs_ptr, 0, sizeof(logical_processor_t) * number_of_logical_processors);
    for(int i = 0; i < number_of_logical_processors; i++) {
        logical_processor_structs_ptr[i].index = i;
    }
}

void set_gdt_entry(gdt_entry_t *entry, uint32_t base, uint32_t limit, uint32_t type, uint8_t privilege_level) {
    entry->base_low = (base & 0xFFFF);
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high = (base >> 24) & 0xFF;

    entry->limit_low = (limit & 0xFFFF);
    entry->limit_high = (limit >> 16) & 0x0F;
    
    entry->accessed = 1;
    entry->read_write = 1;
    entry->direction_or_conforming_bit = 0; // data segment grows up / code segment is executable only from set privilege level
    entry->executable = type; // selects code or data segment
    entry->type = 1;
    entry->privilege_level = privilege_level;
    entry->present = 1;

    entry->long_mode = 0;
    entry->size = 1; // 32-bit protected mode segment
    entry->granularity = 1; // page granularity
}

void set_gdt_system_entry(gdt_entry_t *entry, uint32_t base, uint32_t limit, uint8_t type, uint8_t privilege_level) {
    entry->base_low = (base & 0xFFFF);
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high = (base >> 24) & 0xFF;

    entry->limit_low = (limit & 0xFFFF);
    entry->limit_high = (limit >> 16) & 0x0F;
    
    entry->access_byte = type;
    entry->type = 0; // system segment
    entry->privilege_level = privilege_level;
    entry->present = 1;

    entry->long_mode = 0;
    entry->size = 0;
    entry->granularity = 0; // byte granularity
}

void set_idt_entry(idt_entry_t *entry, uint32_t handler_address, uint32_t allow_user) {
    entry->handler_address_lower = (handler_address & 0xFFFF);
    entry->handler_address_higher = (handler_address >> 16) & 0xFFFF;
    entry->segment_selector = 0x08; // Kernel code segment
    entry->reserved = 0;
    if(allow_user == true) {
        entry->gate_type = 0xE0 | IDT_GATE_TYPE_INTERRUPT_32_BIT; 
    }
    else {
        entry->gate_type = 0x80 | IDT_GATE_TYPE_INTERRUPT_32_BIT;
    }
}

void switch_to_no_lapic_processor_mode(void) {
    number_of_logical_processors = 1; // we will use only one processor
    logical_processor_index_number[0x00000000] = 0; // lapic_get_processor_id() without LAPIC returns 0, so make sure it maps to index 0
}

void invalid_opcode_handler(interrupt_stack_t *stack_of_interrupt) {
    if(invalid_opcode_return_address != NULL) {
        stack_of_interrupt->eip = invalid_opcode_return_address; // after interrupt return here
        return;
    }
}

void general_protection_fault_handler(interrupt_stack_t *stack_of_interrupt) {
    if(general_protection_fault_return_address != NULL) {
        stack_of_interrupt->eip = general_protection_fault_return_address; // after interrupt return here
        return;
    }
}

void initialize_bootstrap_processor(void) {
    int cpuid_supported = false, msr_supported = false;

    // load GDT
    lgdt((uint32_t)gdt_wrap);

    // load IDT
    lidt((uint32_t)idt_wrap);

    // check if CPUID is supported
    set_isr_interrupt_handler(0x06, invalid_opcode_handler);
    extern uint32_t check_cpuid_support();
    if(check_cpuid_support() == 0) {
        log("\n[CPU] CPUID instruction is not supported");
        switch_to_no_lapic_processor_mode();
    }
    else {
        cpuid_supported = true;
    }

    // check if MSR registers are supported
    cpuid_t cpuid = {0};
    if(cpuid_supported == true) {
        cpuid = read_cpuid(0x00000001, 0);
    }
    if((cpuid.edx & (1 << 5)) == 0) {
        log("\n[CPU] MSR registers are not supported");
        switch_to_no_lapic_processor_mode();
    }
    else {
        msr_supported = true;
    }

    // check if LAPIC is supported
    if(msr_supported == false || (cpuid.edx & (1 << 9)) == 0) {
        log("\n[CPU] LAPIC is not supported");
        switch_to_no_lapic_processor_mode();
    }
    else {
        set_isr_interrupt_handler(0x0D, general_protection_fault_handler);
        extern uint32_t check_presence_of_msr();
        if(check_presence_of_msr(CPU_MSR_APIC_BASE) == 0) {
            log("\n[CPU] LAPIC MSR is not supported");
            switch_to_no_lapic_processor_mode();
        }
        else {
            uint64_t lapic_address = read_msr(CPU_MSR_APIC_BASE) & 0xFFFFFFFFFFFFF000;
            if(((lapic_address + PAGE_SIZE) >> 32) != 0) {
                log("\n[CPU] Error: LAPIC address is above 4GB boundary");
                switch_to_no_lapic_processor_mode();
            }
            else {
                // save LAPIC address
                bootstrap_processor_lapic_address = (uint32_t) lapic_address;

                // map LAPIC to virtual memory
                lapic_base = (uint32_t) perm_phy_alloc(lapic_address, PAGE_SIZE, VM_KERNEL | VM_UNCACHEABLE);
            
                // enable LAPIC
                enable_lapic();
            }
        }
    }

    // set bootstrap processor ID
    bootstrap_processor_id = lapic_get_processor_id();

    // load TSS
    logical_processor_t *data = get_current_logical_processor_struct();
    ltr(0x28 + (data->index * 8)); // GDT selector for TSS segment
    data->tss = (tss_t *) (tss + (data->index * PAGE_SIZE)); // set pointer to TSS in processor specific structure
    data->tss->ss0 = 0x10; // kernel data segment
    
    // initialize rest
    initialize_running_logical_processor(cpuid_supported, msr_supported);

    // log
    processor_log_capabilities();
    log("\n[CPU%d] Bootstrap processor initialized", data->index);
}

void processor_log_capabilities(void) {
    logical_processor_t *data = get_current_logical_processor_struct();
    log("\n[CPU%d] Capabilities:", data->index);
    if(data->is_fpu_supported) {
        log(" FPU");
    }
    if(data->is_mmx_supported) {
        log(" MMX");
    }
    if(data->is_sse_supported) {
        log(" SSE");
    }
    if(data->is_sse2_supported) {
        log(" SSE2");
    }
    if(data->is_sse3_supported) {
        log(" SSE3");
    }
    if(data->is_ssse3_supported) {
        log(" SSSE3");
    }
    if(data->is_sse4_1_supported) {
        log(" SSE4.1");
    }
    if(data->is_sse4_2_supported) {
        log(" SSE4.2");
    }
    if(data->is_avx_supported) {
        log(" AVX");
    }
}

void initialize_running_logical_processor(int cpuid_supported, int msr_supported) {
    // get pointer to processor specific structure
    logical_processor_t *data = get_current_logical_processor_struct();

    // set write-protect for virtual memory
    write_cr0(read_cr0() | (1 << 16));

    // set Page Attribute Table
    if(cpuid_supported == false) {
        data->is_running = 1;
        return;
    }
    cpuid_t cpuid = read_cpuid(0x00000001, 0);
    if(msr_supported == false || (cpuid.edx & (1 << 16)) == 0) {
        log("\n[CPU%d] PAT is not supported", data->index);
    }
    else {
        extern uint32_t check_presence_of_msr();
        if(check_presence_of_msr(CPU_MSR_PAT) == 0) {
            log("\n[CPU%d] PAT MSR is not supported", data->index);
        }
        write_msr(CPU_MSR_PAT, (((uint64_t)CPU_MEMORY_TYPE_WRITE_BACK)            |   // index 0 = WB
                                ((uint64_t)CPU_MEMORY_TYPE_WRITE_THROUGH << 8)    |   // index 1 = WT
                                ((uint64_t)CPU_MEMORY_TYPE_UNCACHEABLE << 16)     |   // index 2 = UC
                                ((uint64_t)CPU_MEMORY_TYPE_UNCACHEABLE << 24)     |   // index 3 = UC
                                ((uint64_t)CPU_MEMORY_TYPE_WRITE_COMBINING << 32) |   // index 4 = WC
                                ((uint64_t)CPU_MEMORY_TYPE_WRITE_THROUGH << 40)   |   // index 5 = WT
                                ((uint64_t)CPU_MEMORY_TYPE_UNCACHEABLE << 48)     |   // index 6 = UC
                                ((uint64_t)CPU_MEMORY_TYPE_UNCACHEABLE << 56)));      // index 7 = UC
        vm_refresh_mappings();
    }

    // enable FPU instructions
    if((cpuid.edx & (1 << 0)) == 0) {
        log("\n[CPU%d] FPU is not supported", data->index);
        uint32_t cr0 = read_cr0();
        cr0 &= ~(1 << 1); // clear NE (Numeric Error) flag
        cr0 |= (1 << 2); // set TS (Task Switched) flag
        write_cr0(cr0);
    }
    else {
        uint32_t cr0 = read_cr0();
        cr0 |= (1 << 1); // set NE (Numeric Error) flag
        cr0 &= ~(1 << 2); // clear TS (Task Switched) flag
        cr0 &= ~(1 << 3); // clear EM (Emulation) flag
        cr0 |= (1 << 5); // set MP (Monitor Coprocessor) flag
        write_cr0(cr0);
        fninit();
        data->is_fpu_supported = true;
    }

    // check support for MMX instructions
    if((cpuid.edx & (1 << 23))) {
        data->is_mmx_supported = true;
    }

    // enable SSE instructions
    if((cpuid.edx & (1 << 25))) {
        data->is_sse_supported = true;
        if(cpuid.edx & (1 << 26)) {
            data->is_sse2_supported = true;
        }
        if(cpuid.ecx & (1 << 0)) {
            data->is_sse3_supported = true;
        }
        if(cpuid.ecx & (1 << 9)) {
            data->is_ssse3_supported = true;
        }
        if(cpuid.ecx & (1 << 19)) {
            data->is_sse4_1_supported = true;
        }
        if(cpuid.ecx & (1 << 20)) {
            data->is_sse4_2_supported = true;
        }

        uint32_t cr0 = read_cr0();
        cr0 |= (1 << 1); // set NE (Numeric Error) flag
        cr0 &= ~(1 << 2); // clear TS (Task Switched) flag
        write_cr0(cr0);

        uint32_t cr4 = read_cr4();
        cr4 |= (1 << 9); // set OSFXSR (Operating System Support for FXSAVE and FXRSTOR instructions) flag
        cr4 |= (1 << 10); // set OSXMMEXCPT (Operating System Support for Unmasked SIMD Floating-Point Exceptions) flag
        write_cr4(cr4);
    }

    // enable AVX instructions
    if((cpuid.ecx & (1 << 27)) && (cpuid.ecx & (1 << 28))) {
        data->is_avx_supported = true;

        uint32_t cr4 = read_cr4();
        cr4 |= (1 << 18); // set OSXSAVE (Operating System Support for XSAVE and Processor Extended States) flag
        write_cr4(cr4);

        uint32_t xcr0_low = 0, xcr0_high = 0;
        asm volatile ("xgetbv" : "=a"(xcr0_low), "=d"(xcr0_high) : "c"(0));
        xcr0_low |= 0x00000007; // enable x87 (bit 0), SSE (bit 1) and AVX (bit 2) support
        asm volatile ("xsetbv" : : "a"(xcr0_low), "d"(xcr0_high), "c"(0));
    }

    // allocate stack for idle thread
    data->idle_thread_stack = perm_alloc(sizeof(interrupt_stack_t)); // 4 KB stack
    extern void idle_thread(void);
    data->idle_thread_stack->eip = (uint32_t) &idle_thread;
    data->idle_thread_stack->cs = 0x08; // kernel code segment
    data->idle_thread_stack->eflags = 0x202; // interrupt enable flag
    data->idle_thread_stack->ds = 0x10; // kernel data segment
    data->idle_thread_stack->es = 0x10; // kernel data segment
    data->idle_thread_stack->fs = 0x10; // kernel data segment
    data->idle_thread_stack->gs = 0x10; // kernel data segment

    // allocate floating stack
    data->floating_stack = perm_alloc(KERNEL_STACK_SIZE);

    // allocate page for copy-on-write memory for this processor
    data->copy_on_write_page = perm_alloc(PAGE_SIZE);

    // mark logical processor as running
    data->is_running = PROCESSOR_IS_RUNNING;

    return;
}

void initialize_all_application_processors(void) {
    if(lapic_base == NULL || number_of_logical_processors <= 1) {
        return;
    }

    // identity mapped page at 0x1000 is used as trampoline code for application processors
    // it is still there from bootloader, so we just need to copy our code there
    extern uint32_t ap_stack_pointer;
    extern void application_processor_initialization_sequence(void);
    extern void application_processor_initialization_sequence_end(void);

    // send INIT IPI to all application processors
    for(int i = 0; i < number_of_logical_processors; i++) {
        if(logical_processors[i].hardware_id == bootstrap_processor_id) {
            continue;
        }
        lapic_assert_init_ipi(logical_processors[i].hardware_id);
    }
    wait_microseconds(10000); // wait 10 ms
    for(int i = 0; i < number_of_logical_processors; i++) {
        if(logical_processors[i].hardware_id == bootstrap_processor_id) {
            continue;
        }
        lapic_deassert_init_ipi(logical_processors[i].hardware_id);
    }

    // send STARTUP IPI to all application processors
    for(int i = 0; i < number_of_logical_processors; i++) {
        if(logical_processors[i].hardware_id == bootstrap_processor_id) {
            continue;
        }

        // prepare stack for application processor
        ap_stack_pointer = (uint32_t) perm_alloc(PAGE_SIZE);
        ap_stack_pointer += PAGE_SIZE;

        // copy trampoline code to identity mapped page
        uint32_t trampoline_size = (uint32_t)&application_processor_initialization_sequence_end - (uint32_t)&application_processor_initialization_sequence;
        memcpy((void *)0x1000, (void *)&application_processor_initialization_sequence, trampoline_size);
        *((uint32_t *)(0x1FF8)) = (uint32_t) application_processor_entry_point; // set entry point for application processor
        *((uint32_t *)(0x1FFC)) = 0; // clear signal memory for application processor

        // send STARTUP IPI
        // according to Intel Documentation startup signal should be send twice for compatibility reasons
        lapic_send_startup_ipi(logical_processors[i].hardware_id, 0x1); // start at 0x1000
        wait_microseconds(200);
        lapic_send_startup_ipi(logical_processors[i].hardware_id, 0x1); // start at 0x1000

        // wait for application processor to signal that it has started
        uint64_t time = (*get_time_in_microseconds)();
        while(((*get_time_in_microseconds)() - time) < 1000) {
            asm volatile("pause");
            if(*((uint32_t *)(0x1FFC)) != 0) {
                break;
            }
        }
        if(*((uint32_t *)(0x1FFC)) == 0) {
            log("\n[CPU] ERROR: Application processor with hardware ID 0x%02x did not respond", logical_processors[i].hardware_id);
        }
    }

    // wait max 100ms until all processors are running
    uint64_t time = (*get_time_in_microseconds)();
    while(((*get_time_in_microseconds)() - time) < 100000) {
        int all_running = true;
        for(int i = 0; i < number_of_logical_processors; i++) {
            if(logical_processors[i].hardware_id == bootstrap_processor_id) {
                continue;
            }
            if(get_logical_processor_struct_by_hardware_id(logical_processors[i].hardware_id)->is_running == PROCESSOR_IS_NOT_RUNNING) {
                all_running = false;
                break;
            }
        }
        if(all_running == true) {
            break;
        }
        asm volatile("pause");
    }

    // TODO: remove unusable processors from the list
}

void application_processor_entry_point(void) {
    // set signal that application processor is running
    *((uint32_t *)(0x1FFC)) = 1;
    mfence();

    // load GDT
    lgdt((uint32_t)gdt_wrap);

    // load IDT
    lidt((uint32_t)idt_wrap);

    // TODO: probably delete checking for cpuid and MSR support

    // check if CPUID is supported
    logical_processor_t *data = get_current_logical_processor_struct();
    extern uint32_t check_cpuid_support();
    if(check_cpuid_support() == 0) {
        data->is_running = PROCESSOR_IS_UNUSABLE;
        return;
    }

    // check if MSR registers are supported
    cpuid_t cpuid = read_cpuid(0x00000001, 0);
    if((cpuid.edx & (1 << 5)) == 0) {
        data->is_running = PROCESSOR_IS_UNUSABLE;
        return;
    }

    // check if LAPIC is supported
    extern uint32_t check_presence_of_msr();
    if((cpuid.edx & (1 << 9)) == 0) {
        data->is_running = PROCESSOR_IS_UNUSABLE;
        return;
    }
    if(check_presence_of_msr(CPU_MSR_APIC_BASE) == 0) {
        data->is_running = PROCESSOR_IS_UNUSABLE;
        return;
    }
    uint64_t lapic_address = read_msr(CPU_MSR_APIC_BASE) & 0xFFFFFFFFFFFFF000;
    if((lapic_address >> 32) != 0 || (lapic_address != bootstrap_processor_lapic_address)) {
        data->is_running = PROCESSOR_IS_UNUSABLE;
        return;
    }

    // enable LAPIC
    enable_lapic();
    lapic_calibrate_timer();

    // load TSS
    data = get_current_logical_processor_struct();
    ltr(0x28 + (data->index * 8)); // GDT selector for TSS segment
    data->tss = (tss_t *) (tss + (data->index * PAGE_SIZE)); // set pointer to TSS in processor specific structure
    data->tss->ss0 = 0x10; // kernel data segment

    // initialize rest
    initialize_running_logical_processor(true, true); // we know CPUID and MSR are supported

    // enable interrupts
    sti();

    // log
    processor_log_capabilities();
    log("\n[CPU%d] Application processor 0x%02x has been initialized", data->index, lapic_get_processor_id());
}