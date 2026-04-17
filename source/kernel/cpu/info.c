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
#include <kernel/cpu/commands.h>
#include <kernel/cpu/lapic.h>

/* global variables */
uint32_t number_of_logical_processors;
logical_processor_info_t *logical_processors;
uint32_t *logical_processor_index_number;
logical_processor_t *logical_processor_structs_ptr;
uint32_t bootstrap_processor_id;

/* functions */
uint32_t get_current_logical_processor_index(void) {
    uint32_t lapic_id = lapic_get_processor_id();
    return logical_processor_index_number[lapic_id];
}

logical_processor_t *get_current_logical_processor_struct(void) {
    uint32_t lapic_id = lapic_get_processor_id();
    return &logical_processor_structs_ptr[logical_processor_index_number[lapic_id]];
}

logical_processor_t *get_logical_processor_struct_by_hardware_id(uint32_t hardware_id) {
    return &logical_processor_structs_ptr[logical_processor_index_number[hardware_id]];
}

uint32_t get_copy_on_write_page(void) {
    return (uint32_t) get_current_logical_processor_struct()->copy_on_write_page;
}

uint32_t maximal_cpuid_value(void) {
    cpuid_t cpuid = read_cpuid(0, 0);
    return cpuid.eax;
}

uint32_t does_extended_cpuid_exist(void) {
    cpuid_t cpuid = read_cpuid(0x80000000, 0);
    if(cpuid.eax >= 0x80000000) {
        return true;
    }
    else {
        return false;
    }
}

uint32_t maximal_exended_cpuid_value(void) {
    cpuid_t cpuid = read_cpuid(0x80000000, 0);
    return cpuid.eax;
}